#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mc68000.h>
#include "keyboard.h"
#include "timers.h"

// 🛠️🇧🇷
const unsigned char OE_BASE_KEYMAP[] = {0x00, 0x00, 0x00, 0x00, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',              // 0x
                                    'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',                  // 1x
                                    '3', '4', '5', '6', '7', '8', '9', '0', 0x0D, 0x1B, 0x08, 0x09, ' ', '-', '=', '[',              // 2x  ENTER, ESC, BACKSPACE, TAB
                                    ']', '\\', 0xFF, ';', '\'', '`', ',', '.', '/', 0x02, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,        // 3x  CAPS = 2, F0-F6
                                    0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0x1F, 0xF0, 0xFE, 0x18, 0x14, 0x15, 0x7F, 0x17, 0x16, 0x13,  // 4x F7-F12, PrintScreen, ScrollLock, Pause, Insert, Home, PageUp, DelFwd,End, PageDown, Rightarrow
                                    0x12, 0x11, 0x10, 0x05, '/', '*', '-', '+', 0x0D, '1', '2', '3', '4', '5', '6', '7',             // 5x Left, Down, Up, NumLock, Keypad Symbols
                                    '8', '9', '0', '.', 0xFF, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      // 6x Unknown, Application, Not used symbols
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 7x Not used symbols

const unsigned char OE_SHIFT_KEYMAP[] = {0x00, 0x00, 0x00, 0x00, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',              // 0x
                                    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',                  // 1x
                                    '#', '$', '%', '^', '&', '*', '(', ')', 0x0D, 0x1B, 0x08, 0x09, ' ', '_', '+', '{',              // 2x  Shift-ENTER, ESC, BACKSPACE, TAB, Shift-Space
                                    '}', '|', 0xFF, ':', '"', '~', '<', '>', '?', 0x02, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,          // 3x  CAPS = 2, F0-F6
                                    0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0x1F, 0xF0, 0xFE, 0x18, 0x14, 0x15, 0x7F, 0x17, 0x16, 0x13,  // 4x  F7-F12, PrintScreen, ScrollLock, Pause, Insert, Home, PageUp, DelFwd,End, PageDown, Rightarrow
                                    0x12, 0x11, 0x10, 0x05, '/', '*', '-', '+', 0x0D, '1', '2', '3', '4', '5', '6', '7',             // 5x  Left, Down, Up, NumLock, Keypad Symbols
                                    '8', '9', '0', '.', 0xFF, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      // 6x  Unknown, Application, Not used symbols
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 7x  Not used symbols

const unsigned char OE_CTRL_KEYMAP[] = {0x00, 0x00, 0x00, 0x00, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC,  // 0x
                                    0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xE1, 0xE2,  // 1x
                                    0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xE0, 0x0F, 0x00, 0x08, 0x00, ' ', 0x00, 0x00, 0x00,   // 2x  CTRL-ENTER, --, BACKSPACE, Space
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,  // 3x  F0-F6
                                    0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13,  // 4x  F7-F12 / Rightarrow
                                    0x12, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 5x  Left, Down, Up, NumLock, Keypad Symbols
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 6x
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 7x

const unsigned char OE_ALT_KEYMAP[] = {0x00, 0x00, 0x00, 0x00, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC,  // 0x
                                    0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xBC, 0xBD,  // 1x
                                    0xBE, 0xBF, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xBB, 0x0D, 0x00, 0x08, 0x00, ' ', 0x00, 0x00, 0x00,   // 2x  ENTER, --, BACKSPACE, Space
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,  // 3x  F0-F6
                                    0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13,  // 4x  F7-F12 / Rightarrow
                                    0x12, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 5x  Left, Down, Up, NumLock, Keypad Symbols
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 6x
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 7x

const unsigned char OE_ALTGR_KEYMAP[] = {0x00, 0x00, 0x00, 0x00, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C,  // 0x
                                    0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9C, 0x9D,  // 1x
                                    0x9E, 0x9F, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0x9B, 0x0D, 0x00, 0x08, 0x00, ' ', 0x00, 0x00, 0x00,   // 2x  ENTER, --, BACKSPACE, Space
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,  // 3x  F0-F6
                                    0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13,  // 4x  F7-F12 / Rightarrow
                                    0x12, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 5x  Left, Down, Up, NumLock, Keypad Symbols
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 6x
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 7x

volatile unsigned char key_buffer[48]={0};
volatile unsigned char cmd, length, type;
volatile unsigned char special_key_up=0;
volatile unsigned char special_key_down=0;
volatile unsigned char key_up=0;
volatile unsigned char key_down=0;
volatile unsigned char special_key_status=0;
volatile unsigned char _CapsFlag = 0;
volatile unsigned char mod_caps = 0;
volatile unsigned char debug_pkt = 0;

//volatile uint8_t rx_head = 0;
//volatile uint8_t rx_tail = 0;
//volatile char rx_buffer[RX_BUFFER_SIZE];

unsigned char get_keypress();
static void send_cmd_keyboard(unsigned char led_status);
unsigned char get_packet();
unsigned char get_kbd_key(unsigned char code);
/*
static void *mymemset(void *dest, int ch, unsigned int count)
{
    unsigned char *ptr = (unsigned char *)dest;

    while (count--){
        *ptr++ = (unsigned char)ch;
    }

    return dest;
}
*/
static void printBuffer(char * pkt){
    if(debug_pkt){
        printf("%s - ",pkt);
        for(int i = 0; i < 12; i++) {
            printf("%02x|",key_buffer[i]);
        }
        printf("\n");
    }
}
void liga_debug(){
    debug_pkt ^= debug_pkt;
}
void init_kbd()
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;

    // 1. Entra no modo DLAB para configurar Baud Rate
    *(uart_reg + LCR) = 0x83;
    *(uart_reg + DLL) = 8; // Divisor para 115200 com 14.7456MHz
    *(uart_reg + DLM) = 0;
    // 2. Sai do modo DLAB e define 8N1 (MUITO IMPORTANTE: usar 0x03)
    *(uart_reg + LCR) = 0x03;

    // 3. Configura FIFO (Habilita, limpa buffers e seta trigger de 14 bytes)
    *(uart_reg + FCR) = 0x0D;


    // Nova situação agora gerando interrupção
    // 1. DesHabilita a interrupção de dados recebidos (RDAI) no IER
//    *(uart_reg + IER1) = 0x00; // Garante que interrupções estão desligadas
//    // 2. Configura o FCR com a sua lógica de 1 byte ou com os 14 bytes (já que o buffer vai limpar)
//    *(uart_reg + FCR1) = 0x07; // Habilita FIFO e limpa buffers (Trigger de 1 byte é mais agressivo)
//    *(uart_reg + MCR1) = 0x00;     //limpa controles

    special_key_up=0;
    special_key_down=0;
    key_up=0;
    key_down=0;
    special_key_status=0;
    _CapsFlag = 0;
    mod_caps = 0;
    cmd=0;
    length=0;
    type=0;
    debug_pkt=0;
    //rx_head = 0;
    //rx_tail = 0;
}
/*
__attribute__((interrupt)) void uart_isr(void) {
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;
    // Enquanto houver dados no FIFO da UART (checando o LSR)
    while (*(uart_reg + LSR1) & 0x01) { 
        rx_buffer[rx_head] = *(uart_reg + RBR1); // Lê o hardware
        rx_head++; // Incremento limpo de 8 bits, resseta em 256 automaticamente!
    }
    // Limpa pendência de interrupção no hardware se necessário
}
*/
/*
void enable_kbd_interrupts(){
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;
    *(uart_reg + IER) = 0x01; // Garante que interrupções estão ligadas
    // ATENÇÃO: Seta o bit OUT2 no MCR para conectar a IRQ interna ao pino físico do chip!
    *(uart_reg + MCR) = 0x08;  //enable pin int
}
*/
//static unsigned char read_kbd_uart()
static unsigned char read_kbd()
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;
    unsigned char ch;
    while (!(*(uart_reg + LSR) & 0x01)) ;
    return (unsigned char)*(uart_reg + RHR);
    return ch;
}
//char read_kbd(void) {
//char read_kbd_int(void) {
//    while (rx_head == rx_tail); // Espera travar se buffer vazio
//    char ch = rx_buffer[rx_tail];
//    rx_tail++; // Incremento limpo de 8 bits
//    return ch;
//}


static unsigned char get_0x81()
{
    int i;
 
    type = read_kbd(); 
    length = read_kbd();

    for (i = 0; i < length; i++)        {
        read_kbd();
    }

    return 0;
}

static unsigned char get_0x88()
{
    unsigned char size = read_kbd();
   // printf("\nTamanho do pacote[%d]",size);
    // Não se esqueça eu lia hardcoded de 0 a 12
    // agora com size já sendo lido tenho que começar da posição 1
    // pois o codigo espera os comandos nas posições A=4 e B=2
    //ALTERADO ESSA MERDA POR ULTIMO
    int i=0;
    for (i = 1; i <= size; i++)    {
        key_buffer[i] = read_kbd();
    }
   // printf("\nTamanho lido[%d]",i);
    //printBuffer();
    if (key_buffer[2] == 0x0 && key_buffer[4] == 0x0) /*normal*/       {
        //Esse if é executado em todo key up
        //if( special_key_status > 0 ){
        //    if(  special_key_down ){
        //        special_key_up = 1;
        //        special_key_down=0;
        //        special_key_status=0;
        //    }
        //}
        if(  special_key_down ){
            special_key_up = 1;
            special_key_down=0;
            special_key_status=0;
        }
        if(  key_down ){
            key_down=0;
        }
        //printf("1-key_buffer[2] [%02x] - key_buffer[4] [%02x]\n",key_buffer[2],key_buffer[4]);
        return NO_KEY;
    }
    if (key_buffer[2] == 0x01 || key_buffer[2] == 0x10) /*control*/       {
        special_key_status = KEY_CTRL;
        special_key_down = 1;
        special_key_up = 0;
        return KEY_CTRL;
    }
    if (key_buffer[2] == 0x02 || key_buffer[2] == 0x20) /*shift*/       {
        special_key_down = 1;
        special_key_up = 0;
        special_key_status = KEY_SHIFT;
        return KEY_SHIFT;
    }
    if (key_buffer[2] == 0x04 ) /*Alt*/       {
        special_key_down = 1;
        special_key_up = 0;
        special_key_status = KEY_ALT;
        return KEY_ALT;
    }        
    if (key_buffer[2] == 0x40) /*Altgr*/       {
        special_key_down = 1;
        special_key_up = 0;
        special_key_status = KEY_ALTGR;
        return KEY_ALTGR;
    }        
    if (key_buffer[4] == 0x39) /*capslock*/       {
        if (mod_caps == 0)            {
            mod_caps = 2;
            send_cmd_keyboard(mod_caps);
        }
        else            {
            mod_caps = 0;
            send_cmd_keyboard(mod_caps);
        }
        //printf("6-key_buffer[2] [%02x] - key_buffer[4] [%02x]\n",key_buffer[2],key_buffer[4]);
        special_key_down = 1;
        return KEY_CAPS;
    }
    key_down = 1;
    key_up = 0;
    return VALID_KEY;
}
unsigned char get_packet()
{
    unsigned char ch;
    
    while (1)
    {
        // 1. Sincroniza no primeiro byte do cabeçalho (0x57)
        if (read_kbd() != 0x57)        {
            continue;
        }

        // 2. Confirma o segundo byte (0xAB)
        if (read_kbd() != 0xAB)        {
            continue;
        }

        // 3. Lê o Comando (Pelo seu dump, pode vir 0x88, 0x87 ou 0x81)
        cmd = read_kbd();
        //0x80 only in state 2/3/4
        if (cmd == 0x81)        {
            printf("81 ");
            get_0x81();
            //printBuffer("\nPKT 81 ");
            continue;
        }
        if (cmd == 0x82){
            cmd = read_kbd();
            //printf("82 ");
            continue;
        }
        //0x83 only in state 1
        //0x84 only in state 1
        if (cmd == 0x85){
            cmd = read_kbd();
            printf("85 [%02x]\n",cmd);
            continue;
        }
        //0x86 only in state 1
        if (cmd == 0x87)        {
            //printf("87 ");
            printBuffer("\nPKT 87");
            continue;
        }
        if (cmd == 0x88)        {            
            ch = get_0x88();
            printBuffer("\nPKT 88 ");
            return ch;
        }
    }
    return NO_KEY;
}
static void send_cmd_keyboard(unsigned char cmd)
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;

    unsigned char buf[11];

    buf[0] = 0x57;
    buf[1] = 0xAB;
    buf[2] = 0x12; // Comando de escrita de dados HID
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = cmd; // O bitmask dos LEDs (0x01, 0x02, 0x04)
    buf[8] = 0x00;
    buf[9] = 0x0F; // Constante de preenchimento (comum nesse protocolo)

    // Checksum: soma de buf[2] até buf[9]
    unsigned char ck = 0;
    for (int i = 2; i < 10; i++)    {
        ck += buf[i];
        //printf("[%02x]%02x.",buf[i],ck);
    }
    buf[10] = ck;

    // Envio para a UART
    for (int i = 0; i < 11; i++)    {
        while (!(*(uart_reg + LSR) & 0x20))            ;
        *(uart_reg + THR) = buf[i];
    }
    //printf("\nCAPSLOCK sent ck[%02x]\n",ck);
}
static void send_cmd_shutup(unsigned char cmd)
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)DRV_UART1_BASE;

    unsigned char buf[11];
//57 AB 12 00 00 00 00 FF 80 00 20
//57 AB 12 00 00 00 00 FF 80 00 91

    buf[0] = 0x57;
    buf[1] = 0xAB;
    buf[2] = 0x12; // Comando de escrita de dados HID
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0xFF; // O bitmask dos LEDs (0x01, 0x02, 0x04)
    buf[8] = 0x80;
    buf[9] = 0x00; // Constante de preenchimento (comum nesse protocolo)

    // Checksum: soma de buf[2] até buf[9]
    unsigned char ck = 0;
    for (int i = 2; i < 10; i++)    {
        ck += buf[i];
        //printf("[%02x]%02x.",buf[i],ck);
    }
    ck = 0x20;
    buf[10] = ck;

    // Envio para a UART
    for (int i = 0; i < 11; i++)    {
        while (!(*(uart_reg + LSR) & 0x20));
        *(uart_reg + THR) = buf[i];
    }
    //printf("\nshutup cmd sent ck[%02x]\n",ck);
}

unsigned char get_kbd_key(unsigned char code)
{
    unsigned char RetKey = 0; // default is 0 (No key pressed)

    //printf("special_key_status[%02X] mod_caps[%02X]\n",special_key_status,mod_caps);

    if (special_key_status == 0x00 && mod_caps == 0x00)    { // No modifier
        RetKey = OE_BASE_KEYMAP[code];
    }else
    if (special_key_status & KEY_SHIFT) { // Left & Right Shift modifier
        if (mod_caps == 2)
            RetKey = OE_BASE_KEYMAP[code];
        else    
            RetKey = OE_SHIFT_KEYMAP[code];
        return RetKey;    
    }else
    if ( special_key_status & KEY_CTRL )    { // CTRL modifier
        RetKey = OE_CTRL_KEYMAP[code];
    }else
    if (special_key_status & KEY_ALT)    { // Left ALT modifier
        RetKey = OE_ALT_KEYMAP[code];
    }else
    if (special_key_status & KEY_ALTGR)    { // Right ALT (ALT GR) modifier
        RetKey = OE_ALTGR_KEYMAP[code];
    }else
    if (mod_caps) { // Left & Right Shift modifier
        RetKey = OE_SHIFT_KEYMAP[code];
    }else{
        printf(".");
        return 0;
    }
    return RetKey;
}
void ch9350_shut_up(){
    send_cmd_shutup(0x80);
}
unsigned int get_char(){
    unsigned int ch,ch2;
    unsigned char key_flag=0;

    ch = 0;
    key_flag=0;
    init_kbd();

    while(1){
        memset((void *)key_buffer, 0, 48);
        key_flag=get_packet();
        //printf("key_flag[%02X]\n",key_flag);
        if ( key_flag == NO_KEY ){
            continue;
        }
        if( key_flag == KEY_CAPS ){
            send_cmd_keyboard(mod_caps);
            continue;
        }
        if( (key_flag != KEY_SHIFT) && (key_flag < VALID_KEY) )
            ch = (key_flag << 8) ;
        key_flag = 0;
        //printf("key_buffer[4] [%02x] \n",key_buffer[4]);
        if( key_buffer[4] > 0x0 ){
            ch2 = (unsigned char)get_kbd_key(key_buffer[4]);
            ch |= ch2;
            return ch;
        }else{
            ch &= 0xFF00;
            return ch;
        }
        if( (ch == 0x0A) || (ch == 0x0D)){
            ch = 0x0A;
           // printf("%c",ch);
            ch = 0x0D;
           // printf("%c",ch);
            return 0x0D;
        }
    }
    return ch;
}
void print_capslock(){
    printf("Estado tecla capslock[%02d]\n",mod_caps);
}
