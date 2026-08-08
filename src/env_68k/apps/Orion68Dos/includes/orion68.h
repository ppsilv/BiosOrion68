#ifndef __ORION68_H__
#define __ORION68_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Ddraig hardware addresses
#define SRAM_START      0x00000000       // Start of Static RAM
#define SRAM_END        0x000FFFFF       // End of Static RAM
//#define HEAP_START      0x00020000       // Heap memory start
//#define HEAP_END        0x008FFFFF       // Heap memory end

//#define DUART_BASE      0x00F7F000       // Base address of the Dual Asynchronous Receiver Transmitter
//#define PIT_BASE        0x00F7F100       // Base address of the Peripheral Interface Timer
//#define PS2_BASE        0x00F7F200       // Base address of the VT82C42 Keyboard/Mouse controller
//#define IDE_BASE        0x00F7F300       // Base address of the IDE controller
//#define RTC_BASE        0x00F7F400       // Base address of the RTC72421 clock controller
//#define EXP1_BASE       0x00F7F500       // Base address of the Expansion slot ID
//#define EXP2_BASE       0x00F7F600       // Base address of the Expansion slot ID
//#define EXP3_BASE       0x00F7F700       // Base address of the Expansion slot ID
//#define EXP4_BASE       0x00F7F800       // Base address of the Expansion slot I

#define ROM_START       0x00000000       // Start of ROM
#define ROM_END         0x000FFFFF       // End of ROM

#define STACK_SIZE      0x8000           // 32K for the stack

#define ISR_VECT_EXP1(vec)      (*((long *)0x64) = (long)vec)
#define ISR_VECT_IDE(vec)       (*((long *)0x68) = (long)vec)
#define ISR_VECT_PIT(vec)       (*((long *)0x6C) = (long)vec)
#define ISR_VECT_DUART(vec)     (*((long *)0x70) = (long)vec)
#define ISR_VECT_PS2(vec)       (*((long *)0x74) = (long)vec)
#define ISR_VECT_EXP2(vec)      (*((long *)0x78) = (long)vec)
#define ISR_VECT_EXP3(vec)      (*((long *)0x7C) = (long)vec)

#define EXPID_IDENTREG          0xFE
#define EXPID_REG(id, reg)      (*((volatile uint8_t *) (id + reg)))

#define DISABLE_INTERRUPTS()    __asm__ volatile("move.w %%sr, %%d0; ori.w #0x0700, %%d0; move.w %%d0, %%sr" : : : "d0");
#define ENABLE_INTERRUPTS()     __asm__ volatile("move.w %%sr, %%d0; andi.w #0xF8FF, %%d0; move.w %%d0, %%sr" : : : "d0");


#include "timers.h"

extern void set_ide_bus_mode(char mode);
extern void do_ideinit(int argc, char *argv[]);
extern int rtc_detectar(void);
extern bool init_keyboard();
extern uint8_t keyboard_get_key();
extern void crc32_init(void);
extern uint32_t crc32_calculate(const uint8_t *buffer, size_t length);
extern uint32_t carregar_elf32(const uint8_t *elf_buf, uint8_t *ram_destino);
extern uint32_t carregar_elf32_fatfs(const char *caminho);
extern unsigned int get_char();
extern void duart_opr_init(); 
extern void duart_uartA_init(void);
extern void duart_uartB_init(void);
extern void duart_led_op3_on(void);
extern void duart_led_op3_off(void);


//IDE
//#define ATA_REG_BASE 0x00FF4400
//UARTS
//#define DRV_UART0_BASE 0xFF4000
//#define DRV_UART1_BASE 0xFF4100
//#define DRV_UART2_BASE 0xFF4200
//#define DRV_UART3_BASE 0xFF4300
//PicoVGA
//FF8000
//!Y1 = !AS * !IOWR * !IOCS * !IOCS1 * A15 * !A14 * !A13 * !A12 * !A11 * !A10 * !A09 * !A08
//Keyboard PS2
#define PS2_BASE 0xFF9000
//!CSKBD = !AS * !IOCS * !IOCS1 * A15 * !A14 * !A13 * A12 * !A11 * !A10 * !A09 * !A08
//Super MultiIO
//FF9100
//!CSMIO = !AS * !IOCS * !IOCS1 * A15 * !A14 * !A13 * A12 * !A11 * !A10 * !A09 *  A08
//RTC RP5C01A da Ricoh
//FF9200
//!CSRTC = !AS * !IOCS * !IOCS1 * A15 * !A14 * !A13 * A12 * !A11 * !A10 *  A09 * !A08








#endif
