CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy
VASM = vasmm68k_mot

CFLAGS  = -Wall -O0 -m68000 -ffreestanding -nostdlib -fomit-frame-pointer
CFLAGS += -I ./includes -I../../include

# -Felf gera objeto ELF relocavel (ET_REL), compativel com m68k-elf-ld
# -m68000 define a CPU alvo -- ajuste se seu m68k for outro (68010/68020)
# -quiet reduz output verboso do assembler
VASMFLAGS = -Felf -m68000 -quiet -I ./includes -I../../include

LDFLAGS = -T script.ld -nostdlib -Map=programa.map -L../../lib
LDLIBS = -lc68k

# Arquivos em C
SRCS =

# Arquivos em Assembly (sintaxe Motorola, via vasmm68k_mot)
# Adicione aqui os .s do seu programa
ASMS = basic68k.s

OBJS = $(SRCS:.c=.o) $(ASMS:.s=.o)

TARGET = basic.elf

.PHONY: all clean

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar arquivos .s (sintaxe Motorola) via vasmm68k_mot
%.o: %.s
	$(VASM) $(VASMFLAGS) -o $@ $<

$(TARGET): $(OBJS) script.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	@echo "Gerado: $@ (ET_EXEC, pronto para load_elf_executable)"

clean:
	rm -f *.o *.elf *.map
