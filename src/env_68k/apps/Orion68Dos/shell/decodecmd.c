#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "decodecmd.h"
#include "commands.h"
#include "elf.h"
#include "io.h"

#include "ff.h"
#include "diskio.h"
#include "keycodes.h"
#include "drv_uart.h"
#include "drv_kbd.h"
#include "../kbd/ringbuffer.h"

#define MAXARGS 40

void printerro(int eno);

#define HEADER_EXAMINE_SIZE 4 /* number of bytes we need to load to determine the file type */
const uint8_t g_elf_header_bytes[4]  = { 0x7F, 0x45, 0x4c, 0x46 };
extern bool kb_get(uint8_t *data);

const cmd_entry_t g_cmd_table[] = {
    // name         min max function
    {"basic",       0,  1, &do_ehbasic,     "Start EhBASIC" },
    {"binfile",     1,  1, &do_binfile,     "Recieve binary file over serial and save to disk <filename>" },
    {"binmem",      1,  1, &do_binmem,      "Recieve binary file over serial to location in RAM" },
    {"cat",         1,  1, &do_cat,         "Display contents of <file>"},
    {"cd",          1,  1, &do_cd,          "Change directory <dir>"},
    {"cp",          2,  2, &do_copyfile,    "Copy file <src> <dest>"},
    {"ls",          0,  1, &do_dir,	        "List directory contents" },
    {"dir",         0,  1, &do_ls,	        "List directory contents" },
    {"del",         1,  1, &do_delete,	    "Delete file <file>" },
    {"dump",        2,  2, &do_dump,        "Dump memory <from> <count>" },
    {"exit",        0,  0, &do_exit,        "Exit system in a clean way.." },
    {"help",	    0,  0, &do_help,	    "Show available commands" },
    {"ideinit",     0,  0, &do_ideinit,     "Mount ide fatfs.." },
    {"idemode",     1,  0, &do_idemode,     "Sets ide mode 0=8bits 1=16bits.." },
    {"loadmem",	    1,  2, &do_loadmem,	    "Load a file to the specified memory location <file> <mem>" },
    {"mkdir",	    1,  1, &do_mkdir,	    "Create the specified folder <dir>" },
    {"mv",          2,  2, &do_rename,      "Rename file <src> to <dest>" },
    {"rmdir",	    1,  1, &do_rmdir,	    "Delete the specified folder <dir>" },
    {"run",         1,  1, &do_run,         "Run code at address <addr>" },
    {"shst",        0,  0, &do_shst,        "Show systemtick" },
    {"srec",        0,  0, &do_srecord,     "Recieve S-Record file over serial" },
    {"time",        0,  6, &do_time,        "Display or set the current time and date (time ? for help)" },
  /*  {"trap",        3,  3, &do_traptest,    "Test the trap function <command> <value> <string>" }, */
    {"uptime",      0,  0, &do_uptime,      "Display the time the system has been running" },
    {"writemem",    2,  0, &do_writemem,    "Write memory <addr> [byte ...]" },
    {"rs",          1,  0, &do_readsect,    "Read a sector X from disk..." },
    {"writesect",   1,  0, &do_writesect,   "Write a sector X from disk..." },
    {"writemem1",   2,  0, &do_writemem1,   "Write a memory location" },
    {"tst",         2,  2, &do_tstkbd,      "Testa conversa com o picow...\n" \ 
                                            "\tcmd=0x00 dado=0 => chama receber_arquivo_do_pico\n" \
                                            "\tcmd=0x01 dado=0 => chama receber_setor_do_pico\n" \
                                            "\tcmd=0x04 dado=0 => chama pico write screen\n" \
                                            "\tcmd=0x0A dado=sec.low => chama send_sector_low\n" \ 
                                            "\tcmd=0x0B dado=sec. high => chama send_sector_high\n" \
                                            "\tcmd=0x0C dado=0 => chama send_read_cmd\n" \
                                            "\tcmd=0x0D dado=0 => chama read_sector\n"
                                            "\tcmd=0x10 dado=x => chama pico set color dado=cor\n" \
                                            "\tcmd=0x11 dado=0 => chama pico write string\n" \
                                            "\tcmd=0x12 dado=x => chama pico write char\n" \
                                            "\tcmd-0xff dado=0 => chama read_kbd"
                                            },

    {0, 0, 0, 0, 0 }
};
 
#define NUM_FILE_EXTENSIONS     2
static const char *orion_extensions[NUM_FILE_EXTENSIONS] = {".elf", ".bat" };

extern uint8_t keyboard_handler(uint8_t scancode);
int getline(char *line, int linesize)
{
	static int count = 0;
	int ret = -1;


    if (count < linesize - 1)
    {
        uint8_t ch;
        bool res=true;
        while ( res == true ){      //le do teclado ps2
            res = ring_buf_is_empty();
        }
        ch = ring_buf_get();    //le do teclado ps2
        if( ch == 0x11 || ch == 0x12){
            keyboard_handler(ch) ;
            return 0;
        }
        //uint16_t ch = get_char();   //le do teclado USB que no futuro também estará pondo seus caracteres no ringbuffer
        //ch = get_key(); //le da uart 0
        
        switch(ch)
        {
            case PS2_BACKSPACE:
                          putchar('\b');
                          count--;
                          break;  
            case PS2_TAB: break;
            case PS2_ENTER:
                putchar('\n');
                break;

            case PS2_ESC: break;
            case PS2_INSERT: break;
/*
            case PS2_DELETE:
                if (count > 0)
                {
                    putchar(PS2_DELETE);
                    uart0_write('\b');
                    uart0_write(' ');
                    uart0_write('\b');
                    --count;
                }
                break;
*/
            case PS2_HOME: break;
            case PS2_END: break;
            case PS2_PAGEUP: break;
            case PS2_PAGEDOWN: break;
            case PS2_UPARROW: break;
            case PS2_LEFTARROW: break;
            case PS2_DOWNARROW: break;
            case PS2_RIGHTARROW: break;
            case PS2_F1: break;
            case PS2_F2: break;
            case PS2_F3: break;
            case PS2_F4: break;
            case PS2_F5: break;
            case PS2_F6: break;
            case PS2_F7: break;
            case PS2_F8: break;
            case PS2_F9: break;
            case PS2_F10: break;
            case PS2_F11: break;
            case PS2_F12: break;
            case PS2_SCROLL: break;
            default:
                if (ch >= ' '){
                    line[count++] = ch;
                    putchar(ch);
                }
                break;
        }

        if (ch == PS2_ENTER)
        {
			line[count] = 0;
			ret = count;
			count = 0;
        }
	}
    else
    {
			line[count] = 0;
			ret = count;
			count = 0;
    }

	return ret;
}

int process_command_builtin(int argc, char *argv[])
{
    const cmd_entry_t *cmd;
    FRESULT fr;

    if (argc == 1 && argv[0][1] == ':')
    {
        // Change current drive
        fr = f_chdrive(argv[0]);
        if (fr)
        {
        	printerro(fr);
        	printf("Drive %s not found\n", argv[0]);
        }
        return 1;
    }
    else
    {
        // built-in commands
        for (cmd = g_cmd_table; cmd->name; cmd++)
        {
            if (!strcasecmp(argv[0], cmd->name))
            {
                if ((argc - 1) >= cmd->min_args && (cmd->max_args == 0 || (argc - 1) <= cmd->max_args))
                {
                    cmd->function(argc - 1, argv + 1);
                }
                else
                {
                    if (cmd->min_args == cmd->max_args)
                        printf("%s: requires %d argument%s\n", argv[0], cmd->min_args, cmd->min_args == 1 ? "" : "s");
                    else
                        printf("%s: requires %d to %d arguments\n", argv[0], cmd->min_args, cmd->max_args);
                }
                return 1;
            }
        }
    }
    return 0;
}

int load_elf_executable(int argc, char *argv[], FIL *fd)
{
    unsigned int bytesRead;
    uint32_t memHigh = 0;
    uint32_t memLow = 0xFFFFFFFF;
    uint32_t progIndex = 0;
    elf32_header header;
    elf32_program_header progHeader;

    f_lseek(fd, 0);
    if (f_read(fd, &header, sizeof(header), &bytesRead) != FR_OK || bytesRead != sizeof(header))
    {
        printf("Cannot read ELF file header\n");
        return -1;
    }
    else
    {
        printf("ELF file header read, %d bytes\n", bytesRead);
    }

    if (header.ident_magic[0] != 0x7F ||
        header.ident_magic[1] != 'E' ||
        header.ident_magic[2] != 'L' ||
        header.ident_magic[3] != 'F' ||
        header.ident_version != 1)
    {
        printf("Bad ELF header\n");
        return -1;
    }

    if (header.ident_class != ID_32BIT || header.ident_data != ID_BIG_ENDIAN ||
        header.ident_osabi != 0 || header.ident_abiversion != 0)
    {
        printf("Not a 32-bit ELF file.\n");
        return -1;
    }

    if (header.type != ET_EXEC)
    {
        printf("ELF file is not an executable.\n");
        return -1;
    }

    if (header.machine != EM_68K)
    {
        printf("ELF file is not for 68000 processor.\n");
        return -1;
    }

    while (progIndex < header.phnum)
    {
        f_lseek(fd, progIndex * header.phentsize + header.phoff);
        if (f_read(fd, &progHeader, sizeof(progHeader), &bytesRead) != FR_OK || bytesRead != sizeof(progHeader))
        {
            printf("Cannot read ELF program header.\n");
            return -1;
        }

        switch (progHeader.type)
        {
            case PT_NULL:
            case PT_NOTE:
            case PT_PHDR:
                break;

            case PT_SHLIB:
            case PT_DYNAMIC:
                printf("ELF executable is dynamically linked.\n");
                return -1;

            case PT_LOAD:
				printf("Loading %d byte segment from offset 0x%x to address 0x%x\n\r",
					progHeader.filesz, progHeader.offset, progHeader.paddr);
                f_lseek(fd, progHeader.offset);

                if(f_read(fd, (char*)progHeader.paddr, progHeader.filesz, &bytesRead) != FR_OK || bytesRead != progHeader.filesz)
                {
                    printf("Unable to read segment from ELF file.\n");
                    return -1;
                }

                if (progHeader.memsz > progHeader.filesz)
                {
				    printf("Clearing %d bytes BSS at 0x%x\n\r",
					    progHeader.memsz - progHeader.filesz, progHeader.paddr + progHeader.filesz);
                    memset((char*)progHeader.paddr + progHeader.filesz, 0, progHeader.memsz - progHeader.filesz);
                }

                if (progHeader.paddr < memLow)
                    memLow = progHeader.paddr;

                if (progHeader.paddr + progHeader.filesz > memHigh)
                    memHigh = progHeader.paddr + progHeader.filesz;
                break;

            case PT_INTERP:
                printf("ELF executable requires an interpreter.\n");
                return -1;
        }
        progIndex++;
    }

    printf("Program entry point is at 0x%x\n", header.entry);
    printf("Calling with %d args\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  arg %d : %s\n", i, argv[i]);

    printf("Running program %s\n\n", argv[0]);

    int (*entry)(int, char**) = (int (*)(int, char**))header.entry;
    int ret = (*entry)(argc, argv);
    printf("Program returned value %d\n", ret);
    return ret;
}

FRESULT open_executable_file(FIL *file, char *filename)
{
    char fileext[FF_MAX_LFN];
    FRESULT fr;

    memset(fileext,0,FF_MAX_LFN);
    // Try opening the file as named
    fr = f_open(file, filename, FA_READ);
    if (fr == FR_OK) /* file doesn't exist? */
        return FR_OK;

    //printf("Unable to open file %s, trying with extensions..\n", filename);
    for (int i = 0; i < NUM_FILE_EXTENSIONS; i++)
    {
        printf("Trying extension %s\n", orion_extensions[i]);
        if (strlen(filename) + strlen(orion_extensions[i]) < FF_MAX_LFN)
        {
            strcpy(fileext, filename);
            strcat(fileext, orion_extensions[i]);
            strcat(fileext, "\n");
            // Try opening the file with extension
            //printf("Trying to open file %s ...\n", fileext);
            fr = f_open(file, fileext, FA_READ);
            if (fr == FR_OK) /* file  exists */
                return FR_OK;
        }
    }

   //printf("Unable to open file with known extension\n");
    return FR_NO_FILE;
}

int process_command_executable(int argc, char *argv[])
{
    FIL file;
    FRESULT fr;
    char buffer[HEADER_EXAMINE_SIZE];
    unsigned int br;

    for(int i=0; i < argc; i++){
        printf("process_command_executable: argc=[%d] argv[%s]\n",argc,argv[i]);
    }

    fr = open_executable_file(&file, argv[0]);
    if (fr != FR_OK)
    {
        printf("%s: Command not found\n", argv[0]);
        //printerro(fr);
        return 0;
    }

    memset(buffer, 0, HEADER_EXAMINE_SIZE);

    printf("%s: File size is %d bytes, ", argv[0], f_size(&file));

    /* sniff the first few bytes, then rewind to the start of the file */
    fr = f_read(&file, buffer, HEADER_EXAMINE_SIZE, &br);
    f_lseek(&file, 0);

    if (fr == FR_OK)
    {
        if (memcmp(buffer,(const char *) g_elf_header_bytes, sizeof(g_elf_header_bytes)) == 0)
        {
            printf("ELF executable\n\n");
            load_elf_executable(argc, argv, &file);
        }
        else
        {
            printf("unknown format.\n\n");
            printf("%s: is not an executable file\n", argv[0]);
        }
    }
    else
    {
        printf("%s: Cannot read: ", argv[0]);
        printerro(fr);
    }

    f_close(&file);

    return 1;
}


void process_command(char *argv[], int argc)
{
    if (argc > 0)
    {
        if (!process_command_builtin(argc, argv) && !process_command_executable(argc, argv))
            printf("%s: Unknown command.  Try 'help'.\n", argv[0]);
    }
}

void execute_cmd(char *linebuffer)
{
    char *args[MAXARGS+1];
    char *ptr;
    int numargs;

    numargs = 0;
    ptr = linebuffer;

    while (1)
    {
        if (numargs == MAXARGS)
        {
            printf("Limiting to %d arguments.\n", numargs);
            *ptr = 0;
        }

        if (*ptr == 0)
        {
            // end of string
            args[numargs] = 0;
            break;
        }

        while (isspace(*ptr))
            ptr++;

        if (!isspace(*ptr))
        {
            args[numargs++] = ptr;

            while (*ptr && !isspace(*ptr))
                ptr++;

            if (*ptr == 0)
                continue;

            while (isspace(*ptr))
            {
                *ptr=0;
                ptr++;
            }
        }
    }
    //if(numargs > 0){
    //    printf("numargs=%d args[0]=%s,args[1]=%s,args[2]=%s\n",numargs,args[0],args[1],args[2]);
    //}
    process_command(args, numargs);
}
