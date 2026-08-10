#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ff.h"        // Cabeçalho principal da FatFs
#include "diskio.h"    // Interface de baixo nível da FatFs
#include "drv_kbd.h"


#define BUFFER_SIZE 64
#define BACKSPACE 0x08
#define ENTER 0x0D

extern FATFS FatFs;      // Objeto de controle do sistema de arquivos (Work area)
extern DIR Dir;          // Objeto de diretório
extern FILINFO Fno;      // Estrutura que recebe os metadados do arquivo/pasta
static __attribute__((aligned(2)))FIL Arq;               // Objeto de controle do arquivo (File Object)
extern char buffer_leitura[BUFFER_SIZE];


static void executar_type(char * filename){
    FRESULT res;
    UINT bytes_lidos;
    char arquivo[16];
    sprintf(arquivo,"%s",filename);
 
    //res = f_mount(&FatFs, "", 0);
    //if (res != FR_OK) {
    //    printf("Erro ao montar FAT: %d\n", res);
    //    return;
    //}
    // f_open(Objeto_File, Caminho, Modo_de_Acesso)
    // FA_READ: Abre o arquivo apenas para leitura
    memset(&Arq,0,sizeof(FIL));
    res = f_open(&Arq, arquivo, FA_READ);
    
    if (res != FR_OK) {
        // Se retornar erro 4 (FR_NO_FILE), o arquivo não existe na raiz do cartão
        printf("Erro ao abrir %s Codigo: %d\n",filename, res);
        return;
    }    

    f_lseek(&Arq, 0);
    // Loop de leitura: Lê o arquivo em blocos até chegar ao fim (EOF)
    do {
        res = f_read(&Arq, buffer_leitura, BUFFER_SIZE - 1, &bytes_lidos);
        printf("bytes_lidos %d res %d\n",bytes_lidos,res);
        if (res != FR_OK) {
            printf("\n[Erro durante a leitura do arquivo: %d]\n", res);
            break;
        }

        if (bytes_lidos > 0) {
            // Insere o terminador de string logo após o último byte lido
            // Isso evita que o printf imprima lixo que sobrou no buffer anterior
            buffer_leitura[bytes_lidos] = '\0';
            
            // Cospe o bloco de texto direto no terminal serial
            printf("%s", buffer_leitura);
        }

    } while (bytes_lidos > 0); // Enquanto ler mais que 0 bytes, o arquivo não acabou
    // Obrigatório: Fechar o arquivo para liberar o objeto na FatFs
    f_close(&Arq);
}
static void executar_dir(char * options){

}



void processar_comando(char *cmd_line) {
    char *comando = strtok(cmd_line, " "); // Pega a primeira palavra
    char *argumento = strtok(NULL, " ");   // Pega o resto da linha

    if (comando == NULL) return; // Enter vazio

    if (strcmp(comando, "DIR") == 0) {
        executar_dir("/");
    } 
    else if (strcmp(comando, "TYPE") == 0) {
        if (argumento) {
            executar_type(argumento);
        } else {
            printf("Uso: TYPE <arquivo.txt>\n");
        }
    }
    else if (strcmp(comando, "HELP") == 0) {
        printf("Comandos: DIR, TYPE, HELP\n");
    }
    else {
        printf("Comando desconhecido: %s\n", comando);
    }
}


void ler_comando(char *buffer) {
    int pos = 0;
    char c;

    while (1) {
        // Função que você deve ter mapeada para a UART/Teclado
        c = getchar(); 
        if( c == 0x1b ){
            break;
        }
        if (c == ENTER) {
            buffer[pos] = '\0'; // Finaliza a string
            putchar('\n');         // Pula linha no terminal
            break;
        } 
        else if (c == BACKSPACE) {
            if (pos > 0) {
                pos--;
                putchar(BACKSPACE);
                putchar(' ');      // Apaga o caractere visualmente
                putchar(BACKSPACE);
            }
        } 
        else if (pos < (BUFFER_SIZE - 1)) {
            buffer[pos++] = c;
            putchar(c);            // Eco do caractere na tela
        }
    }
}