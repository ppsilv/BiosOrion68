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

uint16_t ring_buf_get_char(){
	uint16_t ch=0;
	if( ! ring_buf_is_empty() ){
		return 0x8000;
	}
	ch |= ring_buf_get();    //le do teclado ps2 
}

#ifdef BUCETON
void ring_buf_init(void)
{
	rb->buffer = buffer;
	rb->head   = 0;
	rb->tail   = 0;
	rb->count  = 0;
}

uint8_t ring_buf_get(void)
{
	uint8_t ret = rb->buffer[rb->tail];
	rb->tail++;              // uint8_t: wraps 255->0 automatically, no % needed
	rb->count--;
	return ret;
}

void ring_buf_put(const uint8_t c)
{
	rb->buffer[rb->head] = c;
	rb->head++;               // wraps automatically
	if (rb->count == RING_BUFFER_SIZE)
		rb->tail++;            // buffer full: overwrite oldest, advance tail too
	else
		rb->count++;
}

uint8_t ring_buf_is_empty(void)
{
	return rb->count == 0;
}

uint8_t ring_buf_is_full(void)
{
	return rb->count == RING_BUFFER_SIZE;
}
#endif