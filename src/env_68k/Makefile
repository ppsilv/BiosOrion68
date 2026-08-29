# ============================================
# MAKEFILE MESTRE - ORION68DOS
# ============================================
# Este Makefile chama os Makefiles de cada lib
# ============================================

# Diretórios das bibliotecas
LIBS = libc libdiskio libdrivers libfileio librtc libvideo

# ============================================
# TARGETS PRINCIPAIS
# ============================================

# Target padrão: compila tudo
all: $(LIBS)

# Compila cada lib
$(LIBS):
	$(MAKE) -C $@

# ============================================
# LIMPEZA
# ============================================

clean:
	for dir in $(LIBS); do \
		$(MAKE) -C $$dir clean; \
	done

# ============================================
# INSTALAÇÃO (copia as libs para ../lib)
# ============================================

install: all
	mkdir -p ../lib
	for dir in $(LIBS); do \
		cp $$dir/*.a ../lib/; \
	done

# ============================================
# DEPENDÊNCIAS (ordem de compilação)
# ============================================

# libdrivers não depende de nada
libdrivers:

# libdiskio depende de libdrivers
libdiskio: libdrivers

# libc não depende de nada (compila primeiro)
libc:

# libfileio depende de libdiskio e libc
libfileio: libdiskio libc

# librtc não depende de nada
librtc:

# libvideo não depende de nada
libvideo:

# ============================================
# HELP
# ============================================

help:
	@echo "Targets disponíveis:"
	@echo "  all        - Compila todas as libs"
	@echo "  clean      - Limpa todos os objetos"
	@echo "  install    - Copia .a para ../lib"
	@echo "  help       - Mostra esta mensagem"
	@echo ""
	@echo "Libs individuais:"
	@echo "  libc        - Biblioteca C"
	@echo "  libdiskio   - Disk I/O (FatFS interface)"
	@echo "  libdrivers  - Drivers de hardware"
	@echo "  libfileio   - Syscalls (trap #12)"
	@echo "  librtc      - RTC"
	@echo "  libvideo    - Vídeo"

# ============================================
# PHONY (evita conflito com arquivos)
# ============================================

.PHONY: all clean install help $(LIBS)
