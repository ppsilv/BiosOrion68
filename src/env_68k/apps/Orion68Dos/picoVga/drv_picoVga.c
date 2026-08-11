#include "stdio.h"
#include "drv_picoVga.h"
#include "color.h"
#include "timers.h"

void init_picoVga(){

}
inline void run_cmd(unsigned char cmd){
    RUN_CMD = cmd;
    delay10ms(1);
}
void uart0_write(unsigned char ch);
/*
void write_str(){
    char *str="Teste do video...\n";

    while(*str){
        int x=0x60;  //🚀🛠️    funciona para a cpu em 4Mhz
        WRITE_SCREEN = *str++;
        while(x--); //0,11us
    }
}
*/
void picovga_putchar( char ch){
    WRITE_SCREEN = ch;
    uart0_write(ch); 
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