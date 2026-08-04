#include "hardware/pio.h"
#include "vga_bus_read.pio.h"
#include "vga_bus_read.h"

/*
bit 0–7   → D0–D7   (data)
bit 8–12  → A1–A5   (addr = 5 bits)
bit 13    → CS
*/


static uint8_t bus_decode(uint8_t * reg, uint32_t v)
{
    uint8_t data =  v & 0xFF;  // D0–D7
    *reg = (v >> 8) & 0x1F;    // A1–A5 (5 bits)

    return data;
}
/*
uint8_t bus_wait_event(PIO pio, uint sm)
{
    return bus_decode( pio_sm_get_blocking(pio, sm) );
}
*/
bool bus_try_get_event(uint8_t *value,uint8_t *reg,PIO pio, uint sm)
{
    uint8_t res;

    if (!pio_sm_is_rx_fifo_empty(pio, sm)) {
        res = bus_decode( reg, pio_sm_get(pio, sm) );
        *value = res;
        return true;
    }
    return false;
}

bool bus_try_get_event32(uint32_t *value,PIO pio, uint sm)
{
    uint32_t valor;
    if (!pio_sm_is_rx_fifo_empty(pio, sm)) {
        valor = pio_sm_get(pio, sm) ;
        *value = valor;
        return true;
    }
    return false;
}