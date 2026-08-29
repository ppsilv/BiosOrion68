#include "stdio.h"
#include "drv_picoVga.h"
#include "color.h"
#include "timers.h"

// delay.c

void delay_us(uint32_t us) {
    // Cada iteração do loop "dbra" leva ~8 ciclos.
    // 8MHz => 8 ciclos = 1us. 
    // Portanto, 1us = 1 iteração do loop.
    // O loop em C com while pode ser impreciso, esta versão é melhor.
    __asm__ volatile (
        "1:                             \n"  // Label do loop
        "subq.l  #1, %0                 \n"  // Decrementa o contador (4 ciclos)
        "cmpi.l  #0, %0                 \n"  // Compara com zero (6 ciclos)
        "bne     1b                     \n"  // Se não for zero, volta (6 ciclos se tomado)
        : "+r" (us)                          // Entrada e saída: registrador para 'us'
        :
        : "cc"                               // Altera flags
    );
}

void delay_1ms(void) {
    asm volatile (
        "move.w  #1000, %%d0\n"
        "1:\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "dbra    %%d0, 1b\n"
        : : : "d0"
    );
}

void delay_ms(int ms) {
    for(int i = 0; i < ms; i++) {
        delay_1ms();
    }
}



void uart0_write(unsigned char ch);

void picovga_putchar( char ch){
    WRITE_SCREEN = ch;
    uart0_write(ch); 
    //delay_us(1);
}

/*
void init_picoVga(){

}
inline void run_cmd(unsigned char cmd){
    RUN_CMD = cmd;
    delay10ms(1);
}

void picovga_gotoxy(int col,int row){
    delay10ms(5);
    REG_Y_LOW  = row; //(unsigned char)(row & 0x00FF);
    delay10ms(5);
    REG_Y_HIGH = 0;   //(unsigned char)(row > 8);
    delay10ms(5);
    REG_X_LOW  = col; //(unsigned char)(col & 0x00FF);
    delay10ms(5);
    REG_X_HIGH = 0;   //(unsigned char)(col > 8);
    delay10ms(5);
    //printf("Setando a position Y [%02X] X[%02X]\n",row,col);
    //RUN_CMD = CMD_SET_CUR_POS;
    run_cmd(CMD_SET_CUR_POS);
}
void picovga_gohome(){
    run_cmd(CMD_GO_HOME);
}
void picovga_set_color(unsigned char txtcolor,unsigned char bgcolor){
    unsigned int color;
    color = (txtcolor << 4) &0xF0;
    color |= bgcolor;
    //printf("Setando a cor [%02X]\n",color);
    delay10ms(1);
    SET_TXT_COLOR = color;
}
void clrscr(){
    //RUN_CMD = CMD_CLEAR_SCREEN;
    run_cmd(CMD_CLEAR_SCREEN);
    delay10ms(20);
}
*/