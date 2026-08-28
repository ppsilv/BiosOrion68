
.section .text
.align 2

.global vid_writechar
.global vid_gotoxy
.global vid_clrscr
.global vid_setcolor
.global vid_gohome

.global vid_drawHLine
.global vid_drawVLine
.global vid_drawLine
.global vid_drawCircle
.global vid_drawCircleHelper
.global vid_fillCircle
.global vid_fillCircleHelper
.global vid_drawRoundRect
.global vid_fillRoundRect
.global vid_fillRect
.global vid_fillTri


.equ PICO_VGA_BASE, 0x00FF8000
| REGISTRADORES
.equ REG_DATA_OUT,       0x01
.equ REG_CHANGE_CUR_POS, 0x2B
/*|.equ REG_X_HIGH        , 0x2D*/
/*|.equ REG_X_LOW         , 0x2F*/
/*|.equ REG_Y_HIGH        , 0x31*/
/*|.equ REG_Y_LOW         , 0x33*/
.equ REG_RUN_CMD       , 0x3D
.equ REG_SET_TXT_COLOR , 0x29

.equ REG_X1_HIGH,           ((0x03 * 2)+1)
.equ REG_X1_LOW,            ((0x04 * 2)+1)
.equ REG_Y1_HIGH,           ((0x05 * 2)+1)
.equ REG_Y1_LOW,            ((0x06 * 2)+1)
.equ REG_X2_HIGH,           ((0x07 * 2)+1)
.equ REG_X2_LOW,            ((0x08 * 2)+1)
.equ REG_Y2_HIGH,           ((0x09 * 2)+1)
.equ REG_Y2_LOW,            ((0x0A * 2)+1)
.equ REG_PRIMITIVE_COLOR,   ((0x0E * 2)+1)
.equ REG_WIDTH_HIGH,        ((0x0F * 2)+1)
.equ REG_WIDTH_LOW,         ((0x10 * 2)+1)
.equ REG_HEIGHT_HIGH,       ((0x11 * 2)+1)
.equ REG_HEIGHT_LOW,        ((0x12 * 2)+1)
.equ REG_RAIO,              ((0x13 * 2)+1)  
.equ REG_X_HIGH,            ((0x16 * 2)+1)
.equ REG_X_LOW,             ((0x17 * 2)+1)
.equ REG_Y_HIGH,            ((0x18 * 2)+1)
.equ REG_Y_LOW,             ((0x19 * 2)+1)
.equ REG_CORNERNAME,        ((0x1A * 2)+1)    
.equ REG_DELTA,             ((0x1B * 2)+1)    



| COMMANDS
.equ CMD_SYSTEM_ENABLE,        0xA5
.equ CMD_CLEAR_SCREEN ,        0xA4
.equ CMD_SET_CUR_POS  ,        0xA3
.equ CMD_SET_TXT_COLOR,        0xA2
.equ CMD_GO_HOME      ,        0xA1
.equ CMD_DRAW_PIXEL,           0xA0
.equ CMD_DRAW_LINE,            0xBF
.equ CMD_DRAW_VLINE,           0xBE
.equ CMD_DRAW_HLINE,           0xBD
.equ CMD_DRAW_CIRCLE,          0xBC
.equ CMD_DRAW_CIRCLEHELPER,    0xBB
.equ CMD_DRAW_FILLCIRCLE,      0xBA
.equ CMD_DRAW_FILLCIRCLEHELPER,0xB9
.equ CMD_DRAW_ROUNDRECT,       0xB8
.equ CMD_DRAW_FILLROUNDRECT,   0xB7
.equ CMD_DRAW_FILLRECT,        0xB6
.equ CMD_DRAW_FILLTRIANGLE,    0xB5
.equ CMD_DRAW_MULTILINE,       0xB4

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

/* =========================================================================
 * 1. vid_drawVLine
 * Regs used: DO=COLOR,D1=FUNCTION - D2=X,D3=Y,D4=H
 void drawVLine(short x, short y, short h, char color);
 * ========================================================================= */
vid_drawVLine:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH
    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH

    move.b %d4, REG_HEIGHT_LOW(%A0)                      | H LOW
    lsr.w  #8, %d4                                  | 
    move.b %d4, REG_HEIGHT_HIGH(%A0)                     | H HIGH


    move.b #CMD_DRAW_VLINE,REG_RUN_CMD(%A0)

    bra     trap2_exit
/* =========================================================================
 * 2. vid_drawHLine
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X,D3=Y,D4=W(4 PARAMETERS)
void drawHLine(int x, int y, int w, color_t color);
 * ========================================================================= */
vid_drawHLine:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH

    move.b %d4, REG_WIDTH_LOW(%A0)                      | W LOW
    lsr.w  #8, %d4                                  | 
    move.b %d4, REG_WIDTH_HIGH(%A0)                     | W HIGH


    move.b #CMD_DRAW_HLINE,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 3.vid_drawLine
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X0,D3=Y0,D4=X1,D5=Y1(5 PARAMETERS)
void drawLine(short x0, short y0, short x1, short y1, char color);
 * ========================================================================= */
vid_drawLine:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH

    move.b %D4, REG_X1_LOW(%A0)                      | X1 LOW
    lsr.w  #8, %D4                                  | 
    move.b %D4, REG_X1_HIGH(%A0)                     | X1 HIGH

    move.b %D5, REG_Y1_LOW(%A0)                      | Y1 LOW
    lsr.w  #8, %D5                                  | 
    move.b %D5, REG_Y1_HIGH(%A0)                     | Y1 HIGH

    move.b #CMD_DRAW_LINE,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 4.vid_drawCircle
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X0,D3=Y0,D4=R(4 PARAMETERS)
void drawCircle(short x0, short y0, short r, char color);
 * ========================================================================= */
vid_drawCircle:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH

    move.b %d4, REG_RAIO(%A0)                       | RAIO 

    move.b #CMD_DRAW_CIRCLE,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 5.vid_drawCircleHelper
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X0,D3=Y0,D4=R,D5=CORNERNAME(5 PARAMETERS)
void drawCircleHelper( short x0, short y0, short r, unsigned char cornername, char color);
 * ========================================================================= */
vid_drawCircleHelper:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH

    move.b %D4, REG_RAIO(%A0)                      | X1 LOW

    move.b %D5, REG_CORNERNAME(%A0)                      | Y1 LOW

    move.b #CMD_DRAW_CIRCLEHELPER,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 6.vid_fillCircle
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X0,D3=Y0,D4=R(4 PARAMETERS)
void fillCircle(short x0, short y0, short r, char color);
 * ========================================================================= */
vid_fillCircle:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH

    move.b %d4, REG_RAIO(%A0)                       | RAIO 

    move.b #CMD_DRAW_FILLCIRCLE,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 7.vid_fillCircleHelper
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X0,D3=Y0,A1=R,A2=CORNERNAME,A3=DELTA(6 PARAMETERS)
void fillCircleHelper(short x0, short y0, short r, unsigned char cornername, short delta, char color);
 * ========================================================================= */
vid_fillCircleHelper:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH

    move.b %D4, REG_RAIO(%A0)                       | RAIO LOW
    move.b %D5, REG_CORNERNAME(%A0)                 | CORNERNAME LOW
    move.b %D6, REG_DELTA(%A0)                      | DELTA LOW

    move.b #CMD_DRAW_FILLCIRCLEHELPER,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 8.vid_drawRoundRect
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X,D3=Y,D4=W,D5=H,D6=R(6 PARAMETERS)
void drawRoundRect(short x, short y, short w, short h, short r, char color);
 * ========================================================================= */
vid_drawRoundRect:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH
    
    move.b %d4, REG_WIDTH_LOW(%A0)                      | W LOW
    lsr.w  #8, %d4                                  | 
    move.b %d4, REG_WIDTH_HIGH(%A0)                     | W HIGH

    move.b %d5, REG_HEIGHT_LOW(%A0)                      | H LOW
    lsr.w  #8, %d5                                  | 
    move.b %d5, REG_HEIGHT_HIGH(%A0)                     | H HIGH

    move.b %D6, REG_RAIO(%A0)                       | RAIO LOW

    move.b #CMD_DRAW_ROUNDRECT,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 9.vid_fillRoundRect
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X,D3=Y,D4=W,D5=H,D6=R(6 PARAMETERS)
void fillRoundRect(short x, short y, short w, short h, short r, char color); 
 * ========================================================================= */
vid_fillRoundRect:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH
    
    move.b %d4, REG_WIDTH_LOW(%A0)                      | W LOW
    lsr.w  #8, %d4                                  | 
    move.b %d4, REG_WIDTH_HIGH(%A0)                     | W HIGH

    move.b %d5, REG_HEIGHT_LOW(%A0)                      | H LOW
    lsr.w  #8, %d5                                  | 
    move.b %d5, REG_HEIGHT_HIGH(%A0)                     | H HIGH

    move.b %D6, REG_RAIO(%A0)                       | RAIO LOW

    move.b #CMD_DRAW_FILLROUNDRECT,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 10.fillRect DO=COLOR,D1=FUNCTION,D2=X,D3=Y,D4=W,D5=H(6 PARAMETERS)
 * Regs used: A0, D0
void fillRect(short x, short y, short w, short h, color_t color);
 * ========================================================================= */
vid_fillRect:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH
    
    move.b %d4, REG_WIDTH_LOW(%A0)                      | W LOW
    lsr.w  #8, %d4                                  | 
    move.b %d4, REG_WIDTH_HIGH(%A0)                     | W HIGH

    move.b %d5, REG_HEIGHT_LOW(%A0)                      | H LOW
    lsr.w  #8, %d5                                  | 
    move.b %d5, REG_HEIGHT_HIGH(%A0)                     | H HIGH

    move.b #CMD_DRAW_FILLRECT,REG_RUN_CMD(%A0)
    bra     trap2_exit
/* =========================================================================
 * 11.fillTri
 * Regs used: DO=COLOR,D1=FUNCTION,D2=X0,D3=Y0,D4=X1,D5=Y1,D6=X2,D7=Y2(7 PARAMETERS)
void fillTri(float x0, float y0, float x1, float y1, float x2, float y2, char color);
 * ========================================================================= */
vid_fillTri:
    movea.l #PICO_VGA_BASE, %a0

    move.b %d0, REG_PRIMITIVE_COLOR(%A0)            | COLOR

    move.b %d2, REG_X_LOW(%A0)                      | X LOW
    lsr.w  #8, %d2                                  | 
    move.b %d2, REG_X_HIGH(%A0)                     | X HIGH

    move.b %d3, REG_Y_LOW(%A0)                      | Y LOW
    lsr.w  #8, %d3                                  | 
    move.b %d3, REG_Y_HIGH(%A0)                     | Y HIGH

    move.b %D4, REG_X1_LOW(%A0)                      | X1 LOW
    lsr.w  #8, %D4                                  | 
    move.b %D4, REG_X1_HIGH(%A0)                     | X1 HIGH

    move.b %D5, REG_Y1_LOW(%A0)                      | Y1 LOW
    lsr.w  #8, %D5                                  | 
    move.b %D5, REG_Y1_HIGH(%A0)                     | Y1 HIGH

    move.b %D6, REG_X2_LOW(%A0)                      | X1 LOW
    lsr.w  #8, %D6                                  | 
    move.b %D6, REG_X2_HIGH(%A0)                     | X1 HIGH

    move.b %D7, REG_Y2_LOW(%A0)                      | Y1 LOW
    lsr.w  #8, %D7                                  | 
    move.b %D7, REG_Y2_HIGH(%A0)                     | Y1 HIGH

    move.b #CMD_DRAW_FILLTRIANGLE,REG_RUN_CMD(%A0)
    bra     trap2_exit

trap2_exit:
    /* Restaura o mundo (D0 contém o retorno de cconin se foi chamada) */
    movem.l (%sp)+,%d1-%d7/%a0-%a6
    rte
/*
void chamar_trap(void) {
    register uint32_t d0 asm("d0") = 0x11111111;
    register uint32_t d1 asm("d1") = 0x22222222;
    register uint32_t d2 asm("d2") = 0x33333333;
    register uint32_t d3 asm("d3") = 0x44444444;
    register uint32_t d4 asm("d4") = 0x55555555;
    register uint32_t d5 asm("d5") = 0x66666666;
    register uint32_t d6 asm("d6") = 0x77777777;
    register uint32_t d7 asm("d7") = 0x88888888;

    asm volatile (
        "trap #0"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), 
          "r"(d4), "r"(d5), "r"(d6), "r"(d7)
        : "memory"
    );
}
*/