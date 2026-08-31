# Configurações do compilador
CC = m68k-elf-gcc
AR = m68k-elf-ar
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

# Flags
CFLAGS = -Wall -Os -ffreestanding -nostdlib -I../include
ARFLAGS = rcs

# Diretórios
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = ../lib
INC_DIR = include

# Encontrar todos os arquivos fonte
C_SRCS = $(shell find $(SRC_DIR) -name '*.c')
ASM_SRCS = $(shell find $(SRC_DIR) -name '*.S')
C_OBJS = $(C_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
ASM_OBJS = $(ASM_SRCS:$(SRC_DIR)/%.S=$(OBJ_DIR)/%.o)
OBJS = $(C_OBJS) $(ASM_OBJS)

# Nome da biblioteca
LIB_NAME = libc68k.a

.PHONY: all lib

all: lib

# Compilar a biblioteca
lib: $(LIB_DIR)/$(LIB_NAME)

$(LIB_DIR)/$(LIB_NAME): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^
	@echo "Biblioteca criada: $@"

# Compilar arquivos objeto C
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar arquivos assembly .S diretamente pelo GCC (passa pelo pré-processador)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpar
clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR)/libc68k.a

# Ver conteúdo da biblioteca
info:
	@echo "=== CONTEÚDO DA BIBLIOTECA ==="
	@m68k-elf-ar -t $(LIB_DIR)/$(LIB_NAME)
	@echo ""
	@echo "=== SÍMBOLOS DISPONÍVEIS ==="
	@m68k-elf-nm $(LIB_DIR)/$(LIB_NAME) | grep " T " || echo "Nenhum símbolo encontrado"
