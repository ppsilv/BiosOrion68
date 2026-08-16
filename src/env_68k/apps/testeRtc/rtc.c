#include <stdio.h>
#include <string.h>

/* Lê uma linha do teclado (echo manual, suporta backspace).
 * buf: buffer de destino
 * maxlen: tamanho máximo do buffer, incluindo o terminador nulo
 * Retorna o número de caracteres lidos (sem contar o '\0')
 */
int read_line(char *buf, int maxlen) {
    int idx = 0;
    int c;

    if (!buf || maxlen <= 0) return 0;

    while (idx < maxlen - 1) {
        c = getchar();

        if (c == '\r' || c == '\n') {
            putchar('\n');
            break;
        }
        else if (c == '\b' || c == 0x7F) { /* backspace ou DEL */
            if (idx > 0) {
                idx--;
                printf("\b \b"); /* apaga visualmente o último char */
            }
        }
        else if (c >= 0x20 && c < 0x7F) { /* caractere imprimível */
            buf[idx++] = (char)c;
            putchar(c); /* echo */
        }
        /* outros caracteres de controle são ignorados */
    }

    buf[idx] = '\0';
    return idx;
}

int main(int argc, char * argv[]){


    if( argc < 13){
        printf("Usage: %s -a|-m|-d|-h|-m|-s valor\n",argv[0]);
    }
//int ano = (int)strtol(argv[2], NULL, 10);
    if( strstr(argv[1],"-a") ){
        printf("Ano %02d ",strtol(argv[2],NULL,10));
        printf("mes %02d ",strtol(argv[3],NULL,10));
        printf("dia %02d ",strtol(argv[4],NULL,10));
        printf("Hora %02d ",strtol(argv[5],NULL,10));
        printf("Minu %02d ",strtol(argv[6],NULL,10));
        printf("Segu %02d\n ",strtol(argv[7],NULL,10));
    }




//    char linha[64];
//    printf("Digite algo: ");
//    read_line(linha, sizeof(linha));
//    printf("Voce digitou: %s\n", linha);

    return 0;
}
