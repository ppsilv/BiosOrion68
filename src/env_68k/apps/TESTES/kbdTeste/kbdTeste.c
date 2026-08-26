#include <stdio.h>
#include <stdlib.h>
#include <mc68000.h>

//unsigned int get_key();
//void init_kbd();
/*
void delay(unsigned int time) {
    for (volatile unsigned int i = 0; i < time; i++);
}

// Função para verificar stack
void check_stack(void) {
    unsigned long stack_val;
    asm volatile (
        "move.l %%sp, %0\n\t"
        : "=r" (stack_val)
    );
    printf("Stack pointer: 0x%08X\n", stack_val);
}
*/
/*
void aguarda(){
    printf("Pressione q/q tecla para sair.\n");
    getchar();
}

void main1();
void set_keyboard_leds(unsigned char led_status);
void show_menu(){
    int choice;
    unsigned int ch;
    choice = 'A';
    while (choice != 1000){
        printf("\n1 - getkey from keyboard\n");
        printf("2 - chama main e trava na main\n");
        printf("3 - Setando leds do teclado\n");
        printf("0 - sai do programa\n");
        printf("\nEscolha: ");
        choice = getchar();
        printf("choice [%02x]\n",choice);
        switch(choice){
            case '1':
                printf("Getting a keyboard key\n");
                while(1){
                    ch = get_key();
                    if( ch == 0x1B )
                        break;                            
                    if( ch > 0xFF){
                        printf("[%02X]",ch);
                    }else{  
                        if( ch > 0x20 )  
                        printf("%c",ch);
                    }
                }
                aguarda();
                break;
            case '2':
                //main1();
                break;
            case '3':
                printf("Setando leds do teclado\n");
                if( kled == 0 ){
                    set_keyboard_leds(2);
                    kled = 1;
                }else if( kled == 1 ){
                    set_keyboard_leds(0);
                    kled = 0;
                }
                aguarda();
                break;    
            case '4':
                init_kbd();
                break    ;            
            case '0':
                jmp_subloop();

                break;
        }
    }
}
*/
/*
void jmp_subloop(){
    __asm__ __volatile__(
        "jmp 0x0000128C.l\n\t" // JMP absoluto longo para 0x000000
    );
}
*/
int main() {
    //check_stack();  // ✅ Verificar stack no início
    //init_kbd();
    //jmp_subloop();



//    printf("Vou testar a leitura de teclado com ch9350 \n");
//    show_menu();
    return 0;
}


