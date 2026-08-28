#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "drv_uart.h"

//#define UART_KEYBOARD 0xFF4100
#define RX_BUFFER_SIZE 256

//#define RHR 1  // receive holding register (read)
//#define THR 1  // transmit holding register (write)
//#define IER 3  // interrupt enable register
//#define ISR 5  // interrupt status register (read)
//#define FCR 5  // FIFO control register (write)
//#define LCR 7  // line control register
//#define MCR 9  // modem control register
//#define LSR 11 // line status register
//#define MSR 13 // modem status register
//#define SPR 15 // scratchpad register (reserved for system use)
//#define DLL 1  // divisor latch LSB
//#define DLM 3  // divisor latch MSB
//// aliases for register names (used by different manufacturers)cd ..
//#define RBR RHR // receive buffer register
//#define IIR ISR // interrupt identification register
//#define SCR SPR // scratch register

#define BAUD_DIV_L 0x08 //(BAUD_DIV&$FF)
#define BAUD_DIV_U 0x00 //((BAUD_DIV>>8)&$FF)

#define NO_KEY      0x00
#define VALID_KEY   0xFE
#define KEY_CTRL    0x01
#define KEY_SHIFT   0x02
#define KEY_ALT     0x04
#define KEY_ALTGR   0x08
#define KEY_CAPS    0x10



/*
Esse código está enviando um comando de inicialização
crucial para mudar o comportamento interno do CH9350:
ele está tirando o chip do modo padrão e forçando-o a
entrar no Modo Transparente (ou Modo de Transmissão
Direta).Analisando a estrutura do array cmd[] =
{0x57, 0xAB, 0x01, 0x00, 0x01}, o que cada byte faz
de acordo com o protocolo do CH9350 é o seguinte:0x57
0xAB $\rightarrow$ É o cabeçalho de sincronismo
obrigatório que você já conhece.0x01 $\rightarrow$ É
o código do comando para "Definir Modo de Trabalho"
(Set Work Mode).0x00 $\rightarrow$ É o parâmetro do
modo. O valor 0x00 configura o chip para o Modo
Transparente USB HID.0x01 $\rightarrow$ É o Checksum
desse pacote (pulando o cabeçalho, a soma de 0x01 +
0x00 é igual a 0x01).O que esse modo muda no seu
Orion68K?Por padrão de fábrica, o CH9350 tenta ser
"esperto": ele tenta decodificar os relatórios USB
HID do teclado internamente e enviar apenas dados
mastigados na serial.Quando você envia esse comando e
ativa o Modo Transparente (0x00):Desativa o Filtro
Interno: Você está dizendo para o chip: "Não tente
processar ou filtrar nada por conta própria. Tudo o
que o teclado USB mandar, repasse bruto para a minha
serial".Explica o tamanho dos pacotes: É exatamente
por causa desse comando que o comportamento do chip
mudou e ele começou a te enviar aqueles pacotes longos
de 12 bytes em vez de apenas o scancode puro! No modo
transparente, ele encapsula o relatório USB HID inteiro
do teclado (que tem 8 bytes) dentro do frame serial
dele (adicionando os bytes de status, tamanho,
sequenciador e o checksum de rodapé que você descobriu).
*/
/*
void set_transparent_mode()
{
    volatile unsigned char *uart_reg = (volatile unsigned char *)UART_KEYBOARD;
    // Array com o comando completo: Header(57 AB), Cmd(01), Param(00), Checksum(01)
    unsigned char cmd[] = {0x57, 0xAB, 0x01, 0x00, 0x01};
    for (int i = 0; i < 5; i++)    {
        while (!(uart_reg[LSR] & 0x20))            ;
        uart_reg[THR] = cmd[i];
    }
}
*/

#endif
