CC = m68k-elf-gcc
LD = m68k-elf-ld
OBJCOPY = m68k-elf-objcopy
OBJDUMP = m68k-elf-objdump

CFLAGS = -Wall -Os -I../../include -ffreestanding -nostdlib -D __68000__ -D ALIGN_MEMORY -ffunction-sections -fdata-sections -fno-common

# Flags específicas para gerar o listing (compila e já gera o assembly)
# -Wa,-adhln=vfsorion.lst vai gerar o listing de TODOS os arquivos de uma vez
ALL_CFLAGS = $(CFLAGS)

LDFLAGS = -T ./linker.ld -L../../lib -nostdlib -L../../lib --no-undefined --relax --gc-sections --print-gc-sections  --strip-debug  -Map vfsorion.map 
LDLIBS = -lvideo -lrtc  -lfileio -ldiskio -ldrivers -lc68k 

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
ELFS = $(SRCS:.c=.elf)
BINS = $(SRCS:.c=.bin)

.PHONY: all clean


all: shell.elf shell.bin listing

#%.o: %.c
#	$(CC) $(CFLAGS) -c $< -o $@

# Regra específica para cada arquivo para evitar problemas com $<
shell.elf: $(SRCS)  ../../libc/obj/crt0.o ../../libc/obj/startup.o
	# Compila TODOS os .c de uma vez, gerando os .o e o .lst unificado
	$(CC) $(ALL_CFLAGS) -c $(SRCS)
	# Linka tudo
	$(LD) $(LDFLAGS) $(OBJS) ../../libc/obj/crt0.o ../../libc/obj/startup.o -o $@ $(LDLIBS)
	@echo "Linked: $@"

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	@echo "Binary: $@"

listing: shell.elf
	$(OBJDUMP) -d -S shell.elf > shell_full.lst
	@echo "Listing completo gerado em shell_full.lst"
	
	# Gera também o listing apenas do código (sem fonte)
	$(OBJDUMP) -d shell.elf > shell_code.lst
	@echo "Listing de código gerado em shell_code.lst"
	
	# Mostra o tamanho das seções
	$(OBJDUMP) -h shell.elf | grep -E "\.text|\.data|\.bss|\.rodata"
	
	# Mostra os símbolos ordenados por tamanho
	@echo "\n=== Maiores funções ==="
	$(OBJDUMP) -t shell.elf | grep -E "\.text\." | sort -k5 -n -r | head -20

clean:
	rm -f $(OBJS) $(ELFS) $(BINS) *.lst *.map
	@echo "Clean complete"
