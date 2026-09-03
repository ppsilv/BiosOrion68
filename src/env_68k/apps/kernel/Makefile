CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy
VASM = vasmm68k_mot 

CFLAGS = -Wall -O0 -I ./includes -I../../include -ffreestanding -nostdlib
CFLAGS += -DCHIP_$(CHIP_TYPE) -DROM_SIZE_BYTES=$(ROM_SIZE)
CFLAGS += -fomit-frame-pointer

VASMFLAGS = -Felf -m68000 -quiet -I ./includes -I../../include

LDFLAGS = -T ./kernel.ld -L../../lib -nostdlib -Map=kernel.map
LDLIBS = 

# 1. ADICIONADO: Lista de arquivos em Assembly (.S)
ASMS = ctx_switch_68000.S

VASMS =

# Lista de arquivos C (CORRIGIDO: não tinha um espaço)
SRCS =  kernel.c scheduler.c \
./lib/libc/inttostr.c \
./lib/libc/klibc.c \
./lib/libc/kprintf.c \
./lib/libc/strtol.c	\
./lib/libc/__divdi3.c  \
./lib/libc/__divsi3.c  \
./lib/libc/__moddi3.c  \
./lib/libc/__modsi3.c  \
./lib/libc/__muldi3.c  \
./lib/libc/__mulsi3.c  \
./lib/libc/__udivdi3.c \
./lib/libc/__udivsi3.c \
./lib/libc/__umoddi3.c \
./lib/libc/__umodsi3.c \
./lib/libc/__umuldi3.c \
./lib/libc/__umulsi3.c \
./lib/string/malloc.c ./lib/string/memchr.c ./lib/string/memcpy.c ./lib/string/memmove.c ./lib/string/strcat.c ./lib/string/strcmp.c \
./lib/string/strcpy.c ./lib/string/strdup.c ./lib/string/strlen.c ./lib/string/strncmp.c ./lib/string/strndup.c ./lib/string/strrchr.c \
./lib/string/strstr.c ./lib/string/memccpy.c ./lib/string/memcmp.c ./lib/string/memmem.c ./lib/string/memset.c ./lib/string/strchr.c \
./lib/string/strcspn.c ./lib/string/strerror.c ./lib/string/strncat.c ./lib/string/strncpy.c ./lib/string/strnlen.c ./lib/string/strspn.c ./lib/string/strtok.c

# 2. ALTERADO: Agora a lista de objetos junta os .o vindos do .c e do .S
OBJS = $(SRCS:.c=.o) $(ASMS:.S=.o) $(VASMS:.asm=.o)

# Tools
TARGET := kernel
TARGET_OBJ := $(TARGET).o
TARGET_ELF := $(TARGET).elf
TARGET_BIN := $(TARGET).sys
TARGET_LOW := $(TARGET)_low.bin
TARGET_HIGH := $(TARGET)_high.bin

.PHONY: all clean split fill programL programH

all: $(TARGET_BIN)  # ← CORRIGIDO: usa TARGET_BIN

# Regra para compilar os arquivos .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 3. ADICIONADO: Regra explícita para compilar arquivos Assembly (.S)
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(VASM) $(VASMFLAGS) $< -o $@

# 4. CORRIGIDO: Gera kernel.elf a partir dos objetos
$(TARGET_ELF): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	@echo "Linked: $@"
	@echo "╔══════════════════════════╗"
	@echo "║  COMPILACAO TERMINADA!   ║"
	@echo "╚══════════════════════════╝"

# 5. CORRIGIDO: Gera kernel.bin a partir de kernel.elf (NÃO de bios68.elf!)
$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -f *.bin $(OBJS) *.map $(TARGET_LOW) $(TARGET_HIGH) $(TARGET_ELF)
	@echo "Clean complete"