#include "vfs.h"

// ============================================
// DECLARAÇÕES EXTERNAS DOS DRIVERS
// ============================================
extern int tty_open(File *file, const char *path, int flags);
extern int tty_read(File *file, void *buffer, size_t size);
extern int tty_write(File *file, const void *buffer, size_t size);
extern int tty_close(File *file);
extern int tty_ioctl(File *file, int cmd, void *arg);
extern size_t tty_lseek(File *file, size_t offset, int whence);

extern int null_open(File *file, const char *path, int flags);
extern int null_read(File *file, void *buffer, size_t size);
extern int null_write(File *file, const void *buffer, size_t size);
extern int null_close(File *file);
extern size_t null_lseek(File *file, size_t offset, int whence);

extern int zero_open(File *file, const char *path, int flags);
extern int zero_read(File *file, void *buffer, size_t size);
extern int zero_write(File *file, const void *buffer, size_t size);
extern int zero_close(File *file);
extern size_t zero_lseek(File *file, size_t offset, int whence);

extern int proc_meminfo_open(File *file, const char *path, int flags);
extern int proc_cpuinfo_open(File *file, const char *path, int flags);
extern int proc_uptime_open(File *file, const char *path, int flags);
extern int proc_read(File *file, void *buffer, size_t size);
extern int proc_close(File *file);
extern size_t proc_lseek(File *file, size_t offset, int whence);

// ============================================
// TABELA DE DRIVERS
// ============================================
DeviceDriver drivers[] = {
    {"/dev/tty",    tty_open,           tty_read,    tty_write,    tty_close,    tty_ioctl,    tty_lseek},
    {"/dev/null",   null_open,          null_read,   null_write,   null_close,   NULL,         null_lseek},
    {"/dev/zero",   zero_open,          zero_read,   zero_write,   zero_close,   NULL,         zero_lseek},
    {"/proc/meminfo", proc_meminfo_open, proc_read,   NULL,         proc_close,   NULL,         proc_lseek},
    {"/proc/cpuinfo", proc_cpuinfo_open, proc_read,   NULL,         proc_close,   NULL,         proc_lseek},
    {"/proc/uptime",  proc_uptime_open,  proc_read,   NULL,         proc_close,   NULL,         proc_lseek},
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};