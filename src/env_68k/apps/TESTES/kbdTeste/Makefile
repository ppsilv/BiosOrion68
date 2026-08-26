CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

CFLAGS = -Wall -Os -I../../include -ffreestanding -nostdlib
#CFLAGS = -Wall -Os -I../include -ffreestanding -nostdlib -Dmain=main_with_exit
LDFLAGS = -T ./linker.ld -L../../lib -nostdlib  -Map=keyboard.map

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
ELFS = $(SRCS:.c=.elf)
BINS = $(SRCS:.c=.bin)

.PHONY: all clean

all: keyboard.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra específica para cada arquivo para evitar problemas com $<
keyboard.elf:   keyboard.o ../../obj/crt0.o ../../obj/startup.o
	$(LD) $(LDFLAGS) ../../obj/crt0.o ../../obj/startup.o   keyboard.o -lmc68000 -o $@
	@echo "Linked: $@"

keyboard.bin: keyboard.elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -f $(OBJS) $(ELFS) $(BINS) *.map
	@echo "Clean complete"
