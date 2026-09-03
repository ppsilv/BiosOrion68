#include "slots.h"
#include <string.h>

/*Paulo*/

static uint8_t slot_in_use[SLOT_COUNT];

void Slots_Init(void)
{
    memset(slot_in_use, 0, sizeof(slot_in_use));
}

int Slots_Alloc(void)
{
    for (int i = 0; i < SLOT_COUNT; i++) {
        if (!slot_in_use[i]) {
            slot_in_use[i] = 1;
            return i;
        }
    }
    return -1;   /* nenhum slot livre */
}

void Slots_Free(int slot_index)
{
    if (slot_index >= 0 && slot_index < SLOT_COUNT)
        slot_in_use[slot_index] = 0;
}

uint32_t Slots_BaseAddr(int slot_index)
{
    return (uint32_t)(SLOT_BASE + (uint32_t)slot_index * SLOT_SIZE);
}
