#ifndef __vga_TEXT_H__
#define __vga_TEXT_H__    

#include "vga_drv.h"
#include "cursor.h"
#include "colors.h"
#include "font.h"

// For drawLine
#define swap(a, b) { short t = a; a = b; b = t; }

// For writing text
#define tab_space 4 // number of spaces for a tab

// For accessing the font library
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))

void tft_write(uint8_t c) ;
static void printString(char* str) ;
static void pchar(char c);
static void clrscr();

// 5x7 font
static void writeStringBold(char* str);

void drawChar(uint8_t c, color_t color, color_t bg, uint8_t size) ;

/*
//NOVAS IMPLEMENTAÇÕES
//(0=Texto+Scroll, 1=Texto Fixo, 2=320x200, 3=640x200)
enum screenMode {
    MODE_TEXT_40_S,
    MODE_TEXT_40_F,
    MODE_TEXT_80_S,
    MODE_TEXT_80_F,
    MODE_320x240,
    MODE_640x480
};
typedef enum screenMode screenMode_t ;
*/
typedef struct vga_text vga_t ;

struct vga_text {
  //getters
  uint8_t (*getTextColor)(void);
  uint8_t (*getTextSize)(void);
  uint16_t (*get_blink_interval)(void);
  //setters
  void (*setTextColor)(char c, char b);
//  void (*setTextSize)(uint8_t s);
  void (*setTextCursorPos)(uint16_t x, uint16_t y);
  void (*set_blink_interval)(uint16_t interval);
  void (*setTextCursorVisible)(bool v);
  void (*setTextCursorBlink)(bool b);
  void (*set_vga_data_array)(uint8_t video_data_array[]);
  void (*set_vga_home)(void);
  void (*set_vga_mode)(uint8_t mode);
  //functions
  void (*printString)(char* str);
  void (*printString1)(char* str,int32_t num);
  void (*printString2)(char* str,int32_t num);
  void (*pchar)(uint8_t c);
  void (*clrscr)(void);
  void (*change_mode)(uint8_t mode);
  void (*inc_cursor_y)(void);
  void (*inc_cursor_x)(void);
  void (*dec_cursor_y)(void);
  void (*dec_cursor_x)(void);



  void * _private;
};

vga_t* create_screen(screenMode_t mode); //,uint8_t vga_data_array[],uint32_t txcount,font_t * font);
void change_mode(uint8_t mode);
//vga_t* create_vga_instance() ;
//void vga_setup_mode(vga_t* instance, screenMode_t mode) ;
void put_cursor(uint8_t c);
vga_t* get_vga();


#endif