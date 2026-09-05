#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vfs.h"



// ============================================
// NULL_READ
// ============================================
int null_read(File *file, void *buffer, size_t size) {
    return 0;  // EOF
}

// ============================================
// NULL_WRITE
// ============================================
int null_write(File *file, const void *buffer, size_t size) {
    return (int)size;  // Descarta
}

// ============================================
// NULL_CLOSE
// ============================================
int null_close(File *file) {
    return 0;
}

// ============================================
// NULL_LSEEK
// ============================================
size_t null_lseek(File *file, size_t offset, int whence) {
    return offset;
}

// ============================================
// NULL_OPEN
// ============================================
int null_open(File *file, const char *path, int flags) {
    file->private_data = NULL;
    file->position = 0;
    file->read = null_read;
    file->write = null_write;
    file->close = null_close;
    file->ioctl = NULL;
    file->lseek = null_lseek;
    return 0;
}
