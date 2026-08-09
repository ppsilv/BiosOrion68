#include <stdio.h>
#include <stdlib.h>
#include "drv_picoVga.h"
#include "drv_kbd.h"
#include "drv_uart.h"
#include "timers.h"
#include "show_registers.h"

void dump_memory(long addr,int size){
    unsigned char * pcharhex;
    unsigned char * pcharasc;
    unsigned char ch;


    pcharhex = (unsigned char *)addr;
    while(1){
        // Inicializa os ponteiros baseado no 'addr' ATUAL da página
        
        clrscr();
        
        printf("----------------------------------------------------------------------------\n");
        printf("Dumping %03x bytes from addr %08X\n",size, pcharhex);
        printf("Address  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F -             ASCII\n");
        printf("----------------------------------------------------------------------------\n");
        
        int total_linhas = size /16;   
        printf("total_linhas = [%d]\n",total_linhas); 
        for(int j=0; j<total_linhas; j++){
            // Imprime o endereço real dessa linha específica
            printf("%08X  ", (unsigned int)pcharhex);
            
            // Imprime os 16 bytes em Hexa
            for(int i = 0; i<16; i++){
                printf("%02x ", *pcharhex++);            
            }
            
            printf("- ");
            
            // Aponta para o início destes mesmos 16 bytes para fazer o ASCII
            pcharasc = pcharhex - 16;
            for(int i = 0; i<16; i++){
                ch = *pcharasc++;
                if( ch >= 0x20 && ch < 0x80 ) // Mudado para >= 0x20 para incluir o caractere de Espaço
                    printf("%c", ch);            
                else    
                    printf(".");            
            }
            printf("\n");
        }
        
        printf("Hit any <ENTER> to continue <ESC> to terminate: ");
        dump_registradores(); // Seu sensor invisível inline
//        ch = get_key();
//        if( ch == 0x1B ){ // ESC termina
            return;
//        }        
    }
}

//void dump_memory2(long addr){
//    unsigned char * pcharhex = (unsigned char *)addr;
//    unsigned char * pcharasc = (unsigned char *)addr;
//    unsigned char ch;
//    clrscr();
//    delay10ms(1);
//    while(1){
//        clrscr();
//        delay10ms(1);
//        printf("----------------------------------------------------------------\n");
//        printf("Dumping 255 bytes from addr %08X\n",addr);
//        //printf("Address  Data hexacimal                                    Data ASCII\n");
//        printf("Address  00 12 13 14 15 16 17 18 19 10 11 12 13 14 15 16 - ASCII\n");
//        printf("----------------------------------------------------------------\n");
//        for(int j=0;j<16;j++){
//            printf("%07X  ",pcharhex);
//            for(int i = 0; i<16; i++){
//                printf("%02x ",*pcharhex++);            
//            }
//            printf("- ");
//            pcharasc = pcharhex - 16;
//            for(int i = 0; i<16; i++){
//                ch = *pcharasc++;
//                if( ch > 0x20 && ch < 0x80   )
//                    printf("%c",ch);            
//                else    
//                    printf(".");            
//            }
//            printf("\n");
//        }
//        printf("Hit any <ENTER> to continue <ESC> to terminate: ");
//
//
//        dump_registradores();
//
//
//        ch = get_char();
//        if( ch == 0x1B ){
//            return;
//        }
//    }
//
//}


//void dump_memory_by_addr(void *addr, int size){
//    unsigned char * pcharhex;
//    unsigned char * pcharasc;
//    unsigned char ch;
//    
//    pcharhex = (unsigned char *)addr;
//        // Inicializa os ponteiros baseado no 'addr' ATUAL da página
//        
//        clrscr();
//        
//        printf("----------------------------------------------------------------\n");
//        printf("Dumping 256 bytes from addr %08X\n", pcharhex); // Agora vai atualizar de verdade!
//        printf("Address  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F - ASCII\n"); // Corrigido os números hex do cabeçalho
//        printf("----------------------------------------------------------------\n");
//        int total_linhas = size /16;    
//        for(int j=0; j<total_linhas; j++){
//            // Imprime o endereço real dessa linha específica
//            printf("%08X  ", (unsigned int)pcharhex);
//            
//            // Imprime os 16 bytes em Hexa
//            for(int i = 0; i<16; i++){
//                printf("%02x ", *pcharhex++);            
//            }
//            
//            printf("- ");
//            
//            // Aponta para o início destes mesmos 16 bytes para fazer o ASCII
//            pcharasc = pcharhex - 16;
//            for(int i = 0; i<16; i++){
//                ch = *pcharasc++;
//                if( ch >= 0x20 && ch < 0x80 ) // Mudado para >= 0x20 para incluir o caractere de Espaço
//                    printf("%c", ch);            
//                else    
//                    printf(".");            
//            }
//            printf("\n");
//        }
//        
//        printf("Hit any <ENTER> to continue <ESC> to terminate: ");
//        ch = get_char();
//}
