
/*
    Version 1.2.25.16.00: Protothreads eliminated now CORE 0 read bus CORE executes bus task arrived.

*/
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "string.h"
#include "vga_drv.h"
#include "vga_primitives.h"
#include "colors.h"
#include "vga_bus_read.h"
#include "hardware/pio.h"
#include "eeprom.h"
#include "hardware/watchdog.h"

vga_t *vga = NULL ;

#define MAGIC_WARM_BOOT 0xDEADBEEF
#define SCRATCH_REASON_REG watchdog_hw->scratch[0]
#define SCRATCH_MODE_REG   watchdog_hw->scratch[1]
static bool is_coldstart = false;

static repeating_timer_t timer;
static int last_toggle_time = 1;
static bool system_run;
screenMode_t video_mode;


extern PIO bus_pio1;
extern uint bus_sm;
extern bool bus_try_get_event(uint8_t *value,uint8_t *reg,PIO pio, uint sm);

//VGA variables
volatile uint16_t cursor_x = 0;
volatile uint16_t cursor_y = 0;
volatile uint8_t  text_color = 0;
volatile uint8_t  bg_color = 0;
sys_config_t vga_nvc_config;

//Prototypes
void drawPixel(short x, short y, color_t color) ;
void drawHLine(int x, int y, int w, color_t color) ;
void fillRect(short x, short y, short w, short h, color_t color);
bool bus_try_get_event32(uint32_t *value,PIO pio, uint sm);

static bool timer_callback(repeating_timer_t *rt)
{
    if(last_toggle_time == 1){
        put_cursor(1);
        last_toggle_time = 0;
    }
    else{
        put_cursor(0);
        last_toggle_time = 1;
    }
  return true;
}
static void create_timer(bool btimer)
{
    if(btimer){
        cancel_repeating_timer(&timer);
        int16_t tempo = vga->get_blink_interval();
        add_repeating_timer_ms(tempo, timer_callback, NULL, &timer);
    }else{
        cancel_repeating_timer(&timer);
    }
}

void video_welcome_screen(){
    vga->setTextCursorPos(0,0);
    vga->setTextColor(RED, BLACK);
    vga->printString("Orion Vpico2 vga312k   VGA BIOS VRP2350\n");
    vga->setTextColor(YELLOW, BLACK);
    vga->printString("Version 1.2.25.16.00RA\n"); /*1.0 version 21 week 18 day*/
    vga->setTextColor(CYAN, BLACK);
    if ( video_mode < MODE_TEXT_80_S){
        vga->printString("Copyright (C) 2026 pdsilva(aka pgordao).\nV1.2 Vpico2vga312k\n");
    }else{
        vga->printString("Copyright (C) 2026 pdsilva(aka pgordao).V1.2 Vpico2vga312k -2620\n");
    }
    vga->setTextCursorPos(0,4);
    vga->setTextColor(GREEN, BLACK);
}

int verify_start() {

    if (SCRATCH_REASON_REG == MAGIC_WARM_BOOT) {
        is_coldstart = true;
    } else {
        // Coldstart: Primeira vez que liga
        SCRATCH_REASON_REG = MAGIC_WARM_BOOT;
    }
}
#include "pico/multicore.h"
extern void __not_in_flash_func(gerenciar_barramento_m68k)(PIO pio, uint sm);

void core1_entry() {
    while(1){
        gerenciar_barramento_m68k(bus_pio1, bus_sm);
        tight_loop_contents(); 
    }
}

int main(){

    // set the clock
    set_sys_clock_khz(200000, true);

    // start bus read
    initReadBus_Pio();

    // start the serial i/o
    stdio_init_all() ;
    set_sys_clock_khz(150000, true);

    vga = create_screen( MODE_TEXT_80_S ); //, 0, 0, font );
    video_mode = MODE_TEXT_80_S;

    drawHLine(0,48,640,YELLOW);
    drawHLine(0,49,640,YELLOW);
    drawHLine(0,50,640,YELLOW);
    drawHLine(0,51,640,YELLOW);

    video_welcome_screen();

    // === config threads ========================
    // for core 0
    create_timer(CURSOR_BLINK_ON); //Com o timer para o cursor ele não engasga como quando controlado pela protothread
    // Lança a função do Core 1 no segundo núcleo
    multicore_launch_core1(core1_entry);

    while (!pio_sm_is_rx_fifo_empty(bus_pio1, bus_sm)) {
        pio_sm_get(bus_pio1, bus_sm); 
    }    

    while(1) {
        tight_loop_contents(); 

//        uint8_t data, reg;
//        if (bus_try_get_event(&data, &reg, bus_pio1, bus_sm)) {
//            // Compacta data e reg em um único uint32_t para mandar via FIFO de hardware
//            uint32_t packet = ((uint32_t)reg << 8) | data;
////
//            // Envia para o Core 1 de forma ultra-rápida.
//            // Se a FIFO encher, ele espera (bloqueia), mas como a FIFO de hardware
//            // é rápida, o Core 0 quase nunca para.
//            multicore_fifo_push_blocking(packet);
//        }
    }


} // end main

#define __NEW_SCROLL__

#ifdef __NEW_SCROLL__
#include <stdint.h>      /* uint8_t, uintptr_t */
#include <stddef.h>      /* size_t */
#include <stdbool.h>     /* bool */
#include <string.h>      /* memmove, memset */
#include "pico/stdlib.h" /* __not_in_flash_func */
#include "hardware/dma.h"/* dma_channel_config, DMA_SIZE_8/32, dma_channel_configure, etc. */

/* =====================================================================
 * Dependencias externas -- ajuste conforme a declaracao real do seu
 * projeto. Se ja existem em outro .c/.h, REMOVA estas linhas e inclua
 * o header correspondente em vez delas, para nao duplicar simbolos.
 * ===================================================================== */
extern uint8_t vga_video_data_array0[];   /* confirmar tipo real no seu projeto */

#define VBUFFER_SIZE    153600u
#define BYTES_PER_ROW   5120u       //6144u
#define DMA_MIN_XFER    16u        /* ajuste conforme o valor real do seu projeto */

extern uint memcpy_dma_chan;       /* deve ja ter sido reservado via
                                       dma_claim_unused_channel(true) na
                                       inicializacao do sistema */

/* =====================================================================
 * Protótipo -- necessario porque scroll_up_graphics() chama dma_memcpy()
 * antes da definicao dela aparecer no arquivo.
 * ===================================================================== */
static void __not_in_flash_func(dma_memcpy)(void *dest, const void *src, size_t num, bool block);


void scroll_up_graphics(void) {
    /* Aritmetica forcada em bytes, independente do tipo real declarado
       para vga_video_data_array0 (uint8_t* aqui, mas se no seu projeto
       for uint32_t* isso evita que "+ BYTES_PER_ROW" avance 4x mais do
       que o pretendido). */
    uint8_t *fb = (uint8_t *)vga_video_data_array0;

    /* 1. Quantidade de bytes do framebuffer que sobem */
    size_t bytes_para_copiar = VBUFFER_SIZE - BYTES_PER_ROW;

    /* 2. DMA copia da 2a linha de texto em diante para o inicio do buffer */
    dma_memcpy(fb,
               fb + BYTES_PER_ROW,
               bytes_para_copiar,
               true);

    /* 3. Limpa a ultima linha de texto na parte inferior da tela (preto) */
    memset(fb + bytes_para_copiar, 0x00, BYTES_PER_ROW);
}


static void __not_in_flash_func(dma_memcpy)(void *dest, const void *src, size_t num, bool block) {
    if (num == 0) return;

    if (num < DMA_MIN_XFER) {
        /* memmove, not memcpy: some callers (VRAM copy Esc[Z4, scrolls)
           pass overlapping ranges, which the incrementing DMA handled
           correctly for src > dest; memmove is safe for all overlaps. */
        memmove(dest, src, num);
        return;
    }

    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    /* Se uma transferencia assincrona anterior ainda estiver em andamento
       neste canal, espera terminar antes de reconfigurar -- evita corromper
       uma copia em progresso. */
    if (dma_channel_is_busy(memcpy_dma_chan)) {
        dma_channel_wait_for_finish_blocking(memcpy_dma_chan);
    }

    dma_channel_config c = dma_channel_get_default_config(memcpy_dma_chan);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);

    /* Mismatched alignment: word-mode would corrupt -- single byte DMA.
       Works for both blocking and non-blocking (src is caller-owned). */
    if ((((uintptr_t)d ^ (uintptr_t)s) & 3) != 0) {
        channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
        dma_channel_configure(memcpy_dma_chan, &c, d, s, num, true);
        if (block) dma_channel_wait_for_finish_blocking(memcpy_dma_chan);
        return;
    }

    /* Head: CPU-copy 0-3 bytes until both pointers are 4-byte aligned. */
    while (num > 0 && ((uintptr_t)d & 3) != 0) {
        *d++ = *s++;
        num--;
    }
    if (num == 0) return;

    size_t words = num >> 2;
    size_t tail = num & 3;

    if (block) {
        /* Synchronous: word-DMA the bulk (if any), then CPU-copy the tail. */
        if (words > 0) {
            channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
            dma_channel_configure(memcpy_dma_chan, &c, d, s, words, true);
            dma_channel_wait_for_finish_blocking(memcpy_dma_chan);
            size_t bulk = words << 2;
            d += bulk;
            s += bulk;
        }
        while (tail > 0) {
            *d++ = *s++;
            tail--;
        }
    } else if (tail == 0) {
        /* Async fast path: pure word DMA (num >= 4 guaranteed here). */
        channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
        dma_channel_configure(memcpy_dma_chan, &c, d, s, words, true);
    } else {
        /* Async with tail bytes: single byte DMA covers the whole range. */
        channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
        dma_channel_configure(memcpy_dma_chan, &c, d, s, num, true);
    }
}



#endif