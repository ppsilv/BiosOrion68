#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vfs.h"



// ============================================
// ZERO_READ
// ============================================
int zero_read(File *file, void *buffer, size_t size) {
    memset(buffer, 0, size);
    file->position += size;
    return (int)size;
}

// ============================================
// ZERO_WRITE
// ============================================
int zero_write(File *file, const void *buffer, size_t size) {
    return (int)size;
}

// ============================================
// ZERO_CLOSE
// ============================================
int zero_close(File *file) {
    return 0;
}

// ============================================
// ZERO_LSEEK
// ============================================
size_t zero_lseek(File *file, size_t offset, int whence) {
    switch (whence) {
        case 0: file->position = offset; break;
        case 1: file->position += offset; break;
        case 2: file->position = offset; break;
        default: return (size_t)-1;
    }
    return file->position;
}

// ============================================
// ZERO_OPEN
// ============================================
int zero_open(File *file, const char *path, int flags) {
    file->private_data = NULL;
    file->position = 0;
    file->read = zero_read;
    file->write = zero_write;
    file->close = zero_close;
    file->ioctl = NULL;
    file->lseek = zero_lseek;
    return 0;
}
