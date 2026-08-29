#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vfs.h"

/*
 * COMO USAR
 *

 #include <stdio.h>
#include "vfs.h"

void exemplo_abrir_arquivo(void) {
    // 1. Abre o arquivo usando VFS
    int fd = vfs_open("/boot.bat", O_RDONLY);

    if (fd < 0) {
        printf("Erro: não foi possível abrir o arquivo\n");
        return;
    }

    printf("Arquivo aberto! Descritor: %d\n", fd);

    // 2. Lê o conteúdo
    char buffer[128];
    int bytes = vfs_read(fd, buffer, sizeof(buffer) - 1);

    if (bytes > 0) {
        buffer[bytes] = '\0';  // Termina a string
        printf("Conteúdo: %s\n", buffer);
    }

    // 3. Fecha o arquivo
    vfs_close(fd);
    printf("Arquivo fechado!\n");
}

 void exemplo_ler_arquivo_inteiro(const char *path) {
    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0) {
        printf("Erro: não foi possível abrir %s\n", path);
        return;
    }

    char buffer[1024];
    int total = 0;
    int bytes;

    // Lê em blocos até o final
    while ((bytes = vfs_read(fd, buffer, sizeof(buffer))) > 0) {
        vfs_write(1, buffer, bytes);  // Escreve no terminal (stdout)
        total += bytes;
    }

    vfs_close(fd);
    printf("\nTotal de %d bytes lidos\n", total);
}
void exemplo_dispositivos(void) {
    // Abrir o terminal (/dev/tty)
    int tty = vfs_open("/dev/tty", O_RDWR);
    if (tty >= 0) {
        vfs_write(tty, "Digite algo: ", 13);

        char buffer[64];
        int bytes = vfs_read(tty, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            vfs_write(tty, "Você digitou: ", 14);
            vfs_write(tty, buffer, strlen(buffer));
        }

        vfs_close(tty);
    }

    // Abrir /dev/null (descarta dados)
    int null = vfs_open("/dev/null", O_WRONLY);
    if (null >= 0) {
        vfs_write(null, "Isso será descartado", 20);
        vfs_close(null);
    }

    // Abrir /proc/meminfo (informações do sistema)
    int mem = vfs_open("/proc/meminfo", O_RDONLY);
    if (mem >= 0) {
        char buffer[256];
        int bytes = vfs_read(mem, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            vfs_write(1, buffer, bytes);  // Mostra no terminal
        }
        vfs_close(mem);
    }
}

// Implementação do comando 'cat'
int cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        vfs_write(1, "Uso: cat <arquivo>\n", 20);
        return -1;
    }

    // Abre o arquivo (pode ser /dev/tty, /proc/meminfo, etc.)
    int fd = vfs_open(argv[1], O_RDONLY);
    if (fd < 0) {
        vfs_write(1, "Erro: não foi possível abrir ", 29);
        vfs_write(1, argv[1], strlen(argv[1]));
        vfs_write(1, "\n", 1);
        return -1;
    }

    // Lê e escreve no stdout (fd 1)
    char buffer[128];
    int bytes;
    while ((bytes = vfs_read(fd, buffer, sizeof(buffer))) > 0) {
        vfs_write(1, buffer, bytes);
    }

    vfs_close(fd);
    return 0;
}
// Implementação do comando 'echo' com redirecionamento
int cmd_echo(int argc, char **argv) {
    if (argc < 2) {
        vfs_write(1, "Uso: echo <texto>\n", 19);
        return -1;
    }

    // Escreve no stdout (fd 1)
    for (int i = 1; i < argc; i++) {
        vfs_write(1, argv[i], strlen(argv[i]));
        if (i < argc - 1) vfs_write(1, " ", 1);
    }
    vfs_write(1, "\n", 1);

    return 0;
}

// Exemplo de uso no shell:
// echo "Hello World" > /tmp/teste.txt

// O que acontece quando você chama vfs_open()
int vfs_open(const char *path, int flags) {
    File *file = malloc(sizeof(File));

    // 1. Tenta abrir como arquivo FATFS
    if (fat_open(file, path, flags) == 0) {
        // fat_open preencheu:
        // - file->read = fat_read
        // - file->write = fat_write
        // - file->close = fat_close
        // - file->private_data = FIL*
        return allocate_fd(file);
    }

    // 2. Tenta abrir como dispositivo
    if (strcmp(path, "/dev/tty") == 0) {
        tty_open(file, path, flags);
        // tty_open preencheu:
        // - file->read = tty_read
        // - file->write = tty_write
        // - file->close = tty_close
        return allocate_fd(file);
    }

    // 3. Não encontrou
    free(file);
    return -1;
}

// Esta função funciona com qualquer coisa que o VFS suporta!
void copiar_arquivo(const char *origem, const char *destino) {
    int src = vfs_open(origem, O_RDONLY);
    if (src < 0) {
        vfs_write(1, "Erro: origem não encontrada\n", 29);
        return;
    }

    int dst = vfs_open(destino, O_WRONLY | O_CREAT | O_TRUNC);
    if (dst < 0) {
        vfs_write(1, "Erro: destino não pode ser criado\n", 35);
        vfs_close(src);
        return;
    }

    char buffer[128];
    int bytes;
    while ((bytes = vfs_read(src, buffer, sizeof(buffer))) > 0) {
        vfs_write(dst, buffer, bytes);
    }

    vfs_close(src);
    vfs_close(dst);
    vfs_write(1, "Cópia concluída!\n", 18);
}

// Exemplos de uso:
// copiar_arquivo("/boot.bat", "/tmp/boot.bak")
// copiar_arquivo("/proc/meminfo", "/tmp/meminfo.txt")
// copiar_arquivo("/dev/tty", "/tmp/entrada.txt")  // Copia o que for digitado!



 */

// ============================================
// TABELA DE DESCRITORES GLOBAL
// ============================================
File *fd_table[256];
static int fd_count = 0;

extern DeviceDriver drivers[];

// ============================================
// FUNÇÕES DE ACESSO À TABELA DE DESCRITORES
// ============================================

void vfs_set_fd(int fd, File *file) {
    if (fd >= 0 && fd < 256) {
        fd_table[fd] = file;
    }
}

File *vfs_get_fd(int fd) {
    if (fd >= 0 && fd < 256) {
        return fd_table[fd];
    }
    return NULL;
}

void vfs_restore_fd(int fd) {
    if (fd >= 0 && fd < 256) {
        // Restaura para /dev/tty
        int tty_fd = vfs_open("/dev/tty", O_RDWR);
        if (tty_fd >= 0) {
            fd_table[fd] = fd_table[tty_fd];
            fd_table[tty_fd] = NULL;
        }
    }
}

// ============================================
// ALLOCATE_FD
// ============================================
int allocate_fd(File *file) {
    for (int i = 3; i < 256; i++) {
        if (fd_table[i] == NULL) {
            fd_table[i] = file;
            fd_count++;
            return i;
        }
    }
    return -1;
}

// ============================================
// GET_FILE_FROM_FD
// ============================================
File *get_file_from_fd(int fd) {
    if (fd < 0 || fd >= 256) return NULL;
    return fd_table[fd];
}

// ============================================
// FREE_FD
// ============================================
void free_fd(int fd) {
    if (fd >= 0 && fd < 256) {
        fd_table[fd] = NULL;
        fd_count--;
    }
}

// ============================================
// VFS_OPEN
// ============================================
// vfs.c - vfs_open()
extern int fat_open(File *file, const char *path, int flags);
int vfs_open(const char *path, int flags) {
    
    File *file = (File*)malloc(sizeof(File));
    if (!file) return -1;
    memset(file, 0, sizeof(File));
    file->name = strdup(path);
    
    if (fat_open(file, path, flags) == 0) {
        printf("vfs_open: FATFS abriu!\n");
        // ...
    }
    
    for (int i = 0; drivers[i].path != NULL; i++) {
        printf("vfs_open: comparando '%s' com '%s'\n", path, drivers[i].path);
        if (strcmp(path, drivers[i].path) == 0) {
            printf("vfs_open: ENCONTROU! i=%d\n", i);
            if (drivers[i].open(file, path, flags) == 0) {
                int fd = allocate_fd(file);
                if (fd >= 0) {
                    printf("vfs_open: fd=%d (SUCESSO!)\n", fd);
                    return fd;
                }
            }
        }
    }
    
    free(file->name);
    free(file);
    return -1;
}
int vfs_open1(const char *path, int flags) {
    File *file = (File*)malloc(sizeof(File));
    if (!file) return -1;
    
    memset(file, 0, sizeof(File));
    file->name = strdup(path);
    file->type = FILE_TYPE_REGULAR;
    file->position = 0;
    file->ref_count = 1;
    
    if (fat_open(file, path, flags) == 0) {
        int fd = allocate_fd(file);
        if (fd >= 0) return fd;
        free(file->name);
        free(file);
        return -1;
    }
    
    for (int i = 0; drivers[i].path != NULL; i++) {
        if (strcmp(path, drivers[i].path) == 0) {
            file->type = FILE_TYPE_DEVICE;
            file->read = drivers[i].read;
            file->write = drivers[i].write;
            file->close = drivers[i].close;
            file->ioctl = drivers[i].ioctl;
            file->lseek = drivers[i].lseek;
            
            if (drivers[i].open(file, path, flags) == 0) {
                int fd = allocate_fd(file);
                if (fd >= 0) {
                    printf("drivers[i].path[%s] fd=\n",drivers[i].path,fd);
                    return fd;
                }
            }
            break;
        }
    }
    
    free(file->name);
    free(file);
    return -1;
}

// ============================================
// VFS_READ
// ============================================
int vfs_read(int fd, void *buffer, size_t size) {
    File *file = get_file_from_fd(fd);
    if (!file) return -1;
    if (file->read) return file->read(file, buffer, size);
    return -1;
}

// ============================================
// VFS_WRITE
// ============================================
int vfs_write(int fd, const void *buffer, size_t size) {
    File *file = get_file_from_fd(fd);
    if (!file) return -1;
    if (file->write) return file->write(file, buffer, size);
    return -1;
}

// ============================================
// VFS_CLOSE
// ============================================
int vfs_close(int fd) {
    File *file = get_file_from_fd(fd);
    if (!file) return -1;
    
    int result = 0;
    if (file->close) result = file->close(file);
    
    free(file->name);
    free(file);
    free_fd(fd);
    return result;
}

// ============================================
// VFS_IOCTL
// ============================================
int vfs_ioctl(int fd, int cmd, void *arg) {
    File *file = get_file_from_fd(fd);
    if (!file) return -1;
    if (file->ioctl) return file->ioctl(file, cmd, arg);
    return -1;
}

// ============================================
// VFS_LSEEK
// ============================================
size_t vfs_lseek(int fd, size_t offset, int whence) {
    File *file = get_file_from_fd(fd);
    if (!file) return (size_t)-1;
    
    if (file->lseek) return file->lseek(file, offset, whence);
    
    switch (whence) {
        case 0: file->position = offset; break;
        case 1: file->position += offset; break;
        case 2: return (size_t)-1;
        default: return (size_t)-1;
    }
    return file->position;
}

// ============================================
// VFS_INIT
// ============================================
void vfs_init(void) {
    memset(fd_table, 0, sizeof(fd_table));
    fd_count = 0;
    
    int fd0 = vfs_open("/dev/tty", O_RDWR);
    int fd1 = vfs_open("/dev/tty", O_RDWR);
    int fd2 = vfs_open("/dev/tty", O_RDWR);
    
    if (fd0 != 0) {
        fd_table[0] = fd_table[fd0];
        fd_table[fd0] = NULL;
        fd_count--;
    }
    if (fd1 != 1) {
        fd_table[1] = fd_table[fd1];
        fd_table[fd1] = NULL;
        fd_count--;
    }
    if (fd2 != 2) {
        fd_table[2] = fd_table[fd2];
        fd_table[fd2] = NULL;
        fd_count--;
    }
    
    printf("VFS inicializado: stdin=0, stdout=1, stderr=2\n");
}
