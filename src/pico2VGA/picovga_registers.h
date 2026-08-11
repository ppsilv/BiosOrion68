#ifndef __PICO_REGISTERS_H__
#define __PICO_REGISTERS_H__

//Register macros
#define D_WRITE_SCREEN          0x00
#define D_REG_05                0x01
#define D_REG_06                0x02
#define D_REG_X1_HIGH           0x03
#define D_REG_X1_LOW            0x04
#define D_REG_Y1_HIGH           0x05
#define D_REG_Y1_LOW            0x06
#define D_REG_X2_HIGH           0x07
#define D_REG_X2_LOW            0x08
#define D_REG_Y2_HIGH           0x09
#define D_REG_Y2_LOW            0x0A
#define D_REG_01                0x0B
#define D_REG_02                0x0C
#define D_REG_03                0x0D
#define D_REG_PRIMITIVE_COLOR   0x0E
#define D_REG_WIDTH_HIGH        0x0F
#define D_REG_WIDTH_LOW         0x10
#define D_REG_HEIGHT_HIGH       0x11
#define D_REG_HEIGHT_LOW        0x12
#define D_REG_RAIO              0x13  
#define D_SET_TXT_COLOR         0x14
#define D_CHANGE_CUR_POS        0x15
#define D_REG_X_HIGH            0x16
#define D_REG_X_LOW             0x17
#define D_REG_Y_HIGH            0x18
#define D_REG_Y_LOW             0x19
#define D_REG_CORNERNAME        0x1A    
#define D_REG_DELTA             0x1B    
#define D_SET_HORIZONTAL        0x1C    //livre  
#define D_SET_VERTICAL          0x1D    //livre
#define D_RUN_CMD               0x1E
#define D_CORINGA               0x1F    //livre









//Commands
#define CMD_SYSTEM_ENABLE   0xA5
#define CMD_CLEAR_SCREEN    0xA4
#define CMD_SET_CUR_POS     0xA3
#define CMD_SET_TXT_COLOR   0xA2
#define CMD_GO_HOME         0xA1
//Graphos commands
#define CMD_DRAW_PIXEL                  0xA0
#define CMD_DRAW_LINE                   0xBF
#define CMD_DRAW_VLINE                  0xBE
#define CMD_DRAW_HLINE                  0xBD
#define CMD_DRAW_CIRCLE                 0xBC
#define CMD_DRAW_CIRCLEHELPER           0xBB
#define CMD_DRAW_FILLCIRCLE             0xBA
#define CMD_DRAW_FILLCIRCLEHELPER       0xB9
#define CMD_DRAW_ROUNDRECT              0xB8
#define CMD_DRAW_FILLROUNDRECT          0xB7
#define CMD_DRAW_FILLRECT               0xB6
#define CMD_DRAW_FILLTRIANGLE           0xB5
#define CMD_DRAW_MULTILINE              0xB4

#endif
