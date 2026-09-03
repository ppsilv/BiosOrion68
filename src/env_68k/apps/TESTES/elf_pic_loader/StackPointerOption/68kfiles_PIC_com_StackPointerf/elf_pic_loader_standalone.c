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
 */

#include "slots.h"
#include <string.h>
/* ajuste pro header real onde kprintf() esta declarada no seu kernel */
#include "elf.h"

/* implementada em call_entry_with_stack.s */
extern int call_entry_with_stack(int (*entry)(int, char **), int argc,
                                  char **argv, uint32_t new_sp, uint32_t a5_value);

/*
 * Reserva do topo do slot pra pilha da aplicacao carregada. O resto
 * do slot (SLOT_SIZE - STACK_RESERVE) fica pra codigo+dados. Ajuste
 * conforme o que seus programas de teste realmente precisarem.
 */
#define APP_STACK_RESERVE   0x8000UL   /* 32K de pilha por slot */

int load_pic_elf_standalone(FIL *fd, int argc, char *argv[])
{
    elf32_header         header;
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

    f_lseek(fd, 0);
    if (f_read(fd, &header, sizeof(header), &bytesRead) != FR_OK || bytesRead != sizeof(header)) {
        kprintf("Nao foi possivel ler o header do ELF.\n");
        goto fail;
    }

    if (header.ident_magic[0] != 0x7F || header.ident_magic[1] != 'E' ||
        header.ident_magic[2] != 'L'  || header.ident_magic[3] != 'F' ||
        header.ident_version  != 1) {
        kprintf("Header ELF invalido.\n");
        goto fail;
    }
    if (header.ident_class != ID_32BIT || header.ident_data != ID_BIG_ENDIAN) {
        kprintf("Nao e' um ELF 32-bit big-endian.\n");
        goto fail;
    }
    if (header.type != ET_EXEC) {
        kprintf("ELF nao e' executavel.\n");
        goto fail;
    }
    if (header.machine != EM_68K) {
        kprintf("ELF nao e' pra 68k.\n");
        goto fail;
    }

    while (progIndex < header.phnum) {
        f_lseek(fd, progIndex * header.phentsize + header.phoff);
        if (f_read(fd, &progHeader, sizeof(progHeader), &bytesRead) != FR_OK ||
            bytesRead != sizeof(progHeader)) {
            kprintf("Nao foi possivel ler program header do ELF.\n");
            goto fail;
        }

        if (progHeader.type == PT_LOAD) {
            uint32_t actual_addr = task_base + progHeader.paddr;

            f_lseek(fd, progHeader.offset);
            if (f_read(fd, (char *)actual_addr, progHeader.filesz, &bytesRead) != FR_OK ||
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

    /*
     * Pilha da aplicacao: topo do proprio slot, alinhado em word
     * (exigencia do 68k -- SP impar da address error). O codigo+dados
     * (text_load_addr ate' data_load_addr+memsz) precisa caber ANTES
     * dessa reserva, senao a pilha desce em cima do codigo/dados --
     * confira isso pro tamanho real dos seus programas.
     */
    uint32_t app_sp = (task_base + SLOT_SIZE - APP_STACK_RESERVE) & ~1u;

    kprintf("slot=%d task_base=0x%lx text=0x%lx data=0x%lx entry=0x%lx sp=0x%lx\n",
            slot_index, (unsigned long)task_base, (unsigned long)text_load_addr,
            (unsigned long)data_load_addr, (unsigned long)(task_base + header.entry),
            (unsigned long)app_sp);

    int ret = call_entry_with_stack((int (*)(int, char **))(task_base + header.entry),
                                     argc, argv, app_sp, data_load_addr);
    kprintf("Programa retornou %d\n", ret);

    Slots_Free(slot_index);
    return ret;

fail:
    Slots_Free(slot_index);
    return -1;
}
