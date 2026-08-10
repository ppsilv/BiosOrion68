#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

char * gets_s( char* str, int n ){

    n--;
    while(n--){
        *str = getchar();
        str++;
    }
    *str='\0';
    return (str);
}
