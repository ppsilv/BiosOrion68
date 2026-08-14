#include <stdio.h>
#include <stdint.h>


// Endereço base que você definiu
#define RTC_BASE       0xFF9200

// Mapeamento dos registradores do RP5C01A (Modo 0) - Corrigido para Ímpares (D0-D7 / LDS)
#define RTC_SEC_UNI    (*(volatile uint8_t *)(RTC_BASE + 0x01)) 
#define RTC_SEC_DEZ    (*(volatile uint8_t *)(RTC_BASE + 0x03)) 
#define RTC_MIN_UNI    (*(volatile uint8_t *)(RTC_BASE + 0x05)) 
#define RTC_MIN_DEZ    (*(volatile uint8_t *)(RTC_BASE + 0x07)) 
#define RTC_HOR_UNI    (*(volatile uint8_t *)(RTC_BASE + 0x09)) 
#define RTC_HOR_DEZ    (*(volatile uint8_t *)(RTC_BASE + 0x0B)) 
// O deslocamento 0x0D seria o Dia da Semana (Reg 6 do chip)
#define RTC_DIA_UNI    (*(volatile uint8_t *)(RTC_BASE + 0x0F)) 
#define RTC_DIA_DEZ    (*(volatile uint8_t *)(RTC_BASE + 0x11)) 
#define RTC_MES_UNI    (*(volatile uint8_t *)(RTC_BASE + 0x13)) 
#define RTC_MES_DEZ    (*(volatile uint8_t *)(RTC_BASE + 0x15)) 
#define RTC_ANO_UNI    (*(volatile uint8_t *)(RTC_BASE + 0x17)) 
#define RTC_ANO_DEZ    (*(volatile uint8_t *)(RTC_BASE + 0x19)) 


#define ADDR_RAM_POS_00 (*(volatile uint8_t *)(RTC_BASE + 0x01))    //0 BOM
#define ADDR_RAM_POS_0A (*(volatile uint8_t *)(RTC_BASE + 0x03))    //  BOM 
#define ADDR_RAM_POS_01 (*(volatile uint8_t *)(RTC_BASE + 0x05))    //1 BOM
#define ADDR_RAM_POS_0B (*(volatile uint8_t *)(RTC_BASE + 0x07))    //  BOM
#define ADDR_RAM_POS_02 (*(volatile uint8_t *)(RTC_BASE + 0x09))    //2 BOM
#define ADDR_RAM_POS_0C (*(volatile uint8_t *)(RTC_BASE + 0x0B))    //  BAD
#define ADDR_RAM_POS_03 (*(volatile uint8_t *)(RTC_BASE + 0x0D))    //3 BOM
#define ADDR_RAM_POS_0D (*(volatile uint8_t *)(RTC_BASE + 0x0F))    //  BOM
#define ADDR_RAM_POS_04 (*(volatile uint8_t *)(RTC_BASE + 0x11))    //4 BAD
#define ADDR_RAM_POS_0E (*(volatile uint8_t *)(RTC_BASE + 0x13))    //  BAD
#define ADDR_RAM_POS_05 (*(volatile uint8_t *)(RTC_BASE + 0x15))    //5 BAD
#define ADDR_RAM_POS_0F (*(volatile uint8_t *)(RTC_BASE + 0x17))    //  BAD 
#define ADDR_RAM_POS_06 (*(volatile uint8_t *)(RTC_BASE + 0x19))    //6 BOM
/*
0 0 x 2 + 1 =     1
1 1 x 2 + 1 =     3
2 2 x 2 + 1 =     5
3 3 x 2 + 1 =     7
4 4 x 2 + 1 =     9
5 5 x 2 + 1 =     b
6 6 x 2 + 1 =     d
7 7 x 2 + 1 =     f
8 8 x 2 + 1 =     11
9 9 x 2 + 1 =     13
A 10 x 2 + 1 =    15 
B 11 x 2 + 1 =    17 
C 12 x 2 + 1 =    19 

D 13 x 2 + 1 =    1b 
E 14 x 2 + 1 =    1d 
F 15 x 2 + 1 =    1f 
*/




#define RAM_POS_00 0x00 
#define RAM_POS_01 0x01 
#define RAM_POS_02 0x02 
#define RAM_POS_03 0x03 
#define RAM_POS_04 0x14 
#define RAM_POS_05 0x15 




#define RTC_REG_MODE   (*(volatile uint8_t *)(RTC_BASE + 0x1B)) 
#define RTC_REG_CTRL   (*(volatile uint8_t *)(RTC_BASE + 0x1D))
#define RTC_REG_RESET  (*(volatile uint8_t *)(RTC_BASE + 0x1F))

#define RTC_BASE_ADDR ((volatile uint8_t *)0xFF9200)

// Registradores de controle fixos no RP5C01A
//#define RTC_REG_MODE1  0x0E
//#define RTC_REG_TEST   0x0F

// Modos de Operação do Mode Register
#define RTC_MODE_CLOCK      0x0C // Modo relógio
#define RTC_MODE_CLOCK_ALR  0x01 // Modo alarm
#define RTC_MODE_RAM_BLOCK0 0x02 // Seleciona o Bloco 0 da RAM (Endereços 0x0 a 0xD)
#define RTC_MODE_RAM_BLOCK1 0x03 // Seleciona o Bloco 1 da RAM (Endereços 0x0 a 0xD)


char mode = '0';
// Função auxiliar para juntar a Dezena e Unidade de 4 bits em decimal real
static uint8_t bcd_to_dec(uint8_t dez, uint8_t uni) {
    return ((dez & 0x0F) * 10) + (uni & 0x0F);
}

void reset(){
    RTC_REG_RESET = 0;
}
void read_cfg(char mode){
    printf("mode[%02x]\n",mode);
    if( mode == '0' ){
        printf("Setting mode to 0\n");
        RTC_REG_MODE = RTC_MODE_RAM_BLOCK0;
    }
    if( mode == '1' ){
        printf("Setting mode to 1\n");
        RTC_REG_MODE = RTC_MODE_RAM_BLOCK1;
    }

}
// FUNÇÃO DO DRIVER PARA A FATFS
// Retorna um DWORD (uint32_t) no formato exato que a FatFs pede
uint32_t get_fattime(void) {
    uint8_t seg, min, hora, dia, mes, ano_bcd;
    uint32_t ano_fat;

    // 1. Seleciona o Banco 0 (Modo 0) para ler Relógio/Calendário
    // O bit 0 do Reg Mode controla o banco (0 = Relógio, 1 = Alarme)
    RTC_REG_MODE = 0x00; 

    // 2. Ativa o HOLD (Bit 0 do Reg Controle) para congelar a leitura
    // Isso impede que o tempo mude enquanto lemos os registradores
    RTC_REG_CTRL = 0x01;

    // 3. Lê os valores brutos de 4 bits e converte BCD para Decimal
    seg  = bcd_to_dec(RTC_SEC_DEZ, RTC_SEC_UNI);
    min  = bcd_to_dec(RTC_MIN_DEZ, RTC_MIN_UNI);
    hora = bcd_to_dec(RTC_HOR_DEZ, RTC_HOR_UNI);
    dia  = bcd_to_dec(RTC_DIA_DEZ, RTC_DIA_UNI);
    mes  = bcd_to_dec(RTC_MES_DEZ, RTC_MES_UNI);
    
    // O ano no chip costuma ser de 00 a 99
    ano_bcd = bcd_to_dec(RTC_ANO_DEZ, RTC_ANO_UNI);

    // 4. Desativa o HOLD para o chip voltar a atualizar o tempo normalmente
    RTC_REG_CTRL = 0x00;

    // 5. Ajuste do ano para o padrão FatFs (anos desde 1980)
    // Se o chip retornar 26 (referente a 2026): 2026 - 1980 = 46.
    if (ano_bcd >= 80) {
        ano_fat = ano_bcd - 80; // Para anos entre 1980 e 1999
    } else {
        ano_fat = (2000 + ano_bcd) - 1980; // Para anos de 2000 em diante
    }

    // 6. Compacta tudo no formato de 32 bits da FatFs
    return ((ano_fat & 0x7F) << 25) |  // Ano (7 bits)
           ((mes     & 0x0F) << 21) |  // Mês (4 bits)
           ((dia     & 0x1F) << 16) |  // Dia (5 bits)
           ((hora    & 0x1F) << 11) |  // Hora (5 bits)
           ((min     & 0x3F) << 5)  |  // Minuto (6 bits)
           ((seg / 2) & 0x1F);         // Segundo / 2 (5 bits)
}



// FUNÇÃO DO DRIVER PARA CONFIGURAR A HORA
// Valores normais: ano (00-99), mes (1-12), dia (1-31), hora (0-23), min (0-59), seg (0-59)
void set_rtc_time(uint8_t ano, uint8_t mes, uint8_t dia, uint8_t hora, uint8_t min, uint8_t seg) {
    
    // 1. Garante que estamos no Banco 0 (Modo 0)
    RTC_REG_MODE = 0x00;

    // 2. Ativa o HOLD para congelar os contadores internos enquanto escrevemos
    RTC_REG_CTRL = 0x01;

    // 3. Quebra os valores em BCD e escreve nos registradores (4 bits por vez)
    RTC_SEC_UNI = seg % 10;
    RTC_SEC_DEZ = seg / 10;

    RTC_MIN_UNI = min % 10;
    RTC_MIN_DEZ = min / 10;

    RTC_HOR_UNI = hora % 10;
    RTC_HOR_DEZ = hora / 10;

    RTC_DIA_UNI = dia % 10;
    RTC_DIA_DEZ = dia / 10;

    RTC_MES_UNI = mes % 10;
    RTC_MES_DEZ = mes / 10;

    RTC_ANO_UNI = ano % 10;
    RTC_ANO_DEZ = ano / 10;

    // 4. Libera o HOLD. O relógio começa a rodar imediatamente com o novo horário!
    RTC_REG_CTRL = 0x00;
}


// Defina uma função simples de delay baseado em microsegundos ou ciclos de CPU
void rtc_delay(void) {
    // Ajuste este loop/delay conforme a velocidade da sua CPU
    for(volatile int i=0; i<500; i++); 
}

void rtc_write_config_byte(uint16_t endereco, uint8_t valor) {
    RTC_REG_MODE =RTC_MODE_RAM_BLOCK1;
//    rtc_delay(); // Dá tempo para o chip chavear o modo
    
    (*(volatile uint8_t *)(RTC_BASE + endereco))  = (valor & 0x0F);
//    rtc_delay(); // Dá tempo para consolidar a escrita
    
    RTC_REG_MODE = RTC_MODE_CLOCK;
//    rtc_delay();
}

uint8_t rtc_read_config_byte(uint16_t endereco) {
    uint8_t dado;
    
    RTC_REG_MODE = RTC_MODE_RAM_BLOCK1;
//    rtc_delay(); // Tempo de chaveamento do banco
    
    dado = ((*(volatile uint8_t *)(RTC_BASE + endereco)) & 0x0F);
//    rtc_delay();
    
    RTC_REG_MODE = RTC_MODE_CLOCK;
//    rtc_delay();
    
    return dado;
}

uint8_t rtc_read_ram_byte(uint8_t endereco) {
    uint8_t dado;
    if( mode == '0' ){
        printf("Reading memory block 0\n");
        RTC_REG_MODE = RTC_MODE_RAM_BLOCK0;
    }
    if( mode == '1' ){
        printf("Reading memory block 1\n");
        RTC_REG_MODE = RTC_MODE_RAM_BLOCK1;
    }
    rtc_delay();

    switch(endereco){
        case RAM_POS_00:
            dado  = ADDR_RAM_POS_00 & 0x0F;
    rtc_delay();
            dado |= (ADDR_RAM_POS_0A<<4) & 0xF0;
            break;
        case RAM_POS_01:
            dado  = ADDR_RAM_POS_01 & 0x0F;
    rtc_delay();
            dado |= (ADDR_RAM_POS_0B<<4) & 0xF0;
            break;
        case RAM_POS_02:
            dado  = ADDR_RAM_POS_02 & 0x0F;
    rtc_delay();
            dado |= (ADDR_RAM_POS_06<<4) & 0xF0;
            break;
        case RAM_POS_03:
            dado  = ADDR_RAM_POS_03 & 0x0F;
    rtc_delay();
            dado |= (ADDR_RAM_POS_0D<<4) & 0xF0;
            break;
        case RAM_POS_04:
            dado  = ADDR_RAM_POS_04 & 0x0F;
    rtc_delay();
            dado |= (ADDR_RAM_POS_0E<<4) & 0xF0;
            break;
        case RAM_POS_05:
            dado  = ADDR_RAM_POS_05 & 0x0F;
    rtc_delay();
            dado |= (ADDR_RAM_POS_0F<<4) & 0xF0;
        case 6:
            dado  = ADDR_RAM_POS_06 & 0x0F;    
            break;
    }    
    RTC_REG_MODE = RTC_MODE_CLOCK;    
    return dado;
}

void rtc_write_ram_byte(uint16_t endereco,uint8_t dado) {
    if( mode == '0' ){
        printf("Writing memory block 0\n");
        RTC_REG_MODE = RTC_MODE_RAM_BLOCK0;
    }
    if( mode == '1' ){
        printf("Writing memory block 1\n");
        RTC_REG_MODE = RTC_MODE_RAM_BLOCK1;
    }
    rtc_delay();
      
    switch(endereco){
        case RAM_POS_00:
            ADDR_RAM_POS_00 = dado & 0x0F;
    rtc_delay();
            ADDR_RAM_POS_0A = (dado>>4) & 0x0F;
            break;
        case RAM_POS_01:
            ADDR_RAM_POS_01 = dado & 0x0F;
    rtc_delay();
            ADDR_RAM_POS_0B = (dado>>4) & 0x0F;
            break;
        case RAM_POS_02:
            ADDR_RAM_POS_02 = dado & 0x0F;
    rtc_delay();
            ADDR_RAM_POS_06 = (dado>>4) & 0x0F;
            break;
        case RAM_POS_03:
            ADDR_RAM_POS_03 = dado & 0x0F;
    rtc_delay();
            ADDR_RAM_POS_0D = (dado>>4) & 0x0F;
            break;
        case RAM_POS_04:
            ADDR_RAM_POS_04 = dado & 0x0F;
    rtc_delay();
            ADDR_RAM_POS_0E = (dado>>4) & 0x0F;
            break;
        case RAM_POS_05:
            ADDR_RAM_POS_05 = dado & 0x0F;
    rtc_delay();
            ADDR_RAM_POS_0F = (dado>>4) & 0x0F;
        case 6:
            ADDR_RAM_POS_06 = dado & 0x0F;
            break;
    }
    
    RTC_REG_MODE = RTC_MODE_CLOCK;

}


unsigned char ler_segundo()
{
    unsigned char sec=0;
    RTC_REG_MODE = RTC_MODE_CLOCK;
    rtc_delay();

    sec = ((*(RTC_BASE_ADDR + 0x03) & 0x0F) << 4) | (*(RTC_BASE_ADDR + 0x01) & 0x0F);

    return sec;
}

char rtc_inicializar(void) {
    // 1. Desativa registradores de teste e alarmes (zera lixo do power-on)
    unsigned char ch;
    printf("Searching fo RTC ");
    RTC_REG_CTRL = 0x00;
    rtc_delay();

    ch = rtc_read_config_byte(0x01) & 0x0F;
    if( ch == 0xA ){
        printf("rp5c01a already on..\n");
        return 1;
    }else{
        rtc_write_config_byte(0x01,0x0A);
        ch = rtc_read_config_byte(0x01) & 0x0F;
        if( ch == 0xA ){
            printf("rp5c01a found..\n");
            return 0;
        }else{
            printf("NOT PRESENT..\n");
            return 0;
        }
    }
    // 2. Escreve 0x0C no MODE:
    // Bit 3 = 1 (Liga o oscilador / Start)
    // Bit 2 = 1 (Seleciona o modo Clock)
    // Bit 1 = 0 (Formato 24h. Se quiser 12h, use 1)
    // Bit 0 = 0
    RTC_REG_MODE = RTC_MODE_CLOCK; 
    rtc_delay();
    return 1;
} 
