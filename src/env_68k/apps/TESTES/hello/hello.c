#include <stdio.h>
#include <stdlib.h>

#include <mc68000.h>

#define LEDS_ADDRESS 0x4400
#define LEDS (*(volatile unsigned char *)LEDS_ADDRESS)

void delay(unsigned int time) {
    for (volatile unsigned int i = 0; i < time; i++);
}

// Função para verificar stack
void check_stack(void) {
    unsigned long stack_val;
    asm volatile (
        "move.l %%sp, %0\n\t"
        : "=r" (stack_val)
    );
    printf("Stack pointer: 0x%08X\n", stack_val);
}

void led_effects() {
    // Efeito de "carrinho"
    unsigned char patterns[] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
        0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
    };

    for (int i = 0; i < 15; i++) {
        LEDS = patterns[i];
        delay(30000);
    }

    // Efeito de intensidade crescente
    for (int i = 0; i < 8; i++) {
        LEDS |= (1 << i);
        delay(20000);
    }

    for (int i = 0; i < 8; i++) {
        LEDS &= ~(1 << i);
        delay(20000);
    }
}

/* Definindo o endereço base da sua Video Card */
//#define SCREEN_REG 0xB8001
#define S1_REG 0xB8003
#define S2_REG 0xB8005
#define S3_REG 0xB8007
//#define CONFIG_REG 0xB8009


//Register address
#define WRITE_SCREEN   0xFF8001        //Endereço real  3 0x03  o pico enxerga 0x00
#define REG_02         0xFF8003        //Endereço real  3 0x03  o pico enxerga 0x01
#define REG_03         0xFF8005        //Endereço real  5 0x05  o pico enxerga 0x02
#define REG_04         0xFF8007        //Endereço real  7 0x07  o pico enxerga 0x03
#define CONFIG_REG     0xFF8009        //Endereço real  9 0x09  o pico enxerga 0x04
#define REG_06         0xFF800B        //Endereço real 11 0x0b  o pico enxerga 0x05
#define REG_07         0xFF800D        //Endereço real 13 0x0d  o pico enxerga 0x06
#define REG_08         0xFF800F        //Endereço real 15 0x0f  o pico enxerga 0x07
#define REG_09         0xFF8011        //Endereço real 17 0x11  o pico enxerga 0x08
#define REG_0A         0xFF8013        //Endereço real 19 0x13  o pico enxerga 0x09
#define REG_0B         0xFF8015        //Endereço real 21 0x15  o pico enxerga 0x0A
#define REG_0C         0xFF8017        //Endereço real 23 0x17  o pico enxerga 0x0B
#define REG_0D         0xFF8019        //Endereço real 25 0x19  o pico enxerga 0x0C
#define REG_0E         0xFF801B        //Endereço real 27 0x1b  o pico enxerga 0x0D
#define REG_0F         0xFF801D        //Endereço real 29 0x1d  o pico enxerga 0x0E
#define REG_10         0xFF801F        //Endereço real 31 0x1f  o pico enxerga 0x0F
#define REG_11         0xFF8021        //Endereço real 33 0x21  o pico enxerga 0x10
#define REG_12         0xFF8023        //Endereço real 35 0x23  o pico enxerga 0x11
#define REG_13         0xFF8025        //Endereço real 37 0x25  o pico enxerga 0x12
#define SET_MODE       0xFF8027        //Endereço real 39 0x27  o pico enxerga 0x13 (0=Texto+Scroll, 1=Texto Fixo, 2=320x200, 3=640x200)
#define SET_TXT_COLOR  0xFF8029        //Endereço real 41 0x29  o pico enxerga 0x14
#define CHANGE_CUR_POS 0xFF802B        //Endereço real 43 0x2b  o pico enxerga 0x15
#define REG_X_HIGH     0xFF802D        //Endereço real 45 0x2d  o pico enxerga 0x16
#define REG_X_LOW      0xFF802F        //Endereço real 47 0x2f  o pico enxerga 0x17
#define REG_Y_HIGH     0xFF8031        //Endereço real 49 0x31  o pico enxerga 0x18
#define REG_Y_LOW      0xFF8033        //Endereço real 51 0x33  o pico enxerga 0x19
#define CHANGE_BUFFER  0xFF8035        //Endereço real 53 0x35  o pico enxerga 0x1A
#define SELECT_SCREEN  0xFF8037        //Endereço real 55 0x37  o pico enxerga 0x1B
#define SET_HORIZONTAL 0xFF8039        //Endereço real 57 0x39  o pico enxerga 0x1C
#define SET_VERTICAL   0xFF803B        //Endereço real 59 0x3b  o pico enxerga 0x1D
#define RUN_CMD        0xFF803D        //Endereço real 61 0x3d  o pico enxerga 0x1E
#define CORINGA        0xFF803F        //Endereço real 63 0x3f  o pico enxerga 0x1F





//Commands
#define CMD_SYSTEM_ENABLE   0xA5
#define CMD_CLEAR_SCREEN    0xA4
#define CMD_SET_CUR_POS     0xA3
#define CMD_SET_TXT_COLOR   0xA2
#define CMD_GO_HOME         0xA1

unsigned char *vga_run_cmd = (unsigned char *)RUN_CMD;

void vga_set_txt_mode(unsigned char mode){
    unsigned char *config_reg_txt_mode = (unsigned char *)SET_MODE;

    *config_reg_txt_mode = (unsigned char)mode;

}

void vga_set_x(unsigned short x) {
    unsigned char *config_reg_x_low = (unsigned char *)REG_X_LOW;
    unsigned char *config_reg_x_high = (unsigned char *)REG_X_HIGH;
    if( x < 80 ){
        *config_reg_x_low  = (unsigned char)x;
    }else{
        *config_reg_x_low  = (unsigned char)(x & 0xFF);
        *config_reg_x_high = (unsigned char)(x >> 8);
    }
}
void vga_set_y(unsigned short y) {
    unsigned char *config_reg_y_low = (unsigned char *)REG_Y_LOW;
    unsigned char *config_reg_y_high = (unsigned char *)REG_Y_HIGH;
    if ( y < 80 ){
        *config_reg_y_low  = (unsigned char)y;
    }else{
        *config_reg_y_low  = (unsigned char)(y & 0xFF);
        *config_reg_y_high = (unsigned char)(y >> 8);
    }
}
void vga_set_txt_color(unsigned char color){
    unsigned char *config_reg_txt_color = (unsigned char *)SET_TXT_COLOR;

    *config_reg_txt_color = (unsigned char)color;
}
void vga_go_home(){
    *vga_run_cmd = CMD_GO_HOME;
}
unsigned short read_uint() {
    unsigned short val = 0;
    char c;
    while (1) {
        c = getchar(); // Lê um caractere da serial
        if (c == '\r' || c == '\n' || c == ' ') break; // Para no Enter ou Espaço
        if (c >= '0' && c <= '9') {
            putchar(c); // Ecoa o que você digitou
            val = val * 10 + (c - '0');
        }
    }
    putchar('\n');
    return val;
}

void imprime_char(char ch)
{
    unsigned char *screen_reg = (unsigned char *)WRITE_SCREEN;
    *screen_reg = ch;

}

void read_str(char *str) {
    char c;
    char *p = str;

    while (1) {
        c = getchar(); // Lê um caractere
        // Para no Enter (Unix usa \n, alguns terminais mandam \r)
        if (c == '\n' || c == '\r') {
            break;
        }
        // Armazena o caractere no ponteiro e avança
        *p = c;
        p++;
        imprime_char(c);
    }
    *p = '\0'; // FINALIZA A STRING (Essencial em C!)
    putchar('\n');
    imprime_char('\0');
    imprime_char('\n');
}
void clrscr()
{
    *vga_run_cmd = CMD_CLEAR_SCREEN;
}



void imprime_tbl_ascii()
{
    unsigned char *screen_reg = (unsigned char *)WRITE_SCREEN;

    for(unsigned char i=0x20;i<0x80;i++){
        *screen_reg = i;
        delay(50);
    }

}

void show_menu(){
    int choice;
    unsigned short pos_x;
    unsigned short pos_y;

    choice = 'A';
    while (choice != 1000){
        //printf("\n--- TCPBOX68K VIDEO TEST ---\n");
        printf("1 - Clear Screen\n");
        printf("2 - Set X Position\n");
        printf("3 - Set text and bg color \n");
        printf("4 - Envia string\n");
        printf("5 - Imprime tbl ascii\n");
        printf("6 - Imprime um caractere\n");
        printf("7 - Home\n");
        printf("8 - Troca modo tela");

        printf("0 - sai do programa\n");
        printf("\nEscolha: ");
        choice = getchar();
        printf("choice [%02x]\n",choice);
        switch(choice){
            case '1':
                printf("Limpando a tela\n");
                clrscr();
                break;
            case '2':
                printf("Digite a posicao X (0-639): ");
                pos_x = read_uint();
                vga_set_x(pos_x);
                printf("X definido para %d\n", pos_x);
                printf("Digite a posicao Y (0-319): ");
                pos_y = read_uint();
                vga_set_y(pos_y);
                printf("Y definido para %d\n", pos_y);
           // case '4': //Change cursor position.
                *vga_run_cmd =CMD_SET_CUR_POS;
                break;
            case '3':
                unsigned char textColor,bgColor,color;
                printf("Digite   a  cor  do  texto [0-8]: ");
                textColor = read_uint();
                printf("Digite a cor do background [0-8]: ");
                bgColor = read_uint();
                color = (unsigned char )((textColor<<4)&0xF0) | bgColor;
                printf("textColor[%x][%x] bgColor[%x] color[%x]",((textColor<<4)&0xF0),textColor,bgColor,color);
                vga_set_txt_color(color);
                break;
            case '4':
                printf("Escolhido opcao 4\n");
                char cstr[256];
                read_str(cstr);
                break;
            case '5':
                printf("\nVamos imprimir a tabela ascii\n");
                imprime_tbl_ascii();
                break;
            case '6':
                printf("Digite um caractere: ");
                char ch = getchar();
                imprime_char(ch);
                break;
            case '7':
                printf("Go home");
                vga_go_home();
                break;
            case '8':
                unsigned char mode;
                printf("Digite 0=320x200 1=640x400: ");
                mode = read_uint();
                vga_set_txt_mode(mode);
                break;
            case '0':
                return;

                break;
        }
    }
}


int main() {
    //char str[10]={0};
 //   check_stack();  // ✅ Verificar stack no início
/* Criando ponteiros de 8 bits (unsigned char) para os endereços */
    unsigned char *screen_reg = (unsigned char *)WRITE_SCREEN;
    unsigned char *config_reg = (unsigned char *)CONFIG_REG;


    printf("\n--- Teste de Video Card com Pi Pico ---\n");

    printf("Vou testar a placa de video: enviando SYSTEM_ENABLE \n");
    *config_reg = CMD_SYSTEM_ENABLE;

    show_menu();


    for(unsigned char i=0x20;i<0x80;i++){
        *screen_reg = i;
        delay(50);
    }


    LEDS = 0x01;

/*
    printf("Hello MC68000!\n");
    LEDS = 0x02;
    printf("Digite 9 chars ");
    LEDS = 0x03;
    gets_s(str,10);


    printf("\nDigitado %s\n",str);
    led_effects();

        LEDS = 0xFF;    // Todos acesos
        delay(50000);
        LEDS = 0x00;    // Todos apagados
        delay(50000);
*/


    return 0;
}


