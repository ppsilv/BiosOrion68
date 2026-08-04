#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "orion_bus.pio.h" // Cabeçalho gerado automaticamente pelo pioasm
#include "picovga_registers.h"
#include "vga_primitives.h"


// Definições de bits para o status_registro (Reg 0x01)
#define STATUS_BUSY     0x00
#define STATUS_READY    0x01
#define OPER_ESCRITA    0x00
#define OPER_LEITURA    0x01


bool bkd_int=false;

extern vga_t *vga;
extern volatile uint16_t cursor_x;
extern volatile uint16_t cursor_y;
extern volatile uint8_t  text_color;
extern volatile uint8_t  bg_color;

static char buf[256]={0};

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
                    put_cursor(0);
                    vga->setTextCursorVisible(false);
                    put_cursor(0);
                    if( dado_m68k == 0x08 ){
                        vga->dec_cursor_x();                                
                    }else{
                        if( dado_m68k > 0x80 ){
                            sprintf(buf,"%d",dado_m68k);
                            vga->printString(buf);
                        }else{
                            vga->pchar(dado_m68k);  
                        }
                    }
                    vga->setTextCursorVisible(true);                        
                    put_cursor(1);
                    break;
                case D_SET_TXT_COLOR:
                    bg_color = dado_m68k & 0x0F;
                    text_color = (dado_m68k>>4) & 0x0F;
                    //sprintf(buf,"text [%d] bg[%d]\n",text_color,bg_color);
                    //vga->printString(buf);
                    vga->setTextColor(text_color, bg_color);  
                    break;    
                case D_REG_X_HIGH:
                        cursor_x = (uint16_t)(dado_m68k) | (cursor_x & 0x00FF);
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
