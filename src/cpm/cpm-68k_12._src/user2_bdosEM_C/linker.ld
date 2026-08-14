OUTPUT_ARCH(m68k)
ENTRY(_start)

SECTIONS
{
    . = 0x82000;

    .text :
    {
        /* Garante que a interface de trap/inicialização fique no início */
        *bdosif.o(.text .text.*)
        *(.text .text.*)
    }

    .rodata : { *(.rodata .rodata.*) }
    .data   : { *(.data .data.*) }
    .bss    : { *(.bss .bss.*) *(COMMON) }
}
