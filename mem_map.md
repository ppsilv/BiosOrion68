# Mapa de memória inicial (recomendado)

# O Motorola 68000 possui 24 bits de endereço → 16 MB de espaço.

Orion68 Memory Map (Initial)

ROM
000000  -   07FFFF   512KW de espaço para rom

RAM
080000  -   0FFFFF   512KW de espaço para ram - populado
100000  -   17FFFF   512KW de espaço para ram - populado
180000  -   17FFFF   512KW de espaço para ram - NÃO populado
180000  -   1FFFFF   512KW de espaço para ram - NÃO populado

200000  -   27FFFF   512KW de espaço para ram - NÃO populado
280000  -   2FFFFF   512KW de espaço para ram - NÃO populado
300000  -   37FFFF   512KW de espaço para ram - NÃO populado
380000  -   3FFFFF   512KW de espaço para ram - NÃO populado

400000  -   47FFFF   512KW de espaço para ram - NÃO populado
480000  -   4FFFFF   512KW de espaço para ram - NÃO populado
500000  -   57FFFF   512KW de espaço para ram - NÃO populado
580000  -   5FFFFF   512KW de espaço para ram - NÃO populado


# I/O mapeado          FF0xxx -> FFFxxx
0x00FF4000  -  0x00FF40FF---------------->Uart0
0x00FF4100  -  0x00FF41FF---------------->Uart1
0x00FF4200  -  0x00FF42FF---------------->Uart2
0x00FF4300  -  0x00FF43FF---------------->Uart3
0x00FF4400  -  0x00FF44FF---------------->IDE
0x00FF8000  -  0x00FF80FF---------------->PicoVGA
0x00FF9000  -  0x00FF90FF---------------->Duart
0x00FF9100  -  0x00FF91FF---------------->MultiIO





