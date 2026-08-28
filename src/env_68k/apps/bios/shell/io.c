#include "stdio.h"
#include "drv_uart.h"
#include <string.h>

uint8_t get_key()
{
    uint8_t key = 0xFF;
    uint16_t key1 = 0xFF;

    while (key == 0xFF)
    {
        key = uart0_read();
        //key1 = get_char(); //Teclado USB do orion68
    }

    return key;
}

void _putchar_hex(char c)
{
    printf("%02X", (int)c);
}
