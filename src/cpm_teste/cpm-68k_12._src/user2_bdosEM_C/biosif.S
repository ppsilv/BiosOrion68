| ====================================================================
| biosif.s - Interface BDOS <-> BIOS para CP/M-68K (GCC / GAS)
| ====================================================================

    .text
    .even

    .globl bios1
    .globl bios2
    .globl bios3
    .globl bios4
    .globl bios5

| No CP/M-68K, todas as chamadas de BIOS passam a pilha para o TRAP #13.
| O número da função e os parâmetros já foram empilhados pelo C.

bios1:
bios2:
bios3:
bios4:
bios5:
    trap    #13
    rts