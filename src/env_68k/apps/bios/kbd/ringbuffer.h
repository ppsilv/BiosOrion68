#ifndef DDRAIG_OS_RINGBUFFER_H
#define DDRAIG_OS_RINGBUFFER_H

#include "orion68.h"

#define RING_BUFFER_SIZE 256 // Deve ser 256 para o wrap-around automático do uint8_t

typedef struct ring_buffer{
	uint8_t             *buffer;
	volatile uint8_t     head;   // só a ISR (put) escreve
	volatile uint8_t     tail;   // só o loop principal (get) escreve
} ringbuffer_t;


void    ring_buf_init(void);
uint8_t ring_buf_get(void);
void    ring_buf_put(const uint8_t c);
uint8_t ring_buf_is_empty(void);
uint8_t ring_buf_is_full(void);

#endif