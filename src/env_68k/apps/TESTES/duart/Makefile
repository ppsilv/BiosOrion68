CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

CFLAGS = -Wall -Os -Wa,-adhln=tinyBasic.lst -I../../../include -ffreestanding -nostdlib -D __68000__ -D ALIGN_MEMORY
#CFLAGS = -Wall -Os -I../include -ffreestanding -nostdlib -Dmain=main_with_exit
LDFLAGS = -T ./linker.ld -L../../../lib -nostdlib -L../../../lib
LDLIBS = -lc68k -lvideo

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
ELFS = $(SRCS:.c=.elf)
BINS = $(SRCS:.c=.bin)

APP = duart

.PHONY: all clean

#all: $(BINS)
all: $(APP).bin
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra específica para cada arquivo para evitar problemas com $<
$(APP).elf: $(OBJS) ../../../libc/obj/crt0.o ../../../libc/obj/startup.o
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	@echo "Linked: $@"



%.bin: $(APP).elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -f $(OBJS).o $(ELFS) $(BINS).bin $(APP).elf
	@echo "Clean complete"
