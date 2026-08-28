MEMORY
{
    RAM (rwx) : ORIGIN = 0x00002000, LENGTH = 256K
}

SECTIONS
{
    .text : {
        *(.text.bios_entry)  /* Força a Jump Table da BIOS a ficar no topo */
        *(.text*)
        *(.rodata*)
    } > RAM

    .data : {
        *(.data*)
    } > RAM

    .bss : {
        *(.bss*)
        *(COMMON)
    } > RAM
}