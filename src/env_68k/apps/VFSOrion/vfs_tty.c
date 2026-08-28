#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vfs.h"

// ============================================
// FUNÇÕES DO SISTEMA
// ============================================
extern char getchar(void);
extern void putchar(char c);


// ============================================
// TTY_READ
// ============================================
int tty_read(File *file, void *buffer, size_t size) {
    char *buf = (char*)buffer;
    size_t i = 0;
    
    while (i < size) {
        char c = getchar();
        buf[i++] = c;
        putchar(c);
        
        if (c == '\r' || c == '\n') {
            putchar('\n');
            break;
        }
    }
    
    return (int)i;
}

// ============================================
// TTY_WRITE
// ============================================
int tty_write(File *file, const void *buffer, size_t size) {
    const char *data = (const char*)buffer;
    
    for (size_t i = 0; i < size; i++) {
        if (data[i] == '\n') {
            putchar('\r');
        }
        putchar(data[i]);
    }
    
    return (int)size;
}

// ============================================
// TTY_CLOSE
// ============================================
int tty_close(File *file) {
    return 0;
}

// ============================================
// TTY_IOCTL
// ============================================
int tty_ioctl(File *file, int cmd, void *arg) {
    switch (cmd) {
        case TIOC_CLEAR:
            printf("\033[2J\033[H");
            return 0;
        default:
            return -1;
    }
}

// ============================================
// TTY_LSEEK
// ============================================
size_t tty_lseek(File *file, size_t offset, int whence) {
    return (size_t)-1;
}

// ============================================
// TTY_OPEN
// ============================================
int tty_open(File *file, const char *path, int flags) {
    file->private_data = NULL;
    file->position = 0;
    file->read = tty_read;
    file->write = tty_write;
    file->close = tty_close;
    file->ioctl = tty_ioctl;
    file->lseek = tty_lseek;
    return 0;
}
