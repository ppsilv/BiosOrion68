#include <stdio.h>
#include <stdlib.h>
#include <vga_video.h>


int main( int argc, char ** argv )
{
    printf( "Teste TRAP #2 2026...\n" );
    printf( "Compiled for Orion68 cpu MC68000.\n" );

    clrscr();

    gotoxy(20,20);
    printf("TESTE 20,20");

    setcolor(RED,BLACK);
    printf("TESTE RED,BLACK");

    return 1;
}
