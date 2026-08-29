#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fileio.h>
#include "vfs.h"

#define CMD_EXIT_SHELL  99  // Código especial para sair

extern File *fd_table[256];

// ============================================
// ESTRUTURA DE COMANDO
// ============================================
typedef struct {
    char *name;
    int (*func)(int argc, char **argv);
} Command;

// ============================================
// PROTÓTIPOS DOS COMANDOS
// ============================================
int cmd_help(int argc, char **argv);
int cmd_echo(int argc, char **argv);
int cmd_cat(int argc, char **argv);
int cmd_ls(int argc, char **argv);
int cmd_cd(int argc, char **argv);
int cmd_mkdir(int argc, char **argv);
int cmd_reboot(int argc, char **argv);
int cmd_shutdown(int argc, char **argv);
int cmd_meminfo(int argc, char **argv);
int cmd_clear(int argc, char **argv);
int cmd_exit(int argc, char **argv);

// ============================================
// TABELA DE COMANDOS
// ============================================
Command commands[] = {
    {"help",     cmd_help},
    {"echo",     cmd_echo},
    {"cat",      cmd_cat},
    {"ls",       cmd_ls},
    {"cd",       cmd_cd},
    {"mkdir",    cmd_mkdir},
    {"reboot",   cmd_reboot},
    {"shutdown", cmd_shutdown},
    {"meminfo",  cmd_meminfo},
    {"clear",    cmd_clear},
    {"exit",     cmd_exit},
    {NULL, NULL}
};

// ============================================
// GETS_LINE
// ============================================
char *gets_line(char *buffer, int size) {
    int i = 0;
    char c;

    while (i < size - 1) {
        c = getchar();

        if (c == '\r' || c == '\n') {
            buffer[i] = '\0';
            putchar('\n');
            return buffer;
        }
        else if (c == '\b' || c == 0x7F) {
            if (i > 0) {
                i--;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        }
        else if (c >= 0x20 && c < 0x7F) {
            buffer[i++] = c;
            putchar(c);
        }
    }
    buffer[i] = '\0';
    return buffer;
}

// ============================================
// EXECUTE_LINE - Com redirecionamento
// ============================================
int execute_line(char *line) {
    char *args[16];
    int argc = 0;
    char *token;
    char line_copy[256];
    char *redirect_file = NULL;
    int redirect_mode = 0;  // 0 = nenhum, 1 = >, 2 = >>, 3 = <
    int input_fd = -1;
    int output_fd = -1;
    int saved_stdin = -1;
    int saved_stdout = -1;

    // Remove espaços iniciais
    while (*line == ' ') line++;

    // Remove \n e \r
    line[strcspn(line, "\n")] = 0;
    line[strcspn(line, "\r")] = 0;

    if (line[0] == '\0' || line[0] == '#') return 0;

    // ============================================
    // PASSO 1: PROCURA POR REDIRECIONAMENTOS
    // ============================================
    strcpy(line_copy, line);

    // Procura por "<<"
    char *heredoc_pos = strstr(line_copy, "<<");
    if (heredoc_pos) {
        vfs_write(1, "Heredoc nao implementado\n", 26);
        return -1;
    }

    // Procura por ">>" (append)
    char *append_pos = strstr(line_copy, ">>");
    if (append_pos) {
        redirect_mode = 2;
        *append_pos = '\0';
        redirect_file = append_pos + 2;
        while (*redirect_file == ' ') redirect_file++;
        char *p = redirect_file + strlen(redirect_file) - 1;
        while (p > redirect_file && *p == ' ') *p-- = '\0';
    }

    // Procura por ">" (sobrescrever)
    char *output_pos = strstr(line_copy, ">");
    if (output_pos && redirect_mode == 0) {
        if (*(output_pos + 1) != '>') {
            redirect_mode = 1;
            *output_pos = '\0';
            redirect_file = output_pos + 1;
            while (*redirect_file == ' ') redirect_file++;
            char *p = redirect_file + strlen(redirect_file) - 1;
            while (p > redirect_file && *p == ' ') *p-- = '\0';
        }
    }

    // Procura por "<" (entrada)
    char *input_pos = strstr(line_copy, "<");
    if (input_pos) {
        if (*(input_pos + 1) != '<') {
            char *input_file = input_pos + 1;
            while (*input_file == ' ') input_file++;
            char *p = input_file + strlen(input_file) - 1;
            while (p > input_file && *p == ' ') *p-- = '\0';

            input_fd = vfs_open(input_file, O_RDONLY);
            if (input_fd < 0) {
                vfs_write(1, "Erro: nao foi possivel abrir entrada: ", 39);
                vfs_write(1, input_file, strlen(input_file));
                vfs_write(1, "\n", 1);
                return -1;
            }
            *input_pos = '\0';
        }
    }

    // ============================================
    // PASSO 2: PROCESSA O COMANDO
    // ============================================
    char *cmd = line_copy;
    while (*cmd == ' ') cmd++;

    char *args_copy[16];
    int argc_copy = 0;
    token = strtok(cmd, " ");
    while (token != NULL && argc_copy < 16) {
        args_copy[argc_copy++] = token;
        token = strtok(NULL, " ");
    }

    if (argc_copy == 0) {
        if (input_fd >= 0) vfs_close(input_fd);
        return 0;
    }

    // ============================================
    // PASSO 3: EXECUTA COM REDIRECIONAMENTO
    // ============================================

    saved_stdin = 0;
    saved_stdout = 1;

    if (input_fd >= 0) {
        File *new_stdin = get_file_from_fd(input_fd);
        if (new_stdin) {
            fd_table[0] = new_stdin;
        }
    }

    if (redirect_mode > 0 && redirect_file) {
        int flags = O_WRONLY;
        if (redirect_mode == 1) flags |= O_TRUNC;
        if (redirect_mode == 2) flags |= O_APPEND;

        output_fd = vfs_open(redirect_file, flags);
        if (output_fd < 0) {
            vfs_write(1, "Erro: nao foi possivel criar saida: ", 37);
            vfs_write(1, redirect_file, strlen(redirect_file));
            vfs_write(1, "\n", 1);
            if (input_fd >= 0) vfs_close(input_fd);
            return -1;
        }

        File *new_stdout = get_file_from_fd(output_fd);
        if (new_stdout) {
            fd_table[1] = new_stdout;
        }
    }

    // ============================================                                   
    // PASSO 4: EXECUTA O COMANDO
    // ============================================
    int result = -1;
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(commands[i].name, args_copy[0]) == 0) {
            result = commands[i].func(argc_copy, args_copy);
            break;
        }
    }

    if (result != 0 && result != CMD_EXIT_SHELL) {
        vfs_write(1, "Comando desconhecido: ", 23);
        vfs_write(1, args_copy[0], strlen(args_copy[0]));
        vfs_write(1, "\n", 1);
    }

    // ============================================
    // PASSO 5: RESTAURA OS DESCRITORES
    // ============================================

    if (output_fd >= 0) {
        vfs_close(output_fd);
        int tty_fd = vfs_open("/dev/tty", O_RDWR);
        if (tty_fd >= 0) {
            fd_table[1] = get_file_from_fd(tty_fd);
        }
    }

    if (input_fd >= 0) {
        vfs_close(input_fd);
        int tty_fd = vfs_open("/dev/tty", O_RDWR);
        if (tty_fd >= 0) {
            fd_table[0] = get_file_from_fd(tty_fd);
        }
    }

    return result;
}

// ============================================
// COMANDOS
// ============================================

int cmd_help(int argc, char **argv) {
    vfs_write(1, "Comandos disponiveis:\n", 23);
    for (int i = 0; commands[i].name != NULL; i++) {
        vfs_write(1, "  ", 2);
        vfs_write(1, commands[i].name, strlen(commands[i].name));
        vfs_write(1, "\n", 1);
    }
    return 0;
}

int cmd_echo(int argc, char **argv) {
     //printf("cmd_echo: argc=%d\n", argc);  // ← DEBUG
    for (int i = 1; i < argc; i++) {
        printf("cmd_echo: argv[%d]='%s'\n", i, argv[i]);  // ← DEBUG
        vfs_write(1, argv[i], strlen(argv[i]));
        if (i < argc - 1) vfs_write(1, " ", 1);
    }
    vfs_write(1, "\n", 1);
    return 0;
}

int cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        vfs_write(1, "Uso: cat <arquivo>\n", 20);
        return -1;
    }

    int fd = vfs_open(argv[1], O_RDONLY);
    if (fd < 0) {
        vfs_write(1, "Erro: nao foi possivel abrir ", 29);
        vfs_write(1, argv[1], strlen(argv[1]));
        vfs_write(1, "\n", 1);
        return -1;
    }

    char buffer[128];
    int bytes;
    while ((bytes = vfs_read(fd, buffer, sizeof(buffer))) > 0) {
        vfs_write(1, buffer, bytes);
    }

    vfs_close(fd);
    return 0;
}

int cmd_ls(int argc, char **argv) {
    DIR dir;
    FILINFO fno;
    const char *path = (argc > 1) ? argv[1] : "/";

    if (fopendir(&dir, path) != FR_OK) {
        vfs_write(1, "Erro: nao foi possivel listar ", 30);
        vfs_write(1, path, strlen(path));
        vfs_write(1, "\n", 1);
        return -1;
    }

    while (freaddir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
        vfs_write(1, fno.fname, strlen(fno.fname));
        if (fno.fattrib & AM_DIR) {
            vfs_write(1, "/", 1);
        }
        vfs_write(1, "  ", 2);
    }
    vfs_write(1, "\n", 1);

    fclosedir(&dir);
    return 0;
}

int cmd_cd(int argc, char **argv) {
    if (argc < 2) {
        vfs_write(1, "cd: falta diretorio\n", 21);
        return -1;
    }
    vfs_write(1, "cd: mudando para ", 18);
    vfs_write(1, argv[1], strlen(argv[1]));
    vfs_write(1, "\n", 1);
    return 0;
}

int cmd_mkdir(int argc, char **argv) {
    if (argc < 2) {
        vfs_write(1, "mkdir: falta diretorio\n", 24);
        return -1;
    }

    FRESULT result = fmkdir(argv[1]);
    if (result != FR_OK) {
        vfs_write(1, "Erro ao criar diretorio\n", 25);
        return -1;
    }

    vfs_write(1, "mkdir: criado ", 15);
    vfs_write(1, argv[1], strlen(argv[1]));
    vfs_write(1, "\n", 1);
    return 0;
}

int cmd_reboot(int argc, char **argv) {
    vfs_write(1, "Reiniciando sistema...\n", 24);
    return 0;
}

int cmd_shutdown(int argc, char **argv) {
    vfs_write(1, "Desligando sistema...\n", 23);
    return 0;
}

int cmd_meminfo(int argc, char **argv) {
    int fd = vfs_open("/proc/meminfo", O_RDONLY);
    if (fd < 0) {
        vfs_write(1, "Erro: /proc/meminfo nao disponivel\n", 36);
        return -1;
    }

    char buffer[128];
    int bytes;
    while ((bytes = vfs_read(fd, buffer, sizeof(buffer))) > 0) {
        vfs_write(1, buffer, bytes);
    }

    vfs_close(fd);
    return 0;
}

int cmd_clear(int argc, char **argv) {
    int fd = vfs_open("/dev/tty", O_RDWR);
    if (fd >= 0) {
        vfs_ioctl(fd, TIOC_CLEAR, NULL);
        vfs_close(fd);
    }
    return 0;
}

// ============================================
// CMD_EXIT - Sai do shell
// ============================================
int cmd_exit(int argc, char **argv) {
    vfs_write(1, "Saindo do shell...\n", 20);
    return CMD_EXIT_SHELL;  // ← Código especial!
}

// ============================================
// RUN_BOOT_SCRIPT
// ============================================
void run_boot_script(void) {
    int fd = vfs_open("/boot.bat", O_RDONLY);
    if (fd < 0) {
        vfs_write(1, "Nenhum script de boot encontrado\n", 34);
        return;
    }

    vfs_write(1, "\n=== Executando /boot.bat ===\n", 31);

    char line[256];
    int pos = 0;
    char c;
    int bytes;

    while ((bytes = vfs_read(fd, &c, 1)) > 0) {
        if (c == '\n' || c == '\r') {
            if (pos > 0) {
                line[pos] = '\0';
                pos = 0;

                if (line[0] != '#') {
                    vfs_write(1, "[boot] ", 7);
                    vfs_write(1, line, strlen(line));
                    vfs_write(1, "\n", 1);
                    execute_line(line);
                }
            }
            continue;
        }
        if (pos < sizeof(line) - 1) {
            line[pos++] = c;
        }
    }

    if (pos > 0) {
        line[pos] = '\0';
        if (line[0] != '#') {
            vfs_write(1, "[boot] ", 7);
            vfs_write(1, line, strlen(line));
            vfs_write(1, "\n", 1);
            execute_line(line);
        }
    }

    vfs_close(fd);
    vfs_write(1, "=== Script de boot finalizado ===\n\n", 36);
}

// ============================================
// SHELL_LOOP
// ============================================
void shell_loop(void) {
    char line[256];

    run_boot_script();

    vfs_write(1, "--- Shell do Orion68DOS ---\n", 29);
    vfs_write(1, "Digite 'help' para comandos\n\n", 30);

    while (1) {
        vfs_write(1, "> ", 2);
        gets_line(line, sizeof(line));
        int result = execute_line(line);
        
        // Se o comando exit foi executado, sai do loop
        if (result == CMD_EXIT_SHELL) {
            break;
        }
    }

    vfs_write(1, "Shell finalizado.\n", 19);
}

// ============================================
// MAIN
// ============================================
int main(void) {
    // ============================================
    // DEFINE A HEAP CORRETAMENTE
    // ============================================
    #define HEAP_START  0x94000
    #define HEAP_SIZE   (128 * 1024)  // 128KB
    
    void *heap_base = (void*)HEAP_START;
    void *heap_limit = (void*)(HEAP_START + HEAP_SIZE);
    size_t max_blocks = 64;
    size_t split_thresh = 16;
    size_t alignment = 4;
    
//    printf("Heap: 0x%x - 0x%x (%d bytes)\n", 
//           (uint32_t)heap_base, (uint32_t)heap_limit,
//           (uint32_t)(heap_limit - heap_base));
//    
    bool ok = malloc_init(heap_base, heap_limit, max_blocks, split_thresh, alignment);
    
    if (!ok) {
        printf("ERRO: malloc_init() falhou! Verifique heap_base < heap_limit\n");
        return -1;
    }
    

    vfs_init();
    shell_loop();
    vfs_write(1, "Retornando ao kernel...\n", 25);
    return 0;
}