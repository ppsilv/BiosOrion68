#ifndef __PICO_REGISTERS_H__
#define __PICO_REGISTERS_H__

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