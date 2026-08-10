# ==============================================================================
# Makefile para CP/M-68K (BDOS + BIOS) usando m68k-elf-gcc
# ==============================================================================

TOOLCHAIN = m68k-elf-
CC        = $(TOOLCHAIN)gcc
LD        = $(TOOLCHAIN)ld
OBJCOPY   = $(TOOLCHAIN)objcopy

# Flags para m68k, C antigo (K&R / gnu89) e desativação de builtins do GCC
CFLAGS    = -m68000 -std=gnu89 -fno-builtin -Wall -Wno-implicit-function-declaration -I. -O2
LDFLAGS   = -T linker.ld

# Fontes em C do BDOS
C_SRCS    = bdosmain.c bdosread.c dskutil.c fileio.c iosys.c

# Fontes em Assembly (arquivos do BDOS + sua BIOS)
ASM_SRCS  = bdosif.s ldrif.s bios.S

# Converte nomes de fontes para objetos .o
C_OBJS    = $(C_SRCS:.c=.o)
ASM_OBJS  = $(ASM_SRCS:.s=.o)
ASM_OBJS := $(ASM_OBJS:.S=.o)
OBJS      = $(ASM_OBJS) $(C_OBJS)

TARGET    = cpm.bin
ELF       = cpm.elf

.PHONY: all clean

all: $(TARGET)

# Gera o binário flat (bruto) para carregar na RAM
$(TARGET): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@echo ">>> Sucesso: $(TARGET) gerado! <<<"

# Linka todos os arquivos .o usando o linker.ld
$(ELF): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# Regra de compilação para C
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra de compilação para Assembly (.s ou .S)
%.o: %.s
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.elf *.bin
