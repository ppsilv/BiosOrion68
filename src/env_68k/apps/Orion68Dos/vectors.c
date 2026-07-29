#include <stdio.h>
#include "io.h"
#include "keyboard.h"
#include "interrupt.h"
#include "drv_ps2.h"

//extern volatile unsigned char rx_head;
//extern volatile unsigned char rx_tail;
//extern volatile char rx_buffer[RX_BUFFER_SIZE];

// A variável do tick agora é perfeitamente acessível pelo Assembly
extern volatile long systemTick; 

// A TRAP 14 agora é uma função C NORMAL, chamada via JSR pelo vectors.S
unsigned long TRAP14_Handler(unsigned long d0_val, unsigned long d1_val) {
    unsigned long retorno_final = 0; // Cria a variável com 32-bits vazia
    unsigned char ch = 0;
    
    switch(d1_val & 0xFF) {
        case 0: 
            uart0_init(); 
            break;
        case 1: 
            uart0_write(d0_val); 
            break;
        case 2: 
            retorno_final = (unsigned long)uart0_read(); 
            break;
        case 3: 
            ch = uart0_read_timeout(); 
            printf("TRAP14_Handler: Retornando com ch... [%02x]\n", ch);
            retorno_final = (unsigned long)ch; // Garante os 32 bits limpos aqui
            break;
        default:
            printf("\n\nQUE MERDA É ESSA: D1 era %ld\n\n\n\n", d1_val);
            break;
    }
    
    // O único return da função. O GCC é OBRIGADO a jogar 'retorno_final' em D0 aqui e sair direto.
    return retorno_final; 
}

// Sua função de leitura atômica do tick continua linda aqui
unsigned long get_system_tick(void) {
    unsigned long tick;
    unsigned int status_antigo;
    
    status_antigo = m68k_disable_level2_perfect(); 
    tick = systemTick;                     
    m68k_restore_interrupts(status_antigo); 
    
    return tick;
}
/*
void trata_int3_handler(uint8_t tecla){
    //temque ler os dado do teclado e por no buffer circular
    // 1. Lê o byte (essa leitura DEVE limpar a linha INT3 no Pico/Hardware)
    //uint8_t tecla = PICO_PS2_DATA_REG;
    if(tecla >0x00)
        kb_put(tecla);
//    kb_put('X');
//    printf("%02x",tecla);
}
*/
/*
#include "io.h"
#include "keyboard.h"
#include "interrupt.h"

extern volatile unsigned char rx_head;
extern volatile unsigned char rx_tail;
extern volatile char rx_buffer[RX_BUFFER_SIZE];

extern void TRAP14_Handler(void);

static __attribute__((section(".mram"))) long systemTick=0;

void __attribute__((interrupt)) SvcBusError     (){
    
}
void __attribute__((interrupt)) Svcaddress_err  (){
    
}
void __attribute__((interrupt)) SvcIllegalIns   (){
    
}
void __attribute__((interrupt)) SvcDiv0_handler (){
    
}
void __attribute__((interrupt)) SvcChkIns       (){
    
}
void __attribute__((interrupt)) SvcTrapvIns(){
    
}
void __attribute__((interrupt)) SvcPrivViolation(){
    
}
void __attribute__((interrupt)) SvcTrace    (){
    
}
void __attribute__((interrupt)) SvcLineA        (){
    
}
void __attribute__((interrupt)) SvcLineF        (){
    
}
void __attribute__((interrupt)) bad_exception(){
    
}
void __attribute__((interrupt)) SpuriousHandler(){
    
}
void __attribute__((interrupt)) Int1Handler(){
    
}
void __attribute__((interrupt)) Int2Handler(){
    systemTick++;    
}
void __attribute__((interrupt)) Int3Handler(){
    
}
void __attribute__((interrupt)) Int4Handler(){

}
void __attribute__((interrupt)) Int5Handler(){
    
}
void __attribute__((interrupt)) Int6Handler(){
   
}
void __attribute__((interrupt)) Int7Handler(){
    //systemTick++;
}
void __attribute__((interrupt)) Trap0Handler(){
    
}
void __attribute__((interrupt)) Trap1Handler(){ 
    __asm__ __volatile__ ("movem.l %d2-%d7/%a0-%a6,-(%sp)");

    // Amarre variáveis locais diretamente aos registradores físicos
    register unsigned short function_code asm("d0");
    register unsigned char  character     asm("d1");

    // Agora você pode usar variáveis C normais!
    switch(function_code) {
        case 1:
            if (cconin) {
                function_code=cconin();
            }
            break;
        case 2:
            if (cconout) {
                cconout(character);
            }
            break;
    }    
    __asm__ __volatile__ ("movem.l (%sp)+,%d2-%d7/%a0-%a6");
}
void __attribute__((interrupt)) Trap2Handler(){
    
}
void __attribute__((interrupt)) Trap3Handler(){
    
}
void __attribute__((interrupt)) Trap4Handler(){
    
}
void __attribute__((interrupt)) Trap5Handler(){
    
}
void __attribute__((interrupt)) Trap6Handler(){
    
}
void __attribute__((interrupt)) Trap7Handler(){
    
}
void __attribute__((interrupt)) Trap8Handler(){
    
}
void __attribute__((interrupt)) Trap9Handler(){
    
}
void __attribute__((interrupt)) TrapAHandler(){
    
}
void __attribute__((interrupt)) TrapBHandler(){
    
}
void __attribute__((interrupt)) TrapCHandler(){
    
}
void __attribute__((interrupt)) TrapDHandler(){
    __asm__ __volatile__ ("movem.l %d2-%d7/%a0-%a6,-(%sp)");

    __asm__ __volatile__ ("movem.l (%sp)+,%d2-%d7/%a0-%a6");
    
}
void __attribute__((interrupt)) TrapEHandler(){
    __asm__ __volatile__ ("movem.l %d2-%d7/%a0-%a6,-(%sp)");

    TRAP14_Handler();

    __asm__ __volatile__ ("movem.l (%sp)+,%d2-%d7/%a0-%a6");    
}
void __attribute__((interrupt)) TrapFHandler(){
}
merda merda
// A SUA FUNÇÃO DE LEITURA FICA ASSIM:
unsigned long get_system_tick(void) {
    unsigned long tick;
    unsigned int status_antigo;
    
    status_antigo = m68k_disable_level2_perfect(); // Salva o estado atual do SR e barra o nível 2
    tick = systemTick;                     // Copia os 32 bits em segurança
    m68k_restore_interrupts(status_antigo); // Devolve o SR exatamente como estava
    
    return tick;
}
*/