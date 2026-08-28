#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mc68000.h>
#include "keyboard.h"

// 🛠️🇧🇷

volatile unsigned char key_buffer[12];
volatile unsigned char cmd, length, type;

volatile unsigned char special_key_up=0;
volatile unsigned char special_key_down=0;
volatile unsigned char key_up=0;
volatile unsigned char key_down=0;
volatile unsigned char special_key_status=0;

volatile unsigned char _CapsFlag = 0;
volatile unsigned char mod_caps = 0;


unsigned char get_keypress();
void set_keyboard_leds(unsigned char led_status);
unsigned char get_packet();
unsigned char get_kbd_key(unsigned char code);

static void *mymemset(void *dest, int ch, unsigned int count)
{
    unsigned char *ptr = (unsigned char *)dest;

    while (count--){
        *ptr++ = (unsigned char)ch;
    }

    return dest;
}
/*
static void printBuffer(){
    for(int i = 0; i < 12; i++) {
        printf("%02x|",key_buffer[i]);
    }
    printf("\n");
}
*/

static void init_uart()
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)UART_KEYBOARD;

    // 1. Entra no modo DLAB para configurar Baud Rate
    *(uart_reg + LCR) = 0x83;
    *(uart_reg + DLL) = 8; // Divisor para 115200 com 14.7456MHz
    *(uart_reg + DLM) = 0;
    // 2. Sai do modo DLAB e define 8N1 (MUITO IMPORTANTE: usar 0x03)
    *(uart_reg + LCR) = 0x03;

    // 3. Configura FIFO (Habilita, limpa buffers e seta trigger de 14 bytes)
    *(uart_reg + FCR) = 0xC7;

    // 4. Limpa registradores de controle
    *(uart_reg + MCR) = 0x00;
    *(uart_reg + IER) = 0x00; // Garante que interrupções estão desligadas
}

static unsigned char read_kbd()
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)UART_KEYBOARD;
    unsigned char ch;
    while (!(*(uart_reg + LSR) & 0x01)) ;
    ch = (unsigned char)*(uart_reg + RHR);
   // printf("%02x ",ch);
    return ch;
}
/*
static unsigned char uart_read()
{
    return read_kbd();
}

static void write_kbd(unsigned char data)
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)UART_KEYBOARD;
    // Tente ler a versão do chip
    while (!(*(uart_reg + LSR) & 0x20))
    {
    };
    *(uart_reg + THR) = data;
}
*/
static unsigned char get_0x81()
{
    int i;
 
    type = read_kbd(); //*(uart_reg + RHR);
    length = read_kbd(); //*(uart_reg + RHR);
    if (length > 8)    {
        for (i = 0; i < length; i++)        {
            // while (!(*(uart_reg + LSR) & 0x01)) ;
            //*(uart_reg + RHR); // descarta
            read_kbd();
        }
    }
    if (length == 8)    {
        for (i = 0; i < 8; i++)        {
            //, while (!(*(uart_reg + LSR) & 0x01))            ;
            key_buffer[i] = read_kbd(); //*(uart_reg + RHR);
            // printf("[%02x ",key_buffer[i]);
        }
        if (key_buffer[2] != 0)        {
            return key_buffer[2]; // Retorna o ScanCode real!
        }
    }
    else    {
        // Caso venha um tamanho inesperado, limpa para não desalinhar
        for (i = 0; i < length; i++)        {
            //while (!(*(uart_reg + LSR) & 0x01))    ;
            //*(uart_reg + RHR);
            read_kbd();
        }
    }
    return 0;
}
static void get_0x87()
{
    int i;
    length = read_kbd(); //*(uart_reg + RHR);

    // Esvazia os bytes desse aviso rapidamente
    for (i = 0; i < length; i++) {
        // while (!(*(uart_reg + LSR) & 0x01));
        //*(uart_reg + RHR);
        read_kbd();
    }
}
static unsigned char get_0x88()
{
    unsigned char size = read_kbd();
    // Não se esqueça eu lia hardcoded de 0 a 12
    // agora com size já sendo lido tenho que começar da posição 1
    // pois o codigo espera os comandos nas posições A=4 e B=2
    for (int i = 1; i < size; i++)    {
        key_buffer[i] = read_kbd(); //*(uart_reg + RHR);
    }
    //printBuffer();
    if (key_buffer[2] == 0x0 && key_buffer[4] == 0x0) /*normal*/       {
        //Esse if é executado em todo key up
        if( special_key_status > 0 ){
            if(  special_key_down ){
                special_key_up = 1;
                special_key_down=0;
                special_key_status=0;
            }
        }
        if(  key_down ){
            key_up = 1;
            key_down=0;
            return NO_KEY;
        }
        if(  special_key_down ){
            special_key_up = 1;
            special_key_down=0;
            special_key_status=0;
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
            set_keyboard_leds(mod_caps);
        }
        else            {
            mod_caps = 0;
            set_keyboard_leds(mod_caps);
        }
        //printf("6-key_buffer[2] [%02x] - key_buffer[4] [%02x]\n",key_bufferA[2],key_bufferA[4]);
        special_key_down = 1;
        return KEY_CAPS;
    }
    key_down = 1;
    key_up = 0;
    return VALID_KEY;
}
unsigned char get_packet()
{
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
        cmd = read_kbd(); // *(uart_reg + RHR);
        // if ( cmd != 0x82)
        //     printf("cmd %02x\n",cmd);

        if (cmd == 0x81)        {
            get_0x81();
            continue;
        }
        if (cmd == 0x82)
            continue;
        if (cmd == 0x87)        {
            get_0x87();
            continue;
        }
        if (cmd == 0x88)        {
            return get_0x88();
        }
    }
    return NO_KEY;
}
void set_keyboard_leds(unsigned char led_status)
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)UART_KEYBOARD;

    unsigned char buf[11];

    buf[0] = 0x57;
    buf[1] = 0xAB;
    buf[2] = 0x12; // Comando de escrita de dados HID
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = led_status; // O bitmask dos LEDs (0x01, 0x02, 0x04)
    buf[8] = 0x00;
    buf[9] = 0x0F; // Constante de preenchimento (comum nesse protocolo)

    // Checksum: soma de buf[0] até buf[9]
    unsigned char ck = 0;
    for (int i = 2; i < 10; i++)    {
        ck += buf[i];
    }
    buf[10] = ck;

    // Envio para a UART
    for (int i = 0; i < 11; i++)    {
        while (!(*(uart_reg + LSR) & 0x20))            ;
        *(uart_reg + THR) = buf[i];
    }
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

unsigned int get_key(){
    unsigned int ch,ch2;
    unsigned char key_flag=0;

    ch = 0;
    key_flag=0;
    while(1){
        mymemset((void *)key_buffer, 0, sizeof(key_buffer));
        key_flag=get_packet();
        //printf("key_flag[%02X]\n",key_flag);
        if ( key_flag == NO_KEY )
            continue;
        if(  key_buffer[4] == 0x39 )
            return 0;    
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


int main()
{
    unsigned int ch;
    while(1){
        ch = get_key();
        if( ch > 0x20){
            //printf("\nspecial_key_down [%02X] key_down[%02x]\n",special_key_down,key_down);
            //printf("special_key_up   [%02X] key_up    [%02x]\n",special_key_up,key_up);
            if( ch <= 255 )
                printf("%c",ch);
            else    
                printf("%02X",ch);
        }
        if ( ch == 'B' ){
            return 0;
        }
    }    
    return 0;
}
        //printf("key_buffer[4] [%02x] - ch2[%02X] ch[%02X]\n",key_buffer[4],ch2,ch);
//        if( ch > 0x00 ){
          //  printf("%02X",ch);
//            return ch;
//        }
