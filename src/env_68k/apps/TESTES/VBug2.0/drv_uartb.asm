;Uart register offsets
UARTB_BASE   equ     $FF4100


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Initialize uart
; Change A1
; Parametros: none
; ----------------------------------------------------------------------
InitUartb:
        move.l  #UARTB_BASE,A1
        move.l  currentBaudRate,D0
        move.b  #%10000011,LCR(A1) ;DLAB=1
        move.b  #$08,DLL(A1)       ; set divisor latch low byte
        move.b  #$00,DLM(A1)       ; set divisor latch high byte
        move.b  #%00000011,LCR(A1) ; 8 data bits, no parity, 1 stop bit, DLAB=0
        move.b  #%00001101,FCR(A1) ; enable FIFO
        clr.b   SCR(A1)            ; clear the scratch register
        RTS
InitUartb1:
        move.l  #UARTB_BASE,A1
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
UartbWriteChar:
        MOVE.L A1,-(SP)
        move.l  #UARTB_BASE,A1
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
UartbReadChar:
        MOVE.L A1,-(SP)
        move.l  #UARTB_BASE,A1
.WaitRx:
        btst    #0,LSR(A1)        ; RX ready?
        beq     .WaitRx

        move.b  RHR(A1),D0
        MOVE.L (SP)+,A1
        JSR     PrintByteHexConout
        RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Read char block with echo
; Change A1
;
; Return char in D0)
UartbReadCharEcho:
        MOVE.L A1,-(SP)
        move.l  #UARTB_BASE,A1
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

; ----------------------------------------------------------------------
; UART_WriteString - Envia string terminada em null para UART
; Entrada:
;   A0 = Ponteiro para a string (endereço da string)
; ----------------------------------------------------------------------
UartbWriteString:
        MOVE.L  A0,-(SP)      ; Preserva D0
        MOVE.L  D0,-(SP)      ; Preserva D0
.WriteLoop:
        MOVE.B  (A0)+,D0      ; Pega caractere
        BEQ     .Done
        JSR     UartbWriteChar ; Use sua rotine existente
        BRA     .WriteLoop
.Done:
        MOVE.L  (SP)+,D0
        MOVE.L  (SP)+,A0
        RTS

