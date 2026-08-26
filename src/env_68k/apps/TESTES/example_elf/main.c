/*
 * Programa de exemplo, carregavel via load_elf_executable().
 *
 * Nao precisa de crt0/_start: o loader chama esta funcao diretamente
 * como entry point, seguindo a ABI normal de chamada de funcao C
 * (argc/argv empilhados, retorno em D0) -- exatamente como qualquer
 * outra funcao chamada por ponteiro dentro do proprio firmware.
 *
 * Este exemplo e' deliberadamente standalone (sem I/O), pra compilar
 * sem depender de nenhuma biblioteca. Pra adicionar saida de texto,
 * inclua o header/lib do seu firmware (ex: UartWrCh, print_string) e
 * linke junto -- veja o comentario no fim deste arquivo.
 */
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    int i;
    int soma = 0;

    printf("Teste do OrionDOS\n");
    for (i = 0; i < argc; i++) {
        soma += 1;
    }
    printf("Soma=%d\n",soma);
    return soma;   /* valor de retorno chega em D0 para quem chamou */
}

/*
 * Para usar I/O do seu firmware (ex: imprimir algo na UART), adicione
 * algo como:
 *
 *     extern void UartWrCh(char c);
 *     extern void print_string(char *s);
 *
 * e linke este programa incluindo o .o correspondente do firmware
 * principal (ou uma lib estatica com essas funcoes), ajustando o
 * Makefile deste diretorio para incluir esse objeto/lib no LDFLAGS.
 */
