        SECTION .vectors
        ORG     $00000000
        ; --- Vetores de Exceção do 68000 ---
        DC.L    $000FFFF0         ; SP inicial
        DC.L    Vbug2Start        ; PC inicial
        DC.L    SERVICE_BUS_ERR   ; Bus Error
        DC.L    SERVICE_ADDR_ERR  ; Address Error
        DC.L    SERVICE_ILLEGAL   ; Illegal Instruction
        DC.L    SERVICE_DIV0      ; Division by Zero
        DC.L    SERVICE_CHECK     ; CHK Instruction
        DC.L    SERVICE_TRAPV     ; TRAPV Instruction
        DC.L    SERVICE_PRIV      ; Privilege Violation
        DC.L    SERVICE_TRACE     ; Trace
        DC.L    SERVICE_LINE_A    ; Line A Emulator
        DC.L    SERVICE_LINE_F    ; Line F Emulator
        DC.L    DefaultHandler     
        DC.L    DefaultHandler     
        DC.L    DefaultHandler     
        DC.L    DefaultHandler     
        DC.L    DefaultHandler     
        DC.L    DefaultHandler     
        DC.L    DefaultHandler     
        DC.L    DefaultHandler     
        DC.L    DefaultHandler
        DC.L    DefaultHandler
        DC.L    DefaultHandler
        DC.L    DefaultHandler
        DC.L    SpuriousHandler    ; $60: Spurious handler
        DC.L    Int1Handler        ; $64: Level 1 Interrupt
        DC.L    Int2Handler        ; $68: Level 2 Interrupt
        DC.L    Int3Handler        ; $6C: Level 3 Interrupt
        DC.L    Int4Handler        ; $70: Level 4 Interrupt
        DC.L    Int5Handler        ; $74: Level 5 Interrupt
        DC.L    Int6Handler        ; $78: Level 6 Interrupt
        DC.L    Int7Handler        ; $7C: Level 7 Interrupt
        DC.L    Trap0Handler
        DC.L    Trap1Handler
        DC.L    Trap2Handler
        DC.L    Trap3Handler
        DC.L    Trap4Handler
        DC.L    Trap5Handler
        DC.L    Trap6Handler
        DC.L    Trap7Handler
        DC.L    Trap8Handler
        DC.L    Trap9Handler
        DC.L    TrapAHandler
        DC.L    TrapBHandler
        DC.L    TrapCHandler
        DC.L    TrapDHandler
        DC.L    TrapEHandler
        DC.L    TrapFHandler
        DCB.L   128,Universal_Trampoline

        SECTION .jumptable
        ORG     $0400
ROM_JUMPTABLE:
        ; BRA     UART_Init          ;        0x400
        ; BRA     UART_WriteConout     ;        0x404
        ; BRA     UART_ReadConin      ;        0x408
        ; BRA     UART_Select        ;        0x40C
        ; BRA     UART_Setbaudrate   ;        0x410
        ; BRA     DELAY_MS           ;        0x414
        ; BRA     MEMDUMP            ;        0x418
        ; BRA     UART_ReadConinNonEcho;       0x41C
        ; BRA     vbug2_start         ;       0x420
        ; BRA     warm_start          ;       0x424
        ; BRA     MenuLoop            ;       0x428
        ; BRA     NewLine            ;       0x42C
        ; BRA     UART_WriteStringConout    ;       0x430
        ; BRA     UART_INIT1          ;       0x434
; =============================================================================
; FIM DA TABELA DE VETORES (Endereço $000400 alcançado de forma contínua!)
; =============================================================================

ROMBASE           equ $00000
ROMSIZE           equ $3FFFF    ;256K WORDS
RAMBASE           equ $80000
RAMSIZE           equ $7FFFF    ;512K WORDS
USER_SP           equ $6F000
RAM_VECTOR_BASE   equ RAMBASE
RAM_VARIABLES     equ RAMBASE+$400
RAM_VECTOR_CODE   equ RAMBASE+RAM_VARIABLES+$400
RAM_USER_APP_AREA equ $82000

CMD_CCONIN        equ $1
CMD_CCONOUT       equ $2
CMD_CCONOUTSTR    equ $3
CMD_PTERM0        equ $0

;Memory map for vectors
;0x80000 -> 0x803FF - 1024bytes Ram vector table
;0x80400 -> 0x80800 - 1024bytes Ram variables
;0x80800 -> 0x80C00 - 5120bytes Ram vector code
;0x82000 -> 0x84000 - 8192bytes Ram user app area
        SECTION .text
        ORG $00001000

SERVICE_BUS_ERR:
        RTE
SERVICE_ADDR_ERR:
        RTE
SERVICE_ILLEGAL:
        RTE
SERVICE_DIV0:
        RTE
SERVICE_CHECK:
        RTE
SERVICE_TRAPV:
        RTE
SERVICE_PRIV:
        RTE
SERVICE_TRACE:
        RTE
SERVICE_LINE_A:
        RTE
SERVICE_LINE_F:
        RTE
DefaultHandler:
        MOVEM.L D0-D7/A0-A6,-(SP)
        ; --- IDENTIFICA O TIPO DE EXCEÇÃO, AGORA lê corretamente da pilha ---
        MOVE.L  (14,SP),D0      ; ⭐⭐ PC (14 bytes abaixo por causa dos registradores salvos)
        MOVE.W  (18,SP),D1      ; ⭐⭐ SR (18 bytes abaixo)
        MOVE.W  (20,SP),D2      ; ⭐⭐ Vector Offset (20 bytes abaixo)
        ; Mostra informações detalhadas
        LEA     MsgDebug,A0
        JSR     WriteStringConout
        MOVE.L  D0,D0           ; PC
        JSR     PrintHexAddressConout
        JSR     NewLine
        LEA     MsgSr,A0
        JSR     WriteStringConout
        MOVE.W  D1,D0           ; SR
        JSR     PrintHexAddressConout
        JSR     NewLine
        LEA     MsgVector,A0
        JSR     WriteStringConout
        MOVE.W  D2,D0           ; Vector offset
        JSR     PrintHexAddressConout
        JSR     NewLine
        ; --- ANALISA O VETOR ---
        LSR.W   #2,D2           ; Divide por 4 para get vector number
        ANDI.W  #$FF,D2         ; Vector number em D2
        LEA     MsgVectorNum,A0
        JSR     WriteStringConout
        MOVE.W  D2,D0
        JSR     PrintHexAddressConout
        JSR     NewLine
        ; --- MOSTRA QUAL EXCEÇÃO É ---
        CMP.W   #8,D2
        BNE     .not_bus_error
        LEA     MsgBusError,A0
        BRA     .show_msg
.not_bus_error:
        CMP.W   #9,D2
        BNE     .not_address_error
        LEA     MsgAddrError,A0
        BRA     .show_msg
.not_address_error:
        CMP.W   #10,D2
        BNE     .not_illegal
        LEA     MsgIllegal,A0
        BRA     .show_msg
.not_illegal:
        CMP.W   #32,D2
        BLO     .other_exception
        LEA     MsgTrap,A0
        BRA     .show_msg
.other_exception:
        LEA     MsgUnknown,A0
.show_msg:
        JSR     WriteStringConout
        JSR     NewLine
        ; Pequeno delay para visualização
        MOVE.L  #$50000,D3
.DELAY:
        SUBQ.L  #1,D3
        BNE     .DELAY
        MOVEM.L (SP)+,D0-D7/A0-A6
        ;MOVE.W  #$CC00,LED_ADDRESS  ; Indica spurious
        RTE
SpuriousHandler:
        RTE
Trap0Handler:
        ; Manipula o endereço de retorno na pilha
        MOVE.L  monitorStack,A0
        MOVE.L  A0,SP
        JSR     PrintHexAddress
        BRA     MenuLoop ; Substitui na pilha
        RTE
;        MOVE.L  (RAM_VECTOR_BASE),A0     ; Trap 0 mapeada em $80000
;        JMP     (A0)
Trap1Handler:
    cmp.w   #1,d0         ; Compara com CCONIN (ler caractere)
    BEQ     trap_cconin    ; Se for 1, vai para leitura
    cmp.w   #2,d0         ; Compara com CCONOUT (escrever caractere)
    BEQ     trap_cconout   ; Se for 2, vai para escrita
    cmp.w   #3,d0         ; Compara com CCONOUT (escrever string)
    BEQ     trap_strout   ; Se for 2, vai para escrita
    cmp.w   #0,d0         ; Compara com PTERM0 (terminar)
    BEQ     trap_pterm0    ; Se for 0, vai para terminar
    move.l  #-1,d0        ; Retorna erro se função não reconhecida
    RTE                   ; Retorna da exceção
Trap2Handler:
        MOVE.L  (RAM_VECTOR_BASE+8),A0   ; Trap 2 mapeada em $80008
        JMP     (A0)
Trap3Handler:
        MOVE.L  (RAM_VECTOR_BASE+12),A0
        JMP     (A0)
Trap4Handler:
        MOVE.L  (RAM_VECTOR_BASE+16),A0
        JMP     (A0)
Trap5Handler:
        MOVE.L  (RAM_VECTOR_BASE+20),A0
        JMP     (A0)
Trap6Handler:
        MOVE.L  (RAM_VECTOR_BASE+24),A0
        JMP     (A0)
Trap7Handler:
        MOVE.L  (RAM_VECTOR_BASE+28),A0
        JMP     (A0)
Trap8Handler:
        MOVE.L  (RAM_VECTOR_BASE+32),A0
        JMP     (A0)
Trap9Handler:
        MOVE.L  (RAM_VECTOR_BASE+36),A0
        JMP     (A0)
TrapAHandler:
        MOVE.L  (RAM_VECTOR_BASE+40),A0
        JMP     (A0)
TrapBHandler:
        MOVE.L  (RAM_VECTOR_BASE+44),A0
        JMP     (A0)
TrapCHandler:
        MOVE.L  (RAM_VECTOR_BASE+48),A0
        JMP     (A0)
TrapDHandler:
        MOVE.L  (RAM_VECTOR_BASE+52),A0
        JMP     (A0)
TrapEHandler:
        MOVE.L  (RAM_VECTOR_BASE+56),A0
        JMP     (A0)
TrapFHandler:
        MOVE.L  (RAM_VECTOR_BASE+60),A0
        JMP     (A0)

Int1Handler:        
        MOVEM.L D0-D7/A0-A6,-(A7)
        MOVE.L  (RAM_VECTOR_BASE+64),A0
        MOVEM.L (A7)+,D0-D7/A0-A6
        RTE
Int2Handler:        
        MOVEM.L D0-D7/A0-A6,-(A7)
        MOVE.L  (RAM_VECTOR_BASE+68),A0
        MOVEM.L (A7)+,D0-D7/A0-A6
        RTE
Int3Handler:        
        MOVEM.L D0-D7/A0-A6,-(A7)
        MOVE.L  (RAM_VECTOR_BASE+72),A0
        MOVEM.L (A7)+,D0-D7/A0-A6
        RTE
Int4Handler:        
        MOVEM.L D0-D7/A0-A6,-(A7)
        MOVE.L  (RAM_VECTOR_BASE+76),A0
        MOVEM.L (A7)+,D0-D7/A0-A6
        RTE
Int5Handler:        
        MOVEM.L D0-D7/A0-A6,-(A7)
        MOVE.L  (RAM_VECTOR_BASE+80),A0
        MOVEM.L (A7)+,D0-D7/A0-A6
        RTE
Int6Handler:        
        MOVEM.L D0-D7/A0-A6,-(A7)
        ;MOVE.L  (RAM_VECTOR_BASE+84),A0
        move.b  #$41,D0
        jsr     WriteConout
        MOVEM.L (A7)+,D0-D7/A0-A6
        RTE
Int7Handler:        
        MOVEM.L D0-D7/A0-A6,-(A7)
        ADDQ.L  #1,systemTick
        MOVEM.L (A7)+,D0-D7/A0-A6
        RTE

; Tratador para todos os outros vetores que você não mapeou individualmente ainda
Universal_Trampoline:
        RTE
NewLine:
        MOVEM.L  D0/D1,-(SP)          ; Salva D0
        MOVE.B  #10,D0
        JSR     WriteConout
        MOVE.B  #13,D0
        JSR     WriteConout
        MOVEM.L  (SP)+,D0/D1          ; Restaura D0
        RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;END OF BASIC CODE
;
;VBug2.0 init
;

Vbug2Start:
        ori.w   #$0700,SR    ; Desabilita as interrupções (Nível 7)
        LEA     $FFFF0,SP ; Garante o Stack Pointer (se o hardware não carregou)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;This gives hardware time to get up
;
        MOVE.L  #$003FFFF,D1 ; Contador para o delay (ajuste se precisar de mais)
.DELAY01:
        SUBQ.L  #1,D1         ; Subtrai 1 de D1 (4 ciclos)
        BNE.S   .DELAY01     ; Pula se não for zero (10 ciclos se pular, 8 se não)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ;Set USP( user stack pointer)
;        LEA     USER_SP,A0
;        MOVE.L  A0,USP        ; 🔥 Define User Stack Pointe
        ;Clear memory
        LEA     $080000,A0
        LEA     $0FFFFF,A1
        MOVEQ   #0,D0
.ClearLoop:
        MOVE.L  D0,(A0)+
        CMPA.L  A0,A1
        BHI     .ClearLoop
        ;Initialize memory size
        MOVE.L  #$00000000,romBase
        MOVE.L  #$00004000,romSize
        MOVE.L  #RAMBASE,ramBase    ;Total ram 1572864 de 0x8000 até 0x180000
        MOVE.L  #RAMSIZE,ramSize
        ;Initialize variables
        MOVE.L  #PicoWriteChar,A1    ; Initialize cconout console char out
        MOVE.L  A1,cconout
        MOVE.L  #UartReadChar,A1         ; Initialize cconin console char in
        MOVE.L  A1,cconin
        MOVE.L  #UART_BASE,A1    ; Initialize uart output,INPUT and baud
        MOVE.L  A1,currentUart
        MOVE.L  #B115200,currentBaudRate


        ;Initialize picoVga
        JSR  InitPicoVga
        ;Initialize UARTs
        JSR  InitUart1   
        JSR  InitUartb1
        ;Initialize TRAP1
        JSR  InitTrap1

MenuLoop:
subLoop:    
        JSR     PicoClearScreen
        LEA     MsgOrionInit,A0
        JSR     WriteStringConout

        ;JSR     PicoClearScreen

        LEA     MsgMenuText,A0
        JSR     WriteStringConout
        JSR     ReadConin
        JSR     WriteConout

        CMP.B   #'1',D0
        BEQ.S   TmpReadKbd
        CMP.B   #'2',D0
        BEQ.S   TmpReadKbd2
        CMP.B   #'3',D0
        BEQ     Xmodem
        CMP.B   #'4',D0
        BEQ.S   RunTrap1
        CMP.B   #'5',D0
        BEQ     RunProgram
        CMP.B   #'7',D0
        BEQ     MemDump0
        CMP.B   #'8',D0
        BEQ     ReadInHexa
        CMP.B   #'9',D0
        BEQ     ReadHexAddressConin

        JMP     subLoop

Xmodem:
        JSR     XmodemRec
        JMP     subLoop        

MemDump0:
        JSR     MemDump
        JMP     subLoop        
       
;1
TmpReadKbd:
        MOVEM.L D1-D7/A0-A6,-(SP)
        JSR     $8426.L
        MOVEM.L (SP)+,D1-D7/A0-A6
        JSR     WriteConout
        CMP.B   #$42,D0
        BNE     TmpReadKbd
        JMP     subLoop
;2
TmpReadKbd2:
        JSR     Configkbd
        JMP     subLoop


;4 Testa trap 1
RunTrap1:
        LEA     MsgWritePrompt,A0
        JSR     WriteStringConout
        JSR     ReadConin
        MOVE.B  D0,D1
        MOVE.L  #$2,D0
        TRAP    #1
        JSR     ReadConin
        JMP     subLoop
; 5. Executa programa na RAM
RunProgram:
        BTST    #PROGRAM_LOADED,flg_system
        BNE     .run_program
        LEA     MsgNoProgramToRUN,A0
        JSR     WriteStringConout
        BRA     subLoop
.run_program
        LEA     MsgRunPrompt,A0
        JSR     WriteStringConout
        MOVE.L  SP,A0
        MOVE.L  A0,monitorStack
        LEA     buf_pgm,A0   ; A0 aponta para o endereço buffer onde esta o programa
        JSR     (A0)         ; Chama o código como uma sub-rotina (salva o endereço de retorno)
        BRA     subLoop

ReadInHexa:
        LEA     MsgTestHexInput,A0
        JSR     WriteStringConout
        JSR     ReadHexAddressConin
        LEA     MsgTestHexInput,A0
        JSR     WriteStringConout
        JMP     subLoop

; Lê número hexadecimal (retorna em D0)
UART_ReadHex1:
        MOVE.L  D1,-(SP)
        MOVE.L  D2,-(SP)

        LEA     MsgWritePrompt,A0
        JSR     WriteStringConout

        JSR     NewLine

        MOVEQ   #0,D0
        MOVEQ   #0,D1            ; Máximo 8 dígitos
        MOVEQ   #0,D2            ; Resultado em D2

.Loop:
        JSR     ReadConin
        CMP.B   #13,D0
        BEQ     .Done
        CMP.B   #10,D0
        BEQ     .Done
        JSR     buf_put
        BRA     .Loop
.Done:

.loop1:
        JSR     buf_get
        CMP.B   #-1,D0          ; Buffer vazio?
        BEQ     .fim            ; Se sim, ignora
        JSR     WriteConout
        BRA     .loop1
.fim:
        MOVE.L  (SP)+,D2
        MOVE.L  (SP)+,D1
        JSR     NewLine
        ;bra     subLoop ;provisoriamente
        RTS


;Includes
        INCLUDE "pds_std_io.asm"
        INCLUDE "drv_uart.inc"
        INCLUDE "drv_uart.asm"
        INCLUDE "drv_uartb.asm"
        INCLUDE "drv_pico_vga.asm"
        INCLUDE "drv_keyboard.asm"
        INCLUDE "cmd_mem_dump.asm"
        INCLUDE "cmd_xmodem.asm"
        INCLUDE "trap1.asm"
        INCLUDE "rot_print_hex_num.asm"



        INCLUDE "section_data.asm"
        INCLUDE "section_bss.asm"

        END
