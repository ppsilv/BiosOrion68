# Makefile para desenvolvimento MC68000
# Versão otimizada para gerar binários compactos
# VERSÃO: 1.1
# - Acrescentado @$(FILLER) $(TARGET_BIN) $(FLASH_SIZE)
# Configurações
ASM := vasmm68k_mot
ASSEMBLER := vasmm68k_mot
LINKER := m68k-elf-ld
ASM_ELF := vasmm68k_mot
OBJCOPY := m68k-elf-objcopy
#ASMFLAGS := -quiet -L
ELF_FLAGS := -Felf -quiet
SPLITTER := python3 split68k.py
FILLER := python3 fill.py
PROGRAMMER := minipro
CHECKSUMER := python3 checksum_put.py
CHIP_TYPE := SST39SF010 #28C64A  # Altere para o seu chip Flash

# Nomes de arquivos
SOURCE := test.asm
TARGET := test
#TARGET := $(basename $(SOURCE))
TARGET_OBJ := $(TARGET).o
TARGET_ELF := $(TARGET).elf
TARGET_BIN := $(TARGET).bin
TARGET_LOW := $(TARGET)_low.bin
TARGET_HIGH := $(TARGET)_high.bin
LAYOUT_FILE := layout.ld

# Tamanhos (ajuste conforme sua memória)
#FLASH_SIZE := 8192  # 8KB (tamanho da 28C64A)
#ROM_SIZE := 8192     # 8KB usados
#FLASH_SIZE := 16384  # 8KB (tamanho da 28C64A)
#ROM_SIZE := 16384     # 8KB usados
FLASH_SIZE := 131072  # 128KB (tamanho da 39SF010)
ROM_SIZE := 131072     # 128KB usados



#.PHONY: all compile compile_compact split fill program clean
.PHONY: all compile split fill program clean

all: split

# Compilação tradicional (pode gerar binário grande)
compile:
	@echo "Compilando $(SOURCE) para $(TARGET_BIN)..."
	@$(ASM) -Fbin $(ASMFLAGS) -L $(TARGET).lst -o $(TARGET_BIN) $(SOURCE)
	@echo "Tamanho do binário: $$(stat -c%s $(TARGET_BIN)) bytes"
	@echo "╔════════════════════════════════╗"
	@echo "║  Compilação Concluída!         ║"
	@echo "╠════════════════════════════════╣"
	@echo "║ Binário: $(TARGET_BIN)         ║"
	@echo "║ Tamanho: $$(stat -c%s $(TARGET_BIN)) bytes          ║"
	@echo "║ Listagem: $(TARGET).lst        ║"
	@echo "╚════════════════════════════════╝"


# Compilação segura que detecta TODOS os erros
compile_linker:
	@echo "Compiler compile_linker..."
	#vasmm68k_mot -Felf -quiet -L listing.txt -o test_ram2.o test_ram2.asm
	@$(ASSEMBLER) $(ELF_FLAGS) -L $(TARGET).lst -o $(TARGET_OBJ) $(SOURCE)
	@echo "Linker..."
	#m68k-elf-ld -T layout.ld test_ram2.o -o test_ram2.elf
	#m68k-elf-ld -T layout.ld --print-memory-usage -Map=mapfile.txt test_ram2.o -o test_ram2.elf
	@$(LINKER) -T $(LAYOUT_FILE) --print-memory-usage -Map=$(TARGET).map $(TARGET_OBJ) -o $(TARGET_ELF)
	@echo "Objcopy..."
	#m68k-elf-objcopy -O binary test_ram2.elf test_ram2.bin
	@$(OBJCOPY) -O binary $(TARGET_ELF) $(TARGET_BIN)
	@echo "╔══════════════════════════╗"
	@echo "║  COMPILACAO TERMINADA!   ║"
	@echo "╚══════════════════════════╝"

test:
	@echo "Compiler..."
	@build_date.sh
	@$(ASSEMBLER) $(ELF_FLAGS) -L $(TARGET).lst -o $(TARGET_OBJ) $(SOURCE)
	@echo "Linker..."
	@$(LINKER) -T $(LAYOUT_FILE) --print-memory-usage -Map=$(TARGET).map $(TARGET_OBJ) -o $(TARGET_ELF)
	@echo "Objcopy..."
	@$(OBJCOPY) -O binary $(TARGET_ELF) $(TARGET_BIN)
	@$(FILLER) $(TARGET_BIN) $(FLASH_SIZE)
	@$(CHECKSUMER)
	@echo "╔═════════════════════════════════════╗"
	@echo "║  COMPILACAO TTEESSTTEE TERMINADA!   ║"
	@echo "╚═════════════════════════════════════╝"


split: compile_linker
	@echo "Dividindo $(TARGET_BIN) em words 16-bit..."
	@$(SPLITTER) $(TARGET_BIN)
	@$(FILLER) $(TARGET_LOW) $(FLASH_SIZE)
	@$(FILLER) $(TARGET_HIGH) $(FLASH_SIZE)
	@echo "Arquivos gerados: $(TARGET_LOW) e $(TARGET_HIGH)"

fill: split
	@echo "Preenchendo arquivos para $(FLASH_SIZE) bytes..."
	#@$(FILLER) $(TARGET_LOW) $(FLASH_SIZE)
	#@$(FILLER) $(TARGET_HIGH) $(FLASH_SIZE)
	@echo "Preenchimento completo."

programL:
	@echo "Gravando parte LOW na Flash..."
	@$(PROGRAMMER) -p $(CHIP_TYPE) -w $(TARGET_LOW) -s
	@echo "Gravação concluída LOW! BOA SORTE"

programH: split
	@echo "Gravando parte HIGH na Flash..."
	@$(PROGRAMMER) -p $(CHIP_TYPE) -w $(TARGET_HIGH) -s
	@echo "Gravação concluída HIGH! GRAVE A MEMORIA LOW"

pgmL:
	@echo "Gravando parte LOW na Flash..."
	@$(PROGRAMMER) -p $(CHIP_TYPE) -w $(TARGET_LOW) -s
	@echo "Gravação concluída LOW! BOA SORTE"

pgmH:
	@echo "Gravando parte HIGH na Flash..."
	@$(PROGRAMMER) -p $(CHIP_TYPE) -w $(TARGET_HIGH) -s
	@echo "Gravação concluída HIGH! GRAVE A MEMORIA LOW"

program: programL programH

clean:
	@rm -f *.bin *.o *.txt *.lst *.elf *.map
	@echo "Arquivos temporários removidos."

help:
	@echo "Makefile para desenvolvimento MC68000 (versão compacta)"
	@echo "Opções:"
	@echo "  make compile        - Compila tradicional (pode gerar binário grande)"
	@echo "  make compile_linker - Compila gerando binário com sessões code,data e bss (recomendado)"
	@echo "  make split          - Compila (compacto) e divide o binário"
	@echo "  make fill           - Compila, divide e preenche os arquivos"
	@echo "  make program        - Executa todo o fluxo e grava nas Flash ROMs"
	@echo "  make programL       - Grava apenas a parte LOW não compila o binario"
	@echo "  make programH       - Grava apenas a parte HIGH compila o binario"
	@echo "  make pgmL           - Grava apenas a parte LOW não compila o binario"
	@echo "  make pgmH           - Grava apenas a parte HIGH não compila o binario"
	@echo "  make clean          - Remove arquivos gerados"
	@echo "  make help           - Mostra esta ajuda"
