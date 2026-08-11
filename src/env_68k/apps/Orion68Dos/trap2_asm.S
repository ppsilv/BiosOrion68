
.section .text
.align 2

.global vid_writechar
.global vid_gotoxy
.global vid_clrscr
.global vid_setcolor
.global vid_gohome

.equ PICO_VGA_BASE, 0x00FF8000
| REGISTRADORES
.equ REG_DATA_OUT,       0x01
.equ REG_CHANGE_CUR_POS, 0x2B
.equ REG_X_HIGH        , 0x2D
.equ REG_X_LOW         , 0x2F
.equ REG_Y_HIGH        , 0x31
.equ REG_Y_LOW         , 0x33
.equ REG_RUN_CMD       , 0x3D
.equ REG_SET_TXT_COLOR , 0x29

| COMMANDS
.equ CMD_SYSTEM_ENABLE,   0xA5
.equ CMD_CLEAR_SCREEN ,   0xA4
.equ CMD_SET_CUR_POS  ,   0xA3
.equ CMD_SET_TXT_COLOR,   0xA2
.equ CMD_GO_HOME      ,   0xA1

vid_xxx:
    move.l  %d0,-(%sp)          /* argv[2] = mode */
    move.l  %a1,-(%sp)          /* argv[1] = path */
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #3,-(%sp)           /* 1º param: argc = 3 */

    jsr     vid_writechar

    lea     20(%sp),%sp         /* Limpa 5 longs (20 bytes) */
    bra     trap2_exit



/* =========================================================================
 * 1. writechar VGA: 1 argments D0 = char 
 * Regs used: A0 and D0 
 * ========================================================================= */
vid_writechar:
    /* Opção 1: Registrador A0 com Offset d16(An) - RECOMENDADO */
    movea.l #PICO_VGA_BASE, %a0
    move.b  %D0, REG_DATA_OUT(%a0)      | Escreve em 0x00FF8001
    bra     trap2_exit

/* =========================================================================
 * 1. vid_gotoxy VGA: 2 argments D0 = cursor X D2 = cursor Y int16_T 
 * Regs used: A0,D0,D2 
 * ========================================================================= */
vid_gotoxy:
    movea.l #PICO_VGA_BASE, %a0
    move.b %d0, REG_X_LOW(%A0)                      | Salva os 8 bits inferiores primeiro
    lsr.w  #8, %d0                                  | Desloca D0 8 bits para a direita
    move.b %d0, REG_X_HIGH(%A0)                     | Salva o próximo byte
    move.b %d2, REG_Y_LOW(%A0)                      | Salva os 8 bits inferiores primeiro
    lsr.w  #8, %d2                                  | Desloca D0 8 bits para a direita
    move.b %d2, REG_Y_HIGH(%A0)                     | Salva o próximo byte
    move.b  #CMD_SET_CUR_POS,REG_RUN_CMD(%A0)          | Muda o cursor de posição
    bra     trap2_exit

/* =========================================================================
 * 1. vid_clrscr VGA: No argument
 * Regs used: A0
 * ========================================================================= */
vid_clrscr:
    movea.l #PICO_VGA_BASE, %a0
    move.b  #CMD_CLEAR_SCREEN,REG_RUN_CMD(%A0)          | Limpa a tela
    bra     trap2_exit

vid_gohome:
    movea.l #PICO_VGA_BASE, %a0
    move.b  #CMD_GO_HOME,REG_RUN_CMD(%A0)          
    bra     trap2_exit

/* =========================================================================
 * 1. vid_setcolor VGA: D0.B = Nible High = char color Nible Low = Back ground color
 * Regs used: A0, D0
 * ========================================================================= */
vid_setcolor:
    movea.l #PICO_VGA_BASE, %a0
    move.b  %D0,REG_SET_TXT_COLOR(%A0)          
    bra     trap2_exit

trap2_exit:
    /* Restaura o mundo (D0 contém o retorno de cconin se foi chamada) */
    movem.l (%sp)+,%d1-%d7/%a0-%a6
    rte

