
; ----------------------------------------------------------------------
; PrintHexAddress - Imprime endereço de 32 bits
; Entrada: D0 = endereço
; ----------------------------------------------------------------------
PrintHexAddress:
        MOVE.L  D1,-(SP)
        SWAP    D0                ; Imprime parte alta primeiro
        JSR     PrintWordHex
        MOVE.L  D1,D0
        SWAP    D0                ; Parte baixa
        JSR     PrintWordHex
        MOVE.L (SP)+,D1
        RTS
        
PrintWordHex:
        MOVE.L  D0,D1
        ROR     #8,D0             ; Byte mais significativo primeiro
        JSR     PrintByteHex
        MOVE.L  D1,D0             ; Byte menos significativo
        JSR     PrintByteHex
        RTS

PrintByteHex:
        MOVE.B  D0,-(SP)          ; 1. Guarda o byte original na pilha
        ; --- PROCESSA O PRIMEIRO NIBBLE (ALTO) ---
        LSR.B   #4,D0             ; Isola o nibble alto (ex: $A4 -> $0A)
        BSR     .PrintNibble      ; Executa a subrotina para o primeiro dígito
        ; --- PROCESSA O SEGUNDO NIBBLE (BAIXO) ---
        MOVE.B  (SP)+,D0          ; 2. Recupera o byte original da pilha
        ANDI.B  #$0F,D0           ; Isola o nibble baixo (ex: $A4 -> $04)
        BSR     .PrintNibble      ; Executa a subrotina para o segundo dígito
        RTS

.PrintNibble:
        MOVE.B  D0,-(SP)          ; Salva D0 para não destruir o valor na conversão        
        CMP.B   #9,D0
        BLS     .Decimal          ; Se for menor ou igual a 9, vai direto
        ADD.B   #7,D0             ; Ajuste para letras 'A' até 'F'
.Decimal:
        ADD.B   #'0',D0           ; Converte o valor bruto em caractere ASCII
        JSR     WriteConout       ; Chama a sua rotina de exibição na tela/serial
        MOVE.B  (SP)+,D0          ; Restaura o D0 original deste nibble
        RTS
