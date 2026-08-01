#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "orion_bus.pio.h" // Cabeçalho gerado automaticamente pelo pioasm
#include "ringbuffer.h"
#include "ps2_keyboard.h"
#include "sd.h"

extern uint8_t *arquivo_buffer;
extern bool arquivo_pronto;
extern uint8_t arquivo_ok;
extern uint8_t arquivo_tamh;
extern uint8_t arquivo_tamL;
extern uint32_t arquivo_tamanho;
extern uint32_t ponteiro_leitura;
extern uint32_t arquivo_crc32;
uint16_t ponteiro_leitura_setor = 0;

// Definições de bits para o status_registro (Reg 0x01)
#define STATUS_BUSY 0x00
#define STATUS_READY 0x01
#define OPER_ESCRITA    0x00
#define OPER_LEITURA    0x01
int i=0,j=0;

uint32_t crc32_calculate(const uint8_t *buffer, size_t length) ;

void __not_in_flash_func(gerenciar_barramento_m68k)(PIO pio, uint sm){
    if (!pio_sm_is_rx_fifo_empty(pio, sm)) {

        kbd_int_off();

        uint16_t pacote = pio_sm_get_blocking(pio, sm);
        uint8_t operacao  = (pacote >> 14) & 0x03; // Bits 15:14 (0x0 = Escrita, 0x1 = Leitura)
        uint8_t reg       = (pacote >> 8)  & 0x3F; // Bits 13:8  (Endereço A1-A6: 0x00 a 0x3F)
        uint8_t dado_m68k = pacote & 0xFF;        // Bits 7:0   (Dado D0-D7)

        pio_sm_clear_fifos(pio, sm);
        uint8_t byte_resposta = 0x0;
        //printf("operacao[%02x] sm[%02x]  reg[%02x]  \n",operacao,sm,reg);
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
                return;
            case 0x0A:      //sector low
                SECTOR_DEF * sd0 = get_sdcard_instance();
                sd0->sector = dado_m68k;
                ponteiro_leitura_setor = 0;
                return;
            case 0x0B:      //sector high
                sd0 = get_sdcard_instance();
                sd0->sector |= (dado_m68k << 8)&0xFF00;
                ponteiro_leitura_setor = 0;
                return;
            case 0x0C:     //read sector
                sd0 = get_sdcard_instance();
                if (sd0->card){    // Garante que o cartão inicializou com sucesso
                    sd0->card->read_blocks(sd0->card, sd0->buffer, sd0->sector, 1);
                }
                arquivo_crc32 = crc32_calculate((const uint8_t *)sd0->buffer, 512);
                return;
            case 0x0D:      //get sector 
                sd0 = get_sdcard_instance();
                byte_resposta = sd0->buffer[ponteiro_leitura_setor];
                //printf("%02x|",byte_resposta);
                //i++;
                //if( i == 16){
                //    i=0;
                //    printf("\n");
                //}
                ponteiro_leitura_setor++;
                break;
            default:
                byte_resposta = 0xFF;
                break;
        }
        if( operacao == OPER_LEITURA ){
            pio_sm_put(pio, sm, byte_resposta);
//            printf("%02x|",byte_resposta);
//            i++;
//            if( i == 16){
//                i=0;
//                printf("\n");
//            }
        }
        //sio_hw->gpio_clr = (1 << 19);
    }
}

/*

void __not_in_flash_func(gerenciar_barramento1_m68k)(PIO pio, uint sm){
    if (!pio_sm_is_rx_fifo_empty(pio, sm)) {
        //sio_hw->gpio_set = (1 << 19);

        uint16_t pacote = pio_sm_get_blocking(pio, sm);
        uint8_t operacao  = (pacote >> 14) & 0x03; // Bits 15:14 (0x0 = Escrita, 0x1 = Leitura)
        uint8_t reg       = (pacote >> 8)  & 0x3F; // Bits 13:8  (Endereço A1-A6: 0x00 a 0x3F)
        uint8_t dado_m68k = pacote & 0xFF;        // Bits 7:0   (Dado D0-D7)

        pio_sm_clear_fifos(pio, sm);
        uint8_t byte_resposta = 0x0;
        printf("   reg[%02x]  \n",reg);
                kbd_int_off();
        switch (reg) {
            case 0x09:
//                kb_get(&byte_resposta);
                kbd_int_off();
                return;
                break;    
            default:
                byte_resposta = 0xFF;
                break;
        }
        //if( operacao == OPER_LEITURA ){
        //    pio_sm_put(pio, sm, byte_resposta);
        //    //printf("Resposta [%c]\n",byte_resposta);
        //}
        //sio_hw->gpio_clr = (1 << 19);
    }
}
*/