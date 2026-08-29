#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vfs.h"

// ============================================
// TABELA DE DESCRITORES GLOBAL
// ============================================
File *fd_table[256];
static int fd_count = 0;

extern DeviceDriver drivers[];

// ============================================
// FUNÇÕES DE ACESSO À TABELA DE DESCRITORES
// ============================================

void vfs_set_fd(int fd, File *file) {
    if (fd >= 0 && fd < 256) {
        fd_table[fd] = file;
    }
}

File *vfs_get_fd(int fd) {
    if (fd >= 0 && fd < 256) {
        return fd_table[fd];
    }
    return NULL;
}

void vfs_restore_fd(int fd) {
    if (fd >= 0 && fd < 256) {
        // Restaura para /dev/tty
        int tty_fd = vfs_open("/dev/tty", O_RDWR);
        if (tty_fd >= 0) {
            fd_table[fd] = fd_table[tty_fd];
            fd_table[tty_fd] = NULL;
        }
    }
}

// ============================================
// ALLOCATE_FD
// ============================================
int allocate_fd(File *file) {
    for (int i = 3; i < 256; i++) {
        if (fd_table[i] == NULL) {
            fd_table[i] = file;
            fd_count++;
            return i;
        }
    }
    return -1;
}

// ============================================
// GET_FILE_FROM_FD
// ============================================
File *get_file_from_fd(int fd) {
    if (fd < 0 || fd >= 256) return NULL;
    return fd_table[fd];
}

// ============================================
// FREE_FD
// ============================================
void free_fd(int fd) {
    if (fd >= 0 && fd < 256) {
        fd_table[fd] = NULL;
        fd_count--;
    }
}

// ============================================
// VFS_OPEN
// ============================================
// vfs.c - vfs_open()
extern int fat_open(File *file, const char *path, int flags);
int vfs_open(const char *path, int flags) {
    
    File *file = (File*)malloc(sizeof(File));
    if (!file) return -1;
    memset(file, 0, sizeof(File));
    file->name = strdup(path);
    
    if (fat_open(file, path, flags) == 0) {
        int fd = allocate_fd(file);
        if (fd >= 0) {
            return fd;
        }
        free(file->name);
        free(file);
        return -1;
    }
    
    for (int i = 0; drivers[i].path != NULL; i++) {
        //printf("vfs_open: comparando '%s' com '%s'\n", path, drivers[i].path);
        if (strcmp(path, drivers[i].path) == 0) {
           // printf("vfs_open: ENCONTROU! i=%d\n", i);
            if (drivers[i].open(file, path, flags) == 0) {
                int fd = allocate_fd(file);
                if (fd >= 0) {
                    //printf("vfs_open: fd=%d (SUCESSO!)\n", fd);
                    return fd;
                }
            }
        }
    }
    
    free(file->name);
    free(file);
    return -1;
}
int vfs_open1(const char *path, int flags) {
    File *file = (File*)malloc(sizeof(File));
    if (!file) return -1;
    
    memset(file, 0, sizeof(File));
    file->name = strdup(path);
    file->type = FILE_TYPE_REGULAR;
    file->position = 0;
    file->ref_count = 1;
    
    if (fat_open(file, path, flags) == 0) {
        int fd = allocate_fd(file);
        if (fd >= 0) return fd;
        free(file->name);
        free(file);
        return -1;
    }
    
    for (int i = 0; drivers[i].path != NULL; i++) {
        if (strcmp(path, drivers[i].path) == 0) {
            file->type = FILE_TYPE_DEVICE;
            file->read = drivers[i].read;
            file->write = drivers[i].write;
            file->close = drivers[i].close;
            file->ioctl = drivers[i].ioctl;
            file->lseek = drivers[i].lseek;
            
            if (drivers[i].open(file, path, flags) == 0) {
                int fd = allocate_fd(file);
                if (fd >= 0) {
                   //printf("drivers[i].path[%s] fd=\n",drivers[i].path,fd);
                    return fd;
                }
            }
            break;
        }
    }
    
    free(file->name);
    free(file);
    return -1;
}

// ============================================
// VFS_READ
// ============================================
int vfs_read(int fd, void *buffer, size_t size) {
    File *file = get_file_from_fd(fd);
    if (!file) return -1;
    if (file->read) return file->read(file, buffer, size);
    return -1;
}

// ============================================
// VFS_WRITE
// ============================================
int vfs_write(int fd, const void *buffer, size_t size) {
    File *file = get_file_from_fd(fd);
    if (!file) return -1;
    if (file->write) return file->write(file, buffer, size);
    return -1;
}

// ============================================
// VFS_CLOSE
// ============================================
int vfs_close(int fd) {
    File *file = get_file_from_fd(fd);
    if (!file) return -1;
    
    int result = 0;
    if (file->close) result = file->close(file);
    
    free(file->name);
    free(file);
    free_fd(fd);
    return result;
}

// ============================================
// VFS_IOCTL
// ============================================
int vfs_ioctl(int fd, int cmd, void *arg) {
    File *file = get_file_from_fd(fd);
    if (!file) return -1;
    if (file->ioctl) return file->ioctl(file, cmd, arg);
    return -1;
}

// ============================================
// VFS_LSEEK
// ============================================
size_t vfs_lseek(int fd, size_t offset, int whence) {
    File *file = get_file_from_fd(fd);
    if (!file) return (size_t)-1;
    
    if (file->lseek) return file->lseek(file, offset, whence);
    
    switch (whence) {
        case 0: file->position = offset; break;
        case 1: file->position += offset; break;
        case 2: return (size_t)-1;
        default: return (size_t)-1;
    }
    return file->position;
}

// ============================================
// VFS_INIT
// ============================================
void vfs_init(void) {
    memset(fd_table, 0, sizeof(fd_table));
    fd_count = 0;
    
    int fd0 = vfs_open("/dev/tty", O_RDWR);
    int fd1 = vfs_open("/dev/tty", O_RDWR);
    int fd2 = vfs_open("/dev/tty", O_RDWR);
    
    if (fd0 != 0) {
        fd_table[0] = fd_table[fd0];
        fd_table[fd0] = NULL;
        fd_count--;
    }
    if (fd1 != 1) {
        fd_table[1] = fd_table[fd1];
        fd_table[fd1] = NULL;
        fd_count--;
    }
    if (fd2 != 2) {
        fd_table[2] = fd_table[fd2];
        fd_table[fd2] = NULL;
        fd_count--;
    }
    
    printf("VFS inicializado: stdin=0, stdout=1, stderr=2\n");
}
