#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vfs.h"

static void init_hardware(){

}
static void  mount_fatfs(){

}

void kernel_main(void) {
    // 1. Inicializa hardware
    init_hardware();

    // 2. Inicializa VFS
    vfs_init();

    // 3. Monta filesystem (FATFS)
    mount_fatfs();


    // 1. Abre o arquivo binário
    int fd = vfs_open("/bin/shell.bin", O_RDONLY);
    if (fd < 0) {
        // Erro: shell não encontrado
        while(1);
    }

    // 2. Carrega para um endereço fixo (ex: 0x10000)
    uint32_t load_addr = 0x10000;
    vfs_read(fd, (void*)load_addr, 1024*64);  // Lê até 64KB

    vfs_close(fd);

    // 3. Salta para o código (NUNCA RETORNA!)
    void (*program)(void) = (void (*)(void))load_addr;
    program();
}
// loader.h - Para binário puro
#define PROGRAM_LOAD_ADDR  0x10000
#define PROGRAM_MAX_SIZE   (64 * 1024)  // 64KB

int load_binary(const char *path, uint32_t addr, uint32_t max_size) {
    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0) return -1;

    int bytes = vfs_read(fd, (void*)addr, max_size);
    vfs_close(fd);

    return bytes;
}

void jump_to_binary(uint32_t addr) {
    void (*program)(void) = (void (*)(void))addr;
    program();  // NUNCA RETORNA!
}
