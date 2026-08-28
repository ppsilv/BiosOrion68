#ifndef __VBUG21_H__
#define __VBUG21_H__

#include "mc68000.h"

#define flg_program_loaded 0        //bit zero of flg_system
#define flg_sdcard_found   1        //bit one of flg_system
#define flg_printer_found  2        //bit two of flg_system
#define flg_reserved       3        //bit three of flg_system


void enable_interrupts();
void set_flg_program_loaded();
void clr_flg_program_loaded();

#endif


// grep 'RHR' * ./uarts/*
// grep 'THR' * ./uarts/*
// grep 'IER' * ./uarts/*
// grep 'ISR' * ./uarts/*
// grep 'FCR' * ./uarts/*
// grep 'LCR' * ./uarts/*
// grep 'MCR' * ./uarts/*
// grep 'LSR' * ./uarts/*
// grep 'MSR' * ./uarts/*
// grep 'SPR' * ./uarts/*
// grep 'DLL' * ./uarts/*
// grep 'DLM' * ./uarts/*
