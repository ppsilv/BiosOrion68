; ----------------------------------------------------------------------
; SECTION bss
;-----------------------------------------------------------------------
    SECTION .bss
    ORG     RAM_VARIABLES ;RAM VARIABLES               ; Área para variáveis
        ALIGN 4
;=== System variables
romBase             DS.L 1
romSize             DS.L 1
ramBase             DS.L 1
ramSize             DS.L 1
currentUart         DS.L 1
cconout             DS.L 1
cconin              DS.L 1
;currentTimer        DS.L 1
;flg_orion           DS.L   1         ;minhas_flags   
;checksumCalc       DS.L   1
monitorStack        DS.L   1
systemTick          DS.L   1
addressInHex        DS.L   1
debug_regs          DS.L   8
        ALIGN 2
buf_xmodem          DS.B   512        ; Buffer de dados
buf_put             DS.B   512    
buf_get             DS.B   512    
currentBaudRate     DS.W     1
chPayloadBuffer     DS.B    16        ; Buffer temporário para acomodar os dados HID
flg_system          DS.W     1        ; Flags do sistema PROGRAM_LOADED  
;blockNumber         DS.B   1          ; Número do bloco atual
;expectedBlock       DS.B   1          ; Próximo bloco esperado

SECTION DATA
     
; --- Limites da Tabela ---
SCAN_START  EQU     $04             ; Letra 'A'
SCAN_END    EQU     $38             ; Tecla '/' e '?' (no layout US)

; -----------------------------------------------------------------------------
; TABELA ASCII NORMAL (Sem Shift)
; Índice 0 corresponde ao Scan Code $04
; -----------------------------------------------------------------------------
ASCII_Table_Normal:
    ; Letras A-Z (Scan Codes $04 - $1D)
    dc.b    "a","b","c","d","e","f","g","h","i","j","k","l","m"
    dc.b    "n","o","p","q","r","s","t","u","v","w","x","y","z"
    ; Números 1-9, 0 (Scan Codes $1E - $27)
    dc.b    "1","2","3","4","5","6","7","8","9","0"
    ; Controles estruturais (Scan Codes $28 - $2B)
    dc.b    $0D                     ; $28: Enter (CR)
    dc.b    $1B                     ; $29: ESC
    dc.b    $08                     ; $2A: Backspace (BS)
    dc.b    $09                     ; $2B: Tab
    ; Pontuação e Símbolos (Scan Codes $2C - $38)
    dc.b    " "                     ; $2C: Espaço
    dc.b    "-"                     ; $2D: - e _
    dc.b    "="                     ; $2E: = e +
    dc.b    "["                     ; $2F: [ e {
    dc.b    "]"                     ; $30: ] e }
    dc.b    "\"                     ; $31: \ e |
    dc.b    "#"                     ; $32: Não-US # e ~ (geralmente ignorado)
    dc.b    ";"                     ; $33: ; e :
    dc.b    "'"                     ; $34: ' e "
    dc.b    "`"                     ; $35: ` e ~
    dc.b    ","                     ; $36: , e <
    dc.b    "."                     ; $37: . e >
    dc.b    "/"                     ; $38: / e ?
    ALIGN   2

; -----------------------------------------------------------------------------
; TABELA ASCII COM SHIFT ATIVO
; Índice 0 corresponde ao Scan Code $04
; -----------------------------------------------------------------------------
ASCII_Table_Shift:
    ; Letras A-Z Caixa Alta (Scan Codes $04 - $1D)
    dc.b    "A","B","C","D","E","F","G","H","I","Y","K","L","M"
    dc.b    "N","O","P","Q","R","S","T","U","V","W","X","Y","Z"
    ; Símbolos sobre os números (Scan Codes $1E - $27 - Padrão Internacional US)
    dc.b    "!","@","#","$","%","^","&","*","(",")"
    ; Controles estruturais (Mantêm o mesmo efeito com Shift)
    dc.b    $0D                     ; $28: Enter
    dc.b    $1B                     ; $29: ESC
    dc.b    $08                     ; $2A: Backspace
    dc.b    $09                     ; $2B: Tab
    ; Símbolos com Shift (Scan Codes $2C - $38)
    dc.b    " "                     ; $2C: Espaço
    dc.b    "_"                     ; $2D: Underscore
    dc.b    "+"                     ; $2E: Mais
    dc.b    "{"                     ; $2F: Chave Aberta
    dc.b    "}"                     ; $30: Chave Fechada
    dc.b    "|"                     ; $31: Barra Vertical
    dc.b    "~"                     ; $32: ~
    dc.b    ":"                     ; $33: Dois Pontos
    dc.b    '"'                     ; $34: Aspas Duplas
    dc.b    "~"                     ; $35: Til / Crase
    dc.b    "<"                     ; $36: Menor Que
    dc.b    ">"                     ; $37: Maior Que
    dc.b    "?"                     ; $38: Interrogação
    ALIGN   2


        ORG $00082000
        ALIGN 2
buf_pgm             DS.B   8192
        END
