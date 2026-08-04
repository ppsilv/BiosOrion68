#ifndef __SCREEN_MODE__
#define __SCREEN_MODE__

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

#endif