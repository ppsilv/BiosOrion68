
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
#include "pico/util/queue.h"

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

// Cria uma fila na RAM de 512 posições para amortecer o dump
queue_t vga_queue;


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
void core1_entry() {
    uint32_t packet;
    uint8_t data, reg;
    static char buf[256]={0};

    while(1) {
        // Fica esperando chegar um pacote do Core 0
        //Para ler direto do fifo sem buffer
        //packet = multicore_fifo_pop_blocking();
        //Para ler de uma file bufferizada
        queue_remove_blocking(&vga_queue, &packet);

        data = packet & 0xFF;
        reg = (packet >> 8) & 0xFF;

        // O seu SWITCH original entra aqui purinho, sem protothread!
            switch(reg){    
                case D_RUN_CMD:
                    switch(data){
                        case CMD_SET_CUR_POS:
                            //sprintf(buf,"x:%02X y:%02X\n",cursor_x,cursor_y);
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
                        default:
                            vga->printString("VGA-Panic: Comando inexistente...\n");
                    }
                    break;
                case D_SET_MODE:                //0x12
                    sprintf(buf,"\n\nSe voce ver isso, nao funcionou data: [%d]\n",data);
                    vga->printString(buf);
                    change_mode((screenMode_t)data);
                    break;
                case D_SET_TXT_COLOR:
                    bg_color = data & 0x0F;
                    text_color = (data>>4) & 0x0F;
                    sprintf(buf,"text [%d] bg[%d]\n",text_color,bg_color);
                    vga->printString(buf);
                    vga->setTextColor(text_color, bg_color);  
                    break;    
                case D_WRITE_SCREEN:    
                        vga->pchar(data);  
                        break;
                case D_REG_X_HIGH:
                        //cursor_x = (data <<8)|cursor_x;
                        // Isola o lixo limpando os 8 bits baixos antigos antes de injetar o HIGH
                        cursor_x = (uint16_t)(data << 8) | (cursor_x & 0x00FF);
                        break;
                case D_REG_X_LOW:
                        //cursor_x = data;// | cursor_x;
                        // Preserva o HIGH atual e atualiza apenas os 8 bits baixos
                        cursor_x = (cursor_x & 0xFF00) | data;
                        //sprintf(buf,"X-L data:%02X cursor_x:%02X",data,cursor_x);
                        //vga->printString(buf);
                        break;
                case D_REG_Y_HIGH:
                        //cursor_y = (data <<8)|cursor_y;
                        cursor_y = (uint16_t)(data << 8) | (cursor_y & 0x00FF);
                        break;
                case D_REG_Y_LOW:
                        //cursor_y = data;// | cursor_y;
                        cursor_y = (cursor_y & 0xFF00) | data;
                        //sprintf(buf,"Y-L data:%02X cursor_y:%02X",data,cursor_y);
                        //vga->printString(buf);
                        break;
                case D_CHANGE_BUFFER: 
                        vga->printString("NOT impl");
                        break;
                case D_SELECT_SCREEN: 
                        vga->printString("NOT impl");
                        break;
                case D_SET_HORIZONTAL:
                        vga->printString("NOT impl");
                        break;
                case D_SET_VERTICAL:  
                        vga->printString("NOT impl");
                        break;
            } //switch(reg)
    } //while(1)
}

int main(){

    // set the clock
    set_sys_clock_khz(300000, true);

    // start bus read
    initReadBus_Pio();

    // start the serial i/o
    stdio_init_all() ;
    //set_sys_clock_khz(150000, true);
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
    queue_init(&vga_queue, sizeof(uint32_t), 512);

    multicore_launch_core1(core1_entry);
    while (!pio_sm_is_rx_fifo_empty(bus_pio1, bus_sm)) {
        pio_sm_get(bus_pio1, bus_sm); 
    }    

    while(1) {
        uint8_t data, reg;
        if (bus_try_get_event(&data, &reg, bus_pio1, bus_sm)) {
            // Compacta data e reg em um único uint32_t para mandar via FIFO de hardware
            uint32_t packet = ((uint32_t)reg << 8) | data;

            // Envia para o Core 1 de forma ultra-rápida.
            // Se a FIFO encher, ele espera (bloqueia), mas como a FIFO de hardware
            // é rápida, o Core 0 quase nunca para.

            //Se for para escrever direto do fifo sem buffer
            //multicore_fifo_push_blocking(packet);
            //Para por na fila buffer
            queue_try_add(&vga_queue, &packet);
        }
    }


} // end main
