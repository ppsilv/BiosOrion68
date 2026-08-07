#include <stdio.h>
#include <stdlib.h>

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
    printf( "TinyBasic pgordao @copyleft 2025...\n" );
    printf( "Compiled for tcpbox68k cpu MC68000.\n" );
    //setup();
    loop();
    printf("Exiting tBasic...\n");
}
