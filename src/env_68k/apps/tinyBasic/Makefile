CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

CFLAGS = -Wall -Os -Wa,-adhln=tinyBasic.lst -I../../include -ffreestanding -nostdlib -D __68000__ -D ALIGN_MEMORY
#CFLAGS = -Wall -Os -I../include -ffreestanding -nostdlib -Dmain=main_with_exit
LDFLAGS = -T ./linker.ld -L../../lib -nostdlib -L../../lib
LDLIBS = -lc68k

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
ELFS = $(SRCS:.c=.elf)
BINS = $(SRCS:.c=.bin)

.PHONY: all clean

#all: $(BINS)
all: tBasic.bin
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra específica para cada arquivo para evitar problemas com $<
tBasic.elf: tinyBasic.o TinyBasicPlus.o ../../libc/obj/crt0.o ../../libc/obj/startup.o
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	@echo "Linked: $@"

test.elf: test.o ../../obj/crt0.o
	$(LD) $(LDFLAGS) ../../obj/crt0.o test.o -lmc68000 -o $@
	@echo "Linked: $@"

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -f $(OBJS) $(ELFS) $(BINS) tBasic.bin tBasic.elf
	@echo "Clean complete"
