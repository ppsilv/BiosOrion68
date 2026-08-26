CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

CFLAGS = -Wall -Os -I../../include -ffreestanding -nostdlib
#CFLAGS = -Wall -Os -I../include -ffreestanding -nostdlib -Dmain=main_with_exit
LDFLAGS = -T ../../ram_linker.ld -L../../lib -nostdlib

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
ELFS = $(SRCS:.c=.elf)
BINS = $(SRCS:.c=.bin)

.PHONY: all clean

all: $(BINS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra específica para cada arquivo para evitar problemas com $<
hello.elf: hello.o ../../obj/crt0.o ../../obj/startup.o
	$(LD) $(LDFLAGS) ../../obj/crt0.o ../../obj/startup.o hello.o -lmc68000 -o $@
	@echo "Linked: $@"

test.elf: test.o ../../obj/crt0.o
	$(LD) $(LDFLAGS) ../../obj/crt0.o test.o -lmc68000 -o $@
	@echo "Linked: $@"

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -f $(OBJS) $(ELFS) $(BINS)
	@echo "Clean complete"
