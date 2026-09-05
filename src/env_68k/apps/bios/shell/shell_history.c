/*
 * shell_history.c
 *
 * Historico de comandos com navegacao via seta pra cima (0x10) e
 * seta pra baixo (0x11) -- byte unico, sem sequencia de escape, pelo
 * que voce reportou do seu terminal.
 *
 * INTEGRACAO: troque a leitura de caractere (getchar_serial) e escrita
 * (putchar_serial/puts_serial) pelas funcoes reais que seu shell ja
 * usa hoje pra ler/escrever no console.
 */
#include <string.h>
#include <stdint.h>

#define HIST_SIZE   16    /* quantos comandos guardar -- ajuste como quiser */
#define LINE_MAX    128   /* tamanho maximo de uma linha de comando */

#define KEY_UP     0x10
#define KEY_DOWN   0x11
#define KEY_BS     0x08   /* AJUSTE se seu backspace for outro codigo (ex: 0x7F) */
#define KEY_ENTER  '\r'   /* ou '\n', conforme seu terminal manda */

extern int  getchar(void);
extern void putchar(char c);
extern void puts(const char *s);

static char history[HIST_SIZE][LINE_MAX];
static int  hist_count = 0;     /* quantos comandos ja foram guardados no total */
static int  hist_cursor = -1;   /* -1 = "linha nova" (fora do historico) */

/*
 * Apaga visualmente 'len' caracteres da linha atual no terminal e
 * escreve 'text' no lugar. Como terminal nao tem "desfazer", isso e'
 * feito na base do backspace + espaco + backspace.
 */
static void redraw_line(int old_len, const char *text)
{
    for (int i = 0; i < old_len; i++) putchar('\b');
    for (int i = 0; i < old_len; i++) putchar(' ');
    for (int i = 0; i < old_len; i++) putchar('\b');
    puts(text);
}

/*
 * Grava um comando executado no historico (chame isso depois que o
 * usuario der Enter e o comando for de fato executado -- nao grave
 * linhas vazias, e evite gravar duplicata consecutiva se quiser um
 * comportamento mais "bash-like", isso e' opcional).
 */
void history_add(const char *cmd)
{
    if (cmd[0] == '\0')
        return; /* nao guarda linha vazia */

    strncpy(history[hist_count % HIST_SIZE], cmd, LINE_MAX - 1);
    history[hist_count % HIST_SIZE][LINE_MAX - 1] = '\0';
    hist_count++;
    hist_cursor = -1; /* toda vez que roda um comando novo, reseta a navegacao */
}

/*
 * Le uma linha de comando do console, com edicao basica (backspace)
 * e navegacao de historico (setas). Retorna quando Enter e' pressionado.
 *
 * 'buf' precisa ter pelo menos LINE_MAX bytes.
 */
void readline_with_history(char *buf)
{
    int len = 0;
    buf[0] = '\0';
    hist_cursor = -1;

    for (;;) {
        int c = getchar();

        if (c == KEY_ENTER) {
            putchar('\n');
            buf[len] = '\0';
            return;
        }

        if (c == KEY_BS) {
            if (len > 0) {
                len--;
                buf[len] = '\0';
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
            continue;
        }

        if (c == KEY_UP) {
            /* so' navega se existir comando mais antigo disponivel */
            if (hist_cursor + 1 < hist_count && hist_cursor + 1 < HIST_SIZE) {
                hist_cursor++;
                int idx = (hist_count - 1 - hist_cursor + HIST_SIZE) % HIST_SIZE;
                redraw_line(len, history[idx]);
                strcpy(buf, history[idx]);
                len = strlen(buf);
            }
            continue;
        }

        if (c == KEY_DOWN) {
            if (hist_cursor > 0) {
                hist_cursor--;
                int idx = (hist_count - 1 - hist_cursor + HIST_SIZE) % HIST_SIZE;
                redraw_line(len, history[idx]);
                strcpy(buf, history[idx]);
                len = strlen(buf);
            } else if (hist_cursor == 0) {
                /* volta pra linha vazia, saindo do historico */
                hist_cursor = -1;
                redraw_line(len, "");
                buf[0] = '\0';
                len = 0;
            }
            continue;
        }

        /* caractere normal -- so' aceita se couber no buffer */
        if (len < LINE_MAX - 1) {
            buf[len++] = (char) c;
            buf[len] = '\0';
            putchar((char) c);
        }
    }
}

/*
 * Exemplo de uso no loop principal do shell:
 *
 *   char cmdline[LINE_MAX];
 *   for (;;) {
 *       puts_serial("orion> ");
 *       readline_with_history(cmdline);
 *       history_add(cmdline);      // grava ANTES ou DEPOIS de executar,
 *                                  // tanto faz, contanto que seja sempre
 *       executa_comando(cmdline);
 *   }
 */