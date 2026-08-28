


RHR         equ     $01   ; receive holding register (read)
THR         equ     $01   ; transmit holding register (write)
IER         equ     $03   ; interrupt enable register
ISR         equ     $05   ; interrupt status register (read)
FCR         equ     $05   ; FIFO control register (write)
LCR         equ     $07   ; line control register
MCR         equ     $09   ; modem control register
LSR         equ     $0B   ; line status register
MSR         equ     $0D   ; modem status register
SPR         equ     $0F   ; scratchpad register (reserved for system use)
DLL         equ     $01   ; divisor latch LSB
DLM         equ     $03   ; divisor latch MSB
; aliases for register names (used by different manufacturers)cd ..
RBR         equ     RHR ; receive buffer register
IIR         equ     ISR ; interrupt identification register
SCR         equ     SPR ; scratch register


B009600     equ         $0060
B019200     equ         $0030
B038400     equ         $0018
B057600     equ         $0010
B115200     equ         $0008
B230400     equ         $0004
B460800     equ         $0002
B921600     equ         $0001


B009600L    equ         $60
B019200L    equ         $30
B038400L    equ         $18
B057600L    equ         $10
B115200L    equ         $08
B230400L    equ         $04
B460800L    equ         $02
B921600L    equ         $01
B009600H    equ         $00
B019200H    equ         $00     
B038400H    equ         $00     
B057600H    equ         $00     
B115200H    equ         $00     
B230400H    equ         $00
B460800H    equ         $00
B921600H    equ         $00
