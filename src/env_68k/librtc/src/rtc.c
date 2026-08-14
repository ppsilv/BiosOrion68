#include <stdint.h>
#include <orion_rtc.h>

/* Serviço #1: Ler Hora */
void sys_rtc_read_time(rtc_time_t *t) {
    register rtc_time_t* ptr __asm__("d0") = t;
    register uint16_t svc __asm__("d1") = 1;

    __asm__ volatile (
        "trap #4"
        :
        : "d" (ptr), "d" (svc)
        : "memory"
    );
}

/* Serviço #2: Gravar Hora */
void sys_rtc_set_time(rtc_time_t *t) {
    register rtc_time_t* ptr __asm__("d0") = t;
    register uint16_t svc __asm__("d1") = 2;

    __asm__ volatile (
        "trap #4"
        :
        : "d" (ptr), "d" (svc)
        : "memory"
    );
}

/* Serviço #3: Ler EEPROM */
uint8_t sys_eeprom_read_byte(uint16_t addr) {
    register uint32_t reg_d0 __asm__("d0") = addr;
    register uint16_t svc __asm__("d1") = 3;

    __asm__ volatile (
        "trap #4"
        : "=d" (reg_d0)
        : "d" (reg_d0), "d" (svc)
        : "memory"
    );
    return (uint8_t)reg_d0;
}

/* Serviço #4: Escrever EEPROM */
void sys_eeprom_write_byte(uint16_t addr, uint8_t data) {
    register uint32_t reg_d0 __asm__("d0") = addr;
    register uint32_t reg_d2 __asm__("d2") = data;
    register uint32_t reg_d3 __asm__("d3") = data; /* Garante D2 conforme o handler */
    register uint16_t svc __asm__("d1") = 4;

    __asm__ volatile (
        "trap #4"
        :
        : "d" (reg_d0), "d" (reg_d2), "d" (reg_d3), "d" (svc)
        : "memory"
    );
}
