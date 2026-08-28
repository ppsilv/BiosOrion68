
FlushConin:

ReadConin:
    MOVE.L  (cconin),A5
    JSR     (A5)
    ANDI.L  #$FF,D0       ; Mantém apenas o byte inferior
    BTST    #ECHO_ON,flg_system
    BEQ     .fim        
    MOVE.B  D0,-(SP)          ; Salva byte original
    JSR     WriteConout    
    MOVE.B  (SP)+,D0          ; Recupera byte
.fim
    RTS

WriteConout:
    MOVE.L  (cconout),A5        ; Busca o endereço da rotina física da UART
    JSR     (A5)
    RTS     

WriteStringConout:
        MOVE.B  (A0)+,D0                ; Pega caractere e avança ponteiro
        BEQ.S   .Done                   ; Se for zero, termina
        JSR     WriteConout           ; Chama sua função gloriosa!
        BRA.S   WriteStringConout         ; Loop para o próximo
.Done:
        RTS    
ReadHexAddressConin:
        MOVE.L  D1,-(SP)
        MOVE.L  D2,-(SP)

        MOVEQ   #0,D0
        MOVEQ   #28,D1            ; Máximo 8 dígitos
        MOVEQ   #0,D2            ; Resultado em D2
.Loop:
        CLR.L   D0
        JSR     ReadConin
        CMP.B   #13,D0
        BEQ     .Done
        CMP.B   #10,D0
        BEQ     .Done
        ;Tratando os numeros
        CMP.B   #'0',D0
        BGE     .maiorQueZero
        BRA     .Loop
.maiorQueZero:
        CMP.B   #'9',D0
        BLE     .isdigit
        ;Tratando as letras
        CMP.B   #'A',D0
        BGE     .maiorQueA
        BRA     .Loop
.maiorQueA:
        CMP.B   #'F',D0
        BLE     .isletter
        BRA     .Loop
.isdigit:
        SUB.B   #'0',D0
        LSL.L   D1,D0
        SUB.B   #4,D1
        OR.L    D0,D2
        LEA     addressInHex,A0
        MOVE.L  D2,(A0)
        BRA     .Loop
.isletter:
        SUB.B   #$37,D0
        LSL.L   D1,D0
        SUB.B   #4,D1
        OR.L    D0,D2
        LEA     addressInHex,A0
        MOVE.L  D2,(A0)
        BRA     .Loop
.Done:
        JSR     NewLine
        LEA     addressInHex,A0
        MOVE.L  (A0),D0
        JSR     PrintHexAddressConout
        JSR     NewLine
        MOVE.L  (SP)+,D2
        MOVE.L  (SP)+,D1
        RTS

PrintHexAddressConout:
        SWAP    D0                ; Imprime parte alta primeiro
        JSR     .PrintWordHex
        SWAP    D0                ; Parte baixa
.PrintWordHex:
        ROL.W   #8,D0             ; Byte mais significativo primeiro
        JSR     PrintByteHexConout
        ROR.W   #8,D0             ; Byte menos significativo
PrintByteHexConout:
        MOVE.B  D0,-(SP)          ; Salva byte original
        LSR.B   #4,D0             ; Nibble alto
        BSR     .PrintNibble
        MOVE.B  (SP)+,D0          ; Recupera byte
        ANDI.B  #$0F,D0           ; Nibble baixo
.PrintNibble:
        CMP.B   #9,D0
        BLS     .Decimal
        ADD.B   #7,D0             ; Ajuste para A-F
.Decimal:
        ADD.B   #'0',D0
        JMP     WriteConout    ; Usa JMP para tail call optimization        

