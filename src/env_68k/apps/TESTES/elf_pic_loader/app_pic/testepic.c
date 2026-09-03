/*
 *  2.Compila com o linker PIC:
        m68k-elf-gcc -msep-data -m68000 -T linker_pic.ld -c testepic.c -o testepic.o
        m68k-elf-ld -T linker_pic.ld testepic.o -o testepic.elf
*/

int contador_global = 0;   /* .data -- precisa de A5 certo pra funcionar */

int main(int argc, char *argv[])
{
    contador_global = 42;
    contador_global++;
    return contador_global;   /* se voltar 43, .data funcionou */
}



