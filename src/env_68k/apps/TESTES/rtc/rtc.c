#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vga_video.h>
#include <color.h>
#include <vga_video_graphics.h>

#define RUN_CMD        (*((volatile unsigned char *)(0x00FF8000 + 0x3D)))

extern uint8_t rtc_read_config_byte(uint16_t endereco);
extern void    rtc_write_config_byte(uint16_t endereco, uint8_t valor); /* <--- Adicionado */ 
extern char rtc_inicializar(void);

#define RAM_POS_00 0x00 
#define RAM_POS_01 0x01 
#define RAM_POS_02 0x02 
#define RAM_POS_03 0x03 
#define RAM_POS_04 0x14 
#define RAM_POS_05 0x15 

void rtc_tst(){
    printf("Endereco 0x01 [%x] \n",rtc_read_config_byte(0x01));
    printf("Endereco 0x03 [%x] \n",rtc_read_config_byte(0x03));

    printf("Zerando posicao 0x01 e 0x03\n");
    rtc_write_config_byte(0x01,0x00);
    rtc_write_config_byte(0x03,0x00);
    printf("Endereco 0x01 [%x] \n",rtc_read_config_byte(0x01));
    printf("Endereco 0x03 [%x] \n",rtc_read_config_byte(0x03));

    printf("Gravando posicao 0x01=1 e 0x03=1\n");
    rtc_write_config_byte(0x01,0x01);
    rtc_write_config_byte(0x03,0x01);
    printf("Endereco 0x01 [%x] \n",rtc_read_config_byte(0x01));
    printf("Endereco 0x03 [%x] \n",rtc_read_config_byte(0x03));

    printf("Gravando posicao 0x01=2 e 0x03=2\n");
    rtc_write_config_byte(0x01,0x02);
    rtc_write_config_byte(0x03,0x02);
    printf("Endereco 0x01 [%x] \n",rtc_read_config_byte(0x01));
    printf("Endereco 0x03 [%x] \n",rtc_read_config_byte(0x03));

    printf("Gravando posicao 0x01=4 e 0x03=4\n");
    rtc_write_config_byte(0x01,0x04);
    rtc_write_config_byte(0x03,0x04);
    printf("Endereco 0x01 [%x] \n",rtc_read_config_byte(0x01));
    printf("Endereco 0x03 [%x] \n",rtc_read_config_byte(0x03));

    printf("Gravando posicao 0x01=8 e 0x03=8\n");
    rtc_write_config_byte(0x01,0x08);
    rtc_write_config_byte(0x03,0x08);
    printf("Endereco 0x01 [%x] \n",rtc_read_config_byte(0x01));
    printf("Endereco 0x03 [%x] \n",rtc_read_config_byte(0x03));

}

uint8_t rtc_read_ram_byte(uint8_t endereco);
void rtc_write_ram_byte(uint16_t endereco,uint8_t dado);
extern char mode;
void read_rtc(){
    printf("Reading memory block %c \n",mode);
    printf("Pos 00=[%02x] ",rtc_read_ram_byte(RAM_POS_00));
    printf("Pos 01=[%02x] ",rtc_read_ram_byte(RAM_POS_01));
    printf("Pos 02=[%02x] ",rtc_read_ram_byte(RAM_POS_02));
    printf("Pos 03=[%02x] ",rtc_read_ram_byte(RAM_POS_03));
    printf("Pos 04=[%02x] ",rtc_read_ram_byte(RAM_POS_04));
    printf("Pos 05=[%02x]\n",rtc_read_ram_byte(RAM_POS_05));
    printf("Pos 06=[%02x]\n",rtc_read_ram_byte(6));
}
void reset();
void read_cfg(char mode);
int main_ricoch( int argc, char ** argv )
{
    //RUN_CMD = 0xA4;
    //setcolor(4,0);
    //printf( "\nRTC test pgordao @copyleft V1.1 2026...\n" );
    //printf( "Compiled for Orion68 cpu MC68000.\n" );

    if( argc < 2){
        printf("Usage: %s <read | write | cfg 0=blkram0 1=blkram1>\n");
        return 1;
    }

    //printf("argc[%d]\n",argc);
    //for(int i=0; i< argc;i++){
    //    printf("argv[%d]= [%s]\n",i,argv[i]);
    //}
    reset();
    
    char * lixo= strstr("rd",argv[1]);
    //printf("lixo %04x\n",lixo);
    if( strstr("cf",argv[1]) ){
        printf("comando configuracao\n");
        if( argc < 3 ){
            printf("Usage: cf mode 0|1\n");
            return 1;
        }
        read_cfg(*argv[2]);
        return 0;
    }
    if( strstr("rd",argv[1]) ){
        printf("comando read\n");
        read_rtc();
        return 0;
    }
    if( strstr("wr",argv[1]) ){
        if( argc < 4 ){
            printf("Usage: wr addr data\n");
            return 1;
        }
        unsigned char endereco = *argv[2]-'0';
        unsigned char dado = atoi(argv[3]);
        
        rtc_write_ram_byte(endereco,dado);
    }


    return 0;
    setcolor(2,0);
    if ( rtc_inicializar()){
        rtc_tst();
    }else{
        printf("Relogio nao encontrado.!!!\n");
    }
    return 0;
}

/* ====================================================================
   MAPEAMENTO DA MC68681 DUART (Orion68K)
   ==================================================================== */
#define DUART_IPR     (*(volatile uint8_t *)0xFF901B) /* Input Port (IP0 = Pino 7) */
#define DUART_SOPR    (*(volatile uint8_t *)0xFF901D) /* Set Output Port */
#define DUART_ROPR    (*(volatile uint8_t *)0xFF901F) /* Reset Output Port */
/*
// Pinos na DUART (MC68681 DIP-40)
#define SCL_BIT       (1 << 1) // OP1 (Pino 14) 
#define SDA_OUT_BIT   (1 << 2) // OP2 (Pino 15) 
#define SDA_IN_BIT    (1 << 0) // IP0 (Pino 7)  
*/
/* Pinos ajustados para o mapeamento real da sua fiação no Orion68K 
#define SCL_BIT       (1 << 3) //0x08 -> Aciona OP1 (Pino 14)
#define SDA_OUT_BIT   (1 << 4) //0x10 -> Aciona OP2 (Pino 15)
#define SDA_IN_BIT    (1 << 2) //0x04 -> Lê IP0 (Pino 7)    
*/
#define SCL_BIT       0x20  // Pino 14 (OP5)
#define SDA_OUT_BIT   0x80  // Pino 15 (OP7)
#define SDA_IN_BIT    0x01  // Pino 7  (IP0 - Leitura)


   // move.b  #0x1C,0xFF901D        
   // move.b  #0x1C,0xFF901F        

/* Teste isolado de hardware */
void test_duart_pins(void) {
    /* Zera OPCR para garantir OP1 e OP2 como I/O simples */
    (*(volatile uint8_t *)0xFF901B) = 0x00;
    int j=100;
    while(j--) {
        DUART_SOPR = SCL_BIT | SDA_OUT_BIT; /* Força SCL e SDA para NÍVEL BAIXO (0V) */
        for (volatile int i = 0; i < 5000; i++);

        DUART_ROPR = SCL_BIT | SDA_OUT_BIT; /* Libera SCL e SDA para NÍVEL ALTO (5V) */
        for (volatile int i = 0; i < 5000; i++);
    }
}

void le_ivr(){
volatile uint8_t *DUART_IVR = (volatile uint8_t *)0xFF9019;

*DUART_IVR = 0x55;
uint8_t val1 = *DUART_IVR; // Esperado: 0x55

*DUART_IVR = 0xAA;
uint8_t val2 = *DUART_IVR; // Esperado: 0xAA
printf("Val1=[%02X] Val2=[%02X]\n",val1,val2);    
}

void ler_pino7(){
    #define DUART_IPR (*(volatile uint8_t *)0xFF901B) // Leitura das entradas IP0..IP5

    uint8_t ipr = DUART_IPR; // Lê o registrador apenas uma vez
    int j=100;
    while(j--) {
        printf("IP0 (Pino 7): %d\n", (ipr >> 0) & 1); // Pino 7 (SDA In)
        printf("IP1:          %d\n", (ipr >> 1) & 1);
        printf("IP2:          %d\n", (ipr >> 2) & 1);
        printf("IP3:          %d\n", (ipr >> 3) & 1);
    }    
}

int mainTeste( int argc, char ** argv ){
    printf("Teste das poras da duart...\n");
    printf("scl_bit[%02x] SDA_OUT_BIT[%02x]\n",SCL_BIT,SDA_OUT_BIT);
    le_ivr();
    ler_pino7();
    //test_duart_pins();

} 