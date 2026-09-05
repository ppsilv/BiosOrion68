/*
 * elfreloc_m68k.c
 *
 * Implementacao de el_applyrela() para m68k -- a funcao que a lib
 * elfload declara como 'extern' e espera que VOCE forneca, porque so'
 * quem conhece os tipos de relocacao da arquitetura sabe como aplicar
 * cada um.
 *
 * Isso e' essencialmente a mesma logica que voce ja tinha em
 * apply_data_relocations() e process_relocations() dentro do seu
 * loader_pic.c -- so' reescrita no formato que a lib elfload exige
 * (uma funcao por entrada de relocacao, chamada em loop por
 * el_relocate()).
 *
 * DIFERENCA IMPORTANTE em relacao ao seu loader_pic atual:
 *
 *   No loader_pic, voce soma 'task_base' direto (porque seu ELF e'
 *   ET_EXEC carregado em endereco variavel por task, mas SEM usar o
 *   mecanismo padrao de base_load_vaddr/paddr da lib).
 *
 *   Aqui, a lib elfload ja calcula os deltas de endereco pra voce
 *   (base_load_paddr / base_load_vaddr, setados por voce ANTES de
 *   chamar el_load()) -- entao dentro de el_applyrela, o "task_base"
 *   equivalente e' ctx->base_load_paddr. Ajuste esse ponto se seu
 *   fluxo de uso for diferente.
 */

#include "elfload.h"
#include <stdint.h>

/* Tipos de relocacao m68k que voce ja tratava manualmente --
 * confirme esses valores contra o seu elf.h / binutils, mas sao os
 * valores padrao da ABI m68k SysV:
 */
#define R_68K_32       1   /* palavra de 32 bits = S + A            */
#define R_68K_PC32     4   /* palavra de 32 bits = S + A - P        */
#define R_68K_GOT16O  11   /* offset de 16 bits dentro da GOT       */
#define R_68K_GLOB_DAT 20  /* usado em simbolos de dados dinamicos  */
#define R_68K_JMP_SLOT 21  /* usado em PLT (nao deve aparecer aqui  */
                            /* se voce nao suporta PLT/PT_DYNAMIC)  */

/*
 * ATENCAO: o prototipo exato de Elf_RelA e de como pegar o simbolo
 * associado a uma relocacao (via r_info) depende do elf.h que a lib
 * usa. Ajuste os nomes de campo (r_offset/r_info/r_addend, e a forma
 * de extrair o indice de simbolo) conforme o seu elf.h real -- deixei
 * aqui o padrao ELF32 classico.
 */
#define ELF32_R_SYM(info)   ((info) >> 8)
#define ELF32_R_TYPE(info)  ((info) & 0xFF)

el_status el_applyrela(el_ctx *ctx, Elf_RelA *rela)
{
    uint32_t type = ELF32_R_TYPE(rela->r_info);

    /* 'P' = endereco final onde a correcao sera escrita (ja com o
     * delta de carregamento aplicado) */
    uint32_t *patch_addr = (uint32_t *)(ctx->base_load_paddr + rela->r_offset);

    switch (type) {

    case R_68K_32:
        /*
         * S + A: endereco do simbolo (aqui simplificado -- este caso
         * cobre o padrao que voce ja usava em apply_data_relocations,
         * onde o linker ja deixou o valor relativo a 0 gravado no
         * proprio .data, e so' falta somar o delta de carregamento).
         *
         * Se voce precisar resolver simbolos de verdade (nao so'
         * offsets internos), precisa buscar o valor do simbolo na
         * symtab usando ELF32_R_SYM(rela->r_info) -- a lib elfload
         * nao faz isso por voce automaticamente neste ponto, e'
         * trabalho extra se seu ELF tiver simbolos externos de
         * verdade aqui (normalmente nao tem, pra ELF PIC estatico
         * como o seu).
         */
        *patch_addr += ctx->base_load_paddr;
        break;

    case R_68K_GOT16O:
        /*
         * Mesma logica do seu find_got_section/patch manual: soma o
         * delta de carregamento ao slot da GOT. 'r_addend' aqui e'
         * explicito (RELA), diferente do REL onde teria que ler o
         * valor antigo do proprio destino.
         */
        *patch_addr = rela->r_addend + ctx->base_load_paddr;
        //NO FUTURO PARA LER LIB.so ELIMINAR A LINHA ACHIMA E LIBERAR
        //CODIGO ABAIXO.
        //if (símbolo é SHN_UNDEF) {
        //    uint32_t addr = trap_resolve_symbol(sym_name);  // nova syscall
        //    *patch_addr = addr;
        //} else {
        //    *patch_addr = rela->r_addend + ctx->base_load_paddr;  // caso atual, símbolo interno
        //}
        break;

    case R_68K_PC32:
        /*
         * Relativo ao proprio PC do local sendo corrigido -- raro em
         * ELF PIC estatico simples, mas incluido por completude.
         * S + A - P, onde P = endereco do proprio patch_addr.
         */
        *patch_addr = (rela->r_addend) - (uint32_t)patch_addr;
        break;

    default:
        EL_DEBUG("el_applyrela: tipo de relocacao m68k nao tratado: %u\n",
                 (unsigned)type);
        return EL_BADREL;
    }

    return EL_OK;
}

/*
 * el_applyrel() -- m68k usa RELA, nao REL (ver elfarch.h), entao esta
 * funcao nunca deveria ser chamada de verdade. Fica so' pra satisfazer
 * o link caso EL_ARCH_USES_REL esteja definido por engano em algum
 * lugar.
 */
el_status el_applyrel(el_ctx *ctx, Elf_Rel *rel)
{
    (void)ctx;
    (void)rel;
    EL_DEBUG("el_applyrel chamado, mas m68k usa RELA -- verifique elfarch.h\n");
    return EL_BADREL;
}
