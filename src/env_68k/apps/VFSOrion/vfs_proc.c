#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vfs.h"

// ============================================
// ESTRUTURA DE DADOS DO /proc
// ============================================
typedef struct {
    char *data;
    size_t size;
    size_t position;
} ProcData;

// ============================================
// FUNÇÕES AUXILIARES
// ============================================
int get_total_memory(void) { return 1024; }
int get_free_memory(void)  { return 512; }
const char *get_cpu_name(void) { return "Motorola 68000"; }
int get_cpu_clock(void) { return 8; }

// ============================================
// PROC_READ
// ============================================
int proc_read(File *file, void *buffer, size_t size) {
    ProcData *data = (ProcData*)file->private_data;
    if (!data) return -1;
    
    size_t available = data->size - data->position;
    size_t to_read = (size < available) ? size : available;
    
    if (to_read == 0) return 0;
    
    memcpy(buffer, data->data + data->position, to_read);
    data->position += to_read;
    file->position = data->position;
    
    return (int)to_read;
}

// ============================================
// PROC_CLOSE
// ============================================
int proc_close(File *file) {
    ProcData *data = (ProcData*)file->private_data;
    if (data) {
        free(data->data);
        free(data);
        file->private_data = NULL;
    }
    return 0;
}

// ============================================
// PROC_LSEEK
// ============================================
size_t proc_lseek(File *file, size_t offset, int whence) {
    ProcData *data = (ProcData*)file->private_data;
    if (!data) return (size_t)-1;
    
    size_t new_pos;
    switch (whence) {
        case 0: new_pos = offset; break;
        case 1: new_pos = data->position + offset; break;
        case 2: new_pos = data->size + offset; break;
        default: return (size_t)-1;
    }
    
    if (new_pos > data->size) new_pos = data->size;
    data->position = new_pos;
    file->position = new_pos;
    return new_pos;
}

// ============================================
// PROC_MEMINFO_OPEN
// ============================================
int proc_meminfo_open(File *file, const char *path, int flags) {
    ProcData *data = (ProcData*)malloc(sizeof(ProcData));
    if (!data) return -1;
    
    char buffer[256];
    sprintf(buffer,
        "MemTotal: %d KB\n"
        "MemFree:  %d KB\n"
        "MemUsed:  %d KB\n"
        "MemAvail: %d KB\n",
        get_total_memory(),
        get_free_memory(),
        get_total_memory() - get_free_memory(),
        get_free_memory() / 2
    );
    
    data->data = strdup(buffer);
    data->size = strlen(buffer);
    data->position = 0;
    
    file->private_data = data;
    file->position = 0;
    file->read = proc_read;
    file->write = NULL;
    file->close = proc_close;
    file->ioctl = NULL;
    file->lseek = proc_lseek;
    
    return 0;
}

// ============================================
// PROC_CPUINFO_OPEN
// ============================================
int proc_cpuinfo_open(File *file, const char *path, int flags) {
    ProcData *data = (ProcData*)malloc(sizeof(ProcData));
    if (!data) return -1;
    
    char buffer[256];
    sprintf(buffer,
        "CPU:       %s\n"
        "Clock:     %d MHz\n"
        "Features:  FPU (sim)\n"
        "Cores:     1\n"
        "Family:    m68k\n",
        get_cpu_name(),
        get_cpu_clock()
    );
    
    data->data = strdup(buffer);
    data->size = strlen(buffer);
    data->position = 0;
    
    file->private_data = data;
    file->position = 0;
    file->read = proc_read;
    file->write = NULL;
    file->close = proc_close;
    file->ioctl = NULL;
    file->lseek = proc_lseek;
    
    return 0;
}

// ============================================
// PROC_UPTIME_OPEN
// ============================================
int proc_uptime_open(File *file, const char *path, int flags) {
    ProcData *data = (ProcData*)malloc(sizeof(ProcData));
    if (!data) return -1;
    
    static int uptime = 0;
    uptime += 10;
    
    char buffer[64];
    sprintf(buffer, "%d seconds\n", uptime);
    
    data->data = strdup(buffer);
    data->size = strlen(buffer);
    data->position = 0;
    
    file->private_data = data;
    file->position = 0;
    file->read = proc_read;
    file->write = NULL;
    file->close = proc_close;
    file->ioctl = NULL;
    file->lseek = proc_lseek;
    
    return 0;
}