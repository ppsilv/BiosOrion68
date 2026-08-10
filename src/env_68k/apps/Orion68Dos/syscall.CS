Aqui está a implementação do stub `_write` exigido pela Newlib, conectando o `printf()` do GCC ao seu handler de `TRAP #0`.

---

### Convenção da Syscall (`TRAP #0`)

* **`D0.L`**: `1` (ID do `SYS_WRITE`)
* **`D1.L`**: `file` (File Descriptor: `1` = stdout, `2` = stderr)
* **`A0.L`**: `ptr` (Endereço do buffer de texto)
* **`D2.L`**: `len` (Tamanho do buffer em bytes)
* **Retorno em `D0.L**`: Quantidade de bytes transmitidos.

---

### Option 1: Implementação em C (com Inline Assembly)

Arquivo `syscalls.c`:

```c
#include <sys/stat.h>

int _write(int file, char *ptr, int len) {
    register int   r_id  __asm__("d0") = 1;     // Syscall ID = 1
    register int   r_fd  __asm__("d1") = file;  // fd (stdout/stderr)
    register char *r_buf __asm__("a0") = ptr;   // Ponteiro da string
    register int   r_len __asm__("d2") = len;   // Tamanho
    register int   r_ret __asm__("d0");

    __asm__ volatile (
        "trap #0"
        : "=r" (r_ret)
        : "r" (r_id), "r" (r_fd), "r" (r_buf), "r" (r_len)
        : "memory", "cc"
    );

    return r_ret; // Retorna quantidade de bytes impressos
}

```

---

### Option 2: Implementação Pura em Assembly m68k

Se preferir manter os stubs em arquivo `.S` separado (`syscalls.S`):

```assembly
    .global _write
    .type   _write, @function

_write:
    ; C ABI no m68k (GCC): argumentos na pilha
    ;  4(sp) = file (int)
    ;  8(sp) = ptr  (char *)
    ; 12(sp) = len  (int)

    movem.l d2, -(sp)      ; Preserva D2 (exigido pelo ABI de C)

    move.l  8(sp), d1      ; d1 = file (+4 do offset por causa do push de d2)
    move.l  12(sp), a0     ; a0 = ptr
    move.l  16(sp), d2     ; d2 = len
    moveq   #1, d0         ; d0 = 1 (SYS_WRITE)

    trap    #0             ; Chama o Kernel via TRAP

    movem.l (sp)+, d2      ; Restaura D2
    rts                    ; Retorna (resultado da syscall permanece em D0)

```

---

### Do lado do Kernel / Supervisor (`TRAP #0` Handler)

No seu vetor de exceção, você consome os registradores e joga os caracteres no hardware (UART/Vídeo):

```assembly
trap0_handler:
    cmpi.l  #1, d0          ; É SYS_WRITE?
    bne     sys_bad_id

    ; d1 = fd, a0 = ptr, d2 = len
    move.l  d2, d3          ; Salva len original para o retorno

.write_loop:
    tst.l   d2              ; Terminou os bytes?
    beq     .write_done
    
    move.b  (a0)+, d0       ; Pega o caractere
    bsr     uart_putc       ; Envia para a UART / Tela de Vídeo
    
    subq.l  #1, d2
    bra     .write_loop

.write_done:
    move.l  d3, d0          ; Retorna a qtd de bytes gravados em D0
    rte                     ; Retorna da exceção (volta pro User Mode)

```

Com isso montado e linkado, qualquer chamada como `printf("Hello %d\n", 68000);` vai formatar a string na RAM via Newlib e disparar a `TRAP #0` automaticamente.


/***************************************************************************************************************************************************************
**************************************************************************************************************************************************************
**************************************************************************************************************************************************************


**************************************************************************************************************************************************************
**************************************************************************************************************************************************************
***************************************************************************************************************************************************************/

**Sim, está absolutamente certo.** Essa é exatamente a técnica que os sistemas operacionais clássicos (MS-DOS, Atari TOS, AmigaOS, CP/M) usavam para permitir expansão de sistema sem tocar na ROM.

Ao colocar a **Jump Table** na RAM, você ganha três grandes vantagens:

---

### 1. Vantagens do Modelo em RAM

1. **Extensibilidade sem regravar ROM:** Seu programa carregado (via serial, SD/disco) pode registrar novas chamadas na tabela durante a execução.
2. **Hooking / Interceptação:** Você pode interceptar chamadas existentes. Se quiser criar um driver de log ou um depurador, basta salvar o ponteiro antigo e colocar o seu no lugar na RAM.
3. **Atualizações de Drivers:** Se corrigir um bug na rotina de vídeo ou disco, basta carregar a versão nova na RAM e atualizar o ponteiro na tabela.

---

### 2. Estrutura Recomendada na RAM

Para manter o dispatcher flexível, guarde na RAM tanto a **Tabela de Ponteiros** quanto o **Limite Máximo de Funções**:

#### Mapeamento de Memória (Exemplo na RAM)

* `0x00001000`: `SYSCALL_LIMIT` (word - Ex: guarda a quantidade de chamadas ativas)
* `0x00001002`: `SYSCALL_TABLE` (array de longwords - ponteiros para as rotinas)

#### Dispatcher na ROM

```assembly
trap_dispatcher:
    ; Valida contra o limite dinâmico armazenado na RAM
    move.w  (SYSCALL_LIMIT_RAM), d7
    cmp.w   d7, d0
    bhs     sys_bad_id          ; ID maior ou igual ao limite atual

    ; Calcula offset e busca o ponteiro na RAM
    move.l  d0, d7
    lsl.w   #2, d7              ; ID * 4 bytes
    lea     SYSCALL_TABLE_RAM, a6
    move.l  (a6,d7.w), a6       ; Carrega endereço salvo na RAM
    
    jsr     (a6)                ; Salta para a função
    rte

sys_bad_id:
    moveq   #-1, d0
    rte

```

---

### 3. Cuidados Práticos (Boas Práticas)

* **Inicialização no Boot:** Ao ligar o sistema, sua ROM copia a tabela inicial com as funções básicas para a RAM e preenche os slots não usados com o ponteiro de uma função genérica (ex: `sys_unimplemented`).
* **Reserva de Espaço:** Aloque espaço suficiente na RAM para a tabela crescer (ex: reserve 256 bytes para até 64 funções).
* **Registro de Funções:** Crie uma syscall interna `sys_register(id, handler_ptr)` para que programas possam adicionar ou alterar entradas na tabela com segurança.





/***************************************************************************************************************************************************************
**************************************************************************************************************************************************************
**************************************************************************************************************************************************************


**************************************************************************************************************************************************************
**************************************************************************************************************************************************************
***************************************************************************************************************************************************************/









Para não travar na escolha, o segredo é começar pelo modelo mais simples e modular:

1. Começo Simples (Fase 1)
Apenas 1 TRAP (TRAP #0).

Jump Table na RAM copiada da ROM no boot com as funções essenciais:

0: Console Out (printf)

1: Console In (getchar)

2: Read Sector (Disco/SD)

3: Write Sector (Disco/SD)

2. Expansão Futura (Fase 2)
Quando precisar de chamadas de vídeo ou drivers novos, basta registrar a nova função na tabela da RAM.

Começando com esse modelo de vetor único com tabela na RAM, você garante simplicidade imediata e flexibilidade total para o futuro sem ter que refazer o código da ROM.










CP/M-68K  Sim, existe uma enorme quantidade de software preservada e pronta para uso. 
A própria Digital Research liberou o código e os binários do CP/M para uso não comercial.  
The Unofficial CP/M Web Site (cpm.z80.de): É o quartel-general. Lá você encontra:  
Binários do Sistema: Imagens de boot e arquivos do CP/M-68K (v1.1, v1.2, v1.3).  
Compilador C Oficial: O Alcyon C (compilador, assembler, 
linker e runtime libs para o 68000 rodando sob CP/M).  
Linguagens e Ferramentas: CBASIC compilado para 68000, FORTH-83, 
editores de texto como MicroEMACS, e utilitários de transferência 
via serial (Kermit).Emulador 8080 (COM2X): Um utilitário que roda 
no 68000 e emula a CPU Intel 8080 em tempo real, permitindo rodar 
os programas de 8 bits do CP/M clássico diretamente na sua máquina 
de 16/32 bits.GitHub (Homebrew Community): A comunidade de hardware 
retro (projetos como S-100 68K e placas minimalistas) mantém vários 
repositórios com o BIOS do CP/M-68K reescrito em C/Assembly limpo. 
Basta você alterar as funções de I/O de baixo nível (co, ci, disk_read) 
para casar com o seu hardware e o CP/M sobe direto.


A escolha é perfeita para um hardware *homebrew*: o **CP/M-68K** é modular, não exige MMU e você só precisa escrever o driver da sua própria placa (**a BIOS**).

O sistema é dividido em três partes:

1. **CCP (Console Command Processor):** O prompt/shell (genérico, fornecido pronto).
2. **BDOS (Basic Disk Operating System):** O Kernel e sistema de arquivos (genérico, fornecido pronto).
3. **BIOS (Basic Input/Output System):** A única parte que **você escreve** para o seu hardware.

---

## Passo a Passo para o Kickstart

### 1. Entender a Tabela de Saltos da BIOS

O BDOS chama o seu hardware através de uma **Jump Table** no início do código da BIOS. Você só precisa implementar essas rotinas em Assembly m68k:

```assembly
; bios.S - Estrutura básica da BIOS do CP/M-68K
    .text
    .global _bios_entry

_bios_entry:
    bra.w   bios_init     ; 0: Cold Start
    bra.w   bios_wboot    ; 1: Warm Boot
    bra.w   bios_const    ; 2: Status do teclado (retorna FF se tem tecla)
    bra.w   bios_conin    ; 3: Leitura de caractere
    bra.w   bios_conout   ; 4: Escrita de caractere
    bra.w   bios_lstout   ; 5: Impressora (pode ser dummy)
    bra.w   bios_punch    ; 6: Serial Raw out (dummy)
    bra.w   bios_reader   ; 7: Serial Raw in (dummy)
    bra.w   bios_home     ; 8: Vai para Trilha 0
    bra.w   bios_seldsk   ; 9: Seleciona drive (A:, B:, etc)
    bra.w   bios_settrk   ; 10: Define Trilha
    bra.w   bios_setsec   ; 11: Define Setor
    bra.w   bios_setdma   ; 12: Define endereço do Buffer RAM
    bra.w   bios_read     ; 13: Lê 128 bytes do disco
    bra.w   bios_write    ; 14: Escreve 128 bytes no disco

```

---

### 2. O Roteiro de Desenvolvimento

1. **Fase 1 (Console Mínimo):**
* Escreva as rotinas `bios_conin`, `bios_conout` e `bios_const` apontando para a sua **UART / Serial**.
* Faça as rotinas de disco apenas retornarem sucesso (dummy) para testar se o prompt do CP/M abre na serial.


2. **Fase 2 (Abstração de Disco/SD):**
* O CP/M trabalha com blocos lógicos de **128 bytes** (setores CP/M).
* Nas funções `SETTRK`, `SETSEC` e `SELDSK`, você calcula o setor físico (LBA) do seu Cartão SD/IDE.
* Na função `READ`/`WRITE`, transfere os 128 bytes entre o hardware e o endereço salvo por `SETDMA`.


3. **Fase 3 (Bootloader na ROM):**
* A ROM do seu 68000 carrega o arquivo `CPM.SYS` (que é o CCP + BDOS + a sua BIOS linkados) da Serial ou do SD para a RAM em `0x00002000`.
* Salta para o endereço de `bios_init` e o CP/M-68K assume o controle da máquina.



---

### Onde pegar os binários base do BDOS/CCP?

Você baixa o pacote oficial `CPM68K.ZIP` (contendo o `BDOS.REL` e `CCP.REL`) do site **cpm.z80.de**. 
Você junta o seu `bios.o` com esses arquivos usando o linker do GCC ou do próprio kit do CP/M para 
gerar a imagem `CPM.SYS`.

O próximo passo é **juntar o seu `cpm_bios.o` com o núcleo do CP/M-68K** (o CCP e o BDOS) para gerar a imagem final (`CPM.SYS` ou um binário bruto `.bin`) que será carregada na RAM.

Aqui está o roteiro do que fazer agora:

---

## 1. Obter os Módulos do CCP e BDOS

O CP/M-68K precisa dos seus dois outros componentes compilados:

* **`CCP`** (Console Command Processor - o terminal/prompt)
* **`BDOS`** (Basic Disk Operating System - a lógica do sistema)

Se estiver usando o repositório clássico ou uma conversão ELF para GCC, você terá os arquivos `ccp.o` e `bdos.o` (ou `cpm3.rel` / `bdos.rel`).

---

## 2. Criar um Linker Script (`linker.ld`)

Você precisa dizer ao `m68k-elf-ld` onde essa imagem vai ficar residente na RAM (por exemplo, no endereço `0x00002000` para deixar a RAM baixa livre para os vetores de exceção do 68000).

Crie o arquivo `linker.ld`:

```linker-script
MEMORY
{
    RAM (rwx) : ORIGIN = 0x00002000, LENGTH = 256K
}

SECTIONS
{
    .text : {
        *(.text.bios_entry)  /* Força a Jump Table da BIOS a ficar no topo */
        *(.text*)
        *(.rodata*)
    } > RAM

    .data : {
        *(.data*)
    } > RAM

    .bss : {
        *(.bss*)
        *(COMMON)
    } > RAM
}

```

---

## 3. Linkar tudo e gerar o Binário Bruto

Edite seu `Makefile` para adicionar as regras de linkagem e conversão para binário puro:

```makefile
# Chama o linker para juntar BIOS + CCP + BDOS
cpm.elf: cpm_bios.o ccp.o bdos.o
	m68k-elf-ld -T linker.ld cpm_bios.o ccp.o bdos.o -o cpm.elf

# Extrai o arquivo binário bruto (flat binary) para subir na RAM
cpm.bin: cpm.elf
	m68k-elf-objcopy -O binary cpm.elf cpm.bin

```

---

## 4. O Teste de Carga (Sua ROM)

Com o `cpm.bin` gerado:

1. **Via Serial:** Crie uma rotina simples na sua ROM para receber o `cpm.bin` via XMODEM/RAW Serial e gravar na RAM em `0x00002000`.
2. **Execução:** A ROM salta para `0x00002000` (que cai direto no `bra.w bios_init` da sua Jump Table).
3. **Resultado:** Se as funções `bios_conin` e `bios_conout` estiverem corretas na sua UART, a tela deve exibir o prompt clássico:
```text
CP/M-68K(tm) V1.3
Copyright (c) 1983 Digital Research
A> 

```


m68k-elf-gcc -std=gnu89 -fno-builtin -I. -c bdosread.c -o bdosread.o