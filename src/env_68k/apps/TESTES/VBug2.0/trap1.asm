
MERDA:
    DC.B    "MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA MERDA ",13,10,0

    Align 2
; -----------------------------------------------------------------------------
; O SEU CÓDIGO DO PDS317 (Agora rodando na RAM)
; -----------------------------------------------------------------------------

; --------------------------------
; TRAP_CCONIN - Ler caractere do console
; Saída: D0.L - Caractere lido
; --------------------------------
trap_cconin:
    MOVE.L A2,-(SP)
    MOVE.L  (cconin),A2
    JSR     (A2)
    MOVE.L (SP)+,A2

    ANDI.L  #$FF,D0       ; Mantém apenas o byte inferior
    RTE                   ; Retorna da exceção

; --------------------------------
; TRAP_CCONOUT - Escrever caractere no console
; Entrada: D1.L - Caractere a escrever
; --------------------------------
trap_cconout:
    MOVE.L A2,-(SP)
    MOVE.B  D1,D0
    MOVE.L  (cconout),A2        ; Busca o endereço da rotina física da UART
    JSR     (A2)
    MOVE.L (SP)+,A2
    RTE      
                

trap_strout:
    MOVE.L  A1,A0
    JSR     WriteStringConout
    RTE


; --------------------------------
; TRAP_PTERM0 - Terminar programa
; --------------------------------
trap_pterm0:
    move.w  #0,d0         ; Código de saída 0
    bsr     hardware_exit ; Chama rotina de término
    RTE     

; --------------------------------
; hardware_exit - Terminar execução
; Entrada: D0.W - Código de saída
; --------------------------------
hardware_exit:
    ; >>> ADAPTE PARA SUA PLACA <<<
    ; Exemplo: parar o processador
    ; stop    #$2700

    ; Exemplo simples: loop infinito
    BRA     hardware_exit
    RTS

InitTrap1:
    ; -------------------------------------------------------------------------
    ; COMO INSTALAR NA RAM:
    ; Copiamos o endereço do nosso handler real para a posição $404 da RAM
    ; -------------------------------------------------------------------------
    ;LEA     vbug2_trap1,A0             ; Pega o endereço da nossa rotina na RAM
    ;MOVE.L  A0,($00080004)              ; Grava na posição da TRAP 1 na RAM!
    RTS
    ; A partir deste momento, qualquer instrução "TRAP #1" vai cair aqui!
