;Uart register offsets
UART_BASE   equ     $FF4000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Initialize uart
; Change A1
; Parametros: none
; ----------------------------------------------------------------------
;;InitUart:
;;        move.l  currentUart,A1
;;        move.l  currentBaudRate,D0
;;        move.b  #%10000011,LCR(A1) ;DLAB=1
;;        move.b  #$08,DLL(A1)       ; set divisor latch low byte
;;        move.b  #$00,DLM(A1)       ; set divisor latch high byte
;;        move.b  #%00000011,LCR(A1) ; 8 data bits, no parity, 1 stop bit, DLAB=0
;;        move.b  #%00001101,FCR(A1) ; enable FIFO
;;        clr.b   SCR(A1)            ; clear the scratch register
;;        RTS
InitUart1:
        move.l  currentUart,A1
        move.l  currentBaudRate,D0
        move.b  #%00001101,FCR(a1)      ; enable FIFO
        move.b  #%10000011,LCR(a1)      ; 8 data bits, no parity, 1 stop bit, DLAB=1
        move.b  D0,DLL(A1)              ; Byte 0 (LSB) -> DLL
        lsr.l   #8,D0                   ; Shift right 8 bits
        move.b  D0,DLM(A1)              ; Byte 1 (agora no LSB) -> DLM
        bclr.b  #7,LCR(a1)              ; 8 data bits, no parity, 1 stop bit, DLAB=0
        clr.b   SCR(a1)                 ; clear the scratch register
        RTS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Write char
; Change A1
; Receive char in D0
UartWriteChar:
        MOVE.L A1,-(SP)
        move.l  currentUart,A1
.WaitTx:
        btst    #5,LSR(A1)      ; wait until transmit holding register is empty
        beq     .WaitTx
        move.b  D0,THR(A1)      ; transmit byte
        MOVE.L (SP)+,A1
        RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Read char block
; Change A1
;
; Return char in D0)
UartReadChar:
        MOVE.L A1,-(SP)
        move.l  currentUart,A1
.WaitRx:
        btst    #0,LSR(A1)        ; RX ready?
        beq     .WaitRx

        move.b  RHR(A1),D0
        MOVE.L (SP)+,A1
        RTS

UartFlush:
        MOVE.L A1,-(SP)
        MOVE.L  currentUart,A1
.WaitRx:
        btst    #0,LSR(A1)        ; RX ready?
        BNE     .read
        MOVE.L (SP)+,A1
        RTS
.read:
        BRA     .WaitRx

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Read char block with echo
; Change A1
;
; Return char in D0)
DelUartReadCharEcho:
        MOVE.L A1,-(SP)
        move.l  currentUart,A1
.WaitRx:
        btst    #0,LSR(A1)        ; RX ready?
        beq     .WaitRx
        move.b  RHR(A1),D0

.WaitTx:
        btst    #5,LSR(A1)      ; wait until transmit holding register is empty
        beq     .WaitTx
        move.b  D0,THR(A1)      ; transmit byte
        MOVE.L (SP)+,A1
        RTS

DelUartReadCharEcho2:
        MOVE.L A1,-(SP)
        move.l  currentUart,A1
.WaitRx:
        btst    #0,LSR(A1)        ; RX ready?
        beq     .WaitRx
        move.b  RHR(A1),D0

        JSR     PrintByteHex

        MOVE.L (SP)+,A1
        RTS

; ----------------------------------------------------------------------
; UART_WriteString - Envia string terminada em null para UART
; Entrada:
;   A0 = Ponteiro para a string (endereço da string)
; ----------------------------------------------------------------------
DelUartWriteString:
        MOVE.L  A0,-(SP)      ; Preserva D0
        MOVE.L  D0,-(SP)      ; Preserva D0
.WriteLoop:
        MOVE.B  (A0)+,D0      ; Pega caractere
        BEQ     .Done
        JSR     WriteConout ; Use sua rotine existente
        BRA     .WriteLoop
.Done:
        MOVE.L  (SP)+,D0
        MOVE.L  (SP)+,A0
        RTS

; Lê número hexadecimal (retorna em D0)
DelUartReadHex:
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
        JSR     PicoPrintHexAddress
        JSR     NewLine
        MOVE.L  (SP)+,D2
        MOVE.L  (SP)+,D1
        RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; UartPrintHexAddress - Imprime endereço de 32 bits
; Entrada: D0 = endereço
;
DelUartPrintHexAddress:
        SWAP    D0                ; Imprime parte alta primeiro
        JSR     .PrintWordHex
        SWAP    D0                ; Parte baixa
.PrintWordHex:
        ROL.W   #8,D0             ; Byte mais significativo primeiro
        JSR     PicoPrintByteHex
        ROR.W   #8,D0             ; Byte menos significativo
DelUartPrintByteHex:
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

