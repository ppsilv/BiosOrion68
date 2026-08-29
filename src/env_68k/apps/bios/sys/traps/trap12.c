#include <stdint.h>
#include <fatfs/ff.h>

#define NULL    0

/* =========================================================================
 * ARQUIVOS
 * ========================================================================= */

void do_open(int argc, char *argv[]) {
    f_open((FIL *)argv[0], (const char *)argv[1], (BYTE)(uint32_t)argv[2]);
}

void do_close(int argc, char *argv[]) {
    f_close((FIL *)argv[0]);
}

void do_read(int argc, char *argv[]) {
    f_read((FIL *)argv[0], (void *)argv[1], (UINT)(uint32_t)argv[2], (UINT *)argv[3]);
}

void do_write(int argc, char *argv[]) {
    f_write((FIL *)argv[0], (const void *)argv[1], (UINT)(uint32_t)argv[2], (UINT *)argv[3]);
}

void do_sync(int argc, char *argv[]) {
    f_sync((FIL *)argv[0]);
}

void do_lseek(int argc, char *argv[]) {
    f_lseek((FIL *)argv[0], (FSIZE_t)(uint32_t)argv[1]);
}

FSIZE_t do_tell(int argc, char *argv[]) {
    return f_tell((FIL *)argv[0]);
}

FSIZE_t do_size(int argc, char *argv[]) {
    return f_size((FIL *)argv[0]);
}

int do_eof(int argc, char *argv[]) {
    return f_eof((FIL *)argv[0]);
}

void do_unlink(int argc, char *argv[]) {
    f_unlink((const char *)argv[0]);
}

void do_rename(int argc, char *argv[]) {
    f_rename((const char *)argv[0], (const char *)argv[1]);
}

void do_stat(int argc, char *argv[]) {
    f_stat((const char *)argv[0], (FILINFO *)argv[1]);
}

void do_findfirst(int argc, char *argv[]) {
    f_findfirst ((DIR*)argv[0],(FILINFO*)argv[1],(const char *)argv[2],(const char *)argv[3]);
}

void do_findnext(int argc, char *argv[]) {
    f_findnext ((DIR*)argv[0],(FILINFO*)argv[1]);
}
/* =========================================================================
 * DIRETÓRIOS E SISTEMA DE ARQUIVOS
 * ========================================================================= */

void do_opendir(int argc, char *argv[]) {
    f_opendir((DIR *)argv[0], (const char *)argv[1]);
}

void do_readdir(int argc, char *argv[]) {
    f_readdir((DIR *)argv[0], (FILINFO *)argv[1]);
}

void do_closedir(int argc, char *argv[]) {
    f_closedir((DIR *)argv[0]);
}

void do_mkdir(int argc, char *argv[]) {
    f_mkdir((const char *)argv[0]);
}

void do_mount(int argc, char *argv[]) {
    f_mount((FATFS *)argv[0], (const char *)argv[1], (BYTE)(uint32_t)argv[2]);
}

void do_unmount(int argc, char *argv[]) {
    /* argv[0] = path ("0:"), argv[1] = opt */
    f_mount(NULL, (const char *)argv[0], (BYTE)(uint32_t)argv[1]);
}

