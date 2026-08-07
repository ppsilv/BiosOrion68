CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

CFLAGS  = -Wall -O0 -m68000 -ffreestanding -nostdlib -fomit-frame-pointer
CFLAGS += -I ./includes -I../../include
LDFLAGS = -T script.ld -nostdlib -Map=programa.map -L../../lib
LDLIBS = -lc68k

# Adicione mais arquivos .c aqui conforme o programa crescer
SRCS = main.c
OBJS = $(SRCS:.c=.o)

TARGET = programa.elf

.PHONY: all clean

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) script.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	@echo "Gerado: $@ (ET_EXEC, pronto para load_elf_executable)"

clean:
	rm -f *.o *.elf *.map
