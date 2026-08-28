
;=== Uart receiver ================= CONSTANTES =================
SOH         EQU     $01        ; Start Of Header
EOT         EQU     $04        ; End Of Transmission
ACK         EQU     $06        ; Acknowledge
NACK        EQU     $15        ; Negative Acknowledge
CAN         EQU     $18        ; Cancel

XmodemRec:
        BCLR    #PROGRAM_LOADED,flg_system
        LEA     buf_pgm,A3
        clr.l     D5
        JSR     UartFlush
        MOVE.B  #NACK,D0
        JSR     UartWriteChar
        ;MOVE.B  #13,D0
        ;JSR     WriteConout      
        ;MOVE.B  #10,D0
        ;JSR     WriteConout      
.receivePacote:
        JSR     ReadConin
        CMP.B   #EOT,D0
        BEQ     .Transfer_Complete   ; Fim da transmissão          
        ;JSR     PrintByteHexConout      
        JSR     ReadConin
        ;JSR     PrintByteHexConout      
        JSR     ReadConin
        ;JSR     PrintByteHexConout         
        MOVE.W  #$7F,D1
        LEA     buf_xmodem,A1
.centoEvinteOito:
        JSR     ReadConin
        add     D0,D5
        MOVE.B  D0,(A1)+                
        ;JSR     PrintByteHexConout      
        dbra    d1,.centoEvinteOito
        ;Aguardando check sum
        JSR     ReadConin

        ;MOVE.B  D0,-(SP)   
        ;JSR     PrintByteHexConout              
        ;MOVE.B  D5,D0
        ;JSR     PrintByteHexConout 
        ;MOVE.B  (SP)+,D0 

        CMP.B   D0,D5
        BNE     .XMIT_NACK    
        MOVE.W  #32-1,D0          ; Contador para 32 Longs (128 bytes)Subtraímos 1 porque o DBRA para no -1
        LEA     buf_xmodem,A1
.CopyLoop:
        MOVE.L  (A1)+,(A3)+       ; Copia 4 bytes de (A1) para (A3) e avança ambos
        DBRA    D0,.CopyLoop      ; Decrementa D0 e pula para .CopyLoop se D0 >= 0
        MOVE.B  #ACK,D0
        JSR     UartWriteChar 
        ;JSR     NewLine  
        clr.l     D5
        jmp     .receivePacote
.XMIT_NACK
        MOVE.B  #NACK,D0
        JSR     UartWriteChar 
        ;JSR     NewLine                
        clr.l     D5
        jmp     .receivePacote
.Transfer_Complete:
        BSET    #PROGRAM_LOADED,flg_system
        RTS ;
