#ifndef __VGA_BUS_READ_H__
#define __VGA_BUS_READ_H__

//Register address
#define WRITE_SCREEN   0xB8001        //Endereço real  3 0x03  o pico enxerga 0x00
#define REG_02         0xB8003        //Endereço real  3 0x03  o pico enxerga 0x01
#define REG_03         0xB8005        //Endereço real  5 0x05  o pico enxerga 0x02
#define REG_04         0xB8007        //Endereço real  7 0x07  o pico enxerga 0x03
#define CONFIG_REG     0xB8009        //Endereço real  9 0x09  o pico enxerga 0x04
#define REG_06         0xB800b        //Endereço real 11 0x0b  o pico enxerga 0x05
#define REG_07         0xB800d        //Endereço real 13 0x0d  o pico enxerga 0x06
#define REG_08         0xB800f        //Endereço real 15 0x0f  o pico enxerga 0x07
#define REG_09         0xB8011        //Endereço real 17 0x11  o pico enxerga 0x08
#define REG_0A         0xB8013        //Endereço real 19 0x13  o pico enxerga 0x09
#define REG_0B         0xB8015        //Endereço real 21 0x15  o pico enxerga 0x0A
#define REG_0C         0xB8017        //Endereço real 23 0x17  o pico enxerga 0x0B
#define REG_0D         0xB8019        //Endereço real 25 0x19  o pico enxerga 0x0C
#define REG_0E         0xB801b        //Endereço real 27 0x1b  o pico enxerga 0x0D
#define REG_0F         0xB801d        //Endereço real 29 0x1d  o pico enxerga 0x0E
#define REG_10         0xB801f        //Endereço real 31 0x1f  o pico enxerga 0x0F
#define REG_11         0xB8021        //Endereço real 33 0x21  o pico enxerga 0x10
#define REG_12         0xB8023        //Endereço real 35 0x23  o pico enxerga 0x11
#define REG_13         0xB8025        //Endereço real 37 0x25  o pico enxerga 0x12
#define SET_MODE       0xB8027        //Endereço real 39 0x27  o pico enxerga 0x13 (0=Texto+Scroll, 1=Texto Fixo, 2=320x200, 3=640x200)
#define SET_TXT_COLOR  0xB8029        //Endereço real 41 0x29  o pico enxerga 0x14
#define CHANGE_CUR_POS 0xB802b        //Endereço real 43 0x2b  o pico enxerga 0x15
#define REG_X_HIGH     0xB802d        //Endereço real 45 0x2d  o pico enxerga 0x16
#define REG_X_LOW      0xB802f        //Endereço real 47 0x2f  o pico enxerga 0x17
#define REG_Y_HIGH     0xB8031        //Endereço real 49 0x31  o pico enxerga 0x18
#define REG_Y_LOW      0xB8033        //Endereço real 51 0x33  o pico enxerga 0x19
#define CHANGE_BUFFER  0xB8035        //Endereço real 53 0x35  o pico enxerga 0x1A
#define SELECT_SCREEN  0xB8037        //Endereço real 55 0x37  o pico enxerga 0x1B
#define SET_HORIZONTAL 0xB8039        //Endereço real 57 0x39  o pico enxerga 0x1C
#define SET_VERTICAL   0xB803b        //Endereço real 59 0x3b  o pico enxerga 0x1D
#define RUN_CMD        0xB803d        //Endereço real 61 0x3d  o pico enxerga 0x1E
#define CORINGA        0xB803f        //Endereço real 63 0x3f  o pico enxerga 0x1F

//Register macros
#define D_WRITE_SCREEN      0x00
#define D_REG_02            0x01
#define D_REG_03            0x02
#define D_REG_04            0x03
#define D_SYSTEM_RUN        0x04
#define D_REG_06            0x05
#define D_REG_07            0x06
#define D_REG_08            0x07
#define D_REG_09            0x08
#define D_REG_0A            0x09
#define D_REG_0B            0x0A
#define D_REG_0C            0x0B
#define D_REG_0D            0x0C
#define D_REG_0E            0x0D
#define D_REG_0F            0x0E
#define D_REG_10            0x0F
#define D_REG_11            0x10
#define D_REG_12            0x11
#define D_REG_13            0x12
#define D_SET_MODE          0x13
#define D_SET_TXT_COLOR     0x14
#define D_CHANGE_CUR_POS    0x15
#define D_REG_X_HIGH        0x16
#define D_REG_X_LOW         0x17
#define D_REG_Y_HIGH        0x18
#define D_REG_Y_LOW         0x19
#define D_CHANGE_BUFFER     0x1A
#define D_SELECT_SCREEN     0x1B
#define D_SET_HORIZONTAL    0x1C
#define D_SET_VERTICAL      0x1D
#define D_RUN_CMD           0x1E
#define D_CORINGA           0x1F


//Commands
#define CMD_SYSTEM_ENABLE   0xA5
#define CMD_CLEAR_SCREEN    0xA4
#define CMD_SET_CUR_POS     0xA3
#define CMD_SET_TXT_COLOR   0xA2
#define CMD_GO_HOME         0xA1





#endif