#ifndef __GRAPHOS_H__
#define __GRAPHOS_H__

void drawVLine(short x, short y, short h, char color);
void drawHLine(int x, int y, int w, color_t color);
void drawLine(short x0, short y0, short x1, short y1, char color);
void drawCircle(short x0, short y0, short r, char color);
void drawCircleHelper( short x0, short y0, short r, unsigned char cornername, char color);
void fillCircle(short x0, short y0, short r, char color);
void fillCircleHelper(short x0, short y0, short r, unsigned char cornername, short delta, char color);
void drawRoundRect(short x, short y, short w, short h, short r, char color);
void fillRoundRect(short x, short y, short w, short h, short r, char color); 
void fillRect(short x, short y, short w, short h, color_t color);
void fillTri(float x0, float y0, float x1, float y1, float x2, float y2, char color);
void drawMultiLine(int num_lines,  short point_list[][2], char color);



#endif