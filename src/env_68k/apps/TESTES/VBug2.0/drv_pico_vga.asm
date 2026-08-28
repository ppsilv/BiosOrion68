



;;picoVGA Register address
PICO_BASE_ADDR equ $FF8000
WRITE_SCREEN   equ $FF8001        ;;Endereço real  3 0x03  o pico enxerga 0x00
REG_02         equ $FF8003        ;;Endereço real  3 0x03  o pico enxerga 0x01
REG_03         equ $FF8005        ;;Endereço real  5 0x05  o pico enxerga 0x02
REG_04         equ $FF8007        ;;Endereço real  7 0x07  o pico enxerga 0x03
CONFIG_REG     equ $FF8009        ;;Endereço real  9 0x09  o pico enxerga 0x04
REG_06         equ $FF800B        ;;Endereço real 11 0x0b  o pico enxerga 0x05
REG_07         equ $FF800D        ;;Endereço real 13 0x0d  o pico enxerga 0x06
REG_08         equ $FF800F        ;;Endereço real 15 0x0f  o pico enxerga 0x07
;Begin Register for reads
REG_09         equ $FF8011        ;;Endereço real 17 0x11  o pico enxerga 0x08
REG_0A         equ $FF8013        ;;Endereço real 19 0x13  o pico enxerga 0x09
REG_0B         equ $FF8015        ;;Endereço real 21 0x15  o pico enxerga 0x0A
REG_0C         equ $FF8017        ;;Endereço real 23 0x17  o pico enxerga 0x0B
REG_0D         equ $FF8019        ;;Endereço real 25 0x19  o pico enxerga 0x0C
REG_0E         equ $FF801B        ;;Endereço real 27 0x1b  o pico enxerga 0x0D
REG_0F         equ $FF801D        ;;Endereço real 29 0x1d  o pico enxerga 0x0E
REG_10         equ $FF801F        ;;Endereço real 31 0x1f  o pico enxerga 0x0F
;End
REG_11         equ $FF8021        ;;Endereço real 33 0x21  o pico enxerga 0x10
REG_12         equ $FF8023        ;;Endereço real 35 0x23  o pico enxerga 0x11
REG_13         equ $FF8025        ;;Endereço real 37 0x25  o pico enxerga 0x12
SET_MODE       equ $FF8027        ;;Endereço real 39 0x27  o pico enxerga 0x13 (0=Texto+Scroll, 1=Texto Fixo, 2=320x200, 3=640x200)
SET_TXT_COLOR  equ $FF8029        ;;Endereço real 41 0x29  o pico enxerga 0x14
CHANGE_CUR_POS equ $FF802B        ;;Endereço real 43 0x2b  o pico enxerga 0x15
REG_X_HIGH     equ $FF802D        ;;Endereço real 45 0x2d  o pico enxerga 0x16
REG_X_LOW      equ $FF802F        ;;Endereço real 47 0x2f  o pico enxerga 0x17
REG_Y_HIGH     equ $FF8031        ;;Endereço real 49 0x31  o pico enxerga 0x18
REG_Y_LOW      equ $FF8033        ;;Endereço real 51 0x33  o pico enxerga 0x19
CHANGE_BUFFER  equ $FF8035        ;;Endereço real 53 0x35  o pico enxerga 0x1A
SELECT_SCREEN  equ $FF8037        ;;Endereço real 55 0x37  o pico enxerga 0x1B
SET_HORIZONTAL equ $FF8039        ;;Endereço real 57 0x39  o pico enxerga 0x1C
SET_VERTICAL   equ $FF803B        ;;Endereço real 59 0x3b  o pico enxerga 0x1D
RUN_CMD        equ $FF803D        ;;Endereço real 61 0x3d  o pico enxerga 0x1E
CORINGA        equ $FF803F        ;;Endereço real 63 0x3f  o pico enxerga 0x1F    

;Commands
CMD_SYSTEM_ENABLE equ  $A5
CMD_CLEAR_SCREEN  equ  $A4
CMD_SET_CUR_POS   equ  $A3
CMD_SET_TXT_COLOR equ  $A2
CMD_GO_HOME       equ  $A1

InitPicoVga:
        RTS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Cursor go home
; Receive cmd in D0
;
PicoRunCmd:
        MOVE.B  D0,RUN_CMD
        RTS


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Cursor go home
; 
PicoGoHome:
        MOVE.B  #CMD_GO_HOME,D0
        MOVE.B  D0,RUN_CMD
        RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Set cursor position
; Detroy:
; Receive X in D0 high
; Receive Y in D0 low
PicoSetCursor:
        MOVE.W D0,D1
        SWAP    D0
        MOVE.L A1,-(SP)
        MOVE.B  D0,REG_X_LOW
        NOP
        NOP
        NOP
        NOP
        NOP
        LSR.W   #8,D0             ; Desloca o byte alto para a posição baixa em D0
        MOVE.B  D0,REG_X_HIGH
        NOP
        NOP
        NOP
        NOP
        NOP
        MOVE.B  D1,REG_Y_LOW
        NOP
        NOP
        NOP
        NOP
        NOP
        LSR.W   #8,D1             ; Desloca o byte alto para a posição baixa em D0
        MOVE.B  D1,REG_Y_HIGH
        NOP
        NOP
        NOP
        NOP
        NOP
        LEA     RUN_CMD,A1    
        MOVE.B  #CMD_SET_CUR_POS,D0
        MOVE.B  D0,(A1);
        NOP
        NOP
        NOP
        NOP
        NOP
        MOVE.L (SP)+,A1
        RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Clear screen
; Destroy: A1 
; Command in D0
PicoClearScreen:
        MOVE.L A1,-(SP)
        LEA     RUN_CMD,A1    
        MOVE.B  #CMD_CLEAR_SCREEN,D0
        MOVE.B  D0,(A1);
        MOVE.L (SP)+,A1
        RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Write char in picoVGA
; Destroy: A1,D1
PicoWriteChar:
        MOVE.L  A1,-(SP)
        MOVE.L  D1,-(SP)
        MOVE.L  D0,-(SP)
        LEA     WRITE_SCREEN,A1
        MOVE.B  D0,(A1)     ; Escreve no endereço do PicoVGA (LDS ativo)
        MOVE.L  #$3F,D1    ; Contador para o delay (ajuste se precisar de mais)
.DELAY00:
        SUBQ.L  #1,D1       ; Subtrai 1 de D1 (4 ciclos)
        BNE.S   .DELAY00    ; Pula se não for zero (10 ciclos se pular, 8 se não)
        MOVE.L (SP)+,D0
        JSR     UartWriteChar
        MOVE.L (SP)+,D1
        MOVE.L (SP)+,A1

        RTS
;last change
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Imprime string terminada em ZERO
; Destroy: A0 = String's address
;
PicoPrintString:
        MOVE.B  (A0)+,D0                ; Pega caractere e avança ponteiro
        BEQ.S   .Done                   ; Se for zero, termina
        JSR     WriteConout           ; Chama sua função gloriosa!
        BRA.S   PicoPrintString         ; Loop para o próximo
.Done:
        RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PrintHexAddress - Imprime endereço de 32 bits
; Entrada: D0 = endereço
;
PicoPrintHexAddress:
        SWAP    D0                ; Imprime parte alta primeiro
        JSR     .PrintWordHex
        SWAP    D0                ; Parte baixa
.PrintWordHex:
        ROL.W   #8,D0             ; Byte mais significativo primeiro
        JSR     PicoPrintByteHex
        ROR.W   #8,D0             ; Byte menos significativo
        RTS

PicoPrintByteHex:
        MOVE.B  D0,-(SP)          ; Salva byte original
        LSR.B   #4,D0             ; Nibble alto
        BSR     .PrintNibble1
.PrintNibble2
        MOVE.B  (SP)+,D0          ; Recupera byte
        ANDI.B  #$0F,D0           ; Nibble baixo
        CMP.B   #9,D0
        BLS     .Decimal2
        ADD.B   #7,D0             ; Ajuste para A-F
.Decimal2:
        ADD.B   #'0',D0
        JSR     PicoWriteChar    ; Usa JMP para tail call optimization
        RTS
.PrintNibble1:
        CMP.B   #9,D0
        BLS     .Decimal
        ADD.B   #7,D0             ; Ajuste para A-F
.Decimal:
        ADD.B   #'0',D0
        JSR     PicoWriteChar    ; Usa JMP para tail call optimization
        BRA.S   .PrintNibble2
