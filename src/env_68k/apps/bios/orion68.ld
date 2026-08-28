MEMORY {
    /* Região da ROM onde o código vai rodar (VBut + Teclado embutido) */
    ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 64K

    /* Região da sua Shadow RAM alta para variáveis e pilha */
    RAM (rwx) : ORIGIN = 0x00080004, LENGTH = 512K
}

SECTIONS {
    . = 0x00000000;

    .vectors :
    {
        KEEP(*(.vectors))
        
        /* Força a seção de vetores a preencher EXATAMENTE os primeiros 1024 bytes (0x400) */
        /* Isso protege o espaço das 256 exceções possíveis do Motorola 68000 */
        . = ALIGN(0x400); 
    } > ROM

    .text : {
        . = ALIGN(2);
        *(.text*)
        *(.rodata)
        . = ALIGN(2);
    } > ROM

    /* As constantes de texto e tabelas de leitura também ficam na ROM */
    .rodata : {
        . = ALIGN(2);
        *(.rodata*)
        . = ALIGN(2);
    } > ROM

    /* A SEÇÃO .data: Ela roda (VMA) na RAM, mas seus valores iniciais
       ficam salvos fisicamente (LMA) na ROM, logo após o .rodata */
    .data : {
        . = ALIGN(2);
        __data_start = .;   /* Início do bloco na RAM */
        *(.data*)
        . = ALIGN(2);
        __data_end = .;     /* Fim do bloco na RAM */
    } > RAM AT> ROM

    .mram  (NOLOAD) :
    {
        *(?.mram)
    } > RAM

    /* Guarda o endereço físico na ROM de onde o startup deve copiar os dados */
    __data_load_start = LOADADDR(.data);

    /* Variáveis globais não inicializadas ficam direto na RAM */
    .bss (NOLOAD) : {
        . = ALIGN(2);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(2);
        __bss_end = .;
    } > RAM

    /* A Pilha (Stack) também fica alocada na RAM alta de forma segura */
    /* Defina a Pilha no FINAL da sua RAM de 512K (0x00100000) */
    /* Como o 68k empilha para BAIXO, ela decresce com segurança */
    _stack_end = 0x00100000; 
    _stack_start = _stack_end - 0x2000; /* 8KB de pilha segura */
/*
    .stack (NOLOAD) : {
        . = ALIGN(2);
        _stack_start = .;
        . += 0x1000;
        . = ALIGN(2);
        _stack_end = .;
    } > RAM
*/
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.align*)
    }
}
