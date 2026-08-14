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
int main( int argc, char ** argv )
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
