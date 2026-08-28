#ifndef __DRVKBD_H__
#define __DRVKBD_H__

void init_kbd();
void enable_kbd_interrupts();

unsigned int get_char();
void print_capslock();
void ch9350_shut_up();

#endif
