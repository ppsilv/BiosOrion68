###############################################################################
# Tools
###############################################################################
CC      = m68k-elf-gcc
LD      = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

###############################################################################
# Nome do Executável Final
###############################################################################
TARGET_NAME = ledir

###############################################################################
# Flags
###############################################################################
CFLAGS  = -Wall -Os -I../../include/fatlib -I../../include -ffreestanding -nostdlib
LFLAGS  = -T ../../ram_linker.ld -L../../lib -nostdlib

###############################################################################
# Source & Target Lists
###############################################################################
SRCS    = $(wildcard *.c)
OBJS    = $(SRCS:.c=.o)

# Alvos finais baseados no nome do seu programa
ELF     = $(TARGET_NAME).elf
BIN     = $(TARGET_NAME).bin

# Objetos fixos de infraestrutura do VBug/Orion68K que todo app precisa
STARTUP_OBJS = ../../obj/crt0.o ../../obj/startup.o

.PHONY: all clean

###############################################################################
# Rules
###############################################################################
all: $(BIN)

# 1. Regra genérica para compilar qualquer .c em .o (vai compilar o ata.c, o seu main.c, etc)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 2. Regra de Linkagem: Junta o startup, TODOS os seus .o do diretório e as libs
$(ELF): $(OBJS) $(STARTUP_OBJS)
	$(LD) $(LFLAGS) $(STARTUP_OBJS) $(OBJS) -lfatfs -lmc68000 -o $@
	@echo "Linked: $@"

# 3. Transforma o .elf final em binário cru para o 68k
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -f $(OBJS) $(ELF) $(BIN)
	@echo "Clean complete"
