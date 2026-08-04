
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "string.h"
#include "vga_drv.h"
#include "vga_primitives.h"
#include "pt_cornell_v1_4.h"    // protothreads header
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
void dump(uint8_t addr,uint8_t size);
static PT_THREAD (protothread_print_bus_read(struct pt *pt))
{
    static char buf[256]={0};
    uint8_t data;
    uint8_t reg;
    uint32_t valor=0;
    static int idx = 0;
    PT_BEGIN(pt);
    PT_INTERVAL_INIT() ;
    // 1. Aguarda um sinal claro do 68000 ou um tempo de estabilização
    // 2. LIMPEZA CRUCIAL: Antes de começar, esvazie o lixo que o PIO pegou no boot

    if( is_coldstart ){
        while( system_run == false ){
            if( bus_try_get_event(&data,&reg,bus_pio1, bus_sm) == true ){
                //sprintf(buf,"%d-%d ",data,reg);
                //vga->printString(buf);
                if(data == CMD_SYSTEM_ENABLE && reg == D_SYSTEM_RUN ){
                    system_run = true;
                }
            }
            PT_YIELD_INTERVAL(1) ;
        }
    }
    while (!pio_sm_is_rx_fifo_empty(bus_pio1, bus_sm)) {
        pio_sm_get(bus_pio1, bus_sm); 
    }    

    while(1) {
        PT_YIELD_INTERVAL(1) ;
        data=0x00;reg=0x00;
        while( bus_try_get_event(&data,&reg,bus_pio1, bus_sm) == true ){
           // sprintf(buf,"dat:%02X reg:%02X\n",data,reg);
           // vga->printString(buf);
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
                        /*
                        idx++;
                        if(idx > 1920){
                            idx = 0;
                            vga->clrscr();
                            cursor_x = cursor_y = 0;
                        }*/     
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
            }
        }

    } // END WHILE(1)
    PT_END(pt);
}

void video_welcome_screen(){
    vga->setTextCursorPos(0,0);
    vga->setTextColor(RED, BLACK);
    vga->printString("Orion Vpico2 vga312k   VGA BIOS VRP2350\n");
    vga->setTextColor(YELLOW, BLACK);
    vga->printString("Version 1.0.21.18.00RA\n"); /*1.0 version 21 week 18 day*/
    vga->setTextColor(CYAN, BLACK);
    if ( video_mode < MODE_TEXT_80_S){
        vga->printString("Copyright (C) 2026 pdsilva(aka pgordao).\nV1.0 Vpico2vga312k\n");
    }else{
        vga->printString("Copyright (C) 2026 pdsilva(aka pgordao).V1.0 Vpico2vga312k -2620\n");
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
void dump(uint8_t addr,uint8_t size);
int main(){

    // set the clock
    set_sys_clock_khz(300000, true);

    // start bus read
    initReadBus_Pio();

    // start the serial i/o
    stdio_init_all() ;
    //verify what start type is
//    verify_start();
    // Start i2c bus
//    pico_i2c_init();
  
    // Verify if memory exists in the bus
//    eeprom_ping();
    // load configuration
//    eeprom_load_config(&vga_nvc_config);  
    set_sys_clock_khz(150000, true);
//    if( is_coldstart ){
      vga = create_screen( MODE_TEXT_80_S ); //, 0, 0, font );
      video_mode = MODE_TEXT_80_S;
//    }else{
//        vga = create_screen(vga_nvc_config.video_mode ); //, 0, 0, font );
//        video_mode = vga_nvc_config.video_mode;
//    }      
      

    drawHLine(0,48,640,YELLOW);
    drawHLine(0,49,640,YELLOW);
    drawHLine(0,50,640,YELLOW);
    drawHLine(0,51,640,YELLOW);

    video_welcome_screen();

  // === config threads ========================
  // for core 0
  create_timer(CURSOR_BLINK_ON); //Com o timer para o cursor ele não engasga como quando controlado pela protothread
  pt_add_thread(protothread_print_bus_read);


  // === initalize the scheduler ===============
  pt_schedule_start ;
  // NEVER exits
  // ===========================================
} // end main





// Definição de uma variável "mágica" no final da BIOS (via Linker ou endereço fixo)
#define MAGIC_ADDR 0x001FFE 

int check_if_shadowed() {
    volatile unsigned short *check_ptr = (unsigned short *)MAGIC_ADDR;
    unsigned short original_val = *check_ptr; // Deve ser o valor fixo da ROM

    *check_ptr = 0xA5A5; // Tenta "sujar" a memória

    if (*check_ptr == 0xA5A5) {
        // Se mudou, é RAM! Já fizemos o shadow.
        // Opcional: restaurar o valor original se for necessário.
        *check_ptr = original_val; 
        return 1; 
    } else {
        // Se continua o valor original, é ROM. Precisa copiar!
        return 0;
    }
}

























/*
// Estruturas de controle das threads do Core 1
static struct pt pt_video, pt_animacao;

void core1_worker_loop() {
    PT_INIT(&pt_video);
    PT_INIT(&pt_animacao);

    while (1) {
        // O escalonador cooperativo do Core 1:
        protothread_print_bus_read(&pt_video);
       // protothread_outra_tarefa(&pt_animacao);
        
        // No Core 1, evite usar sleeps pesados para não perder o 68000
    }
}
*/



// ========================================
// === core 0 main
// ========================================
//void swap_buffers(char **active_buffer_ptr, unsigned char *new_buffer) {
//    *active_buffer_ptr = (char *)new_buffer;
//    // Opcional: esperar a DMA terminar antes de trocar
//    // dma_channel_wait_for_finish_blocking(rgb_chan_0);
//}
/*
void set_screen(unsigned char screen){
    switch(screen){
        case SCREEN_0:
            swap_buffers(&active_buffer, vga_video_data_array0);
            vga->set_vga_data_array(vga_video_data_array0);
            break;
        case SCREEN_1:
            swap_buffers(&active_buffer, vga_video_data_array1);
            vga->set_vga_data_array(vga_video_data_array1);
            break;
        default:
            swap_buffers(&active_buffer, vga_video_data_array0);
            vga->set_vga_data_array(vga_video_data_array0);
    }
    buffer=screen;
}

static PT_THREAD (protothread_trocatela(struct pt *pt))
{
    PT_BEGIN(pt);
    static bool LED_state = false ;
    //Init something here
    PT_INTERVAL_INIT() ;

    while(1) {
        PT_YIELD_INTERVAL(5000000) ;
        set_screen(0);
        PT_YIELD_INTERVAL(5000000) ;
        set_screen(1);
    }
  PT_END(pt);
} 
*/




/*
int mainOLD(){
    font_t *font;
    // set the clock
    set_sys_clock_khz(150000, true);

    // start the serial i/o
    stdio_init_all() ;
    initReadBus_Pio();

    // Initialize the VGA screen
    initVGA(  &active_buffer, TXCOUNT ) ;

    // 2. LANÇA O CORE 1! 
    // Isso faz o Core 1 começar a executar a função 'core1_entry'
//    multicore_launch_core1(core1_worker_loop);
    font = set_font(FONTE_8X16);
   
    vga = create_screen( MODE_TEXT_80_S, active_buffer, TXCOUNT, font );
    vga->set_vga_data_array(vga_video_data_array1);
    vga->setTextColor(GREEN, BLACK);
    //vga->setTextSize(1);
    vga->set_blink_interval(125);
    vga->setTextCursorVisible(CURSOR_ON);

    drawHLine(0,0,640,YELLOW);
    drawHLine(0,1,640,YELLOW);
    drawHLine(0,2,640,YELLOW);
    drawHLine(0,3,640,YELLOW);

    //fillRect(0,2,400,640,RED);

    vga->setTextCursorPos(0,1);
    vga->printString("         0         1         2         3         4         5         6         7");      
    vga->printString("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
    vga->printString("         0         1         2         3         4         5         6         7");      
    vga->printString("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
    
    vga->setTextCursorPos(0,6);
    vga->printString("Tela numero 1 cor: ");

    char buf[12];
    for( int i=0;i<8;i++){
        if(i==0){
            vga->setTextColor(i, WHITE);
        }else
            vga->setTextColor(i, BLACK);
        sprintf(buf,"[%d - %d",i,vga->getTextColor());
        vga->printString(buf);
        vga->printString("]");
    }
    vga->setTextCursorPos(0,7);
    sprintf(buf,"blink interval: %d", vga->get_blink_interval());
    vga->printString(buf);

    //for(int i=0;i<640;i++)
    //    for( int j=100;j<110;j++)
    //        drawPixel( i,  j, GREEN ) ;
//
    vga->setTextColor(GREEN, BLACK);
    //vga->clrscr();

    
    
    //    swap_buffers existe porque a idéia de duplo buffer é justamente
    //    essa, enquanto o sistema está mostrando uma tela podemos atualizar
    //    a outra sem interferir com a tela sendo mostrada.
    //    Temos 2 situações aqui
    //    1 - Estamos mostrando na tela o buffer1 vga_video_data_array1
    //        usamos: vga->set_vga_data_array(vga_video_data_array0);
    //        para escrever no buffer0 quando termina de atualizar o buffer0
    //        fazemos swap.
    //    2 - Estamos mostrando na tela o buffer0 vga_video_data_array0
    //        usamos: vga->set_vga_data_array(vga_video_data_array1); para fazer
    //        scroll vertical, após fazer o scroll fazemos o swap e mostramos o
    //        buffer1.
    //                    
    
    set_screen(SCREEN_0);
    video_welcome_screen();


  // === config threads ========================
  // for core 0
  create_timer(CURSOR_BLINK_ON); //Com o timer para o cursor ele não engasga como quando controlado pela protothread
//  pt_add_thread(protothread_trocatela);
  pt_add_thread(protothread_print_bus_read);
  //pt_add_thread(protothread_print);


  // === initalize the scheduler ===============
  pt_schedule_start ;
  // NEVER exits
  // ===========================================
} // end main
*/
