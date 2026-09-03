CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

CFLAGS = -Wall -Os -Wa,-adhln=loader.lst -I../../../include -ffreestanding -nostdlib -D __68000__ -D ALIGN_MEMORY
LDFLAGS = -T ./linker.ld -L../../../lib -nostdlib -L../../../lib
LDLIBS = -lc68k -lvideo -lrtc -lfileio

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
ELFS = $(SRCS:.c=.elf)
BINS = $(SRCS:.c=.bin)

.PHONY: all clean

#all: $(BINS)
all: loader.bin
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra específica para cada arquivo para evitar problemas com $<
loader.elf: $(OBJS) ../../../libc/obj/crt0.o ../../../libc/obj/startup.o
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	@echo "Linked: $@"

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -f $(OBJS) $(ELFS) $(BINS) *.lst
	@echo "Clean complete"
