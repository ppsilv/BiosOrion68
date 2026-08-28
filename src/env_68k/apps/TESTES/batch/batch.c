#include <stdint.h>
#include <stdio.h>

#include <fatfs/ff.h>
#include <fileio.h>
#include <string.h>

// ========== PROTÓTIPOS DAS FUNÇÕES ==========
// Coloque ANTES da tabela de comandos
int cmd_help(int argc, char **argv);
int cmd_echo(int argc, char **argv);
int cmd_ls(int argc, char **argv);
int cmd_cd(int argc, char **argv);
int cmd_mkdir(int argc, char **argv);
int cmd_reboot(int argc, char **argv);
int cmd_shutdown(int argc, char **argv);

int cmd_meminfo(int argc, char **argv);



// Estrutura de um comando
typedef struct {
    char *name;
    int (*func)(int argc, char **argv);
} Command;

// Tabela de comandos (você adiciona novos aqui)
Command commands[] = {
    {"help",  cmd_help},
    {"echo",  cmd_echo},
    {"ls",    cmd_ls},
    {"cd",    cmd_cd},
    {"mkdir", cmd_mkdir},
    {"reboot", cmd_reboot},
    {"shutdown", cmd_shutdown},
    {"meminfo", cmd_meminfo},
    // ... você adiciona novos conforme precisa
    {NULL, NULL}  // marcador de fim
};

int cmd_help(int argc, char **argv){}
int cmd_ls(int argc, char **argv){}
int cmd_cd(int argc, char **argv){}
int cmd_mkdir(int argc, char **argv){}
int cmd_shutdown(int argc, char **argv){}

int cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    return 0;
}

int cmd_reboot(int argc, char **argv) {
    printf("Reiniciando sistema...\n");
    // Chama a função de reboot do seu SO
    return 0;
}
int get_total_memory(){
    return 0;
}
int get_free_memory(){
    return 0;
}
int cmd_meminfo(int argc, char **argv) {
    printf("Memória total: %d KB\n", get_total_memory());
    printf("Memória livre: %d KB\n", get_free_memory());
    return 0;
}


// Função que executa UMA linha de comando
int execute_line(char *line) {
    char *args[16];
    int argc = 0;
    char *token;
    char line_copy[256];  // Cópia porque strtok modifica a string

    // Remove espaços iniciais
    while (*line == ' ') line++;

    // Remove \n se existir
    size_t len = strlen(line);
    if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';

    // Remove \r se existir (Windows)
    len = strlen(line);
    if (len > 0 && line[len-1] == '\r') line[len-1] = '\0';

    // Se linha vazia ou comentário, sai
    if (line[0] == '\0' || line[0] == '#') return 0;

    // Copia a linha para não modificar a original
    strcpy(line_copy, line);

    // Quebra em tokens (argumentos)
    token = strtok(line_copy, " ");
    while (token != NULL && argc < 16) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) return 0;  // linha vazia

    // Procura o comando na tabela
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(commands[i].name, args[0]) == 0) {
            // Executa o comando
            int result = commands[i].func(argc, args);
            return result;
        }
    }

    // Se chegou aqui, comando não encontrado
    printf("Comando desconhecido: %s\n", args[0]);
    return -1;
}

void run_boot_script(void) {
    FIL *f;

    fopen(f,"/boot.bat", FA_READ);
    if (!f) return;

    char line[256];
    while (f_gets(line, sizeof(line), f)) {
        // Remove \n
        line[strcmp(line, "\n")] = 0;
        if (line[0] == 0 || line[0] == '#') continue;

        // Executa a linha como se fosse digitada no terminal
        execute_line(line);
    }
    fclose(f);
}

#include <stdio.h>   // Para getchar() e putchar()
#include <string.h>

// ============================================
// gets_line - Lê uma linha do teclado (usando getchar)
// ============================================
char *gets_line(char *buffer, int size) {
    int i = 0;
    char c;

    while (i < size - 1) {
        c = getchar();  // Lê do ringbuffer do teclado

        if (c == '\r' || c == '\n') {  // Enter
            buffer[i] = '\0';
            putchar('\n');  // Echo do newline
            return buffer;
        }
        else if (c == '\b' || c == 0x7F) {  // Backspace
            if (i > 0) {
                i--;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        }
        else if (c >= 0x20 && c < 0x7F) {  // Caracteres imprimíveis
            buffer[i++] = c;
            putchar(c);  // Echo do caractere
        }
        // Ignora outros caracteres (como Ctrl, Shift, etc.)
    }
    buffer[i] = '\0';
    return buffer;
}

void shell_loop(void) {
    char line[256];

    while (1) {
        printf("> ");
        gets_line(line,sizeof(line));

        // Remove o \n
        line[strcmp(line, "\n")] = 0;

        // Se for vazio, continua
        if (line[0] == 0) continue;

        // Quebra em argumentos (argc/argv)
        char *args[16];
        int argc = 0;
        char *token = strtok(line, " ");
        while (token && argc < 16) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }

        // Busca e executa o comando
        int found = 0;
        for (int i = 0; commands[i].name != NULL; i++) {
            if (strcmp(commands[i].name, args[0]) == 0) {
                commands[i].func(argc, args);
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Comando desconhecido: %s\n", args[0]);
        }
    }
}


