#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fatfs/ff.h>      // Biblioteca FatFs

#define PT_LOAD 1

// Estrutura do Cabeçalho Principal (ELF Header)
typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;     // Ponto de entrada (endereço inicial de execução)
    uint32_t e_phoff;     // Offset do Program Header Table
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;     // Quantidade de Program Headers
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

// Estrutura de cada Segmento (Program Header)
typedef struct {
    uint32_t p_type;     // Tipo do segmento (PT_LOAD = 1)
    uint32_t p_offset;   // Offset do segmento dentro do arquivo ELF
    uint32_t p_vaddr;    // Endereço virtual na RAM
    uint32_t p_paddr;    // Endereço físico na RAM
    uint32_t p_filesz;   // Tamanho dos dados no arquivo
    uint32_t p_memsz;    // Tamanho total na RAM (inclui .bss)
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

// Carrega o ELF do buffer para a RAM e retorna o e_entry (0 se inválido)
uint32_t carregar_elf32(const uint8_t *elf_buf, uint8_t *ram_destino) {
    const Elf32_Ehdr *header = (const Elf32_Ehdr *)elf_buf;

    // 1. Valida o Magic Number: 0x7F, 'E', 'L', 'F'
    if (header->e_ident[0] != 0x7F || header->e_ident[1] != 'E' ||
        header->e_ident[2] != 'L'  || header->e_ident[3] != 'F') {
        return 0; // Arquivo inválido
    }

    // 2. Itera por todos os Program Headers
    for (uint16_t i = 0; i < header->e_phnum; i++) {
        uint32_t offset_ph = header->e_phoff + (i * header->e_phentsize);
        const Elf32_Phdr *phdr = (const Elf32_Phdr *)&elf_buf[offset_ph];

        // 3. Processa apenas segmentos do tipo PT_LOAD
        if (phdr->p_type == PT_LOAD) {
            uint8_t *dest_addr = ram_destino + phdr->p_paddr;

            // Copia o código/dados (.text / .data) para a RAM
            if (phdr->p_filesz > 0) {
                memcpy(dest_addr, &elf_buf[phdr->p_offset], phdr->p_filesz);
            }

            // Zera a área restante da RAM se memsz > filesz (seção .bss)
            if (phdr->p_memsz > phdr->p_filesz) {
                uint32_t bss_size = phdr->p_memsz - phdr->p_filesz;
                memset(dest_addr + phdr->p_filesz, 0, bss_size);
            }
        }
    }

    // Returna o endereço de execução inicial (Entry Point)
    return header->e_entry;
}

uint32_t carregar_elf32_fatfs(const char *caminho) {
    FIL file;
    UINT bytes_lidos;
    Elf32_Ehdr header;

    // 1. Abre o arquivo ELF no disco
    if (f_open(&file, caminho, FA_READ) != FR_OK) {
        return 0;
    }

    // 2. Lê o cabeçalho ELF
    if (f_read(&file, &header, sizeof(Elf32_Ehdr), &bytes_lidos) != FR_OK || 
        bytes_lidos < sizeof(Elf32_Ehdr)) {
        f_close(&file);
        return 0;
    }

    // 3. Valida: Assinatura ELF, 32-bit (0x01) e Big-Endian para m68k (0x02)
    if (header.e_ident[0] != 0x7F || header.e_ident[1] != 'E' ||
        header.e_ident[2] != 'L'  || header.e_ident[3] != 'F' ||
        header.e_ident[5] != 2) { 
        f_close(&file);
        return 0;
    }

    // 4. Valida alinhamento (m68k exige Entry Point em endereço par)
    if (header.e_entry & 1) {
        f_close(&file);
        return 0;
    }

    // 5. Itera pelos Program Headers e escreve na RAM
    for (uint16_t i = 0; i < header.e_phnum; i++) {
        Elf32_Phdr phdr;
        DWORD offset_ph = header.e_phoff + (i * header.e_phentsize);

        f_lseek(&file, offset_ph);
        if (f_read(&file, &phdr, sizeof(Elf32_Phdr), &bytes_lidos) != FR_OK) {
            f_close(&file);
            return 0;
        }

        if (phdr.p_type == PT_LOAD) {
            // Aponta direto para o endereço físico da RAM destino no m68k
            uint8_t *dest_ram = (uint8_t *)phdr.p_paddr;

            // Carrega código/dados (.text / .data) do disco direto pra RAM
            if (phdr.p_filesz > 0) {
                f_lseek(&file, phdr.p_offset);
                f_read(&file, dest_ram, phdr.p_filesz, &bytes_lidos);
            }

            // Zera a seção .bss na RAM
            if (phdr.p_memsz > phdr.p_filesz) {
                memset(dest_ram + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
            }
        }
    }

    f_close(&file);
    return header.e_entry; // Endereço inicial pronto para salto (JSR / JMP)
}