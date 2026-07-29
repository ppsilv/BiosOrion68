/**
 * MIT License
 * Copyright (c) 2021-2024 Jim Jagielski
 */

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
// Our assembled program:
// pioasm converts foo.pio to fio.pio.h
#include "ps2_keyboard.pio.h"
// Header file
#include "ps2_keyboard.h"
#include "ringbuffer.h"

static uint ps2_offset;
static uint ps2_sm;
static PIO ps2_pio;
static uint ps2_pio_irq;
static void ps2_ihandler();

static bool release; // Flag indicates the release of a key
static bool shift;   // Shift indication
static bool cntl;    // Control indication
static bool caps;    // Caps Lock

static bool bkd_int=false;

void initPS2(void) {
    ps2_pio = pio1;
    ps2_pio_irq = PIO1_IRQ_1;
    ps2_offset = pio_add_program(ps2_pio, &ps2_program);
    ps2_sm = pio_claim_unused_sm(ps2_pio, true);

    ps2_program_init(ps2_pio, ps2_sm, ps2_offset, PS2_DATA_PIN, PS2FREQ);
    
    // --- REINÍCIO E SINCRONIZAÇÃO DA PIO ---
    pio_sm_set_enabled(ps2_pio, ps2_sm, false); // Pausa
    pio_sm_restart(ps2_pio, ps2_sm);             // Zera registradores internos
    pio_sm_clkdiv_restart(ps2_pio, ps2_sm);      // Zera o divisor de clock
    pio_sm_exec(ps2_pio, ps2_sm, pio_encode_jmp(ps2_offset)); // Força PC para o início
    pio_sm_clear_fifos(ps2_pio, ps2_sm);

    // Configura NVIC e IRQ
    pio_interrupt_clear(ps2_pio, 1);
    pio_set_irq1_source_enabled(ps2_pio, pis_interrupt1, true);
    irq_set_exclusive_handler(ps2_pio_irq, ps2_ihandler);
    irq_set_enabled(ps2_pio_irq, true);

    pio_sm_set_enabled(ps2_pio, ps2_sm, true);


    // get rid of noise on pins when the PS/2 keyboard is enabled
    sleep_ms(1);
    pio_sm_clear_fifos(ps2_pio, ps2_sm);

    // Now the GPIO pins for the Keyboard data to the VIA chip
    // GPIO pin setup
    //Initialize Interrupt request
    gpio_init(PIRQ);
    gpio_set_dir(PIRQ, GPIO_OUT);
    gpio_pull_up(PIRQ);
    printf("Settinf PIRQ to 1\n");
    gpio_put(PIRQ, 1);

    pio_sm_clear_fifos(ps2_pio, ps2_sm);
    release = shift = cntl = caps = 0;
    bkd_int=false;
    printf("PS2 iniciado\n");
}
// clang-format off

void kbd_int_on(){
    if( bkd_int == false){
        gpio_put(PIRQ, 0);
        bkd_int = true;
    }
}
void kbd_int_off(){
   // if( bkd_int == true){
        gpio_put(PIRQ, 1);
        bkd_int = false;
   // }
}

// SPECIAL CASE:
//   Left Arrow:  scancode 107 -> Ascii 0x14   Esc[D
//   Right Arrow: scancode 116 -> Ascii 0x13   Esc[C
//   Down Arrow:  scancode 114 -> Ascii 0x12   Esc[B
//   Up Arrow:    scancode 117 -> Ascii 0x11   Esc[A
//
static const char __in_flash() ps2_to_ascii_lower[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,  '`', 0x00,
0x00, 0x00, 0x00, 0x00, 0x00,  'q',  '1', 0x00, 0x00, 0x00,  'z',  's',  'a',  'w',  '2', 0x00,
0x00,  'c',  'x',  'd',  'e',  '4',  '3', 0x00, 0x00,  ' ',  'v',  'f',  't',  'r',  '5', 0x00,
0x00,  'n',  'b',  'h',  'g',  'y',  '6', 0x00, 0x00, 0x00,  'm',  'j',  'u',  '7',  '8', 0x00,
0x00,  ',',  'k',  'i',  'o',  '0',  '9', 0x00, 0x00,  '.',  '/',  'l',  ';',  'p',  '-', 0x00,
0x00, 0x00, '\'', 0x00,  '[',  '=', 0x00, 0x00, 0x00, 0x00, 0x0D,  ']', 0x00, '\\', 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x12, 0x00, 0x13, 0x11, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const char __in_flash() ps2_to_ascii_upper[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,  '~', 0x00,
0x00, 0x00, 0x00, 0x00, 0x00,  'Q',  '!', 0x00, 0x00, 0x00,  'Z',  'S',  'A',  'W',  '@', 0x00,
0x00,  'C',  'X',  'D',  'E',  '$',  '#', 0x00, 0x00,  ' ',  'V',  'F',  'T',  'R',  '%', 0x00,
0x00,  'N',  'B',  'H',  'G',  'Y',  '^', 0x00, 0x00, 0x00,  'M',  'J',  'U',  '&',  '*', 0x00,
0x00,  '<',  'K',  'I',  'O',  ')',  '(', 0x00, 0x00,  '>',  '?',  'L',  ':',  'P',  '_', 0x00,
0x00, 0x00, 0x22, 0x00,  '{',  '+', 0x00, 0x00, 0x00, 0x00, 0x0D,  '}', 0x00,  '|', 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x12, 0x00, 0x13, 0x11, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const char __in_flash() ps2_to_ascii_cntl[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,  '~', 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x11,  '!', 0x00, 0x00, 0x00, 0x1A, 0x13, 0x01, 0x17,  '@', 0x00,
0x00, 0x03, 0x18, 0x04, 0x05,  '$',  '#', 0x00, 0x00,  ' ', 0x16, 0x06, 0x14, 0x12,  '%', 0x00,
0x00, 0x0E, 0x02, 0x08, 0x07, 0x19,  '^', 0x00, 0x00, 0x00, 0x0D, 0x0A, 0x15,  '&',  '*', 0x00,
0x00,  '<', 0x0B, 0x09, 0x0F,  ')',  '(', 0x00, 0x00,  '>',  '?', 0x0C,  ':', 0x10,  '_', 0x00,
0x00, 0x00, 0x22, 0x00,  '{',  '+', 0x00, 0x00, 0x00, 0x00, 0x0D,  '}', 0x00,  '|', 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x12, 0x00, 0x13, 0x11, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void bus_pulse_byte(PIO pio, uint sm, uint8_t data) {
    // 1. Tira os pinos do PIO e passa para a CPU (SIO)
    for (int i = 0; i <= 7; i++) {
        gpio_set_function(i, GPIO_FUNC_SIO);
    }

    // 2. Configura como saída e escreve
    gpio_set_dir_out_masked(0x000000FF);
    gpio_put_masked(0x000000FF, (uint32_t)data);

    // 3. Volta os pinos para entrada
    gpio_set_dir_in_masked(0x000000FF);

    // 4. Devolve o controle dos pinos para o PIO (ex: PIO0 ou PIO1)
    for (int i = 0; i <= 7; i++) {
        gpio_set_function(i, pio == pio0 ? GPIO_FUNC_PIO0 : GPIO_FUNC_PIO1);
    }
}

// ISR
void ps2_ihandler(void) {
    pio_interrupt_clear(ps2_pio, 1);

    unsigned char ascii = 0;

    if (pio_sm_is_rx_fifo_empty(ps2_pio, ps2_sm)) {
        pio_interrupt_clear(ps2_pio, 1);
        return;
    }
    // pull a scan code from the PIO SM fifo
    // uint8_t code = *((io_rw_8*)&ps2_pio->rxf[ps2_sm] + 3);
    uint8_t code = pio_sm_get(ps2_pio, ps2_sm) >> 24;
    switch (code) {
        case 0xF0:               // key-release code 0xF0
            release = 1;         // release flag
            break;
        case 0x58:               // Caps Lock?
            if (release) {
                release = 0;
            } else {
                caps = !caps;
            }
            break;

        case 0x12:               // Left-side shift
        case 0x59:               // Right-side shift
            if (release) {
                shift = 0;
                release = 0;
            } else {
                shift = 1;
            }
            break;
        case 0x14:               // Left or Right CNTL key (yep, same scancode)
            if (release) {
                cntl = 0;
                release = 0;
            } else {
                cntl = 1;
            }
            break;
        default:
            code &= 0x7F;
            if (!release) {
                if (cntl) {
                    ascii = ps2_to_ascii_cntl[code];
                } else if (shift) {
                    ascii = ps2_to_ascii_upper[code];
                } else {
                    ascii = ps2_to_ascii_lower[code];
                    if (caps && (ascii >= 'a' && ascii <= 'z')) {
                        ascii -= 32;
                    }
                }
                if (ascii) {
                    printf("   putting[%c]    ",ascii);
                    pio_sm_put(pio0, 1, ascii);
                    kbd_int_on();
                }
            }
            release = 0;
            break;
    }
}




