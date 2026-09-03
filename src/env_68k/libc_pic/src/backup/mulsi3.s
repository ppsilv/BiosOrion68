| =====================================================================
| Suporte Bare-Metal para o Orion68K
| Função: __mulsi3 (Multiplicação de 32 bits para m68k-elf-as)
| =====================================================================

.global __mulsi3
.text
.align 2

__mulsi3:
    | Na ABI do GCC para 68k (Sintaxe GNU):
    | Os argumentos estão em 4(%sp) e 8(%sp)
    move.l  4(%sp), %d0       | %d0 = A
    move.l  8(%sp), %d1       | %d1 = B

    | Se qualquer um for zero, o resultado é zero
    tst.l   %d0
    beq     .fim
    tst.l   %d1
    beq     .zero_out

    | Salva os registradores de rascunho na pilha
    move.l  %d2, -(%sp)       
    move.l  %d3, -(%sp)       

    move.l  %d0, %d2          | %d2 = multiplicando
    move.l  %d1, %d3          | %d3 = multiplicador
    moveq   #0, %d0           | %d0 = acumulador (resultado)

.loop:
    lsr.l   #1, %d3           | Desloca o LSB de B para o Carry
    bcc     .no_add           | Se o bit for 0, pula a adição

    add.l   %d2, %d0          | Se o bit for 1, acumula (aqui estava o '=' errado!)

.no_add:
    lsl.l   #1, %d2           | Multiplicando = Multiplicando << 1
    tst.l   %d3               | O multiplicador zerou?
    bne     .loop             | Se não, continua o laço

    | Restaura os registradores na ordem inversa
    move.l  (%sp)+, %d3
    move.l  (%sp)+, %d2

.fim:
    rts

.zero_out:
    moveq   #0, %d0
    rts