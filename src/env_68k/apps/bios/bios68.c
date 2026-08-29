#include <stdio.h>
#include <stdlib.h>
#include <vga_video.h>
#include <fatfs/ff.h>
#include "drv_uart.h"
#include "drv_kbd.h"
#include "io.h"
#include "mc68000.h"
#include "color.h"
#include "timers.h"
#include "interrupt.h"
#include "ata.h"
#include "show_registers.h"
#include "sysflags.h"
#include "orion68.h"
#include "drv_ps2.h"
#include "kbd/ringbuffer.h"

// Aqui a memória é alocada de verdade!
SystemFlags sys_flags;

FATFS FatFs;      // Objeto de controle do sistema de arquivos (Work area)

#define LINELEN 128
char g_cmd_buffer[LINELEN];
extern int getline(char *line, int linesize);
extern void execute_cmd(char *linebuffer);

//__attribute__((section(".mram"))) char vbug_buffer[256];
//__attribute__((section(".minha_ram"))) int vbug_status_flag;
volatile __attribute__((section(".mram"))) long systemTick;
volatile __attribute__((section(".mram"))) unsigned int tick_count;
volatile __attribute__((section(".mram"))) unsigned int flg_system;

#include "./tools/build_counter.h"

void dump_memory(void * addr,int size);
extern void xmodem_loader();
extern uint32_t get_system_tick_nmi_safe(void);
extern void listar_diretorio_raiz(void);
extern void pico_write_ch(uint8_t ch);
extern void UartWriteCh(uint8_t ch);

typedef void (*ProgramaXModem)(void);

void clr_flg_program_loaded(){
       flg_system &= 0xFE;
}
void set_flg_program_loaded(){
       flg_system |= 0x01;
}
unsigned int get_flg_program_loaded(){
       return flg_system & 0x01 ;
}
void print_msg(char *str){
        unsigned char x,y;
        y = 0x1D; //29
        x = 0;
        gotoxy(x,y); //padrao col,row
        printf("%s",str);
}

const char MsgOrionInit[] = 
    "\nPDS317-Hardware copyright (C) pdsilva(pgordao).\n"
    "bios68 V1.0.1 for m68k System.\n"
    "Build Date: " __DATE__ " - " __TIME__ "\n"
    "Build Counter: " BUILD_COUNTER "\n"
    "-----------------------------------------------\n\n";
extern volatile unsigned char debug_pkt;
 
extern void liga_debug(); 
extern int ata_detect(void);
extern int ata_init(void);
extern void abrir_arquivo();

extern void UartWrCh(unsigned char ch);

void write_string(char * str){
    while(*str){
        UartWrCh(*str++);
    }
}
extern void ler_e_exibir_joblog(char * filename);
extern void ler_comando(char *buffer) ;
extern void processar_comando(char *cmd_line);
extern int ata_read_identity(void);

char buffer[128];
char syspath[128];

void display_prompt(void)
{
    char drv;
    if (f_getcwd(syspath, sizeof(syspath)) == FR_OK) {
        // CurrVol armazena o número do drive atual (0, 1, 2...)
        // path armazena o caminho (que no seu caso está vindo apenas "/")
        drv = syspath[0];
        //printf("DRV: [%c]\n",drv);
        drv = drv+0x11;
        syspath[0]=drv;
        if( drv >= 'A' && drv <= 'I'){
                printf("%s>", syspath); 
        }else{
                printf("0:%s>", syspath); 
        }
    } else {
        printf("0:/> ");
    }
}

extern uint8_t rtc_init(void);
extern uint8_t ds3231_probe(void);
extern unsigned char ler_segundo();
extern uint8_t rtc_read_config_byte(uint16_t endereco);
extern void rtc_write_config_byte(uint16_t endereco, uint8_t valor);
extern void printString(char * str);
extern uint8_t ring_buf_get_char();
extern void duart_init_canal_a(void);
extern void video_writechar(char ch);

void main() {
    pico_write_ch('A');
    set_console_output(video_writechar);
    printf("%s",MsgOrionInit);
    pico_write_ch('c');
    m68k_enable_all_interrupts(); 
    printf("* - All Interrupts enabled.\n");
    pico_write_ch('D');
    printf("* - Console output seted to picoVGA.\n");
    pico_write_ch('E');
    set_console_input(ring_buf_get_char);
    printf("* - Console input seted to ps2 keyboard.\n");
    pico_write_ch('F');

    //picovga_set_color(RED,BLACK);
    setcolor(GREEN,BLACK);

    // uart1_init();
    // delay10ms(100);  //100ms    
    // uart2_init();
    // delay10ms(100);  //100ms    
    // uart3_init();
    // delay10ms(100);  //100ms    
    pico_write_ch('G');
    printf("* - Initializing:\n");
    printf("    * duart GPIO\n");
    duart_opr_init();
    pico_write_ch('H');
    printf("    * duart A\n");
    duart_init_canal_a();
    pico_write_ch('I');
    printf("    * duart B\n");
    duart_init_canal_a();
    
    crc32_init();
#ifdef DEBUG_ON
    ata_read_identity();    
#endif
    printf("    * IDE ");
    do_ideinit(0,NULL);
    pico_write_ch('J');

    printf("    * RTC: ");
    rtc_init();
    if(ds3231_probe()){
        printf("RTC-present\n");
    }else{
        printf("RTC- NOT FOUND...\n");
    }
   
    pico_write_ch('K');
    ring_buf_init();
    //********************************************************
    //T H I S   M U S T   B E   T H E  L A S T    T H I N G 
    display_prompt();
    pico_write_ch('L');
    while (1){
        if (getline(g_cmd_buffer, LINELEN) != -1) {
            execute_cmd(g_cmd_buffer);
            display_prompt();
        }
    }

}

#ifdef BUCETON
#include <stdint.h>

typedef struct {
    char     name[8];        /* 0x00: Nome do arquivo ou pasta (completado com espaços) */
    char     ext[3];         /* 0x08: Extensão do arquivo (completado com espaços) */
    uint8_t  attr;           /* 0x0B: Atributos (0x10 = Diretório, 0x20 = Arquivo/Archive, etc.) */
    uint8_t  nt_res;         /* 0x0C: Reservado para o Windows NT (pode deixar 0) */
    uint8_t  crt_time_tenth; /* 0x0D: Décimos de segundo da criação (0 a 199) */
    uint16_t crt_time;       /* 0x0E: Hora de criação (formato MS-DOS compactado) */
    uint16_t crt_date;       /* 0x10: Data de criação (formato MS-DOS compactado) */
    uint16_t lst_acc_date;   /* 0x12: Data do último acesso (formato MS-DOS) */
    uint16_t fst_clus_hi;    /* 0x14: Cluster inicial - Parte Alta (Sempre 0 na FAT16) */
    uint16_t wrt_time;       /* 0x16: Hora da última modificação */
    uint16_t wrt_date;       /* 0x18: Data da última modificação */
    uint16_t fst_clus_lo;    /* 0x1A: Cluster inicial - Parte Baixa (Onde o arquivo começa) */
    uint32_t file_size;      /* 0x1C: Tamanho do arquivo em bytes (0 para diretórios) */
} __attribute__((packed)) FAT_DIR_ENTRY;

#endif
