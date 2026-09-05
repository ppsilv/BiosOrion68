#include <stdio.h>

extern int *ptr_contador;
extern void dumphex(const char *label, const void *buf, size_t len);
void print(){
    printf("*ptr_contador[%04x]\n",*ptr_contador);
    dumphex("label",(void *) 0x92800, 0xff);
}
