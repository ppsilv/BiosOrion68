# ============================================
# MAKEFILE PARA MEUSO - VFS
# ============================================

CC = m68k-elf-gcc
CFLAGS = -Wall -Os -I../../include -I. -ffreestanding -nostdlib -D __68000__ -D ALIGN_MEMORY
LDFLAGS = -nostdlib -T linker.ld

VFS_SRCS = vfs.c vfs_fat.c vfs_tty.c vfs_null.c vfs_zero.c vfs_proc.c vfs_drivers.c shell.c
VFS_OBJS = $(VFS_SRCS:.c=.o)

LIBFILEIO = ../../lib/libfileio.a
LIBC =../../lib/libc68k.a

TARGET = shell.elf

all: $(TARGET)

$(TARGET): $(VFS_OBJS) $(LIBFILEIO) $(LIBC)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(VFS_OBJS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /path/to/your/rootfs/bin/shell

.PHONY: all clean install

