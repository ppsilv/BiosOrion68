#ifndef __SYSFLAGS_H__
#define __SYSFLAGS_H__


typedef union {
    // 1. Visão organizada (Bit-fields): Para usar no código em C
    struct {
        unsigned char pgm_loaded   : 1;  // O ": 1" significa: use apenas 1 bit!
        unsigned char xmodem_error : 1;
        unsigned char uart_ready   : 1;
        unsigned char file_open    : 1;
        unsigned char vga_dirty    : 1;
        unsigned char timer_expired: 1;
        unsigned char rsvd1        : 1;  // Reservado para alinhar
        unsigned char rsvd2        : 1;  // Reservado para fechar 8 bits (1 byte)
    } bit;

    // 2. Visão bruta (Byte): Para zerar tudo de uma vez ou passar para o Assembly
    unsigned char byte; 
} SystemFlags;

// Declaração da sua variável global de flags
extern SystemFlags sys_flags;

#endif