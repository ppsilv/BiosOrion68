#include "timers.h"
#include "interrupt.h"

/*
+--------------------+-----------------+--------------------------+--------------------------------------------------------+
|Nível de Interrupção| Prioridade      | Uso Típico no 68K        |    Por que essa posição?                               |
+--------------------+-----------------+--------------------------+--------------------------------------------------------+
|Nível 7             | Altíssima (NMI) | Botão de Abort           |    Não pode ser desabilitada.                          |
|                    |                 | Queda de Energia         |                                                        | 
+--------------------+-----------------+--------------------------+--------------------------------------------------------+
|Nível 5 ou 6        | Alta            | Controladora de Disquete |    O fluxo de dados do disco não pode esperar,         |
|                    |                 | (WD2793) / Rede          |    ou o setor passa e o dado é perdido.                |
+--------------------+-----------------+--------------------------+--------------------------------------------------------+
|Nível 3 ou 4        | Média           | Porta Serial / UART      |    A UART tem buffers pequenos (FIFO). Precisa         |
|                    |                 | (16C554 / Z8530)         |    ser atendida rápido para não dar Overflow.          |
+--------------------+-----------------+--------------------------+--------------------------------------------------------+
|Nível 1 ou 2        | Baixa           | System Tick              |    Se o Tick atrasar alguns microssegundos para dar    |
|                    |                 | (Timer de 5ms/10ms)      |    passagem à Serial,ninguém morre,nenhum dado perdido.|
+--------------------+-----------------+--------------------------+--------------------------------------------------------+

*/

