#ifndef __RELOC_H__
#define __RELOC_H__

// Tipos de seção ELF
#define SHT_NULL       0
#define SHT_PROGBITS   1
#define SHT_SYMTAB     2
#define SHT_STRTAB     3
#define SHT_RELA       4
#define SHT_HASH       5
#define SHT_DYNAMIC    6
#define SHT_NOTE       7
#define SHT_NOBITS     8
#define SHT_REL        9
#define SHT_SHLIB      10
#define SHT_DYNSYM     11

// Tipos de relocação 68k
#define R_68K_NONE      0
#define R_68K_32        1
#define R_68K_16        2
#define R_68K_8         3
#define R_68K_PC32      4
#define R_68K_PC16      5
#define R_68K_PC8       6
#define R_68K_GOT32     7
#define R_68K_GOT16     8
#define R_68K_GOT16O    11   // ← O que você precisa
#define R_68K_GOT32O    12
#define R_68K_PLT32     13
#define R_68K_PLT16     14
#define R_68K_PLT8      15
#define R_68K_PLT32O    16
#define R_68K_COPY      17
#define R_68K_GLOB_DAT  18
#define R_68K_JMP_SLOT  19
#define R_68K_RELATIVE  20
#endif
