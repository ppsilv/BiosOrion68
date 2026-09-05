CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy

CFLAGS = -Wall -Os -Wa,-adhln=loader.lst -I../../../include -ffreestanding -nostdlib -D __68000__ -D ALIGN_MEMORY
LDFLAGS = -T ./linker.ld -L../../../lib -nostdlib -L../../../lib
LDLIBS = -lc68k -lvideo -lrtc -lfileio

ASMS = call_with_a5.S

VASMS =


SRCS = elfload.c elfloader.c slots.c elfreloc_m68k.c

# 2. ALTERADO: Agora a lista de objetos junta os .o vindos do .c e do .S
OBJS = $(SRCS:.c=.o) $(ASMS:.S=.o) $(VASMS:.asm=.o)

TARGET := elfload
TARGET_OBJ := $(TARGET).o
TARGET_ELF := $(TARGET).elf
TARGET_BIN := $(TARGET).bin

.PHONY: all clean

all: elfload.bin

# Regra para compilar os arquivos .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 3. ADICIONADO: Regra explícita para compilar arquivos Assembly (.S)
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(VASM) $(VASMFLAGS) $< -o $@

# Regra específica para cada arquivo para evitar problemas com $<
elfload.elf: $(OBJS) elfloader.o elfreloc_m68k.o ../../../libc/obj/crt0.o ../../../libc/obj/startup.o
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	@echo "Linked: $@"
	@echo "╔══════════════════════════╗"
	@echo "║  COMPILACAO TERMINADA!   ║"
	@echo "╚══════════════════════════╝"

elfload.bin: elfload.elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

clean:
	rm -f *.bin $(OBJS) *.map $(TARGET_LOW) $(TARGET_HIGH) $(TARGET_ELF) *.elf
	@echo "Clean complete"

