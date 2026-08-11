#ifndef __VIDEO_GRAPHICS_H__
#define __VIDEO_GRAPHICS_H__
#include <stdint.h>
#include <color.h>

#define VID_DRAW_VLINE          6
#define VID_DRAW_HLINE          7
#define VID_DRAW_LINE           8
#define VID_DRAW_CIRCLE         9
#define VID_DRAW_CIRCLEHELPER   10
#define VID_FILLCIRCLE          11
#define VID_FILLCIRCLEHELPER    12
#define VID_DRAWROUNDRECT       13
#define VID_FILLROUNDRECT       14
#define VID_FILLRECT            15



void drawVLine(uint16_t x, uint16_t y, uint16_t h, char color);
void drawHLine(int x, int y, int w, color_t color);
void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, char color);
void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, char color);
void drawCircleHelper( uint16_t x0, uint16_t y0, uint16_t r, unsigned char cornername, char color);
void fillCircle(uint16_t x0, uint16_t y0, uint16_t r, char color);
void fillCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, unsigned char cornername, uint16_t delta, char color);
void drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, char color);
void fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, char color);
void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, color_t color);
//void fillTri(float x0, float y0, float x1, float y1, float x2, float y2, char color);
//void drawMultiLine(int num_lines,  uint16_t point_list[][2], char color);

#endif



