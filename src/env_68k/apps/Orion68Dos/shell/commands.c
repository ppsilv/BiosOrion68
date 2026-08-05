#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>

#include "orion68.h"
#include "commands.h"
#include "decodecmd.h"
#include "ata.h"
#include "io.h"

#include "rtc.h"
#include "ff.h"
#include "srecord.h"
#include "diskio.h"
#include "drv_uart.h"
#include "picow.h"
#include "color.h"
#include "drv_picoVga.h"


extern FATFS FatFs;      // Objeto de controle do sistema de arquivos (Work area)
extern char syspath[128];

//static DIR Dir;          // Objeto de diretório
//static FILINFO Fno;      // Estrutura que recebe os metadados do arquivo/pasta
static __attribute__((aligned(2)))FIL Arq;               // Objeto de controle do arquivo (File Object)
extern struct ata_drive drives[]; 

extern void printerro(int eno);
extern void dump_memory(void * addr,int size);
extern const cmd_entry_t g_cmd_table[];
extern void listar_diretorio_raiz();
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

void do_notimplemented(int argc, char *argv[])
{
    printf("This command is currently not implemented\n");
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

void do_dump(int argc, char *argv[]){
    unsigned long start, count;

    //printf("numargs=%d args[0]=%s,args[1]=%s,args[2]=%s\n",argc,argv[0],argv[1],argv[2]);

    start = strtoul((const char *)argv[0], NULL, 16);
    count = strtoul((const char *)argv[1], NULL, 16);


    //printf("do_dump [%lu] [%lu]",start,count);

    dump_memory((void*)start, count);
}

uint8_t serial_has_char(){
    return 1;
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

void do_ideinit(int argc, char *argv[])
{
    FRESULT fr;
    fr = f_mount(&FatFs, "", 0);
    if (fr != FR_OK) {
        printf("PANIC: Erro ao montar FAT: %d\n", fr);
    }else{
        printf("FAT montado com sucesso!\n");
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


void do_run(int argc, char *argv[])
{
    unsigned long start;
    start = strtoul(argv[0], NULL, 16);

    void (*entry)(void) = (void (*)(void))start;
    entry();
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

void do_ls(int argc, char *argv[])
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

        if (fat_file.fname[0] == 0){ /* end of directory? */
            if( total_files == 0 ){
                printf("Empty directory\n");
            }
            break;
        }
        total_files++;
        
        filename = fat_file.fname;

        dir = fat_file.fattrib & AM_DIR;

        if (dir) {
            /* directory */
            printf("%04d/%02d/%02d %02d:%02d    <DIR> \t%s/", 1980 + ((fat_file.fdate >> 9) & 0x7F),
                    (fat_file.fdate >> 5) & 0xF, fat_file.fdate & 0x1F,
                    fat_file.ftime >> 11, (fat_file.ftime >> 5) & 0x3F, filename);
        }else if (!(fat_file.fattrib & AM_HID))  {
            /* regular file */
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

void do_exit(int argc, char *argv[]){
    f_mount(NULL, "0:", 0);
    printf("YOU CAN TURN OFF THE SYSTEM.\n");
}

void do_mkdir(int argc, char *argv[])
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
void do_rename(int argc, char *argv[])
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

void do_cd(int argc, char *argv[])
{
    FRESULT fr;

    fr = f_chdir(argv[0]);
    if(fr != FR_OK)
       printerro(fr);
}

void do_dir(int argc, char *argv[]){
    listar_diretorio_raiz();
}
void do_shst(int argc, char *argv[]){
    printf("Systemtick = %ld\n",get_system_tick());
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
extern void main_teste_teclado(void); 
extern char teste01();
extern bool receber_arquivo_do_pico(uint8_t *destino_ram,uint8_t reg);
extern bool receber_setor_do_pico(uint8_t *destino_ram, uint16_t sector);
extern void pico_write_ch(uint8_t ch);
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
                picovga_set_color(data,BLACK);
                break;
        case 0x03:    
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

/*
void do_traptest(int argc, char *argv[]){
    uint32_t cmd, value;
    char *str;

    syscall_data sys;

    int ret;

    cmd = strtoul(argv[0], NULL, 16);
    value = strtoul(argv[1], NULL, 16);
    str = argv[2];
    sys.command = cmd;
    sys.d0 = value;
    sys.a0 = str;

	__asm__ volatile(
	"move.l	%1, %%a0\n"
	"trap	#15\n"
	"move.l %%d0, %0\n"
	: "=g" (ret)
	: "g" (&sys)
	: "%a0"
	);

    printf("Return value = %ld\n", ret);
    printf("  return data = %s\n", (char *)sys.a1);
}
*/
int ehbasic(void);

uint8_t basic_running = 0;

void do_ehbasic(int argc, char *argv[])
{
    //ehbasic();
}

int le_setor(int sector);
int wr_setor(int sector);

void do_readsect(int argc, char *argv[]) {
    int sector = atoi(argv[0]);
    printf("do_readsect: sector[%d]\n",sector);
    le_setor(sector);
}
void do_writesect(int argc, char *argv[]){
    int sector = atoi(argv[0]);
    printf("do_writesect: sector[%d]\n",sector);
    wr_setor(sector);
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

#include <stdio.h>
#include <stdint.h>
#include "ff.h"      /* FatFs (elm-chan) */

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
int do_mem2file(const void *origem, size_t tamanho, const char *nome_arquivo){
    FIL     arquivo;
    FRESULT fr;
    UINT    bytes_escritos;

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

    fr = f_close(&arquivo);
    if (fr != FR_OK) {
        printf("dump_memoria_para_arquivo: falha ao fechar '%s' (erro %d)\n", nome_arquivo, fr);
        return -1;
    }

    printf("dump_memoria_para_arquivo: '%s' gravado com sucesso (%u bytes)\n",
           nome_arquivo, (unsigned)tamanho);
    return 0;
}