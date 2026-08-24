 /*
 * Nenhum conflito. Os auto-vetores usam estritamente os números de 25 a 31
 * (offsets $000064 a $00007C). A faixa definida na GAL com base no bit 6 (0x40)
 * vai dos vetores 64 a 76 (offsets $000100 a $000130), que ficam na área livre
 * reservada para o usuário ($40 a $FF).
 * Instalação dos Vetores em C
 * Como o 68000 mapeia a tabela de vetores a partir do endereço $00000000,
 * basta escrever o ponteiro da função ISR diretamente na posição número_do_vetor * 4.
 * Perfeitamente correto.Os sinais /VPA e /DTACK são
 * mutuamente exclusivos no mesmo ciclo de resposta.
 *
 * Ao reconhecer uma interrupção, a CPU indica qual nível
 * está atendendo colocando o valor do nível (1 a 7) nas
 * linhas de endereço $A_1, A_2, A_3$
 * (enquanto $FC_0..FC_2 = 111$).
 *
 * O seu decodificador central lê essa combinação para definir
 * o comportamento:
 *
 * Nível X (Vetorado):
 * O decodificador bloqueia o /VPA e ativa o /IACK para a sua
 * GAL de vetores. A GAL coloca o vetor no barramento e responde
 * com /DTACK.
 * Outros Níveis (Auto-vetorados):
 * O decodificador gera o /VPA e o 68000 ignora o barramento de dados.
 *
 * Exemplo na GAL de Decodificação Central
 *
 * Considerando o Nível 4 ($A_3 A_2 A_1 = 100_b$) reservado para
 * o sistema vetorado da GAL:
 *
 * // Nível 4 (100b) em ciclo de IACK (FC=7) -> Dispara a GAL de Vetor
 * IACK_VETOR = FC0 & FC1 & FC2 & !AS & A3 & !A2 & !A1 ;
 *
 * // Outros níveis (ex: Nível 2 = 010b) -> Gera VPA para auto-vetor
 * VPA = FC0 & FC1 & FC2 & !AS & !A3 & A2 & !A1 ;
 *
 *
 *
 * Durante o ciclo de IACK ($FC_2..FC_0 = 111$ e $A_{19}..A_{16} = 1111$),
 * a CPU indica em binário o nível da interrupção que está sendo atendida
 * através das linhas de endereço $A_3, A_2, A_1$:
 *
 *+-----------+----+----+----+---------+---------------------------+
 *| Nivel IRQ | A3 | A2 | A1 | Binario | Resposta Hardware         |
 *+-----------+----+----+----+---------+---------------------------+
 *| Nivel 1   | 0  | 0  | 1  |  001b   | Auto-vetorado (/VPA)      |
 *| Nivel 2   | 0  | 1  | 0  |  010b   | Auto-vetorado (/VPA)      |
 *| Nivel 3   | 0  | 1  | 1  |  011b   | Auto-vetorado (/VPA)      |
 *| Nivel 4   | 1  | 0  | 0  |  100b   | Vetorado via GAL (/DTACK) |
 *| Nivel 5   | 1  | 0  | 1  |  101b   | Auto-vetorado (/VPA)      |
 *| Nivel 6   | 1  | 1  | 0  |  110b   | Auto-vetorado (/VPA)      |
 *| Nivel 7   | 1  | 1  | 1  |  111b   | NMI / Auto-vetorado (/VPA)|
 *+-----------+----+----+----+---------+---------------------------+
 *
 * O MC68000 não possui um pino físico chamado IACK.Quem inicia o ciclo
 * é a CPU, mas ela faz isso mudando os pinos de controle de estado:
 * A CPU coloca FC0 = 1, FC1 = 1 e FC2 = 1 (espaço de CPU) e coloca 1111
 * nas linhas $A_{19}..A_{16}$.A GAL de decodificação lê essa combinação
 * da CPU e gera o sinal elétrico /IACK (um pino de saída na GAL) para
 * avisar os periféricos ou a placa de vetores que aquele ciclo em
 * andamento é um reconhecimento de interrupção.
 *
 *
 *
 *
 *
 *
 * Dessa forma, você pode misturar na mesma placa níveis auto-vetorados
 * simples com níveis expansíveis e vetorados.
 * // Validação global do ciclo IACK
 * CPU_SPACE = FC0 & FC1 & FC2 & !AS ;
 * IACK_CYCLE = CPU_SPACE & A19 & A18 & A17 & A16 ;
 *
 * // Nível 4 selecionado -> Dispara a GAL de Vetores
 * IACK_VEC4 = IACK_CYCLE & A3 & !A2 & !A1 ;
 *
 * // Qualquer outro nível (1, 2, 3, 5, 6, 7) -> Responde VPA
 * VPA = IACK_CYCLE & !IACK_VEC4 ;
 *
 *
 */

#include <stdint.h>


// Tipo de ponteiro para rotina de interrupção (ISR)
typedef void (*isr_handler_t)(void);

// Mapeamento direto do início da RAM de vetores
#define VECTOR_TABLE ((volatile isr_handler_t *) 0x00000000)

// Definição dos números dos vetores gerados pela GAL
#define VEC_REQ1  0x40  // Vetor 64 (Offset 0x000100)
#define VEC_REQ2  0x44  // Vetor 68 (Offset 0x000110)
#define VEC_REQ3  0x46  // Vetor 70 (Offset 0x000118)
#define VEC_REQ4  0x48  // Vetor 72 (Offset 0x000120)
#define VEC_REQ5  0x4C  // Vetor 76 (Offset 0x000130)

// ISRs (O atributo força o compilador a usar 'rte' em vez de 'rts' e salvar os regs)
void __attribute__((interrupt)) isr_req1_handler(void) {
    // Trata dispositivo 1 (Maior prioridade)
}

void __attribute__((interrupt)) isr_req2_handler(void) {
    // Trata dispositivo 2
}

void __attribute__((interrupt)) isr_req3_handler(void) {
    // Trata dispositivo 3
}

void __attribute__((interrupt)) isr_req4_handler(void) {
    // Trata dispositivo 4
}

void __attribute__((interrupt)) isr_req5_handler(void) {
    // Trata dispositivo 5
}

// Rotina de inicialização das interrupções no Boot
void install_interrupt_vectors(void) {
    VECTOR_TABLE[VEC_REQ1] = isr_req1_handler;
    VECTOR_TABLE[VEC_REQ2] = isr_req2_handler;
    VECTOR_TABLE[VEC_REQ3] = isr_req3_handler;
    VECTOR_TABLE[VEC_REQ4] = isr_req4_handler;
    VECTOR_TABLE[VEC_REQ5] = isr_req5_handler;
}
