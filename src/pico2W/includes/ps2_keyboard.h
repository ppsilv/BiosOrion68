
#ifndef P2_KEYBOARD_H_
#define P2_KEYBOARD_H_
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

#define PS2FREQ   133600.0f

#define PS2_DATA_PIN     16
#define PIRQ             19

// GPIO pins to VIA chip
//enum ps2_pins {PA0=0, PA1, PA2, PA3, PA4, PA5, PA6, PS2_DATA_PIN=15, PS2_CLK_PIN, PIRQ=27};

void initPS2(void);
void kbd_int_on();
void kbd_int_off();

#endif // P2_KEYBOARD_H_
