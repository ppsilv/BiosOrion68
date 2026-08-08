CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy
VASM = vasmm68k_mot 

CFLAGS = -Wall -O0 -I ./includes -I../../include -ffreestanding -nostdlib
CFLAGS += -DCHIP_$(CHIP_TYPE) -DROM_SIZE_BYTES=$(ROM_SIZE)
CFLAGS += -fomit-frame-pointer

VASMFLAGS = -Felf -m68000 -quiet -I ./includes -I../../include

LDFLAGS = -T ./orion68.ld -L../../lib -nostdlib  -Map=orion68.map
LDLIBS = -lc68k

# 1. ADICIONADO: Lista de arquivos em Assembly (.S)
ASMS = ./xmodem/xmodem_asm.S vectors_asm.S


VASMS =

# Lista de arquivos C
SRCS = orion68.c vectors.c console.c \
./dump/mem_dump.c \
./shell/io.c \
./shell/picow.c \
./shell/elf_loader.c \
./shell/printerro.c \
./shell/commands.c \
./shell/decodecmd.c \
./shell/crc32.c \
./shell/cmd_repeat.c \
./kbd/drv_kbd.c \
./kbd/ringbuffer.c \
./rtc/rp5c01a.c \
./picoVga/drv_picoVga.c \
./uarts/drv_uart0.c \
./duart/duart.c \
./ide/ata.c \
./fatfs/ff.c \
./fatfs/ffsystem.c \
./fatfs/diskio.c \
./tst_fat/tstfat.c \
timers.c interrupt.c

#./shell/srecord.c \


#./fatfs/diskio.c \
#./fatfs/ff.c \
#./tst_fat/tstfat.c \
#./tst_fat/ler_joblog.c \


# 2. ALTERADO: Agora a lista de objetos junta os .o vindos do .c e do .S
OBJS = $(SRCS:.c=.o) $(ASMS:.S=.o) $(VASMS:.asm=.o)

#Tools
SPLITTER := python3 ./tools/split68k.py
FILLER := python3 ./tools/fill.py
PROGRAMMER := minipro
CHECKSUMER := python3 ./tools/checksum_put.py

CHIP ?= AT28C256
ifeq ($(CHIP), AT29C512)
    CHIP_TYPE  := AT29C512
    FLASH_SIZE := 65536  # 64KB
    ROM_SIZE   := 65536  # 64KB
else ifeq ($(CHIP), AT28C256)
    CHIP_TYPE  := AT28C256
    FLASH_SIZE := 32768  # 32KB
    ROM_SIZE   := 32768  # 32KB
else
    $(error Chip desconhecido: $(CHIP). Escolha AT29C512 ou AT28C256)
endif

TARGET := orion68
TARGET_OBJ := $(TARGET).o
TARGET_ELF := $(TARGET).elf
TARGET_BIN := $(TARGET).bin
TARGET_LOW := $(TARGET)_low.bin
TARGET_HIGH := $(TARGET)_high.bin

.PHONY: all clean split fill programL programH

all: orion68.bin

# Regra para compilar os arquivos .c
%.o: %.c
	@./tools/build_counter.sh
	$(CC) $(CFLAGS) -c $< -o $@

# 3. ADICIONADO: Regra explícita para compilar arquivos Assembly (.S)
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(VASM) $(VASMFLAGS) $< -o $@

orion68.elf: ./tools/build_counter.sh $(OBJS)
	$(LD) $(LDFLAGS) ../../obj/orion68_startup.o $(OBJS) $(LDLIBS) -o $@
	@echo "Linked: $@"
	@echo "╔══════════════════════════╗"
	@echo "║  COMPILACAO TERMINADA!   ║"
	@echo "╚══════════════════════════╝"

orion68.bin: orion68.elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

cplib:
	cp fatlib/lib/libfatfs.a ../../lib/.

clean:
	rm -f *.bin $(OBJS) *.map $(TARGET_LOW) $(TARGET_HIGH) $(TARGET_ELF)
	@echo "Clean complete"

split: orion68.elf
	@echo "Dividindo $(TARGET_BIN) em words 16-bit..."
	@$(SPLITTER) $(TARGET_BIN)
	@$(FILLER) $(TARGET_LOW) $(FLASH_SIZE)
	@$(FILLER) $(TARGET_HIGH) $(FLASH_SIZE)
	@echo "Arquivos gerados: $(TARGET_LOW) e $(TARGET_HIGH)"

fill: split
	@echo "Preenchendo arquivos para $(FLASH_SIZE) bytes..."
	@echo "Preenchimento completo."

programL:
	@echo "Gravando parte LOW na Flash..."
	$(PROGRAMMER) -p $(CHIP_TYPE) -w $(TARGET_LOW) -s
	@echo "Gravação concluída LOW! BOA SORTE"

programH: split
	@echo "Gravando parte HIGH na Flash..."
	@echo $(CHIP_TYPE)
	$(PROGRAMMER) -p $(CHIP_TYPE) -w $(TARGET_HIGH) -s
	@echo "Gravação concluída HIGH! GRAVE A MEMORIA LOW"
