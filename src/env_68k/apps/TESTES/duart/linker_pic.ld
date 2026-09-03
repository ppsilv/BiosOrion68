OUTPUT_FORMAT("elf32-m68k")
OUTPUT_ARCH(m68k)
ENTRY(main)

MEMORY
{
    SLOT (rwx) : ORIGIN = 0x00000000, LENGTH = 0x00040000
}

PHDRS
{
    text PT_LOAD FLAGS(5);   /* R+X -- so' texto/rodata */
    data PT_LOAD FLAGS(6);   /* R+W -- data/bss */
}

SECTIONS
{
    . = ORIGIN(SLOT);
    .text : {
        *(.text*)
        *(.rodata*)
    } > SLOT :text

    . = ALIGN(4);
    .data : {
        _data_start = .;
        *(.data*)
    } > SLOT :data

    .bss : {
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > SLOT :data

    _end = .;
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
