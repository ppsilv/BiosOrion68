#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include <vga_video.h>
#include <fatfs/ff.h>

#include "orion68.h"
#include "commands.h"
#include "decodecmd.h"
#include "ata.h"
#include "io.h"

#include "rtc.h"
#include "srecord.h"
#include "diskio.h"
#include "drv_uart.h"
#include "picow.h"
#include "color.h"
#include "../kbd/ringbuffer.h"


#define PICO_STATUS_REG  (*(volatile uint8_t *)0xFF9103)
#define PICO_STATE_IDLE       0x00
#define PICO_STATE_HAS_FILE   0x01
#define PICO_STATE_EOF        0x02

extern FATFS FatFs;      // Objeto de controle do sistema de arquivos (Work area)
extern char syspath[128];
extern void main_teste_teclado(void); 
extern char teste01();
extern bool receber_arquivo_do_pico(uint8_t *destino_ram,uint8_t reg);
extern bool noblk_receber_arquivo_do_pico(uint8_t *destino_ram, uint8_t preg);
extern bool receber_setor_do_pico(uint8_t *destino_ram, uint16_t sector);
extern void pico_write_ch(uint8_t ch);

static DIR Dir;          // Objeto de diretório
static FILINFO Fno;      // Estrutura que recebe os metadados do arquivo/pasta
static __attribute__((aligned(2)))FIL Arq;               // Objeto de controle do arquivo (File Object)
extern struct ata_drive drives[]; 

extern void printerro(int eno);
extern void dump_memory(void * addr,int size);
extern const cmd_entry_t g_cmd_table[];

extern unsigned long get_system_tick(void);

static int fromhex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';

    if (c >= 'A' && c <= 'F')
        return 10 + c - 'A';

    return -1;
}
static uint8_t serial_has_char(){
    return 1;
}
void do_help(int argc, char *argv[])
{
	int i = 0;

	printf("\nAvailable commands:\n");

	while (g_cmd_table[i].name)
    {
		printf("%12s - %s\n", g_cmd_table[i].name, g_cmd_table[i].helpme);
		++i;
	}
}
void do_binfile(int argc, char *argv[])
{
    char *rec_buf = (char *)0x00100000;
    char *buffer = (char *)0x00100000;
    unsigned int timeout = 10000;
    unsigned int bytes_transferred = 0;

    if (argc != 1)
    {
        printf("binfile: Filename should be specified\n");
    }

    char *filename = argv[0];
    printf("Download binary file to %s, waiting for serial transfer start\n", filename);

    DISABLE_INTERRUPTS();
    // wait for the first character
    *rec_buf++ = uart0_read();
    bytes_transferred++;

    // recieve characters until transmit stops
    while (timeout--)
    {
        if (serial_has_char())
        {
            *rec_buf++ = uart0_read();
            bytes_transferred++;
            timeout = 10000;

            if (((uint32_t)rec_buf & 0x000000FF) == 0)
                uart0_write('.');
        }
    }
    printf("\nData recieved, saving to file\n");
    ENABLE_INTERRUPTS();

    FIL dest;

    if (f_open(&dest, filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    {
        printf("Unable to open destination file %s\n", filename);
        return;
    }

    unsigned int bytes_written = 0;
    if (f_write(&dest, buffer, bytes_transferred, &bytes_written) != FR_OK)
    {
        printf("Unable to write to destination file %s\n\t", filename);
    }
    else
        printf("Written %d bytes to %s\n", bytes_written, filename);

    f_close(&dest);
}

void do_binmem(int argc, char *argv[])
{
    char *rec_buf = (char *)0x00100000;
    unsigned int timeout = 10000;
    unsigned int bytes_transferred = 0;

    if (argc != 1)
    {
        printf("binmem: memory address should be specified\n");
    }

    unsigned long addr;
    addr = strtoul(argv[0], NULL, 16);

    printf("Download binary file to 0x%06x, waiting for serial transfer start\n", addr);

    rec_buf = (char *)addr;

    DISABLE_INTERRUPTS();
    // wait for the first character
    *rec_buf++ = uart0_read();
    bytes_transferred++;

    // recieve characters until transmit stops
    while (timeout--)
    {
        if (serial_has_char())
        {
            *rec_buf++ = uart0_read();
            bytes_transferred++;
            timeout = 10000;

            if (((uint32_t)rec_buf & 0x000000FF) == 0)
                uart0_write('.');
        }
    }
    ENABLE_INTERRUPTS();
    printf("\nWritten %d bytes to memory at 0x%06X\n", bytes_transferred, addr);
}
void do_copyfile(int argc, char *argv[])
{
    FIL src;
    FIL dest;

    char *srcname = argv[0];
    char *destname = argv[1];

    char *buffer = NULL;

    if (f_open(&src, srcname, FA_READ) == FR_OK)
    {
        unsigned int len = f_size(&src);

        buffer = malloc(len);
        if (buffer == NULL)
        {
            printf("Unable to allocate %d for file.\n", len);
            f_close(&src);
            return;
        }

        unsigned int bytes_read = 0;

        if (f_read(&src, buffer, len, &bytes_read) != FR_OK)
        {
            printf("Unable to load source file.\n");
            f_close(&src);
            return;
        }

        if (f_open(&dest, destname, FA_WRITE | FA_CREATE_NEW) != FR_OK)
        {
            printf("Unable to open destination file %s\n", destname);
            f_close(&src);
            free(buffer);
            return;
        }
        unsigned int bytes_written = 0;
        if (f_write(&dest, buffer, len, &bytes_written) != FR_OK)
        {
            printf("Unable to create destination file %s\n\t", destname);
        }
        else
            printf("Copied %d bytes from %s to %s\n", bytes_written, srcname, destname);
    }
    else
    {
        printf("Unable to open source file %s\n", srcname);
        return;
    }

    free(buffer);

    f_close(&src);
    f_close(&dest);
}

void do_cat(int argc, char *argv[])
{
    FRESULT res;
    UINT bytes_lidos;
    char arquivo[16];
    char buffer_leitura[512];

    sprintf(arquivo,"%s",argv[0]);

    memset(&Arq,0,sizeof(FIL));
    res = f_open(&Arq, arquivo, FA_READ);
    if (res != FR_OK) {
        // Se retornar erro 4 (FR_NO_FILE), o arquivo não existe na raiz do cartão
        printf("Erro ao abrir %s Codigo: %d\n",argv[0], res);
        return;
    }    
    
    printf("Conteudo: %s\n",argv[0]);
    printf("--------------------------------------------------\n");
    f_lseek(&Arq, 0);
    // Loop de leitura: Lê o arquivo em blocos até chegar ao fim (EOF)
    do {
        res = f_read(&Arq, buffer_leitura, 512 - 1, &bytes_lidos);
        //printf("bytes_lidos %d res %d\n",bytes_lidos,res);
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
    //printf("Arquivo [%s] fechado.\n",arquivo);

}

void do_cd(int argc, char *argv[])
{
    FRESULT fr;

    fr = f_chdir(argv[0]);
    if(fr != FR_OK)
       printerro(fr);
}

void do_delete(int argc, char *argv[])
{
    FRESULT fr;
    const char *path;

    path = argv[0];

    fr = f_unlink(path);

    if (fr != FR_OK)
    {
       // printf("Error deleting file %s: ", path);
       printerro(fr);
    }
}
void do_dump(int argc, char *argv[]){
    unsigned long start, count;

    //printf("numargs=%d args[0]=%s,args[1]=%s,args[2]=%s\n",argc,argv[0],argv[1],argv[2]);

    start = strtoul((const char *)argv[0], NULL, 16);
    count = strtoul((const char *)argv[1], NULL, 16);


    //printf("do_dump [%lu] [%lu]",start,count);

    dump_memory((void*)start, count);
}

void do_exit(int argc, char *argv[]){
    f_mount(NULL, "0:", 0);
    printf("YOU CAN TURN OFF THE SYSTEM.\n");
}
void do_ideinit(int argc, char *argv[])
{
    FRESULT fr;
    fr = f_mount(&FatFs, "", 0);
    if (fr != FR_OK) {
        printf("PANIC: Erro ao montar FAT: %d\n", fr);
    }else{
        printf(": FAT success mounted!\n");
    }
}

void do_idemode(int argc, char *argv[])
{
    char mode=0;
    if(argc == 1){
        mode = atoi(argv[0]);
        printf("Setting ide mode to %d\n",mode);
        set_ide_bus_mode(mode);
    }
}
void do_ls(int argc, char *argv[]){
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


/*void do_ls(int argc, char *argv[])
{
    FRESULT fr;
    const char *filename;
    DIR fat_dir;
    FILINFO fat_file;
    uint16_t dir = 1;
    char path[128];
    strcpy(path,syspath);    
    uint16_t total_files=0;

    if(argc > 0){
        strcat(path,argv[0]);
    }
    if( path[0] >= 'A' && path[0] <= 'H'){
        path[0] = 'A' - 0x41;
    }

    fr = f_opendir(&fat_dir, path);
    if (fr != FR_OK)
    {
        printf("f_opendir(\"%s\"): ", path);
        printerro(fr);
        return;
    }

    while (1)
    {
        fr = f_readdir(&fat_dir, &fat_file);
        if (fr != FR_OK)
        {
            printf("f_readdir(): ");
           printerro(fr);
            break;
        }

        if (fat_file.fname[0] == 0){ // end of directory? 
            if( total_files == 0 ){
                printf("Empty directory\n");
            }
            break;
        }
        total_files++;
        
        filename = fat_file.fname;

        dir = fat_file.fattrib & AM_DIR;

        if (dir) {
            // directory 
            printf("%04d/%02d/%02d %02d:%02d    <DIR> \t%s/", 1980 + ((fat_file.fdate >> 9) & 0x7F),
                    (fat_file.fdate >> 5) & 0xF, fat_file.fdate & 0x1F,
                    fat_file.ftime >> 11, (fat_file.ftime >> 5) & 0x3F, filename);
        }else if (!(fat_file.fattrib & AM_HID))  {
            // regular file
            printf("%04d/%02d/%02d %02d:%02d %8ld \t%12s", 1980 + ((fat_file.fdate >> 9) & 0x7F),
                    (fat_file.fdate >> 5) & 0xF, fat_file.fdate & 0x1F, fat_file.ftime >> 11,
                    (fat_file.ftime >> 5) & 0x3F, fat_file.fsize, filename);
        }

        printf(" \n");
    }

    fr = f_closedir(&fat_dir);
    if (fr != FR_OK)
    {
        printf("f_closedir(): ");
       printerro(fr);
        return;
    }
}*/
char do_load_basic(int argc, char *argv[])
{
    FIL file;
    uint32_t size_buffer=(uint32_t )argv[2];
    int *ret_bytes_lidos = (int *)argv[3];
    char *buffer = NULL;

    if (f_open(&file, argv[0], FA_READ) == FR_OK)
    {
        unsigned int len = f_size(&file);
        if( len > size_buffer){
            printf("Unable to load file, file bigger than buffer.\n");
            buffer[0]='\0';
            f_close(&file);
            return  -1;
        }

        buffer = (char*)argv[1];

        unsigned int bytes_read = 0;

        if (f_read(&file, buffer, len, &bytes_read) != FR_OK)
        {
            printf("Unable to load file.\n");
            f_close(&file);
            return -1;
        }
        
        if (ret_bytes_lidos != NULL) {
            *ret_bytes_lidos = bytes_read;
        }

        printf("Loaded %d bytes from file %s to location %lX\n", bytes_read, argv[0], (uint32_t)buffer);
    }
    else
    {
        printf("Unable to open file %s\n", argv[0]);
    }

    if (argc == 1)
        free(buffer);

    f_close(&file);
    return 0;
}
void do_loadmem(int argc, char *argv[])
{
    FIL file;
    uint32_t location;
    char *buffer = NULL;

    if (argc == 2)
        location = strtoul(argv[1], NULL, 16);


    if (f_open(&file, argv[0], FA_READ) == FR_OK)
    {
        unsigned int len = f_size(&file);

        if (argc == 2)
            buffer = (char *)location;
        else
        {
            buffer = malloc(len);
            if (buffer == NULL)
            {
                printf("Unable to allocate %d for file.\n", len);
                f_close(&file);
                return;
            }
        }
        unsigned int bytes_read = 0;

        if (f_read(&file, buffer, len, &bytes_read) != FR_OK)
        {
            printf("Unable to load file.\n");
            f_close(&file);
            return;
        }

        printf("Loaded %d bytes from file %s to location %lX\n", bytes_read, argv[0], (uint32_t)buffer);
    }
    else
    {
        printf("Unable to open file %s\n", argv[0]);
    }

    if (argc == 1)
        free(buffer);

    f_close(&file);
}
void do_mkdir_shel(int argc, char *argv[])
{
    FRESULT fr;
    char path[128];
    memset(path,0,128);
    int size = strlen(syspath);
    strcpy(path,&syspath[0]);
    if( path[size-1] != '/'){
    path[strlen(path)]='/';
    }
    strcat(path,argv[0]);
    if( path[0] >= 'A' && path[0] <= 'I'){
        path[0] = path[0] - 0x11;
    }
    
    printf("path to mkdir %s\n",path);
    fr = f_mkdir(path);

    if (fr != FR_OK) {
        printf("do_mkdir: Error creating folder %s: ", path);
        printerro(fr);
    }
}
void do_notimplemented(int argc, char *argv[])
{
    printf("This command is currently not implemented\n");
}
void do_rename_shel(int argc, char *argv[])
{
    FRESULT fr;
    const char *srcpath;
    const char *destpath;

    srcpath = argv[0];
    destpath = argv[1];

    fr = f_rename(srcpath, destpath);

    if (fr != FR_OK)
    {
        printf("Error renaming file %s to %s: ", srcpath, destpath);
       printerro(fr);
    }
}
void do_rmdir(int argc, char *argv[]){
FRESULT res;
char path[128];

    memset(path,0,128);
    int size = strlen(syspath);
    strcpy(path,&syspath[0]);
    if( path[size-1] != '/'){
        path[strlen(path)]='/';
    }
    strcat(path,argv[0]);
    if( path[0] >= 'A' && path[0] <= 'I'){
        path[0] = path[0] - 0x11;
    }
    res = f_rmdir(path);

    if (res == FR_OK) {
        printf("Directory remove succeded!\n");
    } else if (res == FR_NO_PATH || res == FR_NO_FILE) {
        printf("Erro: Directory not found.\n");
    } else if (res == FR_DENIED) {
        printf("Erro: Empty directory or protected directory .\n");
    } else {
        printf("Remove error: %d\n", res);
    }    
}

void do_runelf_fromhd(int argc, char *argv[]){
    const char *filename = argv[0] ;
    uint32_t entry = carregar_elf32_fatfs(filename);

    if (entry != 0) {
        // Salta e executa o programa no m68k
        void (*app)(void) = (void (*)(void))entry;
        app();
    } else {
        // Tratar erro (arquivo não encontrado ou desalinhado)
    }
}

void do_runelf(int argc, char *argv[]){
    // Retorna o endereço da primeira instrução
    uint8_t * buffer_elf_recebido = (uint8_t * )argv[0];
    uint8_t * mem_addr = (uint8_t *)argv[1];
    uint32_t entry_point = carregar_elf32(buffer_elf_recebido, mem_addr);

    if (entry_point != 0) {
        // Executa o binário a partir do entry point obtido
        void (*executar)(void) = (void (*)(void))entry_point;
        executar();
    }
}
void do_run(int argc, char *argv[])
{
    unsigned long start;
    start = strtoul(argv[0], NULL, 16);

    void (*entry)(void) = (void (*)(void))start;
    entry();
}
/*
 * Grava 'tamanho' bytes a partir do endereco 'origem' num arquivo chamado
 * 'nome_arquivo' no cartao SD. Sobrescreve o arquivo se ja existir.
 * Sobrescreve o arquivo se já existir (FA_CREATE_ALWAYS). 
 * Se você preferir dar erro em vez de sobrescrever, troco pra FA_CREATE_NEW.
 * Assume que f_mount() já foi chamado antes (montagem do FS geralmente é feita uma vez na inicialização, não a cada gravação).
 * nome_arquivo aceita path relativo tipo "dump.bin" ou 
 * path completo "0:/dumps/dump.bin", 
 * dependendo de como você configurou o FatFs (multi-drive ou não).
 *
 * Retorna 0 em sucesso, -1 em erro (mensagem impressa via printf).
 */
 void do_save(int argc, char *argv[]){
    FIL     arquivo;
    FRESULT fr;
    UINT    bytes_escritos;
    const void *origem = (const void *)0x82015;
    const char *nome_arquivo = argv[0];   /* usa direto, sem copiar */
    size_t tamanho;

    if (argc < 2) {
        printf("uso: save <nome_arquivo> <tamanho_hex>\n");
        return;
    }
   
    tamanho = strtoul(argv[1], NULL, 16);
   
    printf("Save file [%s] size of %d\n", nome_arquivo, tamanho);

    if (origem == NULL || nome_arquivo == NULL || tamanho == 0) {
        printf("dump_memoria_para_arquivo: parametros invalidos\n");
        return;
    }

    fr = f_open(&arquivo, nome_arquivo, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao abrir '%s' (erro %d)\n", nome_arquivo, fr);
        return;
    }

    fr = f_write(&arquivo, origem, (UINT)tamanho, &bytes_escritos);
    if (fr != FR_OK || bytes_escritos != tamanho) {
        printf("dump_memoria_para_arquivo: falha ao escrever '%s' (erro %d, escrito %u de %u bytes)\n",
               nome_arquivo, fr, bytes_escritos, (unsigned)tamanho);
        f_close(&arquivo);
        return;
    }

    fr = f_close(&arquivo);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao fechar '%s' (erro %d)\n", nome_arquivo, fr);
        return;
    }

    printf("dump_memoria_para_arquivo: '%s' gravado com sucesso (%u bytes)\n",
           nome_arquivo, (unsigned)tamanho);
}
void do_save2(int argc, char *argv[]){
    uint8_t status=0;
    status = PICO_STATUS_REG;
    if ( status == PICO_STATE_IDLE) {
        return;
    }
    if (status < 1 ) {
        return;
    }

    if(!noblk_receber_arquivo_do_pico((uint8_t *)0x82000,0) ){
        puts("noblk_receber_arquivo_do_pico CRC error...\n");
        return;
    }

    const void *origem = (const void *)0x82015;
    char *nome_arquivo1 = (void *)0x82008;
    *(nome_arquivo1+12)='\0';
    
    volatile uint8_t *p1 = (volatile uint8_t *)0x82004;

    uint32_t tamanho1 = ((uint32_t)p1[3] << 24) |
                       ((uint32_t)p1[2] << 16) |
                       ((uint32_t)p1[1] << 8)  |
                       ((uint32_t)p1[0]);
    tamanho1=tamanho1-0x15;
    //printf("0-origem [%08X] nome[%s] tamanho[%04x]\n",origem,nome_arquivo1,tamanho1);
    //dump_memory((void*)0x82000, 0x32);

    FIL     arquivo;
    FRESULT fr;
    UINT    bytes_escritos;
    const char *nome_arquivo = argv[0];   /* usa direto, sem copiar */
    size_t tamanho;

    nome_arquivo=nome_arquivo1;
    tamanho = strtoul(argv[1], NULL, 16);
    tamanho = tamanho1;
//    printf("1-origem [%08X] nome[%s] tamanho[%04x]\n",origem,nome_arquivo1,tamanho1-0x15);
//    printf("Save file [%s] size of %d\n", nome_arquivo, tamanho);

    if (origem == NULL || nome_arquivo == NULL || tamanho == 0) {
        printf("dump_memoria_para_arquivo: parametros invalidos\n");
        return;
    }

    fr = f_open(&arquivo, nome_arquivo, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao abrir '%s' (erro %d)\n", nome_arquivo, fr);
        return;
    }

    fr = f_write(&arquivo, origem, (UINT)tamanho, &bytes_escritos);
    if (fr != FR_OK || bytes_escritos != tamanho) {
        printf("dump_memoria_para_arquivo: falha ao escrever '%s' (erro %d, escrito %u de %u bytes)\n",
               nome_arquivo, fr, bytes_escritos, (unsigned)tamanho);
        f_close(&arquivo);
        return;
    }

    fr = f_close(&arquivo);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao fechar '%s' (erro %d)\n", nome_arquivo, fr);
        return;
    }

    //printf("dump_memoria_para_arquivo: '%s' gravado com sucesso (%u bytes)\n",nome_arquivo, (unsigned)tamanho);
}
char do_save_basic(int argc, char *argv[]){
    FIL     arquivo;
    FRESULT fr;
    UINT    bytes_escritos;
    int *ret_bytes_escritos = (int *)argv[3];
    const void *origem = (const void *)argv[1];
    const char *nome_arquivo = argv[0];   /* usa direto, sem copiar */
    size_t tamanho;

    if (argc < 2) {
        printf("uso: save <nome_arquivo> <tamanho_hex>\n");
        return -1;
    }

    tamanho = (size_t)argv[2];
    printf("Save file [%s] size of %d\n", nome_arquivo, tamanho);

    if (origem == NULL || nome_arquivo == NULL || tamanho == 0) {
        printf("dump_memoria_para_arquivo: parametros invalidos\n");
        return -1;
    }

    fr = f_open(&arquivo, nome_arquivo, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao abrir '%s' (erro %d)\n", nome_arquivo, fr);
        return -1;
    }

    fr = f_write(&arquivo, origem, (UINT)tamanho, &bytes_escritos);
    if (fr != FR_OK || bytes_escritos != tamanho) {
        printf("dump_memoria_para_arquivo: falha ao escrever '%s' (erro %d, escrito %u de %u bytes)\n",
               nome_arquivo, fr, bytes_escritos, (unsigned)tamanho);
        f_close(&arquivo);
        return -1;
    }
    if (ret_bytes_escritos != NULL) {
       *ret_bytes_escritos = bytes_escritos; // Ex: 512
    }
    fr = f_close(&arquivo);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao fechar '%s' (erro %d)\n", nome_arquivo, fr);
        return -1;
    }

    printf("dump_memoria_para_arquivo: '%s' gravado com sucesso (%u bytes)\n", nome_arquivo, (unsigned)tamanho);
    return 0;
}


void do_shst(int argc, char *argv[]){
    printf("Systemtick = %ld\n",get_system_tick());
}
void do_time(int argc, char *argv[])
{
    if (argc != 6)
    {
        printf("To set the date and time use: time <year> <mon> <day> <hour> <min> <sec>\n");
        return;
    }

    rtc_date_t date;
    date.tm_year = atoi(argv[0]);
    date.tm_mon  = atoi(argv[1]);
    date.tm_day  = atoi(argv[2]);
    date.tm_hour = atoi(argv[3]);
    date.tm_min  = atoi(argv[4]);
    date.tm_sec  = atoi(argv[5]);

    printf("Setting the date and time to %02d/%02d/%04d %02d:%02d:%02d\n", date.tm_day, date.tm_mon, date.tm_year, date.tm_hour, date.tm_min, date.tm_sec);
  //  rtc_set_time(&date);
}
void do_tstkbd(int argc, char *argv[])
{
    uint8_t cmd =(uint8_t ) strtoul((const char *)argv[0], NULL, 16);
    uint8_t data=(uint8_t ) strtoul((const char *)argv[1], NULL, 16);

    switch(cmd){
        case 0x01:
                bool res = receber_setor_do_pico((uint8_t *)0x82000,cmd);
                if( res )
                    printf("receber_setor_do_pico retornou[Success]\n"); 
                else
                    printf("receber_setor_do_pico retornou[Error]\n"); 
                break;
        case 0x02:
                setcolor(data,BLACK);
                break;
        case 0x03:    
                break;            
        case 0x04:    
                duart_led_op3_on();
                break;            
        case 0x05:    
                duart_led_op3_off();
                break;            
        case 0xFF:
                break;
        case 0x10:
                break;
        case 0x11:
                break;
        case 0x12:
                pico_write_ch(data);
                break;
        case  SECTOR_LOW_REG:
                send_sector_low(data);
                break;
        case  SECTOR_HIGH_REG:
                send_sector_high(data);
                break;
        case  SECTOR_SEC_LOAD_REG:
                send_read_cmd();
                break;
        case  SECTOR_READ_REG:
                read_sector((uint8_t *)0x82000);
                dump_memory((void*)0x82000, 256);
                break;
        default:        
                res = receber_arquivo_do_pico((uint8_t *)0x82000,cmd);
                if( res )
                    printf("receber_arquivo_do_pico retornou[Success]\n"); 
                else
                    printf("receber_arquivo_do_pico retornou[Error]\n"); 
    }
}

unsigned long get_system_tick(void) ;
void do_uptime(int argc, char *argv[])
{
    uint32_t uptime = get_system_tick();

    uptime /= 10;   // convert to seconds
    uint16_t seconds = (uint16_t)(uptime % 60);
    uptime /= 60;
    uint16_t minutes = (uint16_t)(uptime % 60);
    uptime /= 60;
    uint16_t hours = (uint16_t)(uptime % 24);
    uint16_t days = (uint16_t)(uptime /= 60);

    printf("Uptime: %d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);
}

void do_writemem(int argc, char *argv[])
{
    unsigned long value;
    unsigned char *ptr;
    int i, j, l;

    value = strtoul(argv[0], NULL, 16);
    ptr = (unsigned char*)value;

    /* This can deal with values like: 1, 12, 1234, 123456, 12345678.
       Values > 2 characters are interpreted as big-endian words ie
       "12345678" is the same as "12 34 56 78" */

    /* first check we're happy with the arguments */
    for (i = 1; i < argc; i++)
    {
        l = strlen(argv[i]);

        if (l != 1 && l % 2)
        {
            printf("Ambiguous value: \"%s\" (odd length).\n", argv[i]);
            return; /* abort! */
        }

        for (j = 0; j < l; j++)
            if(fromhex(argv[i][j]) < 0)
            {
                printf("Bad hex character \"%c\" in value \"%s\".\n", argv[i][j], argv[i]);
                return; /* abort! */
            }
    }

    /* then we do the write */
    for (i = 1; i < argc; i++)
    {
        l = strlen(argv[i]);
        if (l <= 2) /* one or two characters - a single byte */
            *(ptr++) = strtoul(argv[i], NULL, 16);
        else
        {
            /* it's a multi-byte value */
            j = 0;
            while(j < l)
            {
                value = (fromhex(argv[i][j]) << 4) | fromhex(argv[i][j+1]);
                *(ptr++) = (unsigned char)value;
                j += 2;
            }
        }
    }
}

void do_writemem1(int argc, char *argv[]){
    char text[64];
    char * addr=NULL;
    printf("argv[0]%s argv[1]%s\n",argv[0],argv[1]);
    if(argc < 2){
        printf("Usage: <address> <data>\n");
        return;
    }
    memcpy(text,argv[0],strlen(argv[0]));
    text[strlen(argv[0])]='\0';
    int textsize=strlen(text);
    addr = (char *)strtoul(argv[1],NULL,16);
    printf("Writing [%s] to address[%x] %d bytes\n",text,(long)addr,textsize);
    for(int i=0; i< strlen(text);i++){
        *(addr+i)=text[i];
    }
}



























void do_srecord(int argc, char *argv[])
{
    char *rec_buf = (char *)0x00300000;
    unsigned int timeout = 10000;

    printf("Download for S-Record file, waiting for serial transfer\n");

    // Disable printing to display as currently the display output is slow
    // This can cause timing issues if the display scrolls

    // wait for the first character
    *rec_buf++ = uart0_read();

    // recieve characters until transmit stops
    while (timeout--)
    {
        if (serial_has_char())
        {
            *rec_buf++ = uart0_read();
            timeout = 10000;
            if (((uint32_t)rec_buf % 250) == 0)
                putchar('.');
        }
    }

    printf("\nData recieved, processing S-Record file\n");
    rec_buf = (char *)0x00300000;
//    if (read_srecord(rec_buf))
//        printf("SRecord load failed\n");
}
