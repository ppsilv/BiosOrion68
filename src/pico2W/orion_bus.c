#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "orion_bus.pio.h" // Cabeçalho gerado automaticamente pelo pioasm
#include "ringbuffer.h"
#include "ps2_keyboard.h"

extern uint8_t *arquivo_buffer;
extern bool arquivo_pronto;
extern uint8_t arquivo_ok;
extern uint8_t arquivo_tamh;
extern uint8_t arquivo_tamL;
extern uint32_t arquivo_tamanho;
extern uint32_t ponteiro_leitura;
extern uint32_t arquivo_crc32;

// Definições de bits para o status_registro (Reg 0x01)
#define STATUS_BUSY 0x00
#define STATUS_READY 0x01
#define OPER_ESCRITA    0x00
#define OPER_LEITURA    0x01

void __not_in_flash_func(gerenciar_barramento_m68k)(PIO pio, uint sm){
    if (!pio_sm_is_rx_fifo_empty(pio, sm)) {
        //sio_hw->gpio_set = (1 << 19);

        uint16_t pacote = pio_sm_get_blocking(pio, sm);
        uint8_t operacao  = (pacote >> 14) & 0x03; // Bits 15:14 (0x0 = Escrita, 0x1 = Leitura)
        uint8_t reg       = (pacote >> 8)  & 0x3F; // Bits 13:8  (Endereço A1-A6: 0x00 a 0x3F)
        uint8_t dado_m68k = pacote & 0xFF;        // Bits 7:0   (Dado D0-D7)

        pio_sm_clear_fifos(pio, sm);
        uint8_t byte_resposta = 0x0;
        //printf("   reg[%02x]  ",reg);
        if ( arquivo_pronto == 0){
            byte_resposta = 0x0;
        }else if ( ponteiro_leitura >= arquivo_tamanho ) {
            ponteiro_leitura = 0;
            arquivo_pronto = 0;
        }
        switch (reg) {
            case 0x00: // --- m68k leu 0xFF9101: REGISTRADOR DE DADOS ---
                //if (arquivo_pronto && ponteiro_leitura < arquivo_tamanho) {
                    byte_resposta = arquivo_buffer[ponteiro_leitura];
                    ponteiro_leitura++; // Avança o ponteiro do arquivo
                //}
                break;

            case 0x01: // --- m68k leu 0xFF9103: REGISTRADOR DE STATUS ---
                if (!arquivo_pronto) {
                    byte_resposta = 0x00; // PICO_STATE_IDLE
                } else if (ponteiro_leitura < arquivo_tamanho) {
                    byte_resposta = 0x01; // PICO_STATE_HAS_FILE
                } else {
                    byte_resposta = 0x02; // PICO_STATE_EOF
                }
                break;

            case 0x02: // --- m68k leu 0xFF9105: SIZE HIGH (Byte Alto) ---
                //if (arquivo_pronto) {
                    byte_resposta = (uint8_t)((arquivo_tamanho >> 8) & 0xFF);
                //}
                break;

            case 0x03: // --- m68k leu 0xFF9107: SIZE LOW (Byte Baixo) ---
                //if (arquivo_pronto) {
                    byte_resposta = (uint8_t)(arquivo_tamanho & 0xFF);
                //}
                break;
            case 0x04:
                byte_resposta = (arquivo_crc32 >> 24)& 0xFF;
                break;
            case 0x05:
                byte_resposta = (arquivo_crc32 >> 16)& 0xFF;
                break;
            case 0x06:
                byte_resposta = (arquivo_crc32 >> 8 )& 0xFF;
                break;
            case 0x07:
                byte_resposta = arquivo_crc32 & 0xFF;
                break;
            case 0x08:
                arquivo_pronto = 0x0;
                byte_resposta = 0x0;
                break;
            case 0x09:
                kbd_int_off();
                //printf("  int down  ");
                if(kb_available()){
                    kb_get(&byte_resposta);
                   // printf("   R[%02x]    \n",byte_resposta);
                }else{
                   // printf("   R[0xFE]    \n");
                    byte_resposta=0xFE;
                }
                break;    
            default:
                byte_resposta = 0xFF;
                break;
        }
        if( operacao == OPER_LEITURA ){
            pio_sm_put(pio, sm, byte_resposta);
            printf("Resposta [%c]\n",byte_resposta);
        }
        //sio_hw->gpio_clr = (1 << 19);
    }
}

