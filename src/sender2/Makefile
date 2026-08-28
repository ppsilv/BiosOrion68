# ==============================================================================
# MAKEFILE PARA O CLIENTE TRANSMISSOR - ORION68
# ==============================================================================

# 1. Configurações do Compilador e Flags
CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -MMD -MP
TARGET   := sender

# 2. Diretórios do Projeto
SRC_DIR  := .
OBJ_DIR  := obj

# 3. Mapeamento de Arquivos Fontes e Objetos
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS     := $(OBJS:.o=.d)

# 4. Regra Padrão (Executada ao digitar apenas 'make')
.PHONY: all
all: $(TARGET)

# 5. Linkagem do Executável Final
$(TARGET): $(OBJS)
	@echo "  LINK    $@"
	@$(CC) $(OBJS) -o $@

# 6. Compilação dos Arquivos Objetos (.c -> .o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# 7. Cria o diretório de objetos se ele não existir
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# 8. Regra para Limpeza do Projeto ('make clean')
.PHONY: clean
clean:
	@echo "  CLEAN   Removendo arquivos temporários e executável..."
	@rm -rf $(OBJ_DIR) $(TARGET)
	@echo "  Pronto!"

# 9. Regra para recompilar do zero ('make rebuild')
.PHONY: rebuild
rebuild: clean all

# 10. Inclui as dependências geradas pelo GCC (.d) para monitorar alterações em headers
-include $(DEPS)
