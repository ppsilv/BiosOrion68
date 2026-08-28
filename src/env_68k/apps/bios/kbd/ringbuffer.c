#include <stdio.h>
#include "ringbuffer.h"

static uint8_t     buffer[RING_BUFFER_SIZE];
static ringbuffer_t rb_static;
static ringbuffer_t *rb = &rb_static;   // point at init time, not run time — no ordering bug possible


void ring_buf_init(void)
{
	rb->buffer = buffer;
	rb->head = 0;
	rb->tail = 0;
}

uint8_t ring_buf_get(void)
{
	uint8_t ret = rb->buffer[rb->tail];

	rb->tail++;   // só get() mexe em tail
	return ret;
}

void ring_buf_put(const uint8_t c)
{
	uint8_t next_head = rb->head + 1;   // wrap automático (uint8_t)
	if (next_head == rb->tail) {
		return;                          // buffer cheio: descarta o byte novo, não sobrescreve
	}
	rb->buffer[rb->head] = c;
	rb->head = next_head;               // só put()/ISR mexe em head
}

uint8_t ring_buf_is_empty(void)
{
	return rb->head == rb->tail;
}

uint8_t ring_buf_is_full(void)
{
	uint8_t next_head = rb->head + 1;
	return next_head == rb->tail;
}

uint8_t ring_buf_get_char(){
	uint16_t ch=0;
	while( ring_buf_is_empty() );
	ch = ring_buf_get(); 
	return ch;
}
/*
uint8_t     ringbufferUSB[RING_BUFFER_SIZE];
static ringbuffer_t rb_static1;
static ringbuffer_t *rb1 = &rb_static1;   // point at init time, not run time — no ordering bug possible

void ring_buf_init1(void)
{
	rb1->buffer = ringbufferUSB;
	rb1->head = 0;
	rb1->tail = 0;
}
uint8_t ring_buf_get1(void)
{
	uint8_t ret = rb1->buffer[rb1->tail];

	rb1->tail++;   // só get() mexe em tail
	return ret;
}

void ring_buf_put1(const uint8_t c)
{
	printf("%02x",c);
	uint8_t next_head = rb1->head + 1;   // wrap automático (uint8_t)
	if (next_head == rb1->tail) {
		return;                          // buffer cheio: descarta o byte novo, não sobrescreve
	}
	rb1->buffer[rb1->head] = c;
	rb1->head = next_head;               // só put()/ISR mexe em head
}
uint8_t ring_buf_is_empty1(void)
{
	return rb1->head == rb1->tail;
}

uint8_t ring_buf_is_full1(void)
{
	uint8_t next_head = rb1->head + 1;
	return next_head == rb1->tail;
}
*/