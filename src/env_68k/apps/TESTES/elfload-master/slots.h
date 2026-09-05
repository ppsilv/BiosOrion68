#ifndef SLOTS_H
#define SLOTS_H

#include <stdint.h>


/*
 * 10 slots de 256K a partir de 0x00092000.
 *
 * CONFIRME antes de usar: isso precisa bater com o resto do seu mapa
 * de memoria (kernel.bin em 0x82000, a regiao antiga USERRAM de
 * 512K do linker.ld original, etc -- essas faixas NAO podem se
 * sobrepor). Ajuste SLOT_BASE/SLOT_SIZE/SLOT_COUNT conforme o que
 * sobrar de RAM livre na sua placa.
 */
#define SLOT_BASE   0x00092000UL
#define SLOT_SIZE   0x00040000UL   /* 256K por slot */
#define SLOT_COUNT  10

void     Slots_Init(void);
int      Slots_Alloc(void);          /* devolve indice 0..SLOT_COUNT-1, ou -1 se nao tiver slot livre */
void     Slots_Free(int slot_index);
uint32_t Slots_BaseAddr(int slot_index);

#endif
