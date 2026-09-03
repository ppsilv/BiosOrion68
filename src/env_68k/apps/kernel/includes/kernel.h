#ifndef __KERNEL_H__
#define __KERNEL_H__

#ifdef NULL
#undef NULL
#endif

#ifndef NULL
#define NULL    0x00
#endif

extern char *int_to_string(int value, char *str, int base);
extern void kprintf(const char *fmt, ...);
extern long strtol(const char *str, char **endptr, int base);


#endif