#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keyboard.h"

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

const unsigned char OE_CTRL_KEYMAP[] = {0x00, 0x00, 0x00, 0x00, 0xA1, 0xA2, 0x03, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC,  // 0x
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

volatile unsigned char cmd, length, type;
volatile unsigned char special_key_up=0;
volatile unsigned char special_key_down=0;
volatile unsigned char key_up=0;
volatile unsigned char key_down=0;
volatile unsigned char special_key_status=0;
volatile unsigned char _CapsFlag = 0;
volatile unsigned char mod_caps = 0;
volatile unsigned char debug_pkt = 0;
volatile unsigned char key_buffer[1048]={0};

extern void setup_uart1(void);
extern void uart1_putc(char c);
extern char uart1_has_data(void);
extern char uart1_getc(void);

void init_kbd()
{
    setup_uart1();
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
}


static void send_cmd_keyboard(unsigned char cmd)
{
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
        //while (!(*(uart_reg + LSR) & 0x20))            ;
        //*(uart_reg + THR) = buf[i];
        uart1_putc(buf[i]);
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

#include <stdio.h>
#include <string.h>
#include "keyboard.h"

/* ... (mantenha aqui as mesmas tabelas OE_BASE_KEYMAP, OE_SHIFT_KEYMAP,
   etc., e as mesmas globais key_buffer/mod_caps/special_key_status/...
   do keyboard.c original -- nao precisam mudar) ... */

/* =========================================================================
 * MAQUINA DE ESTADOS NAO-BLOQUEANTE
 *
 * Ideia central: em vez de "while(!uart1_has_data());" (que trava o core
 * inteiro), cada chamada de kbd_poll() consome SO os bytes que ja estao
 * na FIFO da UART agora, avanca o estado do parser, e retorna na hora se
 * a FIFO esvaziar -- nunca espera. O progresso fica guardado em variaveis
 * static, entao a proxima chamada continua de onde parou.
 * ========================================================================= */

typedef enum {
    KBD_WAIT_HDR1,      /* esperando 0x57 */
    KBD_WAIT_HDR2,      /* esperando 0xAB */
    KBD_WAIT_CMD,       /* esperando o byte de comando (0x81/0x82/0x85/0x87/0x88) */

    /* sub-estados do comando 0x81 (descarta 'length' bytes) */
    KBD_81_WAIT_TYPE,
    KBD_81_WAIT_LEN,
    KBD_81_SKIP_PAYLOAD,

    /* sub-estados do comando 0x82 (descarta 1 byte) */
    KBD_82_WAIT_BYTE,

    /* sub-estado do comando 0x85 (descarta 1 byte, so pra log) */
    KBD_85_WAIT_BYTE,

    /* sub-estados do comando 0x88 (payload real do teclado) */
    KBD_88_WAIT_SIZE,
    KBD_88_READ_PAYLOAD,
} kbd_state_t;

static kbd_state_t kbd_state = KBD_WAIT_HDR1;
static unsigned char kbd_size = 0;
static unsigned int  kbd_index = 0;

/* Resultado pendente: -1 = nenhuma tecla pronta ainda */
static int kbd_pending_char = -1;

/* Escopo do arquivo (nao mais dentro do if) -- precisa ser acessivel
   tambem no ramo de "tecla solta", para resetar corretamente. */
static unsigned char capslock_ja_pressionado = 0;

/*
 * Chamado quando o comando 0x88 termina de ser lido por completo.
 * Reaproveita a MESMA logica de decisao que seu get_0x88()/get_kbd_key()
 * originais tinham -- so movida pra fora do loop bloqueante.
 */
static void kbd_finish_0x88(void)
{
    unsigned char key_flag;

    if (key_buffer[2] == 0x0 && key_buffer[4] == 0x0) { /* normal, nenhuma tecla */
        if (special_key_down) {
            special_key_up = 1;
            special_key_down = 0;
            special_key_status = 0;
        }
        if (key_down) key_down = 0;
        capslock_ja_pressionado = 0;   /* tecla solta: libera pro proximo toggle */
        return; /* kbd_pending_char continua -1 */
    }

    /* IMPORTANTE: os ramos de modificador NAO retornam mais cedo --
       eles so setam o estado e caem para o trecho comum no final, que
       verifica key_buffer[4] e decodifica a letra do MESMO pacote,
       exatamente como o get_char() original fazia depois de chamar
       get_0x88(). */
    if (key_buffer[2] == 0x01 || key_buffer[2] == 0x10) {
        special_key_status = KEY_CTRL; special_key_down = 1; special_key_up = 0;
    }
    else if (key_buffer[2] == 0x02 || key_buffer[2] == 0x20) {
        special_key_status = KEY_SHIFT; special_key_down = 1; special_key_up = 0;
    }
    else if (key_buffer[2] == 0x04) {
        special_key_status = KEY_ALT; special_key_down = 1; special_key_up = 0;
    }
    else if (key_buffer[2] == 0x40) {
        special_key_status = KEY_ALTGR; special_key_down = 1; special_key_up = 0;
    }
    else if (key_buffer[4] == 0x39) { /* capslock -- deteccao de borda */
        if (!capslock_ja_pressionado) {
            capslock_ja_pressionado = 1;
            if (mod_caps == 0) { mod_caps = 2; }
            else                { mod_caps = 0; }
            send_cmd_keyboard(mod_caps);
        }
        return;   /* capslock nao produz caractere, so alterna o LED */
    }
    else {
        key_down = 1; key_up = 0;
    }

    /* Trecho comum: decodifica key_buffer[4] usando o special_key_status
       que acabou de ser definido acima (ou 0x00 se nenhum modificador
       estava ativo neste pacote) -- roda para TODOS os casos exceto
       "nenhuma tecla" e "capslock", igual ao get_char() original. */
    if (key_buffer[4] > 0x0) {
        key_flag = (unsigned char)get_kbd_key(key_buffer[4]);
        kbd_pending_char = (int)key_flag;
    }
}

/*
 * kbd_poll() -- chame isso a cada volta do loop principal, junto com
 * cyw43_arch_poll(). NUNCA bloqueia: consome so os bytes que a FIFO da
 * UART ja tiver, e retorna na hora quando ela esvaziar.
 */
void kbd_poll(void)
{
    unsigned char c;

    while (uart1_has_data()) {
        c = uart1_getc();

        switch (kbd_state) {

        case KBD_WAIT_HDR1:
            if (c == 0x57) kbd_state = KBD_WAIT_HDR2;
            break;

        case KBD_WAIT_HDR2:
            kbd_state = (c == 0xAB) ? KBD_WAIT_CMD : KBD_WAIT_HDR1;
            break;

        case KBD_WAIT_CMD:
            if (c == 0x81) {
                kbd_state = KBD_81_WAIT_TYPE;
            } else if (c == 0x82) {
                kbd_state = KBD_82_WAIT_BYTE;
            } else if (c == 0x85) {
                kbd_state = KBD_85_WAIT_BYTE;
            } else if (c == 0x87) {
                kbd_state = KBD_WAIT_HDR1;   /* nada a fazer alem de re-sincronizar */
            } else if (c == 0x88) {
                kbd_state = KBD_88_WAIT_SIZE;
            } else {
                kbd_state = KBD_WAIT_HDR1;   /* comando desconhecido: re-sincroniza */
            }
            break;

        /* ---- 0x81: descarta 'length' bytes de payload ---- */
        case KBD_81_WAIT_TYPE:
            type = c;
            kbd_state = KBD_81_WAIT_LEN;
            break;

        case KBD_81_WAIT_LEN:
            length = c;
            kbd_index = 0;
            kbd_state = (length > 0) ? KBD_81_SKIP_PAYLOAD : KBD_WAIT_HDR1;
            break;

        case KBD_81_SKIP_PAYLOAD:
            kbd_index++;
            if (kbd_index >= length)
                kbd_state = KBD_WAIT_HDR1;
            break;

        /* ---- 0x82: descarta 1 byte ---- */
        case KBD_82_WAIT_BYTE:
            kbd_state = KBD_WAIT_HDR1;
            break;

        /* ---- 0x85: descarta 1 byte ---- */
        case KBD_85_WAIT_BYTE:
            kbd_state = KBD_WAIT_HDR1;
            break;

        /* ---- 0x88: payload real do teclado ---- */
        case KBD_88_WAIT_SIZE:
            kbd_size = c;
            kbd_index = 1;
            memset((void *)key_buffer, 0, sizeof(key_buffer));
            kbd_state = (kbd_size > 0) ? KBD_88_READ_PAYLOAD : KBD_WAIT_HDR1;
            if (kbd_size == 0) kbd_finish_0x88();
            break;

        case KBD_88_READ_PAYLOAD:
            if (kbd_index < sizeof(key_buffer))
                key_buffer[kbd_index] = c;
            kbd_index++;
            if (kbd_index > kbd_size) {
                kbd_finish_0x88();
                kbd_state = KBD_WAIT_HDR1;
            }
            break;
        }

        /* Se uma tecla ja ficou pronta, para de consumir mais bytes agora
           -- devolve o controle pro loop principal (cyw43_arch_poll etc)
           e continua exatamente daqui na proxima chamada. */
        if (kbd_pending_char != -1)
            break;
    }
}

/*
 * Retorna a tecla pronta (0-255), ou -1 se nenhuma tecla nova estiver
 * disponivel ainda. NUNCA bloqueia.
 */
int get_char_nonblocking(void)
{
    int ch = kbd_pending_char;
    kbd_pending_char = -1;   /* consome o resultado */
    return ch;
}


/*
    NESSE MOMENTO NÃO PRECISO MAS TALVEZ NO FUTURO PRECISE
    ISSO JÁ FUNCIONOU NO MC68000.


    static void send_cmd_shutup(unsigned char cmd)
{
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
        //while (!(*(uart_reg + LSR) & 0x20));
        //*(uart_reg + THR) = buf[i];
        uart1_putc(buf[i]);
    }
    //printf("\nshutup cmd sent ck[%02x]\n",ck);
    
}




*/