Como chamar isso a partir do seu kernel.bin já compilado:

Coloque o test_tasks.c, scheduler.c e o ctx_switch_68000.s (ou _68010.s, conforme seu CPU) no seu build.
No lugar de onde seu kernel chama o shell hoje (ou logo depois do boot, antes de qualquer coisa que fique 
num loop infinito próprio), chame run_scheduler_test(); em vez disso.
run_scheduler_test() nunca retorna — a partir do OS_Start(), quem passa a controlar tudo é o OS_TickISR, 
então não pode ter mais nada relevante depois dessa chamada no seu ma_in() do kernel.

Duas coisas que faltam e são específicas do seu hardware, que eu não consigo escrever por você:

Configurar o timer pra gerar interrupção periódica e apontar o vetor dele pro OS_TickISR. Sem isso, nenhuma 
tarefa nunca vai ser trocada — elas ficam presas na primeira que o OS_Start() escalar.
Garantir que printf já esteja funcionando (mandando pra sua serial) antes de chamar run_scheduler_test() — 
se seu printf depender de alguma inicialização que ainda não rodou nesse ponto do boot, o teste não vai 
mostrar nada.

Se quiser, me diz como está configurado o vetor de interrupção do seu timer (número do vetor, frequência) 
que eu ajudo a plugar o OS_TickISR nele.


#include <stdio.h>
#include "scheduler.h"

/*
 * Teste isolado: duas tarefas bobas, cada uma so incrementando um
 * contador proprio e imprimindo de vez em quando. Se elas alternarem
 * no terminal, o context switch esta funcionando.
 *
 * Cada tarefa precisa da SUA PROPRIA pilha, alocada estaticamente
 * (sem malloc). O tamanho e' um chute razoavel pra uma tarefa simples
 * -- se sua tarefa usar buffers grandes na pilha (arrays locais, etc)
 * aumente esse valor. Estourar a pilha aqui e' silencioso (sem MMU),
 * entao se algo comportar estranho, suspeite disso primeiro.
 */
#define STACK_SIZE 1024

static uint8_t stack_task_a[STACK_SIZE];
static uint8_t stack_task_b[STACK_SIZE];

static void TaskA(void)
{
    uint32_t count = 0;
    for (;;) {
        count++;
        if ((count % 100000) == 0)
            kprintf("Tarefa A: %lu\n", (unsigned long)count);
    }
}

static void TaskB(void)
{
    uint32_t count = 0;
    for (;;) {
        count++;
        if ((count % 100000) == 0)
            kprintf("Tarefa B: %lu\n", (unsigned long)count);
    }
}

void run_scheduler_test(void)
{
    OS_Init();

    /* arg = NULL porque essas tarefas nao precisam de nenhum dado externo */
    OS_TaskCreate(TaskA, NULL, stack_task_a, STACK_SIZE, /*id=*/1);
    OS_TaskCreate(TaskB, NULL, stack_task_b, STACK_SIZE, /*id=*/2);

    OS_Start();   /* nunca retorna -- a partir daqui quem manda e' o OS_TickISR */

    /* nunca chega aqui */
}


Sim, exatamente essa ideia — é o ponto onde o kernel monta as tarefas iniciais e entrega o controle pro scheduler.

Diferença pro conceito clássico de init (PID 1) do Unix: lá é uma única tarefa inicial que depois cria as outras (shell, daemons, etc). Aqui run_scheduler_test() já cria duas de cara (TaskA/TaskB), só pra testar.

Quando for pra produção, o padrão fica mais parecido com init de verdade:

c
void kernel_init_tasks(void)
{
    OS_Init();
    OS_TaskCreate(ShellTask, NULL, stack_shell, STACK_SIZE, /*id=*/1);
    OS_Start();
}

E o ShellTask (rodando como a única tarefa "init") é quem depois chama load_elf_as_task() do elf_task_integration.c pra criar novas tarefas conforme o usuário pede pra rodar programas — em vez de já nascer com tudo criado estaticamente como no teste.


Sim, pra esse teste inicial — sim, compile junto com o kernel.bin.

Faz sentido assim porque:

O scheduler roda continuamente a partir do boot (o OS_Start() nunca retorna), 
então ele precisa estar onde o seu kernel já assume controle total do sistema.
TaskA/TaskB são só funções C comuns linkadas no mesmo binário — não são 
executáveis .elf separados carregados em USERRAM como os programas que você 
carrega com o load_elf_executable. São mais parecidas com "threads internas 
do kernel" do que com processos de usuário.
Isso evita de cara a armadilha que a gente identificou lá atrás (o loader ELF 
rodando dentro da mesma região de memória que ele sobrescreve) — aqui não tem 
esse risco, porque TaskA/TaskB moram em endereços fixos do próprio kernel.bin, 
fora da faixa 0x82000–0x102000 reservada pra programas carregados.

Depois que confirmar que a troca de contexto funciona com essas duas tarefas 
bobas, o próximo passo natural seria fazer o shell virar uma tarefa desse mesmo 
scheduler (ao lado de outras tarefas do kernel), e só então usar o elf_task_integration.c 
pra fazer o load_elf_executable criar uma tarefa nova pro programa carregado — 
mantendo a regra de que só um .elf de usuário ocupa USERRAM por vez, como já 
discutimos.


Beleza, combinado — não toco mais nesse ponto.

**Tamanho de slot**: com no máximo ~10 processos, dá pra ser bem mais generoso. Sugiro **256K por slot** (10 × 256K = 2,5MB — você mencionou que a placa suporta até 8MB com a modificação no inversor A22, então cabe folgado até com RAM do sistema reservada). Se quiser meio termo mais conservador, **128K** também é razoável (10 × 128K = 1,25MB). Eu iria de 256K — 64K é apertado até pra programas simples com buffers (você mesmo notou isso), e memória você tem sobrando pra essa quantidade de processos.

**Linker script pra PIC** — antes do código, um alerta técnico real (não é sobre proteção de memória, é sobre viabilidade): m68k GCC não tem um `-fPIC` completo e maduro como ARM/x86 — o suporte histórico é via `-msep-data`/`-mid-shared-library`, específico de algumas variantes de toolchain (ex: uClinux m68k), e nem todo `m68k-elf-gcc` genérico suporta bem. Vale confirmar com `m68k-elf-gcc -Q --help=target` se essas flags existem no seu toolchain antes de investir tempo nisso — senão a rota realista continua sendo múltiplos linker scripts fixos (um por slot), que você já tem o padrão pronto.

Segue o linker script assumindo que seu toolchain suporta `-msep-data` (código PIC via A5 como base de dados):

```ld
OUTPUT_FORMAT("elf32-m68k")
OUTPUT_ARCH(m68k)
ENTRY(main)

MEMORY
{
    SLOT (rwx) : ORIGIN = 0x00000000, LENGTH = 0x00040000  /* 256K, base relativa -- realocada pelo loader na carga */
}

PHDRS
{
    text PT_LOAD FLAGS(7);
}

SECTIONS
{
    . = ORIGIN(SLOT);
    .text : {
        *(.text*)
        *(.rodata*)
    } > SLOT :text

    /* codigo PIC (-msep-data) referencia .data via A5 -- precisa ficar junto */
    .data : {
        _data_start = .;
        *(.data*)
    } > SLOT :text

    .bss : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > SLOT :text

    _end = .;
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
```

Diferenças-chave do seu `linker.ld` atual: `ORIGIN = 0x00000000` (código gerado relativo, não fixo), e seu loader passa a fazer o que o `ELF_TaskTrampoline` já faz hoje só que **adicionando um passo de relocação** — calcular o endereço real do slot escolhido e ajustar A5 (base de dados) antes de saltar pro `entry`. Essa parte de relocação em tempo de carga não está no que te passei até agora; se decidir seguir por PIC de verdade, é o próximo bloco de código que eu escreveria.