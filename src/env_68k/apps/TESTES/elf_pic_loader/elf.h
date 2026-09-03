#ifndef __ELF_H__
#define __ELF_H__

#include <stdint.h>

/* valores de e_ident[EI_CLASS] / e_ident[EI_DATA] */
#define ID_32BIT       1
#define ID_64BIT       2
#define ID_LITTLE_ENDIAN 1
#define ID_BIG_ENDIAN  2

/* e_type */
#define ET_NONE  0
#define ET_REL   1
#define ET_EXEC  0x0002
#define ET_DYN   3
#define ET_CORE  4

/* e_machine */
#define EM_68K   0x0004

/* p_type (program header) */
#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6

/*
 * Layout achatado (sem o array e_ident[16] aninhado do padrao ELF) --
 * mantido assim de proposito, igual ao seu load_elf_executable
 * original, pra nao precisar mudar nenhum acesso tipo
 * header.ident_magic[0] no resto do codigo.
 *
 * Compilado pra rodar EM m68k (big-endian nativo) -- os campos batem
 * byte a byte com o arquivo .elf sem precisar de byte-swap manual,
 * desde que o compilador que gera este .o tambem seja m68k-elf-gcc.
 */
#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long

typedef struct {
    uint8_t  ident_magic[4];     /* 0x7F 'E' 'L' 'F' */
    uint8_t  ident_class;        /* ID_32BIT */
    uint8_t  ident_data;         /* ID_BIG_ENDIAN */
    uint8_t  ident_version;      /* sempre 1 */
    uint8_t  ident_osabi;
    uint8_t  ident_abiversion;
    uint8_t  ident_pad[7];       /* preenchimento -- e_ident tem 16 bytes no total */

    uint16_t type;               /* ET_EXEC, etc */
    uint16_t machine;            /* EM_68K */
    uint32_t version;
    uint32_t entry;              /* endereco (relativo, se PIC) do ponto de entrada */
    uint32_t phoff;              /* offset no arquivo pra tabela de program headers */
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;          /* tamanho de CADA entrada de program header */
    uint16_t phnum;              /* quantidade de program headers */
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf32_header;

typedef struct {
    uint32_t type;                /* PT_LOAD, PT_DYNAMIC, etc */
    uint32_t offset;              /* offset no arquivo onde o segmento comeca */
    uint32_t vaddr;               /* endereco virtual (nao usado pelo loader hoje -- usamos paddr) */
    uint32_t paddr;               /* endereco fisico/de carga -- e' o que o loader soma a task_base */
    uint32_t filesz;              /* tamanho do segmento NO ARQUIVO */
    uint32_t memsz;               /* tamanho do segmento NA MEMORIA (pode ser maior por causa do bss) */
    uint32_t flags;               /* R/W/X -- nao verificado pelo loader hoje */
    uint32_t align;
} elf32_program_header;

#endif
