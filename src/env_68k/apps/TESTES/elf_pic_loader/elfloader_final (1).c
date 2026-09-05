/*
 * elfloader.c -- versao final, OrionDOS
 *
 * Usa a lib elfload (Owen Shepherd) so' pro parsing generico
 * (el_init/el_load/el_relocate) -- a parte chata de ler header e
 * program headers.
 *
 * A parte de GOT/A5 (que a lib NAO sabe fazer, ela so' entende
 * program headers, nao secoes) foi PORTADA DIRETO do seu
 * loader_pic.c, que voce ja testou funcionando. Comentei "// == do
 * loader_pic.c ==" em cada trecho que veio de la, pra voce comparar
 * lado a lado.
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include "slots.h"
#include "elfload.h"
#include <fileio.h>

#define kprintf(...) printf(__VA_ARGS__)

FIL *fd;
void *buf;

extern int call_with_a5(uint32_t entry_addr, uint32_t a5_value, int argc, char *argv[]);

/* --- callback de leitura que a lib elfload exige --- */
static bool fpread(el_ctx *ctx, void *dest, size_t nb, size_t offset)
{
    (void) ctx;
    unsigned int bytesRead;
    if (flseek(fd, offset))
        return false;
    if (fread(fd, dest, nb, &bytesRead) != FR_OK || bytesRead != nb)
        return false;
    return true;
}

/* --- callback de alocacao: devolve o endereco dentro do nosso slot --- */
static void *alloccb(el_ctx *ctx, Elf_Addr phys, Elf_Addr virt, Elf_Addr size)
{
    (void) ctx;
    (void) phys;
    (void) size;
    return (void *) virt;
}

static void check(el_status stat, const char *expln)
{
    if (stat) {
        kprintf("%s: erro %d\n", expln, stat);
    }
}

/*
 * == do loader_pic.c ==
 * Isto e' EXATAMENTE find_got_section() do seu loader_pic.c, so' que
 * trocando a leitura via FIL/flseek/fread direto pelo callback
 * ctx->pread (que por baixo e' a mesma fpread() acima -- e' so' uma
 * camada de indireção que a lib elfload usa, o efeito e' identico).
 *
 * O QUE ISSO FAZ, EM PORTUGUES CLARO:
 * O ELF tem uma tabela de "section headers" (diferente dos "program
 * headers" que a lib elfload ja lida sozinha). Cada entrada dessa
 * tabela descreve uma secao (.text, .data, .got, .symtab, etc) --
 * nome, endereco, tamanho. Esta funcao varre essa tabela procurando
 * a secao cujo nome (via .shstrtab) comeca com ".got", e devolve o
 * endereco e tamanho dela DENTRO do binario (ainda sem somar
 * task_base -- isso quem soma e' quem chama esta funcao).
 */
static int find_got_section(el_ctx *ctx, uint32_t *got_addr, uint32_t *got_size)
{
    uint8_t hdr_buf[40];
    uint32_t strtab_offset;
    uint32_t min_addr = 0xFFFFFFFF, max_end = 0;
    int found = 0;

    uint32_t shoff     = ctx->ehdr.e_shoff;
    uint16_t shnum     = ctx->ehdr.e_shnum;
    uint16_t shentsize = ctx->ehdr.e_shentsize;
    uint16_t shstrndx  = ctx->ehdr.e_shstrndx;

    /* le o section header da propria .shstrtab, pra achar onde ficam
     * os NOMES das secoes (strtab_offset) */
    if (!fpread(ctx, hdr_buf, sizeof(hdr_buf), (size_t)shstrndx * shentsize + shoff))
        return 0;
    strtab_offset = ((uint32_t)hdr_buf[0x10] << 24) | ((uint32_t)hdr_buf[0x11] << 16) |
                    ((uint32_t)hdr_buf[0x12] << 8)  |  (uint32_t)hdr_buf[0x13];

    for (uint16_t i = 0; i < shnum; i++) {
        if (!fpread(ctx, hdr_buf, sizeof(hdr_buf), (size_t)i * shentsize + shoff))
            return 0;

        uint32_t name_idx = ((uint32_t)hdr_buf[0] << 24) | ((uint32_t)hdr_buf[1] << 16) |
                             ((uint32_t)hdr_buf[2] << 8)  |  (uint32_t)hdr_buf[3];

        char name_buf[10];
        /* le ate' 9 bytes do nome desta secao, a partir de strtab_offset+name_idx */
        if (!fpread(ctx, name_buf, sizeof(name_buf) - 1, strtab_offset + name_idx))
            continue;
        name_buf[sizeof(name_buf) - 1] = 0;

        if (name_buf[0] == '.' && name_buf[1] == 'g' && name_buf[2] == 'o' && name_buf[3] == 't') {
            uint32_t addr = ((uint32_t)hdr_buf[0x0C] << 24) | ((uint32_t)hdr_buf[0x0D] << 16) |
                             ((uint32_t)hdr_buf[0x0E] << 8)  |  (uint32_t)hdr_buf[0x0F];
            uint32_t size = ((uint32_t)hdr_buf[0x14] << 24) | ((uint32_t)hdr_buf[0x15] << 16) |
                             ((uint32_t)hdr_buf[0x16] << 8)  |  (uint32_t)hdr_buf[0x17];
            if (addr < min_addr) min_addr = addr;
            if (addr + size > max_end) max_end = addr + size;
            found = 1;
        }
    }

    if (!found) return 0;
    *got_addr = min_addr;
    *got_size = max_end - min_addr;
    return 1;
}

/*
 * == do loader_pic.c ==
 * Isto substitui o patch manual que o loader_pic fazia dentro do
 * loop de PT_LOAD ("for (w = 0; w < got_words; w++) got[w] += task_base").
 *
 * A DIFERENCA: no loader_pic, essa soma acontecia LOGO APOS ler o
 * segmento PT_LOAD de dados do arquivo pra memoria. Aqui, como quem
 * fez essa leitura foi a lib elfload (dentro de el_load(), que voce
 * ja chamou antes de chegar aqui), a GOT ja esta em memoria com os
 * valores CRUS do arquivo (relativos a 0) -- so' falta esta funcao
 * rodar por cima somando task_base em cada slot de 4 bytes.
 */
static void patch_got(uint32_t task_base, uint32_t got_addr, uint32_t got_size)
{
    uint32_t *got = (uint32_t *)(task_base + got_addr);
    uint32_t got_words = got_size / 4;
    for (uint32_t w = 0; w < got_words; w++)
        got[w] += task_base;
}

/*
 * == do loader_pic.c ==
 * Isto e' apply_data_relocations() do seu loader_pic.c, portada pro
 * callback ctx->pread. Trata a secao .rela.data (R_68K_32) --
 * ponteiros globais que apontam pra OUTROS globais (diferente da
 * GOT, que trata acesso a simbolos externos/funcoes).
 *
 * IMPORTANTE: isso e' MECANISMO SEPARADO do el_relocate() da lib --
 * aquele so' olha PT_DYNAMIC (que seus binarios ET_EXEC nao tem).
 * Isso aqui acha ".rela.data" via section header table, igual
 * find_got_section faz pra achar ".got".
 */
static void apply_data_relocations(el_ctx *ctx, uint32_t task_base)
{
    uint8_t hdr_buf[40];
    uint32_t strtab_offset;

    uint32_t shoff     = ctx->ehdr.e_shoff;
    uint16_t shnum     = ctx->ehdr.e_shnum;
    uint16_t shentsize = ctx->ehdr.e_shentsize;
    uint16_t shstrndx  = ctx->ehdr.e_shstrndx;

    if (!fpread(ctx, hdr_buf, sizeof(hdr_buf), (size_t)shstrndx * shentsize + shoff))
        return;
    strtab_offset = ((uint32_t)hdr_buf[0x10] << 24) | ((uint32_t)hdr_buf[0x11] << 16) |
                    ((uint32_t)hdr_buf[0x12] << 8)  |  (uint32_t)hdr_buf[0x13];

    for (uint16_t i = 0; i < shnum; i++) {
        if (!fpread(ctx, hdr_buf, sizeof(hdr_buf), (size_t)i * shentsize + shoff))
            continue;

        uint32_t name_idx = ((uint32_t)hdr_buf[0] << 24) | ((uint32_t)hdr_buf[1] << 16) |
                             ((uint32_t)hdr_buf[2] << 8)  |  (uint32_t)hdr_buf[3];

        char name_buf[16];
        if (!fpread(ctx, name_buf, sizeof(name_buf) - 1, strtab_offset + name_idx))
            continue;
        name_buf[sizeof(name_buf) - 1] = 0;

        /* compara com ".rela.data" caractere a caractere, pra nao
         * depender de <string.h> aqui */
        static const char target[] = ".rela.data";
        int match = 1;
        for (int k = 0; target[k]; k++) {
            if (name_buf[k] != target[k]) { match = 0; break; }
        }
        if (!match)
            continue;

        uint32_t rela_offset = ((uint32_t)hdr_buf[0x10] << 24) | ((uint32_t)hdr_buf[0x11] << 16) |
                                ((uint32_t)hdr_buf[0x12] << 8)  |  (uint32_t)hdr_buf[0x13];
        uint32_t rela_size   = ((uint32_t)hdr_buf[0x14] << 24) | ((uint32_t)hdr_buf[0x15] << 16) |
                                ((uint32_t)hdr_buf[0x16] << 8)  |  (uint32_t)hdr_buf[0x17];

        uint8_t rela_buf[12];
        uint32_t num_relas = rela_size / 12;

        for (uint32_t r = 0; r < num_relas; r++) {
            if (!fpread(ctx, rela_buf, 12, rela_offset + r * 12))
                continue;

            uint32_t r_offset = ((uint32_t)rela_buf[0] << 24) | ((uint32_t)rela_buf[1] << 16) |
                                 ((uint32_t)rela_buf[2] << 8)  |  (uint32_t)rela_buf[3];
            uint32_t r_info   = ((uint32_t)rela_buf[4] << 24) | ((uint32_t)rela_buf[5] << 16) |
                                 ((uint32_t)rela_buf[6] << 8)  |  (uint32_t)rela_buf[7];
            uint32_t r_type = r_info & 0xFF;

            if (r_type != 1) /* so' R_68K_32 */
                continue;

            uint32_t *patch_addr = (uint32_t *)(task_base + r_offset);
            *patch_addr += task_base;
        }
        return; /* achou .rela.data, nao precisa continuar procurando */
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        kprintf("uso: %s [elf-a-carregar]\n", argv[0]);
        return 1;
    }

    if (fopen(fd, argv[1], FA_READ) != FR_OK) {
        kprintf("erro abrindo arquivo\n");
        return 1;
    }

    el_ctx ctx;
    ctx.pread = fpread;
    check(el_init(&ctx), "inicializando");

    /* == do loader_pic.c == : Slots_Alloc/Slots_BaseAddr no lugar de
     * posix_memalign -- ja discutimos essa troca antes */
    int slot_index = Slots_Alloc();
    if (slot_index < 0) {
        kprintf("sem slots de memoria livres\n");
        fclose(fd);
        return 1;
    }
    uint32_t task_base = Slots_BaseAddr(slot_index);
    buf = (void *) task_base;

    ctx.base_load_vaddr = ctx.base_load_paddr = (uintptr_t) task_base;

    check(el_load(&ctx, alloccb), "carregando segmentos");
    check(el_relocate(&ctx), "aplicando relocacoes dinamicas"); /* so' faz algo se for ET_DYN;
                                                                     pro seu ET_EXEC estatico,
                                                                     retorna OK sem fazer nada --
                                                                     ver el_relocate() no elfload.c:
                                                                     "if (ctx->ehdr.e_type != ET_DYN) return EL_OK;" */

    /* == do loader_pic.c == : achar e patchear a GOT -- a lib elfload
     * nunca faria isso sozinha, e' 100% especifico da sua ABI m68k */
    uint32_t got_addr = 0, got_size = 0;
    if (find_got_section(&ctx, &got_addr, &got_size)) {
        patch_got(task_base, got_addr, got_size);
    } else {
        kprintf("aviso: secao .got nao encontrada, pulando patch\n");
    }

    /* == do loader_pic.c == : patch de .rela.data -- faltava isso,
     * corrigido agora */
    apply_data_relocations(&ctx, task_base);

    /* == do loader_pic.c == : chamada final, identica */
    uint32_t entry_addr = task_base + ctx.ehdr.e_entry;
    uint32_t a5_value    = task_base + got_addr;

    kprintf("slot=%d task_base=0x%08lx entry=0x%08lx a5=0x%08lx\n",
            slot_index, (unsigned long)task_base,
            (unsigned long)entry_addr, (unsigned long)a5_value);

    int ret = call_with_a5(entry_addr, a5_value, argc, argv);
    kprintf("programa retornou %d\n", ret);

    Slots_Free(slot_index);
    fclose(fd);
    return ret;
}
