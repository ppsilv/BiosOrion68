#include <stdint.h>
#include <stdio.h>
#include <color.h>

#include "vga_video_graphics.h"


void drawVLine(uint16_t x, uint16_t y, uint16_t h, char color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_DRAW_VLINE;
    register uint32_t d2 asm("d2") = x;
    register uint32_t d3 asm("d3") = y;
    register uint32_t d4 asm("d4") = h;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4)
        : "memory"
    );
}

void drawHLine(int x, int y, int w, color_t color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_DRAW_HLINE;
    register uint32_t d2 asm("d2") = x;
    register uint32_t d3 asm("d3") = y;
    register uint32_t d4 asm("d4") = w;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4)
        : "memory"
    );
}
void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, char color){
    //printf("VGA_Video: x0[%d] y0[%d] x1[%d] x1[%d] col[%d]\n",x0,y0,x1,y1,color);

    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_DRAW_LINE;
    register uint32_t d2 asm("d2") = x0;
    register uint32_t d3 asm("d3") = y0;
    register uint32_t d4 asm("d4") = x1;
    register uint32_t d5 asm("d5") = y1;
    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5)
        : "memory"
    );
}
void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, char color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_DRAW_CIRCLE;
    register uint32_t d2 asm("d2") = x0;
    register uint32_t d3 asm("d3") = y0;
    register uint32_t d4 asm("d4") = r;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4)
        : "memory"
    );
}
void drawCircleHelper( uint16_t x0, uint16_t y0, uint16_t r, unsigned char cornername, char color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_DRAW_CIRCLEHELPER;
    register uint32_t d2 asm("d2") = x0;
    register uint32_t d3 asm("d3") = y0;
    register uint32_t d4 asm("d4") = r;
    register uint32_t d5 asm("d5") = cornername;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5)
        : "memory"
    );
}
void fillCircle(uint16_t x0, uint16_t y0, uint16_t r, char color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_FILLCIRCLE;
    register uint32_t d2 asm("d2") = x0;
    register uint32_t d3 asm("d3") = y0;
    register uint32_t d4 asm("d4") = r;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4)
        : "memory"
    );
}
void fillCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, unsigned char cornername, uint16_t delta, char color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_FILLCIRCLEHELPER;
    register uint32_t d2 asm("d2") = x0;
    register uint32_t d3 asm("d3") = y0;
    register uint32_t d4 asm("d4") = r;
    register uint32_t d5 asm("d5") = cornername;
    register uint32_t d6 asm("d6") = delta;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5), "r"(d6)
        : "memory"
    );
}
void drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, char color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_DRAWROUNDRECT;
    register uint32_t d2 asm("d2") = x;
    register uint32_t d3 asm("d3") = y;
    register uint32_t d4 asm("d4") = w;
    register uint32_t d5 asm("d5") = h;
    register uint32_t d6 asm("d6") = r;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5), "r"(d6)
        : "memory"
    );
}
void fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, char color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_FILLROUNDRECT;
    register uint32_t d2 asm("d2") = x;
    register uint32_t d3 asm("d3") = y;
    register uint32_t d4 asm("d4") = w;
    register uint32_t d5 asm("d5") = h;
    register uint32_t d6 asm("d6") = r;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5), "r"(d6)
        : "memory"
    );
}
void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, color_t color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d1 asm("d1") = VID_FILLRECT;
    register uint32_t d2 asm("d2") = x;
    register uint32_t d3 asm("d3") = y;
    register uint32_t d4 asm("d4") = w;
    register uint32_t d5 asm("d5") = h;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5)
        : "memory"
    );
}
/*
void fillTri(float x0, float y0, float x1, float y1, float x2, float y2, char color){
    register uint32_t d0 asm("d0") = color;
    register uint32_t d2 asm("d2") = x0;
    register uint32_t d3 asm("d3") = y0;
    register uint32_t d4 asm("d4") = x1;
    register uint32_t d5 asm("d5") = y1;
    register uint32_t d6 asm("d6") = x2;
    register uint32_t d7 asm("d7") = y2;

    asm volatile (
        "trap #2"
        :
        : "r"(d0), "r"(d2), "r"(d3), "r"(d4), "r"(d5), "r"(d6), "r"(d7)
        : "memory"
    );

}
void drawMultiLine(int num_lines,  uint16_t point_list[][2], char color){

}
*/
