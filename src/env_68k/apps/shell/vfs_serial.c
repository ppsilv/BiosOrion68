#include <stdint.h>
#include "vfs.h"

// ============================================
// ENDEREÇOS DA SERIAL (68k: só D0-D7 ligado,
// cada registrador ocupa 2 bytes, só offset ímpar é válido)
// offset = (numero_do_registrador * 2) + 1
// ============================================
#define SERIAL_BASE     0xFF4000

#define REG_THR         (SERIAL_BASE + 0x01)  // reg 0, escrita: Transmit Holding Register
#define REG_RBR         (SERIAL_BASE + 0x01)  // reg 0, leitura: Receive Buffer Register (mesmo endereço do THR)
#define REG_LSR         (SERIAL_BASE + 0x0B)  // reg 5: Line Status Register

#define LSR_THRE        0x20  // bit "pronto pra transmitir" -- confirmado igual ao uart0_write()
#define LSR_DR          0x01  // bit "tem byte pra ler" -- CONFIRMAR se necessário

#define REG8(addr)  (*(volatile uint8_t *)(addr))

// ============================================
// ACESSO AO HARDWARE
// ============================================
static int serial_tx_ready(void) {
    return (REG8(REG_LSR) & LSR_THRE) != 0;
}

static int serial_rx_ready(void) {
    return (REG8(REG_LSR) & LSR_DR) != 0;
}

static void serial_putc(uint8_t c) {
    while (!serial_tx_ready()) {
        // busy-wait; igual ao uart0_write() que já funciona
    }
    REG8(REG_THR) = c;
}

static int serial_getc(void) {
    if (!serial_rx_ready()) return -1;
    return REG8(REG_RBR);
}

// ============================================
// SERIAL_READ
// ============================================
int serial_read(File *file, void *buffer, size_t size) {
    char *buf = (char *)buffer;
    size_t i = 0;

    while (i < size) {
        int c = serial_getc();
        if (c < 0) break;
        buf[i++] = (char)c;
    }

    return (int)i;
}

// ============================================
// SERIAL_WRITE
// ============================================
int serial_write(File *file, const void *buffer, size_t size) {
    const char *data = (const char *)buffer;

    for (size_t i = 0; i < size; i++) {
        serial_putc((uint8_t)data[i]);
    }

    return (int)size;
}

// ============================================
// SERIAL_CLOSE
// ============================================
int serial_close(File *file) {
    return 0;
}

// ============================================
// SERIAL_LSEEK
// ============================================
size_t serial_lseek(File *file, size_t offset, int whence) {
    return (size_t)-1;
}

// ============================================
// SERIAL_OPEN
// ============================================
int serial_open(File *file, const char *path, int flags) {
    file->private_data = NULL;
    file->position = 0;
    file->read  = serial_read;
    file->write = serial_write;
    file->close = serial_close;
    file->ioctl = NULL;
    file->lseek = serial_lseek;
    return 0;
}