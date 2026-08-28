ENTRY(_start)

MEMORY {
    ROM (rx) : ORIGIN  = 0x00000000, LENGTH = 256k
    RAM (rwx) : ORIGIN = 0x00080000, LENGTH = 1024k
}

SECTIONS {
    /* --- Vetores de exceção DEVEM estar no início absoluto --- */
    .vectors 0x00000000 : {
        . = ALIGN(2);
        KEEP(*(.vectors))
    } > ROM

    .jumptable 0x00000400 : {
        KEEP(*(.jumptable))
    } > ROM

    /* --- Seu código principal --- */
    .text 0x00001000 : {
        _start = .;
        *(.text .text.*)
        . = ALIGN(2);
        _start_end = .;
    } > ROM

    .rodata : {
        *(.rodata .rodata.*)
        . = ALIGN(2);
    } > ROM

    .data : AT(ADDR(.rodata) + SIZEOF(.rodata)) {
        __data_load = LOADADDR(.data);
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(2);
        __data_end = .;
    } > ROM

    /* --- Nova seção para o checksum (últimos 4 bytes da ROM) --- */
    .checksum (NOLOAD) : {
        /* Posiciona o checksum nos últimos 4 bytes da ROM */
        . = ALIGN(2);
        __checksum_addr = .;
        LONG(0);  /* Espaço reservado (será preenchido pós-build) */
    } > ROM

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(2);
        __bss_end = .;
    } > RAM

    /DISCARD/ : {
        *(.comment)
        *(.note.*)
    }
}
