#include <stdint.h>
#include <trap12.h>
#include "fatfs/ff.h"

/*
 * IMPLEMENTAR
    truncate
    findfirst
    findnext
    chmod
    utime
    chdir
    chdrive
    getcwd
    getfree
    getlabel
    setlabel
    forward
    expand
    mkfs
    fdisk
    setcp
 */

/* =========================================================================
 * 1. Operações Básicas de Arquivo
 * ========================================================================= */

FRESULT fopen_old(FIL* fp, const TCHAR* path, BYTE mode) {
    // Salva todos os registradores

    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FOPEN;
    register void*    arg_a0 __asm__("a0") = (void*)fp;
    register void*    arg_a1 __asm__("a1") = (void*)path;
    register uint32_t arg_d0 __asm__("d0") = (uint32_t)mode;

    __asm__ volatile ( "movem.l %d1-%d7/%a0-%a6,-(%sp)"  );

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1), "r"(arg_d0) : "memory");

        // Restaura todos os registradores
    __asm__ volatile ("movem.l (%sp)+,%d1-%d7/%a0-%a6");

    return (FRESULT)res;
}

FRESULT fopen(FIL* fp, const TCHAR* path, BYTE mode) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FOPEN;
    register void*    arg_a0 __asm__("a0") = (void*)fp;
    register void*    arg_a1 __asm__("a1") = (void*)path;
    register uint32_t arg_d0 __asm__("d0") = (uint32_t)mode;

    __asm__ volatile (
        "trap #12\n\t" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1), "r"(arg_d0) : "memory");

    return (FRESULT)res;
}

FRESULT fclose(FIL* fp) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FCLOSE;
    register void*    arg_a0 __asm__("a0") = (void*)fp;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (FRESULT)res;
}

FRESULT fread(FIL* fp, void* buff, UINT btr, UINT* br) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FREAD;
    register void*    arg_a0 __asm__("a0") = (void*)fp;
    register void*    arg_a1 __asm__("a1") = buff;
    register uint32_t arg_d0 __asm__("d0") = (uint32_t)btr;
    register void*    arg_a2 __asm__("a2") = (void*)br;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1), "r"(arg_d0), "r"(arg_a2) : "memory");
    return (FRESULT)res;
}

FRESULT fwrite(FIL* fp, const void* buff, UINT btw, UINT* bw) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FWRITE;
    register void*    arg_a0 __asm__("a0") = (void*)fp;
    register void*    arg_a1 __asm__("a1") = (void*)buff;
    register uint32_t arg_d0 __asm__("d0") = (uint32_t)btw;
    register void*    arg_a2 __asm__("a2") = (void*)bw;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1), "r"(arg_d0), "r"(arg_a2) : "memory");
    return (FRESULT)res;
}

FRESULT fsync(FIL* fp) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FSYNC;
    register void*    arg_a0 __asm__("a0") = (void*)fp;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (FRESULT)res;
}

FRESULT funlink(const TCHAR* path) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FUNLINK;
    register void*    arg_a0 __asm__("a0") = (void*)path;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (FRESULT)res;
}

FRESULT flseek(FIL* fp, FSIZE_t offset) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FLSEEK;
    register void*    arg_a0 __asm__("a0") = (void*)fp;
    register uint32_t arg_d0 __asm__("d0") = (uint32_t)offset;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_d0) : "memory");
    return (FRESULT)res;
}

/* =========================================================================
 * 2. Posição, Tamanho e Status
 * ========================================================================= */

FSIZE_t ftell(FIL* fp) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FTELL;
    register void*    arg_a0 __asm__("a0") = (void*)fp;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (FSIZE_t)res;
}

FSIZE_t fsize(FIL* fp) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FSIZE;
    register void*    arg_a0 __asm__("a0") = (void*)fp;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (FSIZE_t)res;
}

int feof(FIL* fp) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FEOF;
    register void*    arg_a0 __asm__("a0") = (void*)fp;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (int)res;
}

/* =========================================================================
 * 3. Renomear e Metadados
 * ========================================================================= */

FRESULT frename(const TCHAR* path_old, const TCHAR* path_new) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FRENAME;
    register void*    arg_a0 __asm__("a0") = (void*)path_old;
    register void*    arg_a1 __asm__("a1") = (void*)path_new;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1) : "memory");
    return (FRESULT)res;
}

FRESULT fstat(const TCHAR* path, FILINFO* fno) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FSTAT;
    register void*    arg_a0 __asm__("a0") = (void*)path;
    register void*    arg_a1 __asm__("a1") = (void*)fno;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1) : "memory");
    return (FRESULT)res;
}

/* =========================================================================
 * 4. Diretórios
 * ========================================================================= */

FRESULT fopendir(DIR* dp, const TCHAR* path) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FOPENDIR;
    register void*    arg_a0 __asm__("a0") = (void*)dp;
    register void*    arg_a1 __asm__("a1") = (void*)path;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1) : "memory");
    return (FRESULT)res;
}

FRESULT freaddir(DIR* dp, FILINFO* fno) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FREADDIR;
    register void*    arg_a0 __asm__("a0") = (void*)dp;
    register void*    arg_a1 __asm__("a1") = (void*)fno;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1) : "memory");
    return (FRESULT)res;
}

FRESULT fclosedir(DIR* dp) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FCLOSEDIR;
    register void*    arg_a0 __asm__("a0") = (void*)dp;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (FRESULT)res;
}

FRESULT fmkdir(const TCHAR* path) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FMKDIR;
    register void*    arg_a0 __asm__("a0") = (void*)path;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (FRESULT)res;
}

/* =========================================================================
 * 5. Montagem e Desmontagem
 * ========================================================================= */

FRESULT fmount(FATFS* fs, const TCHAR* path, BYTE opt) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FMOUNT;
    register void*    arg_a0 __asm__("a0") = (void*)fs;
    register void*    arg_a1 __asm__("a1") = (void*)path;
    register uint32_t arg_d0 __asm__("d0") = (uint32_t)opt;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1), "r"(arg_d0) : "memory");
    return (FRESULT)res;
}

FRESULT funmount(const TCHAR* path) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FUNMOUNT;
    register void*    arg_a0 __asm__("a0") = (void*)path;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0) : "memory");
    return (FRESULT)res;
}

/* =========================================================================
 * 6. Procura de arquivos
 * ========================================================================= */

FRESULT findfirst(DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FFINDFIRST;
    register void*    arg_a0 __asm__("a0") = (void*)dp;
    register void*    arg_a1 __asm__("a1") = (void*)fno;
    register uint8_t* arg_d0 __asm__("d0") = (uint8_t*)path;
    register uint8_t* arg_d2 __asm__("d2") = (uint8_t*)pattern;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1), "r"(arg_d0), "r"(arg_d2) : "memory");
    return (FRESULT)res;
}

FRESULT findnext(DIR* dp, FILINFO* fno) {
    register uint32_t res    __asm__("d0");
    register uint32_t cmd    __asm__("d1") = SYS_FFINDNEXT;
    register void*    arg_a0 __asm__("a0") = (void*)dp;
    register void*    arg_a1 __asm__("a1") = (void*)fno;

    __asm__ volatile ("trap #12" : "=r"(res) : "r"(cmd), "r"(arg_a0), "r"(arg_a1) : "memory");
    return (FRESULT)res;
}
