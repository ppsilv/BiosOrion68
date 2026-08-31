#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fileio.h>
#include "vfs.h"
#include "elf.h"

#include <string.h>
#include <ctype.h>

/* Retorna 1 se 'name' termina com '.elf' (ou '.ELF', case-insensitive), 0 caso contrário */
static int has_elf_extension(const char *name) {
    size_t len = strlen(name);
    if (len < 4) return 0;  /* nem cabe ".elf" */
    printf("has_elf_extension:name[%s]\n",name);
    const char *ext = name + (len - 4);  /* aponta pros ultimos 4 caracteres */

    return (tolower((unsigned char)ext[0]) == '.' &&
            tolower((unsigned char)ext[1]) == 'e' &&
            tolower((unsigned char)ext[2]) == 'l' &&
            tolower((unsigned char)ext[3]) == 'f');
}

/* Garante que 'out' termine em .elf, copiando de 'name' e acrescentando se faltar.
 * 'out' precisa ter espaço suficiente (tamanho de name + 4 + 1 pro terminador). */
void ensure_elf_extension(const char *name, char *out) {
    if (!has_elf_extension(name)) {
        sprintf(out,"%s.elf", name);
    }
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
    if (fread(fd, &header, sizeof(header), &bytesRead) != FR_OK || bytesRead != sizeof(header))
    {
        printf("Cannot read ELF file header\n");
        return -1;
    }
#ifdef DEBUG_ELF
    else
    {
        printf("ELF file header read, %d bytes\n", bytesRead);
    }
#endif
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
#ifdef DEBUG_ELF
				printf("Loading %d byte segment from offset 0x%x to address 0x%x\n\r",	progHeader.filesz, progHeader.offset, progHeader.paddr);
#endif
                f_lseek(fd, progHeader.offset);

                if(f_read(fd, (char*)progHeader.paddr, progHeader.filesz, &bytesRead) != FR_OK || bytesRead != progHeader.filesz)
                {
                    printf("Unable to read segment from ELF file.\n");
                    return -1;
                }

                if (progHeader.memsz > progHeader.filesz)
                {
				    printf("Clearing %d bytes BSS at 0x%x\n\r", progHeader.memsz - progHeader.filesz, progHeader.paddr + progHeader.filesz);
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
#ifdef DEBUG_ELF
    printf("Program entry point is at 0x%x\n", header.entry);
    printf("Calling with %d args\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  arg %d : %s\n", i, argv[i]);

    printf("Running program %s\n\n", argv[0]);
#endif
    int (*entry)(int, char**) = (int (*)(int, char**))header.entry;
    int ret = (*entry)(argc, argv);
#ifdef DEBUG_ELF
    printf("Program returned value %d\n", ret);
#endif
    return ret;
}




#define NUM_FILE_EXTENSIONS     2
static const char *orion_extensions[NUM_FILE_EXTENSIONS] = {".elf", ".bat" };

FRESULT open_executable_file(FIL *file, char *filename)
{
    char fileext[FF_MAX_LFN];
    FRESULT fr;

    memset(fileext,0,FF_MAX_LFN);
    // Try opening the file as named
    fr = fopen(file, filename, FA_READ);
    if (fr == FR_OK) /* file doesn't exist? */
        return FR_OK;

    //printf("Unable to open file %s, trying with extensions..\n", filename);
    for (int i = 0; i < NUM_FILE_EXTENSIONS; i++)
    {
        //printf("Trying extension %s\n", orion_extensions[i]);
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


#define HEADER_EXAMINE_SIZE 4 /* number of bytes we need to load to determine the file type */
const uint8_t g_elf_header_bytes[4]  = { 0x7F, 0x45, 0x4c, 0x46 };

int process_command_executable(int argc, char *argv[]){
    FIL file;
    FRESULT fr;
    char buffer[HEADER_EXAMINE_SIZE];
    unsigned int br;

    if( strstr(argv[0], ".elf") == 0){
        char name[13];
        char *filename=name;
        memset(filename,0,13);
        sprintf(filename,"%s.elf", argv[0]);
        memcpy(argv[0],filename,13);
        printf("process_command_executable: Filename[%s]: argv[0]=%s\n",filename,argv[0]);
    }
    printf("process_command_executable: argv[0]=%s\n",argv[0] );

    fr = open_executable_file(&file, argv[0]);
    if (fr != FR_OK) {
        printf("open_executable_file does not find the excutable\n");
        return 0;
    }

    memset(buffer, 0, HEADER_EXAMINE_SIZE);
#ifdef DEBUG_ELF    
    printf("%s: File size is %d bytes, ", filename, f_size(&file));
#endif
    /* sniff the first few bytes, then rewind to the start of the file */
    fr = fread(&file, buffer, HEADER_EXAMINE_SIZE, &br);
    flseek(&file, 0);

    if (fr == FR_OK) {
        if (memcmp(buffer,(const char *) g_elf_header_bytes, sizeof(g_elf_header_bytes)) == 0)
        {
            //printf("ELF executable\n\n");
            load_elf_executable(argc, argv, &file);
        }
        else
        {
            printf("unknown format.\n\n");
            printf("%s: is not an executable file\n", argv[0]);
        }
    }
    else {
        printf("%s: Cannot read: ", argv[0]);
    }

    fclose(&file);

    return 1;
}

