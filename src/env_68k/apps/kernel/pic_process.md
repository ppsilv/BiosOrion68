

Dá pra fazer sem precisar de relocação de verdade (sem tabela R_68K_GOT, sem parser de relocations)
 — o truque do -msep-data é justamente esse: o .text já nasce independente de posição (usa modos de 
 endereçamento relativos a PC), e o .data/.bss só precisam que A5 aponte pro endereço real onde foram 
 carregados. Isso eu consigo fazer só ajustando onde carrego os segmentos e um registrador — sem 
 parser de ELF relocations.

Vou montar em 4 partes: o linker script novo, o alocador dos 10 slots, uma extensão pequena no 
scheduler pra poder pré-carregar A5 numa tarefa nova, e o loader PIC em si.

Created 4 files, edited 3 files, and 3 more tools
Resumo direto:

linker_pic.ld — compila o programa com endereços relativos a 0, em 2 segmentos PT_LOAD separados (texto 
/ dados) — o loader depende dessa ordem.
slots.h/slots.c — 10 slots de 256K a partir de 0x92000, com Slots_Alloc()/Slots_Free(). Confira se essa 
faixa não colide com o USERRAM antigo (0x82000, 512K) e o resto do seu mapa de memória — pode ter 
sobreposição, é você quem sabe o layout real.
scheduler.h/.c — só ganhou OS_TaskCreateWithA5(), que reaproveita o OS_TaskCreate normal e depois escreve 
o valor de A5 direto no offset 52 da pilha já montada.
elf_pic_loader.c — o loader novo. Aloca um slot, carrega os 2 PT_LOAD somando task_base a cada endereço, 
guarda onde o segmento de dados caiu, e cria a tarefa com A5 = esse endereço.
Compile seus programas com:

m68k-elf-gcc -msep-data -m68000 -T linker_pic.ld -o programa.elf programa.c
Duas pontas soltas, de propósito (documentadas no final do elf_pic_loader.c): não libero o slot automaticamente
 quando a tarefa termina (OS_TaskExit hoje só marca TASK_UNUSED e trava, não "libera" nada de verdade — precisa
  decidir esse comportamento antes de automatizar), e assumo #include "elf.h" com os mesmos tipos que seu 
  load_elf_executable original já usa — se o nome do seu header for outro, é só ajustar esse include.

  Codigo 68kfiles_pic.zip




##  Pra testar o elf_pic_loader.c isolado, o caminho mais direto:

# 1. Programa de teste bem simples, com uma variável global (pra provar que o A5/.data está funcionando):

c
* /* teste_pic.c */
int contador_global = 0;   /* .data -- precisa de A5 certo pra funcionar */

int main(int argc, char *argv[])
{
    contador_global = 42;
    contador_global++;
    return contador_global;   /* se voltar 43, .data funcionou */
}

2. Compila com o linker PIC:

m68k-elf-gcc -msep-data -m68000 -T linker_pic.ld -c teste_pic.c -o teste_pic.o
m68k-elf-ld -T linker_pic.ld teste_pic.o -o teste_pic.elf

3. Chama o loader a partir do kernel (via shell, ou direto num teste no main()):

c
Slots_Init();

FIL fd;
f_open(&fd, "teste_pic.elf", FA_READ);

char *argv[] = { "teste_pic" };
uint8_t *stack = /* aloque um buffer estatico, ex: static uint8_t stack[1024]; */;

int slot = load_pic_elf_as_task(&fd, 1, argv, stack, 1024);
if (slot < 0)
    kprintf("Falha ao carregar\n");
else
    kprintf("Carregado no slot %d\n", slot);

OS_Start();

4. Confirme o que testar: como você ainda não tem OS_TaskExit liberando nada nem 
devolvendo valor pro chamador, o jeito mais simples de ver se funcionou é o próprio
 main() do teste_pic.c escrever num endereço fixo conhecido (ex: um registrador 
 de I/O da sua serial, ou uma posição de memória que você inspeciona depois via 
 monitor) em vez de só fazer return — porque return de uma tarefa hoje só cai no 
 OS_TaskExit e trava, sem devolver nada visível.

Quer que eu ajuste o teste_pic.c pra imprimir via alguma saída sua (serial, 
kprintf se ele for chamável de dentro do programa carregado) em vez de só usar 
return?




## Teste do loader diretamente
Beleza, mantendo FatFS via libfileio então. As outras coisas que dá pra mockar/
simplificar pra esse teste isolado:

1. OS_TaskCreateWithA5 / scheduler inteiro — pro teste de hoje, você não precisa
 do scheduler rodando de verdade. Troca por um jsr manual: monta A5 com move.l 
 data_load_addr, A5 antes de chamar o entry point diretamente como função C normal 
 (igual seu load_elf_executable antigo já fazia com entry(argc, argv)). Isso 
 valida "o A5 está certo e o programa acessa .data direito" sem precisar de 
 ctx_switch_*.s, scheduler.c nem stack de tarefa nenhuma.

2. kprintf() — se seu oriondos hoje já tem alguma função de print pra serial/console
 (mesmo que se chame diferente), só aponta pra ela; se não tiver nada ainda, 
 um mock trivial serve:

c
#define kprintf(...) printf(__VA_ARGS__)   /* ou sua própria saída, se já existir */

3. elf.h — não precisa mockar, só reaproveitar o mesmo header que o load_elf_executable
 original já usa no oriondos de hoje (você confirmou que já tem os tipos elf32_header 
 etc).

4. Slots_Init()/Slots_Alloc() — esses não têm dependência nenhuma (slots.c é só array 
estático), compila direto sem mock.

Resumindo: pra rodar esse teste standalone no oriondos de hoje, você precisa de 
slots.c/slots.h (sem mudança), elf.h (já existe), libfileio (já existe), e uma versão 
do loader que troca a criação de tarefa por uma chamada direta com A5 pré-setado — sem 
tocar em scheduler/ctx_switch.

Quer que eu escreva essa versão simplificada do elf_pic_loader.c (sem scheduler, com 
jsr direto) agora?


## SO PARA TESTE CRIOU OS_TaskCreateWithA5
Faz sentido — o elf_pic_loader.c que te passei ainda chama OS_TaskCreateWithA5, que só 
existe dentro do scheduler.c que você decidiu não incluir. Precisa de uma versão enxuta, 
sem essa chamada. Vou criar uma variante só pra esse teste:

Zero dependência de scheduler.c/ctx_switch_*.s agora. Só slots.h/.c, elf.h, libfileio 
e um kprintf seu.

Aviso honesto sobre a parte mais arriscada (deixei documentado no próprio arquivo): a 
função call_with_a5 usa assembly inline pra garantir que A5 esteja setado bem na hora 
do jsr — mas a lista de constraints que escrevi ("a" (a5_val) etc) é um esqueleto, não 
testei contra a convenção de chamada real do seu m68k-elf-gcc. Se der erro de compilação 
ou o retorno vier corrompido, o caminho mais seguro é confirmar com -S (gerando o .s) se 
um código mais simples — só atribuir register uint32_t a5_val asm("a5") = a5_value; e 
chamar entry(argc, argv) normalmente — realmente mantém A5 intacto entre a atribuição e 
a chamada, sem o compilador recarregar o registrador no meio.

Se travar nessa parte, me manda o erro de compilação ou o .s gerado que eu ajusto.



// Extração direta por offset (livre de padding do compilador)
uint8_t  *ident_magic   =&header_buf[0];
uint8_t  ident_class    = header_buf[4];
uint8_t  ident_data     = header_buf[5];
uint8_t  ident_version  = header_buf[6];
uint8_t  ident_osabi    = header_buf[7];
uint8_t  ident_abiversion = header_buf[8];
uint8_t  *ident_pad   = &header_buf[9];
uint16_t type    = (header_buf[0x10] << 8) | header_buf[0x11];
uint16_t machine = (header_buf[0x12] << 8) | header_buf[0x13];
uint32_t version = ((uint32_t)header_buf[0x14] << 24) |
                   ((uint32_t)header_buf[0x15] << 16) |
                   ((uint32_t)header_buf[0x16] << 8)  |
                   (uint32_t)header_buf[0x17];
uint32_t entry = ((uint32_t)header_buf[0x18] << 24) |
                   ((uint32_t)header_buf[0x19] << 16) |
                   ((uint32_t)header_buf[0x1A] << 8)  |
                   (uint32_t)header_buf[0x1B];
uint32_t phoff   = ((uint32_t)header_buf[0x1C] << 24) |
                   ((uint32_t)header_buf[0x1D] << 16) |
                   ((uint32_t)header_buf[0x1E] << 8)  |
                   (uint32_t)header_buf[0x1F];
uint32_t shoff   = ((uint32_t)header_buf[0x20] << 24) |
                   ((uint32_t)header_buf[0x21] << 16) |
                   ((uint32_t)header_buf[0x22] << 8)  |
                   (uint32_t)header_buf[0x23];
uint32_t flags   = ((uint32_t)header_buf[0x24] << 24) |
                   ((uint32_t)header_buf[0x25] << 16) |
                   ((uint32_t)header_buf[0x26] << 8)  |
                   (uint32_t)header_buf[0x27];
uint16_t ehsize  = (header_buf[0x28] << 8) | header_buf[0x29];
uint16_t phentsize  = (header_buf[0x2A] << 8) | header_buf[0x2B];
uint16_t phnum  = (header_buf[0x2C] << 8) | header_buf[0x2D];
uint16_t shentsize  = (header_buf[0x2E] << 8) | header_buf[0x2F];
uint16_t shnum  = (header_buf[0x30] << 8) | header_buf[0x31];
uint16_t shstrndx  = (header_buf[0x32] << 8) | header_buf[0x33];


    uint8_t  ident_magic[4];     
    uint8_t  ident_class;        
    uint8_t  ident_data;         
    uint8_t  ident_version;      
    uint8_t  ident_osabi;
    uint8_t  ident_abiversion;
    uint8_t  ident_pad[7];       

    uint16_t type;               
    uint16_t machine;            
    uint32_t version;
    uint32_t entry;              
    uint32_t phoff;              
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;          
    uint16_t phnum;  
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;










loader.elf para PIC 




    Certíssimo — 43 é exatamente o esperado.

Isso confirma a cadeia inteira funcionando:

O slot foi alocado e o endereço base calculado certo
Os dois segmentos PT_LOAD foram copiados pro lugar certo (task_base + p_vaddr)
O patch da GOT corrigiu o ponteiro pra contador_global (de 0x44 relativo pra task_base + 0x44 real)
A5 apontou pro lugar certo, o código indireto (A5@(0) → ponteiro → variável) leu o endereço certo da GOT já corrigida
contador_global = 42; contador_global++; executou de verdade em cima da memória real
O return voltou por entry(argc, argv) com a pilha montada certinho pelo compilador (não pelo jsr manual que travava antes)

Isso valida o modelo PIC via GOT ponta a ponta — carga em endereço relocável + patch de GOT + A5 + chamada. Quando for levar isso pro elf_pic_loader.c de verdade (o que cria tarefa via OS_TaskCreateWithA5 em vez de chamar direto), o patch da GOT que acabamos de validar aqui precisa entrar lá também — é a mesma lógica, só trocando o "chamar direto" pelo "criar tarefa".



Boa pergunta, e a resposta é: não completamente — o teste que você acabou de rodar fez menos que isso.

O que o load_pic_elf_standalone realmente montou:

Código carregado em task_base + 0
Dados (.got/.got.plt/.bss) carregados em task_base + 0x34, com a GOT já corrigida
A5 apontando pro segmento de dados

O que não foi montado, e é importante você saber: a pilha (Stack Pointer/A7) nunca foi trocada. O call_with_a5 chama entry(argc, argv) como uma chamada C normal — isso significa que o main() do programa carregado rodou usando a mesma pilha do seu kernel/teste, não uma pilha própria dentro dos 256K do slot. Funcionou porque o main() de teste é minúsculo (poucas variáveis locais, sem recursão, sem arrays grandes) — qualquer programa um pouco mais pesado ia estourar ou corromper a pilha do chamador.

Isso é diferente do elf_pic_loader.c "de verdade" (o que usa OS_TaskCreateWithA5) — aquele sim recebe task_stack/task_stack_size como parâmetro e monta uma pilha dedicada pra tarefa, dentro da região que você passar. Só que hoje essa pilha é alocada fora do slot de 256K (você passa um buffer separado) — não é automática dentro do slot.

Se você quiser que a pilha fique dentro dos 256K do próprio slot (fazendo cada slot ser um "processo completo": código+dados+pilha, tudo isolado num único bloco), dá pra fazer — é só reservar um pedaço do fim do slot (ex: últimos 16K ou 32K) como pilha, calcular o topo (task_base + SLOT_SIZE), e passar isso pro OS_TaskCreateWithA5 em vez de um buffer separado. Fica mais "processo de verdade" (tudo do programa dentro de uma única região contígua), ao custo de reservar espaço fixo de pilha por slot mesmo pra programas que precisem de menos.

Quer que eu ajuste o loader pra reservar a pilha automaticamente dentro do próprio slot?