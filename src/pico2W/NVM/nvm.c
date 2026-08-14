#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

// No Pico 2 (4MB Flash), reservamos o último setor (4KB)
#define FLASH_TARGET_OFFSET (4 * 1024 * 1024 - FLASH_SECTOR_SIZE)

// Ponteiro para LEITURA direta via XIP
const uint8_t *nvm_read_ptr = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

void nvm_write(const uint8_t *data, size_t size) {
    // Buffer precisa ter o tamanho de pelo menos 1 página (256 bytes)
    uint8_t page_buf[FLASH_PAGE_SIZE];
    memset(page_buf, 0xFF, FLASH_PAGE_SIZE);
    memcpy(page_buf, data, (size < FLASH_PAGE_SIZE) ? size : FLASH_PAGE_SIZE);

    // 1. Trava interrupções
    uint32_t ints = save_and_disable_interrupts();

    //Obrigatorio parar o core 1 antes de apagar
    multicore_lockout_start();
    // 2. Apaga o setor de 4KB
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);

    // 3. Grava a página de 256 bytes
    flash_range_program(FLASH_TARGET_OFFSET, page_buf, FLASH_PAGE_SIZE);

    // Libera o Core 1 para voltar a rodar
    multicore_lockout_end();

    // 4. Restaura interrupções
    restore_interrupts(ints);
}

int main() {
    stdio_init_all();

    uint8_t minhavariavel[10] = "ORION68K";

    // Grava na NVM
    nvm_write(minhavariavel, sizeof(minhavariavel));

    // Leitura direta do endereço de memória
    printf("Dado salvo na Flash: %s\n", nvm_read_ptr);

    while(1);
}
