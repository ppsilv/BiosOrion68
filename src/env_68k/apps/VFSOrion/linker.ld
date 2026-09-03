OUTPUT_FORMAT("elf32-m68k")
OUTPUT_ARCH(m68k)

/*
 * ENTRY(main): o programa carregado NAO precisa de crt0/_start.
 * O load_elf_executable() chama o entry point como uma funcao C normal:
 *     int (*entry)(int, char**) = (int(*)(int,char**))header.entry;
 *     entry(argc, argv);
 * Como isso e' uma chamada de funcao comum (jsr/rts), a propria main()
 * compilada pelo GCC ja e' um alvo valido -- ela segue a ABI padrao
 * (argumentos empilhados, retorno em D0), entao serve como entry point
 * direto, sem stub em assembly.
 */
ENTRY(main)

MEMORY
{
    /*
     * AJUSTE conforme o mapa de memoria real do seu sistema:
     * - ORIGIN deve ser uma regiao de RAM livre, que NAO colida com
     *   onde o monitor/kernel do Orion68 roda, nem com a pilha dele.
     * - LENGTH e' o espaco maximo reservado pra esse programa (ajuste
     *   pro tamanho que fizer sentido pros seus programas carregaveis).
     * 0x00100000 (1MB) foi escolhido so como ponto de partida, assumindo
     * RAM total de 2MB e o kernel ocupando a parte mais baixa.
     */
    USERRAM (rwx) : ORIGIN = 0x00092000, LENGTH = 0x0000FFFF
}

PHDRS
{
    /* Um unico segmento PT_LOAD, com todas as permissoes -- mantem o
       loader simples (ele so entende PT_LOAD, PT_NULL, PT_NOTE, PT_PHDR
       de forma neutra, e rejeita PT_DYNAMIC/PT_INTERP). */
    text PT_LOAD FLAGS(7);
}

SECTIONS
{
    . = ORIGIN(USERRAM);

    .text : {
        *(.text*)
        *(.rodata*)
    } > USERRAM :text

    .data : {
        *(.data*)
    } > USERRAM :text

    /*
     * .bss nao ocupa espaco no arquivo (SHT_NOBITS) -- fica de fora do
     * p_filesz do segmento, mas dentro do p_memsz. O load_elf_executable
     * ja zera essa regiao sozinho:
     *     if (progHeader.memsz > progHeader.filesz)
     *         memset(...);
     * entao nao e' preciso nenhum codigo de inicializacao extra pra isso.
     */
    .bss : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > USERRAM :text

    _end = .;

    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
