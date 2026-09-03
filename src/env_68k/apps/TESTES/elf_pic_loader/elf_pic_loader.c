/*
 * elf_pic_loader_standalone.c -- mesma logica de carga/relocacao do
 * elf_pic_loader.c, mas SEM depender do scheduler.c/ctx_switch_*.s.
 * Em vez de criar uma tarefa, monta A5 e chama o entry point
 * DIRETO, como uma chamada de funcao C comum (igual seu
 * load_elf_executable original fazia com entry(argc, argv)).
 *
 * Serve so' pra validar a parte de carga+relocacao (slot certo,
 * A5 certo) antes do kernel novo com scheduler existir de verdade.
 * Quando o kernel novo estiver pronto, troque por elf_pic_loader.c
 * (a versao que cria tarefa via OS_TaskCreateWithA5).
 *
 * loader tratando -fPIC
 * precisa que a libc seja compilada com -fPIC -msep-data
 * Eu fiz isso e ela se chama libc68k_pic.a
 *
 *
 */
#include <stddef.h>
#include <stdio.h>
#include "slots.h"
#include <string.h>
/* ajuste pro header real onde kprintf() esta declarada no seu kernel */
#include "./elf.h"
#include <fileio.h>
#include "scheduler.h"
#include "slots.h"
#include "reloc.h"

#define kprintf(...) printf(__VA_ARGS__)
/*
 * Chama 'entry_addr(argc, argv)' com A5 pre-setado pra 'a5_value'.
 * Escrito em assembly inline porque C nao tem como garantir que A5
 * fique com o valor certo bem no instante da chamada (o compilador
 * e' livre pra usar A5 como quiser antes disso).
 */
/*
 * Chama 'entry_addr(argc, argv)' com A5 pre-setado pra 'a5_value'.
 * 'register ... __asm__("a5")' forca o compilador a colocar
 * a5_value literalmente no registrador A5 antes da chamada, e o
 * resto (empilhar argc/argv, jsr, limpar pilha, pegar retorno em D0)
 * fica por conta do compilador, que sabe a ABI certa -- ao contrario
 * de tentar montar isso a mao em inline asm (o que deu o bug do
 * travamento: argc/argv nunca eram empilhados).
 *
 * CONFIRME com 'm68k-elf-gcc -S' que o codigo gerado realmente
 * carrega A5 e chama entry() logo em seguida, sem nada no meio
 * mexendo em A5 -- em teoria o "register" garante isso, mas vale
 * checar contra a versao/otimizacao real do seu compilador.
 */
static int call_with_a5(uint32_t entry_addr, uint32_t a5_value, int argc, char *argv[])
{
    register uint32_t a5_val __asm__("a5") = a5_value;
    int (*entry)(int, char **) = (int (*)(int, char **))entry_addr;

    __asm__ volatile ("" : : "r" (a5_val) : "memory");  /* impede o compilador de "otimizar" a5_val por nao ver uso explicito */

    return entry(argc, argv);
}

static void process_relocations(uint32_t task_base, FIL *fd, uint32_t shoff,
                              uint16_t shnum, uint16_t shstrndx) {
    uint8_t section_header[40];
    uint32_t symtab_offset = 0;
    uint32_t strtab_offset = 0;
    uint32_t rela_offset = 0;
    uint32_t rela_size = 0;
    unsigned int bytesRead;

    for (int i = 0; i < shnum; i++) {
        flseek(fd, shoff + i * 40);
        if (fread(fd, section_header, 40, &bytesRead) != FR_OK || bytesRead != 40)
            continue;

        uint32_t sh_type = ((uint32_t)section_header[4] << 24) |
                          ((uint32_t)section_header[5] << 16) |
                          ((uint32_t)section_header[6] << 8)  |
                          (uint32_t)section_header[7];

        uint32_t sh_offset = ((uint32_t)section_header[16] << 24) |
                            ((uint32_t)section_header[17] << 16) |
                            ((uint32_t)section_header[18] << 8)  |
                            (uint32_t)section_header[19];

        uint32_t sh_size = ((uint32_t)section_header[20] << 24) |
                          ((uint32_t)section_header[21] << 16) |
                          ((uint32_t)section_header[22] << 8)  |
                          (uint32_t)section_header[23];

        if (sh_type == SHT_SYMTAB) {
            symtab_offset = sh_offset;
        } else if (sh_type == SHT_STRTAB) {
            if (sh_size > strtab_offset) {
                strtab_offset = sh_offset;
            }
        } else if (sh_type == SHT_RELA) {
            rela_offset = sh_offset;
            rela_size = sh_size;
        }
    }

    if (!symtab_offset || !strtab_offset || !rela_offset) {
        kprintf("Aviso: Seções de relocação não encontradas\n");
        return;
    }

    uint8_t rela_buf[12];
    uint32_t num_relas = rela_size / 12;

    kprintf("Processando %d relocações...\n", num_relas);

    for (uint32_t i = 0; i < num_relas; i++) {
        flseek(fd, rela_offset + i * 12);
        if (fread(fd, rela_buf, 12, &bytesRead) != FR_OK || bytesRead != 12)
            continue;

        uint32_t r_offset = ((uint32_t)rela_buf[0] << 24) |
                           ((uint32_t)rela_buf[1] << 16) |
                           ((uint32_t)rela_buf[2] << 8)  |
                           (uint32_t)rela_buf[3];

        uint32_t r_info = ((uint32_t)rela_buf[4] << 24) |
                          ((uint32_t)rela_buf[5] << 16) |
                          ((uint32_t)rela_buf[6] << 8)  |
                          (uint32_t)rela_buf[7];

        uint32_t r_addend = ((uint32_t)rela_buf[8] << 24) |
                           ((uint32_t)rela_buf[9] << 16) |
                           ((uint32_t)rela_buf[10] << 8) |
                           (uint32_t)rela_buf[11];

        uint32_t r_type = r_info & 0xFF;
        uint32_t r_sym = r_info >> 8;

        if (r_type != 11) // Apenas R_68K_GOT16O
            continue;

        uint8_t sym_buf[16];
        flseek(fd, symtab_offset + r_sym * 16);
        if (fread(fd, sym_buf, 16, &bytesRead) != FR_OK || bytesRead != 16)
            continue;

        uint32_t sym_value = ((uint32_t)sym_buf[4] << 24) |
                            ((uint32_t)sym_buf[5] << 16) |
                            ((uint32_t)sym_buf[6] << 8)  |
                            (uint32_t)sym_buf[7];

        uint32_t sym_name_idx = ((uint32_t)sym_buf[0] << 24) |
                               ((uint32_t)sym_buf[1] << 16) |
                               ((uint32_t)sym_buf[2] << 8)  |
                               (uint32_t)sym_buf[3];

        char sym_name[64];
        flseek(fd, strtab_offset + sym_name_idx);
        for (int j = 0; j < 63; j++) {
            if (fread(fd, &sym_name[j], 1, &bytesRead) != FR_OK || bytesRead != 1)
                break;
            if (sym_name[j] == '\0')
                break;
        }
        sym_name[63] = '\0';

        /*
         * No modelo GOT de 16-bit com -msep-data, o slot da GOT localizado em r_offset
         * recebe o endereço absoluto real do símbolo somado ao addend e à task_base.
         */
        uint32_t *patch_addr = (uint32_t *)(task_base + r_offset);

        // Se o símbolo for externo ou interno à tarefa, ajustamos com task_base se necessário.
        // Se sym_value já for relativo ao segmento, somamos task_base.
        *patch_addr = sym_value + r_addend + task_base;

        kprintf("  R_68K_GOT16O: %s -> 0x%08lx\n", sym_name, (unsigned long)*patch_addr);
    }
}

int load_pic_elf_standalone(FIL *fd, int argc, char *argv[])
{
    uint8_t header_buf[52];

    elf32_program_header progHeader;
    unsigned int          bytesRead;
    uint32_t              progIndex = 0;
    int                    pt_load_seen = 0;
    uint32_t               text_load_addr = 0;
    uint32_t               data_load_addr = 0;

    int slot_index = Slots_Alloc();
    if (slot_index < 0) {
        kprintf("Sem slots de memoria livres pra carregar o programa.\n");
        return -1;
    }
    uint32_t task_base = Slots_BaseAddr(slot_index);

    flseek(fd, 0);
    if (fread(fd, header_buf, sizeof(header_buf), &bytesRead) != FR_OK || bytesRead != sizeof(header_buf)) {
        kprintf("Nao foi possivel ler o header do ELF.\n");
        goto fail;
    }

    uint8_t  *ident_magic   =&header_buf[0];
    uint8_t  ident_class    = header_buf[4];
    uint8_t  ident_data     = header_buf[5];
    uint8_t  ident_version  = header_buf[6];
    uint8_t  ident_osabi    = header_buf[7];
    uint8_t  ident_abiversion = header_buf[8];
    uint8_t  *ident_pad   = &header_buf[9];
    uint16_t type    = (header_buf[0x10] << 8) | header_buf[0x11];
    uint16_t machine = (header_buf[0x12] << 8) | header_buf[0x13];
    uint32_t version = ((uint32_t)header_buf[0x14] << 24) |
                    ((uint32_t)header_buf[0x15] << 16) |
                    ((uint32_t)header_buf[0x16] << 8)  |
                    (uint32_t)header_buf[0x17];
    uint32_t entry = ((uint32_t)header_buf[0x18] << 24) |
                    ((uint32_t)header_buf[0x19] << 16) |
                    ((uint32_t)header_buf[0x1A] << 8)  |
                    (uint32_t)header_buf[0x1B];
    uint32_t phoff   = ((uint32_t)header_buf[0x1C] << 24) |
                    ((uint32_t)header_buf[0x1D] << 16) |
                    ((uint32_t)header_buf[0x1E] << 8)  |
                    (uint32_t)header_buf[0x1F];
    uint32_t shoff   = ((uint32_t)header_buf[0x20] << 24) |
                    ((uint32_t)header_buf[0x21] << 16) |
                    ((uint32_t)header_buf[0x22] << 8)  |
                    (uint32_t)header_buf[0x23];
    uint32_t flags   = ((uint32_t)header_buf[0x24] << 24) |
                    ((uint32_t)header_buf[0x25] << 16) |
                    ((uint32_t)header_buf[0x26] << 8)  |
                    (uint32_t)header_buf[0x27];
    uint16_t ehsize  = (header_buf[0x28] << 8) | header_buf[0x29];
    uint16_t phentsize  = (header_buf[0x2A] << 8) | header_buf[0x2B];
    uint16_t phnum  = (header_buf[0x2C] << 8) | header_buf[0x2D];
    uint16_t shentsize  = (header_buf[0x2E] << 8) | header_buf[0x2F];
    uint16_t shnum  = (header_buf[0x30] << 8) | header_buf[0x31];
    uint16_t shstrndx  = (header_buf[0x32] << 8) | header_buf[0x33];







    if (ident_magic[0] != 0x7F || ident_magic[1] != 'E' ||
        ident_magic[2] != 'L'  || ident_magic[3] != 'F' ||
        ident_version  != 1) {
        kprintf("Header ELF invalido.\n");
        goto fail;
    }
    if (ident_class != ID_32BIT || ident_data != ID_BIG_ENDIAN) {
        kprintf("Nao e' um ELF 32-bit big-endian.\n");
        goto fail;
    }
    if (type != ET_EXEC) {
        kprintf("ELF nao e' executavel.\n");
        goto fail;
    }
    if (machine != EM_68K) {
        kprintf("ELF nao e' pra 68k.\n");
        goto fail;
    }

    while (progIndex < phnum) {
        flseek(fd, progIndex * phentsize + phoff);
        if (fread(fd, &progHeader, sizeof(progHeader), &bytesRead) != FR_OK ||
            bytesRead != sizeof(progHeader)) {
            kprintf("Nao foi possivel ler program header do ELF.\n");
            goto fail;
        }

        if (progHeader.type == PT_LOAD) {
            uint32_t actual_addr = task_base + progHeader.paddr;

            flseek(fd, progHeader.offset);
            if (fread(fd, (char *)actual_addr, progHeader.filesz, &bytesRead) != FR_OK ||
                bytesRead != progHeader.filesz) {
                kprintf("Falha lendo segmento PT_LOAD do ELF.\n");
                goto fail;
            }
            if (progHeader.memsz > progHeader.filesz) {
                memset((char *)actual_addr + progHeader.filesz, 0,
                       progHeader.memsz - progHeader.filesz);
            }

            if (pt_load_seen == 0) {
                text_load_addr = actual_addr;
            } else if (pt_load_seen == 1) {
                data_load_addr = actual_addr;
                /*
                 * PATCH DA GOT -- necessario porque este toolchain nao
                 * gera relocacoes (confirmado com 'readelf -r': "There
                 * are no relocations in this file"). O linker calcula
                 * o conteudo da .got assumindo que o programa roda em
                 * ORIGIN(SLOT)=0, entao cada entrada de 4 bytes ali
                 * guarda um endereco "relativo a 0" que precisa virar
                 * endereco real somando task_base.
                 *
                 * CAVEAT: isso trata TODO byte copiado do arquivo
                 * nesse segmento (filesz bytes) como se fosse uma
                 * entrada de endereco de 4 bytes. Funciona pro seu
                 * teste (.got + .got.plt, sem .data literal de
                 * verdade) porque nao ha nenhum dado nao-endereco
                 * misturado ali. Se um dia voce tiver uma variavel
                 * .data inicializada com um valor literal (nao um
                 * ponteiro), esse valor tambem seria "corrigido" por
                 * engano -- nesse caso o patch precisa ficar restrito
                 * so' ao intervalo real da GOT (via secao .got do
                 * ELF ou via simbolo _GLOBAL_OFFSET_TABLE_ do
                 * .symtab), nao ao segmento inteiro.
                 */
                uint32_t *got = (uint32_t *)actual_addr;
                uint32_t  got_words = progHeader.filesz / 4;
                for (uint32_t w = 0; w < got_words; w++)
                    got[w] += task_base;
            }
            pt_load_seen++;
        } else if (progHeader.type == PT_DYNAMIC || progHeader.type == PT_SHLIB) {
            kprintf("ELF dinamicamente linkado -- nao suportado.\n");
            goto fail;
        } else if (progHeader.type == PT_INTERP) {
            kprintf("ELF exige interpretador -- nao suportado.\n");
            goto fail;
        }

        progIndex++;
    }

    if (pt_load_seen != 2) {
        kprintf("ELF PIC esperava exatamente 2 segmentos PT_LOAD, achou %d.\n", pt_load_seen);
        goto fail;
    }

    kprintf("slot=%d task_base=0x%lx text=0x%lx data=0x%lx entry=0x%lx\n",
            slot_index, (unsigned long)task_base, (unsigned long)text_load_addr,
            (unsigned long)data_load_addr, (unsigned long)(task_base + entry));

    //process_relocations(task_base, fd, shoff, shnum, shstrndx);
    //kprintf("Chamando entry em 0x%lx com A5=0x%lx\n",(unsigned long)(task_base + entry), (unsigned long)data_load_addr);


    int ret = call_with_a5(task_base + entry, data_load_addr, argc, argv);
    //int ret = call_with_a5(0x92000, data_load_addr, argc, argv);
    kprintf("Programa retornou %d\n", ret);

    Slots_Free(slot_index);
    return ret;

fail:
    Slots_Free(slot_index);
    return -1;
}

int main(int argc,char *argv[]){
    FIL fd;
    kprintf("Loader for .elf files compiled with -fPIC and -msep-data\n");
    kprintf("Version 1.0.0 2026 copyleft(C)\n");
    kprintf("Author: pdsilva AKA(pgordao)\n");
    if(argv[1] == 0){
        kprintf("Usage: elpicldr <prog name>\n");
        return 1;
    }
    kprintf("Nome do arquivo[%s]\n",argv[1]);
    // 1. Abre o arquivo ELF no disco
    if (fopen(&fd, argv[1], FA_READ) != FR_OK) {
        kprintf("Erro ao tentar abrir o arquivo\n");
        return 0;
    }

    load_pic_elf_standalone(&fd,argc,argv);
    return 0;
}
