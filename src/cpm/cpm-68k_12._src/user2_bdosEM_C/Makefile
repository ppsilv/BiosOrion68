# Configuração das ferramentas do GCC para o 68k
CC      = m68k-elf-gcc
LD      = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

# Flags de compilação (Alvo: Motorola 68000 puro, otimização O2, sem bibliotecas padrão)
CFLAGS  = -m68000 -nostdlib -O2 -Wall
ASFLAGS = -m68000

# Script de linkagem absoluto
LDSCRIPT = linker.ld

# Localiza todos os arquivos C e Assembly da pasta de forma dinâmica
C_SOURCES  = $(wildcard *.c)
ASM_SOURCES = $(wildcard *.s)

# Define os arquivos de objeto intermediários (.o)
OBJECTS = $(ASM_SOURCES:.s=.o) $(C_SOURCES:.c=.o)

# Alvo principal (Gera o binário final puro)
all: cpm.bin

# Passo 1: Extrai o binário bruto do arquivo ELF compilado
cpm.bin: cpm.elf
	$(OBJCOPY) -O binary cpm.elf cpm.bin
	@echo "--------------------------------------------------"
	@echo "Sucesso! Arquivo cpm.bin gerado para 0x82000."
	@echo "Verifique o arquivo cpm.map para encontrar o endereço do TRAP #2."
	@echo "--------------------------------------------------"

# Passo 2: Linka todos os objetos usando o script de linkagem e gera o mapa de memória
cpm.elf: $(OBJECTS)
	$(CC) $(CFLAGS) -T $(LDSCRIPT) $(OBJECTS) -o cpm.elf -Wl,-Map=cpm.map

# Regra para compilar os arquivos de código C (.c -> .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para montar os arquivos em Assembly (.s -> .o)
%.o: %.s
	$(CC) $(ASFLAGS) -c $< -o $@

# Limpa os arquivos gerados para forçar uma nova compilação limpa
clean:
	rm -f *.o cpm.elf cpm.bin cpm.map
