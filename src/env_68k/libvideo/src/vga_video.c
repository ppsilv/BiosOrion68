#include "stdio.h"
#include <stdint.h>
#include <vga_video.h>
#include "vga_video.h"
#include "color.h"

#define SYS_VGAWRITECHAR    1
#define SYS_VGAGOTOXY       2
#define SYS_VGACLS          3
#define SYS_VGAHOME         4
#define SYS_VGASETCOLOR     5


void run_cmd(uint8_t cmd){

}

void video_writechar(char ch){
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_VGAWRITECHAR;

    __asm__ volatile ("trap #2" : "=r"(res) : "r"(cmd) : "memory");
}

void gotoxy(uint16_t x,uint16_t y){
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_VGAGOTOXY;
    register uint16_t arg_d2 __asm__("d2") = (uint16_t)y;
    register uint16_t arg_d0 __asm__("d0") = (uint16_t)x;

    __asm__ volatile ("trap #2" : "=r"(res) : "r"(cmd), "r"(arg_d2), "r"(arg_d0) : "memory");
}
void gohome(){
    register uint32_t cmd    __asm__("d1") = SYS_VGAHOME;
    __asm__ volatile ("trap #2" :: "r"(cmd): "memory");
}
void setcolor(uint8_t txtcolor,uint8_t bgcolor){
    uint8_t color;
    color  = (txtcolor << 4) & 0xF0;
    color |= (bgcolor & 0x0F);
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_VGASETCOLOR;
    register uint32_t arg_d0 __asm__("d0") = (uint8_t)color;

    __asm__ volatile ("trap #2" : "=r"(res) : "r"(cmd), "r"(arg_d0) : "memory");
}
void clrscr(){
    register uint32_t cmd    __asm__("d1") = SYS_VGACLS;
    __asm__ volatile ("trap #2" :: "r"(cmd): "memory");
}


