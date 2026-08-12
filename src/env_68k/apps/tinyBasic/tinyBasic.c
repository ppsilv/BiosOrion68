#include <stdio.h>
#include <stdlib.h>
#include <vga_video.h>
#include <color.h>
#include <vga_video_graphics.h>

#define RUN_CMD        (*((volatile unsigned char *)(0x00FF8000 + 0x3D)))


#if defined(__MINGW32__ )
#endif

#if __APPLE__ || __linux__
#   include <dirent.h>
#   include <sys/stat.h>
#elif __68000__
#else
#   error Needs fixing to compile on your system
#endif

/* these are used in the .ino */

void outchar( char ch )
{
    printf( "%c", ch );
}


/* other helpers */
#ifndef __68000__
int cmd_Files( void )
{
    DIR * theDir;

    theDir = opendir( "." );
    if( !theDir ) return -2;

    struct dirent *theDirEnt = readdir( theDir );
    while( theDirEnt ) {
	printf( "  %s\n", theDirEnt->d_name );
	theDirEnt = readdir( theDir );
    }
    closedir( theDir );

    return 0;
}
#endif

//void setup( void );
void loop( void );

int main( int argc, char ** argv )
{
    RUN_CMD = 0xA4;
    setcolor(4,0);
    printf( "\nTinyBasic pgordao @copyleft V1.1 2026...\n" );
    printf( "Compiled for Orion68 cpu MC68000.\n" );
    //setup();
    setcolor(2,0);
    loop();
    printf("Exiting tBasic...\n");
}
