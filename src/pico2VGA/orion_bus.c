#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "orion_bus.pio.h" // Cabeçalho gerado automaticamente pelo pioasm
#include "picovga_registers.h"
#include "vga_primitives.h"
#include "vga_graphos.h"



// Definições de bits para o status_registro (Reg 0x01)
#define STATUS_BUSY     0x00
#define STATUS_READY    0x01
#define OPER_ESCRITA    0x00
#define OPER_LEITURA    0x01


bool bkd_int=false;

extern vga_t *vga;
volatile uint16_t cursor_x;
volatile uint16_t cursor_y;
volatile uint16_t cursor_x1;
volatile uint16_t cursor_y1;
volatile uint16_t cursor_x2;
volatile uint16_t cursor_y2;
volatile uint8_t  text_color;
volatile uint8_t  bg_color;
volatile uint16_t primitive_width;
volatile uint16_t primitive_height;
volatile uint8_t primitive_color;
volatile uint16_t primitive_radio;
volatile uint16_t primitive_height;
volatile uint16_t raio;
volatile uint16_t cornername;
volatile uint16_t delta;

static char buf[256]={0};

// Converte 1 byte em 2 caracteres HEX
static inline void u8_to_hex(uint8_t val, char *buf) {
    static const char hex[] = "0123456789ABCDEF";
    buf[0] = hex[(val >> 4) & 0x0F];
    buf[1] = hex[val & 0x0F];
    buf[2] = '\0';
}

void __not_in_flash_func(gerenciar_barramento_m68k)(PIO pio, uint sm){
    if (!pio_sm_is_rx_fifo_empty(pio, sm)) {

        uint16_t pacote = pio_sm_get_blocking(pio, sm);
        uint8_t operacao  = (pacote >> 14) & 0x03; // Bits 15:14 (0x0 = Escrita, 0x1 = Leitura)
        uint8_t reg       = (pacote >> 8)  & 0x3F; // Bits 13:8  (Endereço A1-A6: 0x00 a 0x3F)
        uint8_t dado_m68k = pacote & 0xFF;        // Bits 7:0   (Dado D0-D7)

        pio_sm_clear_fifos(pio, sm);
        uint8_t byte_resposta = 0x0;
        switch (reg) {
                case D_WRITE_SCREEN:    
                    vga->setTextCursorVisible(false);
                    vga->pchar(dado_m68k);  
                    vga->setTextCursorVisible(true);                        
                    break;
                case D_SET_TXT_COLOR:
                    bg_color = dado_m68k & 0x0F;
                    text_color = (dado_m68k>>4) & 0x0F;
                    //sprintf(buf,"Color text [%d] bg[%d]\n",text_color,bg_color);
                    //vga->printString(buf);
                    vga->setTextColor(text_color, bg_color);  
                    break;    
                case D_REG_PRIMITIVE_COLOR:
                        primitive_color = dado_m68k;
                        break;
                case D_REG_WIDTH_HIGH:
                        primitive_width = (uint16_t)(dado_m68k << 8) | (cursor_x & 0x00FF);
                        break;
                case D_REG_WIDTH_LOW:
                        primitive_width = (primitive_width & 0xFF00) | dado_m68k;
                        break;
                case D_REG_HEIGHT_HIGH:
                        primitive_height = (uint16_t)(dado_m68k << 8) | (cursor_x & 0x00FF);
                        break;
                case D_REG_HEIGHT_LOW:
                        primitive_height = (primitive_height & 0xFF00) | dado_m68k;
                        break;
                case D_REG_X_HIGH:
                        cursor_x = (uint16_t)(dado_m68k << 8) | (cursor_x & 0x00FF);
                        break;
                case D_REG_X_LOW:
                        cursor_x = (cursor_x & 0xFF00) | dado_m68k;
                        break;
                case D_REG_Y_HIGH:
                        cursor_y = (uint16_t)(dado_m68k << 8) | (cursor_y & 0x00FF);
                        break;
                case D_REG_Y_LOW:
                        cursor_y = (cursor_y & 0xFF00) | dado_m68k;
                        break;
                case D_REG_X1_HIGH:
                        cursor_x1 = (uint16_t)(dado_m68k << 8) | (cursor_x & 0x00FF);
                        break;
                case D_REG_X1_LOW:
                        cursor_x1 = (cursor_x & 0xFF00) | dado_m68k;
                        break;
                case D_REG_Y1_HIGH:
                        cursor_y1 = (uint16_t)(dado_m68k << 8) | (cursor_y & 0x00FF);
                        break;
                case D_REG_Y1_LOW:
                        cursor_y1 = (cursor_y & 0xFF00) | dado_m68k;
                        break;
                case D_REG_RAIO:
                        raio = dado_m68k;
                        break; 
                case D_REG_CORNERNAME:
                        cornername = dado_m68k;
                        break; 
                case D_REG_DELTA:
                        delta = dado_m68k;
                        break; 
                case D_RUN_CMD:
                    switch(dado_m68k){
                        case CMD_SET_CUR_POS:
                            vga->setTextCursorPos(cursor_x,cursor_y);
                            break;
                        case CMD_CLEAR_SCREEN:
                            vga->clrscr();
                            cursor_x = cursor_y = 0;
                            break;
                        case CMD_GO_HOME:
                            cursor_x = cursor_y = 0;
                            vga->set_vga_home();
                            break;
                        case CMD_DRAW_PIXEL:           
                            break;
                        case CMD_DRAW_LINE:  
                            //sprintf(buf,"PICO:x0[%d] y0[%d] x1[%d] x1[%d] col[%d]\n",cursor_x,cursor_y,cursor_x1,cursor_y1,primitive_color);
                            //vga->printString(buf);
                            drawLine( cursor_x,  cursor_y,  cursor_x1,  cursor_y1,  primitive_color);
                            break;
                        case CMD_DRAW_VLINE:           
                            drawVLine( cursor_x,  cursor_y,  primitive_height,  primitive_color);
                            break;
                        case CMD_DRAW_HLINE:           
                            drawHLine( cursor_x,  cursor_y,  primitive_width,  primitive_color);
                            break;
                        case CMD_DRAW_CIRCLE:          
                            drawCircle( cursor_x,  cursor_y,  raio,  primitive_color);
                            break;
                        case CMD_DRAW_CIRCLEHELPER:
                            drawCircleHelper(  cursor_x,  cursor_y,  raio,   cornername,  primitive_color);
                            break;
                        case CMD_DRAW_FILLCIRCLE:   
                            fillCircle( cursor_x,  cursor_y,  raio,  primitive_color);
                            break;
                        case CMD_DRAW_FILLCIRCLEHELPER:
                            fillCircleHelper( cursor_x,  cursor_y,  raio,   cornername,  delta,  primitive_color);
                            break;
                        case CMD_DRAW_ROUNDRECT:
                            drawRoundRect( cursor_x,  cursor_y,  primitive_width,  primitive_height,  raio,  primitive_color);
                            break;
                        case CMD_DRAW_FILLROUNDRECT:
                            fillRoundRect( cursor_x,  cursor_y,  primitive_width,  primitive_height,  raio,  primitive_color); 
                            break;
                        case CMD_DRAW_FILLRECT:  
                            fillRect( cursor_x,  cursor_y,  primitive_width,  primitive_height,  primitive_color);
                            break;
                        case CMD_DRAW_FILLTRIANGLE:
                            fillTri( cursor_x,  cursor_y,  cursor_x1,  cursor_y1,  cursor_x2,  cursor_y2,  primitive_color);
                            break;
                        case CMD_DRAW_MULTILINE:                                   
                           // drawMultiLine( num_lines,   point_list[][2],  primitive_color);
                            break;
                    }
                    break;

            default:
                byte_resposta = 0xFF;
                break;
        }
        if( operacao == OPER_LEITURA ){
            pio_sm_put(pio, sm, byte_resposta);
        }else{
            pio_sm_put(pio, sm, 0xA5);
        }
    }
}
