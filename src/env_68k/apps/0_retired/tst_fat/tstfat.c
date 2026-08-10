#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ff.h"        // Cabeçalho principal da FatFs
#include "diskio.h"    // Interface de baixo nível da FatFs
#include "drv_kbd.h"
#include "ata.h"
#include "../kbd/ringbuffer.h"

// Estrutura do seu driver que guarda a partição encontrada no MBR
// (Assumindo a estrutura que você já fez funcionar)
typedef unsigned long sector_t;
struct partition {
    sector_t base;                                                                                                                               
    sector_t size;
    uint8_t fstype;
    uint8_t flags;
};
struct ata_drive {
    struct partition parts[4];
};
extern struct ata_drive drives[]; 
//extern int ata_read_sector(int sector, char *buffer);
void dump_memory(long addr,int size);
extern FATFS FatFs;      // Objeto de controle do sistema de arquivos (Work area)
DIR Dir;          // Objeto de diretório
FILINFO Fno;      // Estrutura que recebe os metadados do arquivo/pasta



// Funções stubs obrigatórias para compilar o diskio.c (mantenha o básico)
//DSTATUS disk_status (BYTE pdrv) { return (pdrv == 0) ? 0 : STA_NOINIT; }
//DSTATUS disk_initialize (BYTE pdrv) { return (pdrv == 0) ? 0 : STA_NOINIT; }
extern char syspath[128];

void listar_diretorio_raiz(void) {
    FRESULT res;
    int i=0;
    char path[128];
    strcpy(path,syspath);    
    if( path[0] >= 'A' && path[0] <= 'H'){
        path[0] = 'A' - 0x41;
    }

    res = f_opendir(&Dir, path);
    if (res != FR_OK) {
        printf("Erro ao abrir diretorio: %d\n", res);
        return;
    }

    // Agora o loop lê APENAS UMA VEZ por iteração
    for (;;) {
        //dump_memory((long)&FatFs.win[0],512);
        res = f_readdir(&Dir, &Fno);                   // Lê a próxima entrada
        //printf("Nome: %02x%02x%02x%02x%02x%02x%02x%02x  ",Fno.fname[0],Fno.fname[1],Fno.fname[2],Fno.fname[3],Fno.fname[4],Fno.fname[5],Fno.fname[6],Fno.fname[7]);
       // printf("Nome: %02x%02x%02x%02x%02x%02x  ",Fno.fname[8],Fno.fname[9],Fno.fname[10],Fno.fname[11],Fno.fname[12],Fno.fname[13]);
        
        if (res != FR_OK || Fno.fname[0] == 0) break;  // Erro ou Fim do diretório

        // Ignora arquivos ocultos ou do sistema se quiser, ou imprime tudo
        //if (Fno.fname[0] == '.') continue; 

        // Verifica se é um diretório ou um arquivo
        if (Fno.fattrib & AM_DIR) {
//            printf(" [DIR]  %s\n", Fno.fname);
            printf("%04d/%02d/%02d %02d:%02d    <DIR> \t%s\n", 1980 + ((Fno.fdate >> 9) & 0x7F),
                    (Fno.fdate >> 5) & 0xF, Fno.fdate & 0x1F,
                    Fno.ftime >> 11, (Fno.ftime >> 5) & 0x3F, Fno.fname);

        } else {
            // Imprime o nome original e o tamanho
           //printf(" FILE   %s  (%ld bytes)\n", Fno.fname, (unsigned long)Fno.fsize);
            printf("%04d/%02d/%02d %02d:%02d %8ld \t%12s\n", 1980 + ((Fno.fdate >> 9) & 0x7F),
                    (Fno.fdate >> 5) & 0xF, Fno.fdate & 0x1F, Fno.ftime >> 11,
                    (Fno.ftime >> 5) & 0x3F, Fno.fsize, Fno.fname);

        }
        i++;
        if( i>=30 ){
            printf("Press to continue: ");
            while ( res == true ){      //le do teclado ps2
                res = ring_buf_is_empty();
            }
            //ch = ring_buf_get();    //le do teclado ps2
            i=0;
        }
    }

    f_closedir(&Dir);
    printf("-----------------------------------\n");
}

void open_dir(){
    FRESULT res;

    //printf("\n--- Montando Sistema de Arquivos FAT ---\n");
    //res = f_mount(&FatFs, "", 0);
    //if (res != FR_OK) {
    //    printf("Erro ao montar FAT: %d\n", res);
    //    return;
    //}
    //printf("FAT montado com sucesso!\n");
    //printf("--- Listando Diretorio Raiz (/) ---\n");
    
    res = f_opendir(&Dir, "/");
    if (res != FR_OK) {
        printf("Erro ao abrir diretorio: %d\n", res);
        return;
    }

    for (;;) {
        res = f_readdir(&Dir, &Fno);
        if (res != FR_OK || Fno.fname[0] == 0) break;

        if (!(Fno.fattrib & AM_DIR)) {
           // printf("Achei o arquivo: %s. Tentando abrir...\n", Fno.fname);
            
            // Tenta abrir usando o exato ponteiro de string que veio do disco
            FIL teste_arq;
            FRESULT teste_res = f_open(&teste_arq, Fno.fname, FA_READ);
            
            if (teste_res == FR_OK) {
               // printf("-> BINGO! Abriu direto da listagem!\n");
                f_close(&teste_arq);
            } else {
               // printf("-> Erro mesmo direto da listagem: %d\n", teste_res);
            }
        }
    }

    f_closedir(&Dir);
    printf("-----------------------------------\n");

}

// Reutilizando as variáveis globais da FatFs
extern FATFS FatFs;
static __attribute__((aligned(2)))FIL Arq;               // Objeto de controle do arquivo (File Object)

// Buffer temporário para ler os blocos do arquivo de texto.
// 128 bytes é um tamanho seguro e amigável para a memória do 68k na bancada.
#define BUFFER_SIZE 128
char buffer_leitura[BUFFER_SIZE];

void ler_e_exibir_joblog(char * filename) {
    FRESULT res;
    UINT bytes_lidos;
    char arquivo[16];
    sprintf(arquivo,"%s",filename);

    //res = f_mount(&FatFs, "", 0);
    //if (res != FR_OK) {
    //    printf("Erro ao montar FAT: %d\n", res);
    //    return;
    //}
    printf("FAT montado com sucesso!\n");
    printf("drives[0].parts[0].base[%d]\n",drives[0].parts[0].base);


    printf("\n--- Abrindo Arquivo %s ---\n",arquivo);

    // f_open(Objeto_File, Caminho, Modo_de_Acesso)
    // FA_READ: Abre o arquivo apenas para leitura
    memset(&Arq,0,sizeof(FIL));
    res = f_open(&Arq, arquivo, FA_READ);
    
    if (res != FR_OK) {
        // Se retornar erro 4 (FR_NO_FILE), o arquivo não existe na raiz do cartão
        printf("Erro ao abrir %s Codigo: %d\n",filename, res);
        return;
    }    
    printf("Arquivo aberto com sucesso! Conteudo:\n");
    printf("--------------------------------------------------\n");

    f_lseek(&Arq, 0);
    // Loop de leitura: Lê o arquivo em blocos até chegar ao fim (EOF)
    do {
        res = f_read(&Arq, buffer_leitura, BUFFER_SIZE - 1, &bytes_lidos);
        printf("bytes_lidos %d res %d\n",bytes_lidos,res);
        if (res != FR_OK) {
            printf("\n[Erro durante a leitura do arquivo: %d]\n", res);
            break;
        }

        if (bytes_lidos > 0) {
            // Insere o terminador de string logo após o último byte lido
            // Isso evita que o printf imprima lixo que sobrou no buffer anterior
            buffer_leitura[bytes_lidos] = '\0';
            
            // Cospe o bloco de texto direto no terminal serial
            printf("%s", buffer_leitura);
        }

    } while (bytes_lidos > 0); // Enquanto ler mais que 0 bytes, o arquivo não acabou

    printf("\n--------------------------------------------------\n");

    // Obrigatório: Fechar o arquivo para liberar o objeto na FatFs
    f_close(&Arq);
    printf("Arquivo [%s] fechado.\n",arquivo);
}


int le_setor(int sector)
{


	char *buffer=( char *)0x82000;

	ata_read_sector(sector, buffer);

    dump_memory(0x82000,0x200);
	return 1;
}

int wr_setor(int sector)
{
	char *buffer=( char *)0x82000;

	ata_write_sector(sector, buffer);

	return 1;
}
/*
Dir teste que foi criado pelo PC.

0008009c  43 4f 4d 41 4e 44 4f 53 54 58 54 20 00 00 00 00 - COMANDOSTXT ....
000800ac  00 00 d8 5c 00 00 8d 08 8e 5a d1 00 3f 01 00 00 - ...\.....Z..?...
000800bc  42 55 49 4c 44 20 20 20 42 41 54 20 00 00 00 00 - BUILD   BAT ....
000800cc  00 00 d8 5c 00 00 2a 13 8e 5a d2 00 01 01 00 00 - ...\..*..Z......
000800dc  41 20 20 20 20 20 20 20 54 58 54 20 00 00 00 00 - A       TXT ....
000800ec  00 00 d8 5c 00 00 dc 9c 8b 5a d3 00 21 00 00 00 - ...\.....Z..!...
000800fc  54 45 53 54 45 20 20 20 20 20 20 10 00 00 00 00 - TESTE      .....
0008010c  00 00 d8 5c 00 00 6a 7f d8 5c de 00 00 00 00 00 - ...\..j.\......
0008011c  50 41 55 4c 4f 20 20 20 20 20 20 10 00 00 00 00 - PAULO      .....
0008012c  00 00 d8 5c 00 00 75 7f d8 5c df 00 00 00 00 00 - ...\..u.\......
0008013c  41 42 43 44 45 46 47 48 20 20 20 10 00 00 00 00 - ABCDEFGH   .....
0008014c  00 00 d8 5c 00 00 7d 7f d8 5c e0 00 00 00 00 00 - ...\..}.\......
0008015c  50 41 55 4c 4f 53 49 4c 20 20 20 10 00 00 00 00 - PAULOSIL   .....
0008016c  00 00 d8 5c 00 00 34 80 d8 5c e1 00 00 00 00 00 - ...\..4..\......
0008017c  50 41 55 4c 4f 53 49 56 20 20 20 10 00 00 00 00 - PAULOSIV   .....
0008018c  00 00 d8 5c 00 00 53 80 d8 5c e2 00 00 00 00 00 - ...\..S..\......

abcdefgh criado pelo orion68os

00082000  43 4f 4d 41 4e 44 4f 53 54 58 54 20 00 00 00 00 - COMANDOSTXT ....
00082010  00 00 d8 5c 00 00 8d 08 8e 5a d1 00 3f 01 00 00 - ...\.....Z..?...
00082020  42 55 49 4c 44 20 20 20 42 41 54 20 00 00 00 00 - BUILD   BAT ....
00082030  00 00 d8 5c 00 00 2a 13 8e 5a d2 00 01 01 00 00 - ...\..*..Z......
00082040  41 20 20 20 20 20 20 20 54 58 54 20 00 00 00 00 - A       TXT ....
00082050  00 00 d8 5c 00 00 dc 9c 8b 5a d3 00 21 00 00 00 - ...\.....Z..!...

00082060  41 42 43 44 45 46 47 48 20 20 20 10 00 00 00 00 - ABCDEFGH   .....
00082070  21 5a 00 00 00 00 00 00 21 5a dd 00 00 00 00 00 - !Z......!Z......
0008013c  41 42 43 44 45 46 47 48 20 20 20 10 00 00 00 00 - ABCDEFGH   .....
0008014c  00 00 d8 5c 00 00 7d 7f d8 5c e0 00 00 00 00 00 - ...\..}.\......

Analisando os campos críticos em Little Endian (bytes invertidos):
Status do Boot (0x000821BE): 00 (Não ativa/Não bootável por padrão).
Tipo de Partição (0x000821C2): 01 $\rightarrow$ FAT12 
(Sistema de arquivos clássico do DOS / Disquetes).
Setor Inicial LBA (0x000821C6): 20 00 00 00 $\rightarrow$ 
Invertendo para Little Endian vira 0x00000020, que é o Setor 32 do disco.
Total de Setores (0x000821CA): a0 7c 00 00 $\rightarrow$ 0x00007CA0 
(31.904 setores, cerca de 16MB).

Para o sistema de arquivos FAT16, o byte no endereço 0x000821C2 muda 

Se o sistema fosse FAT16, o byte no endereço 0x000821C2 seria um destes valores, 
dependendo do tamanho da partição:


000821b0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 - ................
000821c0  01 00(01)01 60 f2 20 00 00 00 a0 7c 00 00 00 00 - ....`. ....|....
000821d0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 - ................
000821e0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 - ................
000821f0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 55 aa - ..............U.

01 : FAT12 antiga 
04 : FAT16 antiga (para partições menores que 32 MB).
06 : FAT16 padrão (para partições maiores que 32 MB).
0E : FAT16 moderna (para partições maiores que 32 MB que usam LBA).

No seu caso atual, como o valor é 01, o sistema é FAT12.


A:/>MKDIR TESTE
path to mkdir 0:/TESTE
Diretorio livre... não um com mesmo nome...
create_chain retornou [2742]
calling dir_clear
pdrv=[0] sector=[9] count[1]
pdrv=[0] sector=[21] count[1]
clst2sect: returned sect 21977
pdrv=[0] sector=[21977] count[1]
pdrv=[0] sector=[21978] count[1]
pdrv=[0] sector=[21979] count[1]
pdrv=[0] sector=[21980] count[1]
pdrv=[0] sector=[21981] count[1]
pdrv=[0] sector=[21982] count[1]
pdrv=[0] sector=[21983] count[1]
pdrv=[0] sector=[21984] count[1]
dir_clear:n=[8] fs->csize=[8]: returning  0
calling dir_register
pdrv=[0] sector=[
] count[1]
calling creations functions
calling sync_fs
pdrv=[0] sector=[29] count[1]
disk_ioctl: pdrv=[0] cmd=[0] buff=[0x00000000]
disk_ioctl: retorno OK
calling resp[0]
Returning res=[0]
A:/>LS
 
 
1999/01/07 07:03 66785 	COMMAND.COM 
1994/09/13 02:49 4704 	SR.EXE 
1994/09/13 02:43 25104 	ST.EXE 
2001/11/03 20:38 67 	DOOR1.BAK 
1998/12/10 00:25 322 	START.BAT 
1993/11/22 09:08 8693 	NE2000.COM 
1997/09/05 17:15 47 	NET.CFG 
1991/07/31 10:47 51201 	NETX.COM 
1992/09/28 09:46 26511 	PDIPX.COM 
1998/12/10 04:35 199 	AUTOEXEC.BAT 
2003/06/12 20:14 0 	MANUTENC.ON 
2002/06/03 16:53 103 	REDE.BAT 
1998/12/10 02:04 167 	AUTOEXEC.BAK 
2002/06/03 13:50    <DIR> 	QEMM/ 
2002/06/03 13:50    <DIR> 	AFAXSERV/ 
2002/06/03 13:50    <DIR> 	AFAXUSER/ 
2002/06/03 18:07    <DIR> 	DOS/ 
2002/06/03 13:51    <DIR> 	DRDOS/ 
2002/06/03 16:38    <DIR> 	TEMP/ 
2002/06/03 13:53    <DIR> 	AFAXLINK/ 
2001/11/03 20:34 15 	DOOR2.BAT 
2001/10/17 14:28 76 	DOORDOS.BAT 
2001/10/22 14:10 22224 	DOORMNP.EXE 
2001/10/24 18:42 8560 	DOORTCP.EXE 
2001/10/17 14:34 32243 	DOORWAY.EXE 
2001/10/19 09:27 227 	DOORWAY.WLC 
1996/02/02 12:00 416 	DWREDIR.COM 
1997/05/20 00:00 56719 	FDSZ.EXE 
1995/05/03 15:45 37558 	RLFOSSIL.EXE 
1995/04/07 15:18 9024 	RTL8029.COM 
2001/10/20 22:28 38 	RZ.BAT 
2002/06/03 19:18 21 	CLOSE.BAT 
2001/10/20 22:28 41 	SZ.BAT 
2003/06/12 19:24 1601 	WATTCP.CFG 
2002/06/03 14:10 1601 	WATTCP.BAK 
2003/06/12 19:24 68 	DOOR1


*/