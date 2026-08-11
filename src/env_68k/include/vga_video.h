#ifndef __PICOVGA_H__
#define __PICOVGA_H__
#include "color.h"
/*
#define PICO_VGA_BASE 0x00FF8000
#define WRITE_SCREEN   (*((volatile unsigned char *)(PICO_VGA_BASE + 0x01)))
#define REG_02         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x03)))
#define REG_03         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x05)))
#define REG_04         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x07)))
#define CONFIG_REG     (*((volatile unsigned char *)(PICO_VGA_BASE + 0x09)))
#define REG_06         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x0B)))
#define REG_07         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x0D)))
#define REG_08         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x0F)))
#define REG_09         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x11)))
#define REG_0A         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x13)))
#define REG_0B         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x15)))
#define REG_0C         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x17)))
#define REG_0D         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x19)))
#define REG_0E         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x1B)))
#define REG_0F         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x1D)))
#define REG_10         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x1F)))
#define REG_11         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x21)))
#define REG_12         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x23)))
#define REG_13         (*((volatile unsigned char *)(PICO_VGA_BASE + 0x25)))
#define SET_MODE       (*((volatile unsigned char *)(PICO_VGA_BASE + 0x27)))
#define SET_TXT_COLOR  (*((volatile unsigned char *)(PICO_VGA_BASE + 0x29)))
#define CHANGE_CUR_POS (*((volatile unsigned char *)(PICO_VGA_BASE + 0x2B)))
#define REG_X_HIGH     (*((volatile unsigned char *)(PICO_VGA_BASE + 0x2D)))
#define REG_X_LOW      (*((volatile unsigned char *)(PICO_VGA_BASE + 0x2F)))
#define REG_Y_HIGH     (*((volatile unsigned char *)(PICO_VGA_BASE + 0x31)))
#define REG_Y_LOW      (*((volatile unsigned char *)(PICO_VGA_BASE + 0x33)))
#define CHANGE_BUFFER  (*((volatile unsigned char *)(PICO_VGA_BASE + 0x35)))
#define SELECT_SCREEN  (*((volatile unsigned char *)(PICO_VGA_BASE + 0x37)))
#define SET_HORIZONTAL (*((volatile unsigned char *)(PICO_VGA_BASE + 0x39)))
#define SET_VERTICAL   (*((volatile unsigned char *)(PICO_VGA_BASE + 0x3B)))
#define RUN_CMD        (*((volatile unsigned char *)(PICO_VGA_BASE + 0x3D)))
#define CORINGA        (*((volatile unsigned char *)(PICO_VGA_BASE + 0x3F)))
*/
/*
ALL OF THIS REGISTER IS NOT USED NOW
#define CHANGE_BUFFER  (*((volatile unsigned char *)(PICO_VGA_BASE + 0x35)))
#define SELECT_SCREEN  (*((volatile unsigned char *)(PICO_VGA_BASE + 0x37)))
#define SET_HORIZONTAL (*((volatile unsigned char *)(PICO_VGA_BASE + 0x39)))
#define SET_VERTICAL   (*((volatile unsigned char *)(PICO_VGA_BASE + 0x3B)))
#define SET_MODE       (*((volatile unsigned char *)(PICO_VGA_BASE + 0x27)))
*/
/*
#define CMD_SYSTEM_ENABLE   0xA5
#define CMD_CLEAR_SCREEN    0xA4
#define CMD_SET_CUR_POS     0xA3
#define CMD_SET_TXT_COLOR   0xA2
#define CMD_GO_HOME         0xA1
*/

void run_cmd(uint8_t cmd);
void gotoxy(uint16_t col,uint16_t row);
void gohome();
void setcolor(uint8_t txtcolor,uint8_t bgcolor);
void clrscr();

#endif
