; ----------------------------------------------------------------------
; MemDump - Imprime dump de memória formatado
; Entrada:
;   A0 = Endereço inicial (ex: $80000)
;   D0 = Quantidade de bytes (ex: 256)
; ----------------------------------------------------------------------
MemDump:
        JSR     PicoClearScreen

        LEA     MsgDumpHeader,A0
        JSR     WriteStringConout
        MOVE.L  (addressInHex),A0
        MOVE.L  A0,D0
        JSR     PrintHexAddress
        JSR     NewLine

        LEA     MsgDumpHeader1,A0
        JSR     WriteStringConout

        MOVE.L  (addressInHex),A0
        ; Calcula endereço final

DUMPLOOPMASTER:
        MOVE.W  #$0,D0
        SWAP    D0
        MOVE.W  #$5,D0
        JSR     PicoSetCursor
        
        CLR.L   D1
        MOVE.L  A0,D1
        ADDI.L  #$000000FF,D1             ; D1 = endereço final

DumpLoop:
        ; Nova linha a cada 16 bytes
        MOVE.L  A0,D0
        ANDI.L  #$0000000F,D0     ; Verifica se é início de linha
        BNE     NoNewLine

        ; Imprime endereço
        MOVE.L  A0,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        MOVE.B  #':',D0
        JSR     WriteConout
        MOVE.B  #' ',D0
        JSR     WriteConout

NoNewLine:
        ; Imprime byte em hex
        MOVE.B  (A0)+,D0
        JSR     PrintByteHex

        MOVE.B  #' ',D0
        JSR     WriteConout

        ; Verifica fim da linha (16 bytes)
        MOVE.L  A0,D0
        ANDI.L  #$0000000F,D0
        BNE     NoEndLine

        ; Imprime caracteres ASCII
        MOVE.B  #' ',D0
        JSR     WriteConout
        MOVE.B  #'|',D0
        JSR     WriteConout

        LEA     -16(A0),A1        ; Volta ao início da linha
        MOVEQ   #15,D2            ; 16 caracteres

AsciiLoop:
        MOVE.B  (A1)+,D0
        CMP.B   #32,D0            ; Verifica se é imprimível
        BLT     NonPrintable
        CMP.B   #$7E,D0
        BGT     NonPrintable

        JSR     WriteConout
        BRA     NextAscii

NonPrintable:
        MOVE.B  #'.',D0           ; Substitui não imprimíveis
        JSR     WriteConout

NextAscii:
        DBRA    D2,AsciiLoop

        MOVE.B  #'|',D0
        JSR     WriteConout
        MOVE.B  #13,D0            ; CR
        JSR     WriteConout
        MOVE.B  #10,D0            ; LF
        JSR     WriteConout

NoEndLine:
        ; Verifica fim do dump
        CMP.L   D1,A0
        BLS     DumpLoop

        ;JSR     PrintAx
        ;JSR     PrintDx

        MOVE.L  A0,-(SP)          ; Salva endereço atual
        LEA     MsgHitAnyKey,A0
        JSR     WriteStringConout
        JSR     ReadConin
        CMP.B   #$1B,D0
        BEQ     .fim

        MOVE.L  (SP)+,A0          ; Recupera endereço atual
        BRA     DUMPLOOPMASTER
.fim
        MOVE.L  (SP)+,A0          ; Recupera endereço atual
        RTS


PrintAx:
        move.l  A0,(debug_regs)
        move.l  A1,(debug_regs+4)
        move.l  A2,(debug_regs+8)
        move.l  A3,(debug_regs+12)
        move.l  A4,(debug_regs+16)
        move.l  A5,(debug_regs+20)
        move.l  A6,(debug_regs+24)
        move.l  A7,d0
        addq.l  #4,d0
        move.l  d0,(debug_regs+28)

        MOVE.L  D0,-(SP)          ; Salva endereço atual
        MOVE.L  A0,-(SP)          ; Salva endereço atual

        ;MOVE.W  #40,D0
        ;SWAP    D0
        ;MOVE.W  #$5,D0
        ;JSR     PicoSetCursor

        LEA     MsgA0,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA1,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+4),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA2,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+8),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA3,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+12),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        ;LEA     MsgNewLine,A0
        ;JSR     WriteStringConout

        LEA     MsgA4,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+16),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA5,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+20),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA6,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+24),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA7,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+28),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        MOVE.L  (SP)+,A0          ; Recupera endereço atual
        MOVE.L  (SP)+,D0          ; Recupera endereço atual

        RTS

PrintDx:
        MOVE.L  D0,-(SP)          ; Salva endereço atual
        MOVE.L  A0,-(SP)          ; Salva endereço atual
        ;D0
        LEA     MsgD0,A0
        JSR     WriteStringConout
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D1
        LEA     MsgD1,A0
        JSR     WriteStringConout
        MOVE.L  D1,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D2
        LEA     MsgD2,A0
        JSR     WriteStringConout
        MOVE.L  D2,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D3
        LEA     MsgD3,A0
        JSR     WriteStringConout
        MOVE.L  D3,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        ;LEA     MsgNewLine,A0
        ;JSR     WriteStringConout

        ;D4
        LEA     MsgD4,A0
        JSR     WriteStringConout
        MOVE.L  D4,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D5
        LEA     MsgD5,A0
        JSR     WriteStringConout
        MOVE.L  D5,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D6
        LEA     MsgD6,A0
        JSR     WriteStringConout
        MOVE.L  D6,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D7
        LEA     MsgD7,A0
        JSR     WriteStringConout
        MOVE.L  D7,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgNewLine,A0
        JSR     WriteStringConout

        
        MOVE.L  (SP)+,A0          ; Recupera endereço atual
        MOVE.L  (SP)+,D0          ; Recupera endereço atual

        RTS                

PrintAx2:
        MOVE.L  D0,-(SP)          ; Salva endereço atual
        MOVE.L  A0,-(SP)          ; Salva endereço atual

        move.l  A0,(debug_regs)
        move.l  A1,(debug_regs+4)
        move.l  A2,(debug_regs+8)
        move.l  A3,(debug_regs+12)
        move.l  A4,(debug_regs+16)
        move.l  A5,(debug_regs+20)
        move.l  A6,(debug_regs+24)
        move.l  A7,d0
        addq.l  #4,d0
        move.l  d0,(debug_regs+28)


        MOVE.W  #0,D0
        SWAP    D0
        MOVE.W  #17,D0
        JSR     PicoSetCursor
        LEA     MsgAntes,A0
        JSR     WriteStringConout

        LEA     MsgA0,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA1,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+4),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA2,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+8),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA3,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+12),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        ;LEA     MsgNewLine,A0
        ;JSR     WriteStringConout

        LEA     MsgA4,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+16),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA5,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+20),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA6,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+24),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA7,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+28),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        MOVE.L  (SP)+,A0          ; Recupera endereço atual
        MOVE.L  (SP)+,D0          ; Recupera endereço atual

        RTS

PrintDx2:
        MOVE.L  D0,-(SP)          ; Salva endereço atual
        MOVE.L  A0,-(SP)          ; Salva endereço atual
        ;D0
        LEA     MsgD0,A0
        JSR     WriteStringConout
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D1
        LEA     MsgD1,A0
        JSR     WriteStringConout
        MOVE.L  D1,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D2
        LEA     MsgD2,A0
        JSR     WriteStringConout
        MOVE.L  D2,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D3
        LEA     MsgD3,A0
        JSR     WriteStringConout
        MOVE.L  D3,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        ;LEA     MsgNewLine,A0
        ;JSR     WriteStringConout

        ;D4
        LEA     MsgD4,A0
        JSR     WriteStringConout
        MOVE.L  D4,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D5
        LEA     MsgD5,A0
        JSR     WriteStringConout
        MOVE.L  D5,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D6
        LEA     MsgD6,A0
        JSR     WriteStringConout
        MOVE.L  D6,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D7
        LEA     MsgD7,A0
        JSR     WriteStringConout
        MOVE.L  D7,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgNewLine,A0
        JSR     WriteStringConout

        
        MOVE.L  (SP)+,A0          ; Recupera endereço atual
        MOVE.L  (SP)+,D0          ; Recupera endereço atual

        RTS                        

PrintAx3:
        MOVE.L  D0,-(SP)          ; Salva endereço atual
        MOVE.L  A0,-(SP)          ; Salva endereço atual

        move.l  A0,(debug_regs)
        move.l  A1,(debug_regs+4)
        move.l  A2,(debug_regs+8)
        move.l  A3,(debug_regs+12)
        move.l  A4,(debug_regs+16)
        move.l  A5,(debug_regs+20)
        move.l  A6,(debug_regs+24)
        move.l  A7,d0
        addq.l  #4,d0
        move.l  d0,(debug_regs+28)


        MOVE.W  #0,D0
        SWAP    D0
        MOVE.W  #22,D0
        JSR     PicoSetCursor

        LEA     MsgDepois,A0
        JSR     WriteStringConout

        LEA     MsgA0,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA1,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+4),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA2,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+8),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA3,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+12),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        ;LEA     MsgNewLine,A0
        ;JSR     WriteStringConout

        LEA     MsgA4,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+16),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA5,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+20),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA6,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+24),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgA7,A0
        JSR     WriteStringConout
        MOVE.L  (debug_regs+28),D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        MOVE.L  (SP)+,A0          ; Recupera endereço atual
        MOVE.L  (SP)+,D0          ; Recupera endereço atual

        RTS

PrintDx3:
        MOVE.L  D0,-(SP)          ; Salva endereço atual
        MOVE.L  A0,-(SP)          ; Salva endereço atual
        ;D0
        LEA     MsgD0,A0
        JSR     WriteStringConout
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D1
        LEA     MsgD1,A0
        JSR     WriteStringConout
        MOVE.L  D1,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D2
        LEA     MsgD2,A0
        JSR     WriteStringConout
        MOVE.L  D2,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D3
        LEA     MsgD3,A0
        JSR     WriteStringConout
        MOVE.L  D3,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        ;LEA     MsgNewLine,A0
        ;JSR     WriteStringConout

        ;D4
        LEA     MsgD4,A0
        JSR     WriteStringConout
        MOVE.L  D4,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D5
        LEA     MsgD5,A0
        JSR     WriteStringConout
        MOVE.L  D5,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D6
        LEA     MsgD6,A0
        JSR     WriteStringConout
        MOVE.L  D6,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex
        ;D7
        LEA     MsgD7,A0
        JSR     WriteStringConout
        MOVE.L  D7,D0
        JSR     PrintHexAddress    ; Imprime 8 dígitos hex

        LEA     MsgNewLine,A0
        JSR     WriteStringConout

        
        MOVE.L  (SP)+,A0          ; Recupera endereço atual
        MOVE.L  (SP)+,D0          ; Recupera endereço atual

        RTS                        