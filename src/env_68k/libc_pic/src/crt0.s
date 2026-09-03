.section .text
.global _start

_start:
    /* Configurar stack pointer para o topo da RAM em 0x82000 */
    move.l #_stack_end, %sp  /* Ajuste conforme sua RAM */

    /* Limpar BSS - ajustar para seus endereços reais */
    move.l #__bss_start, %a0
    move.l #__bss_end, %a1


clear_bss:
    cmp.l %a0, %a1
    beq.s skip_clear
    clr.l (%a0)+
    bra.s clear_bss

skip_clear:
    /* Chamar _main_wrapper */
    jsr _main_wrapper

    trap #0
    /* Parar a CPU */
halt:
    stop #0x2700
    bra halt
