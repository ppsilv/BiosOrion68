MEMORY {
    /* Região da ROM onde o código vai rodar (VBut + Teclado embutido) */
    ROM (rx)  : ORIGIN = 0x00008000, LENGTH = 32K

    /* Região da sua Shadow RAM alta para variáveis e pilha */
    RAM (rwx) : ORIGIN = 0x00080400, LENGTH = 0x2000
}

SECTIONS {
    . = ALIGN(2);

    /* O código principal fica armazenado e roda direto na ROM */
    .text : {
        . = ALIGN(2);
        *(.text*)
        *(.init)
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
    .stack (NOLOAD) : {
        . = ALIGN(2);
        _stack_start = .;
        . += 0x1000;
        . = ALIGN(2);
        _stack_end = .;
    } > RAM

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.align*)
    }
}
