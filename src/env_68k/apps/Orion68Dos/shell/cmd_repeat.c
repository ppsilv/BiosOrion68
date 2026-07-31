#include "orion68.h"
#include "../kbd/ringbuffer.h"

#define HIST_SIZE      256   // buffer circular bruto (mesmo esquema uint8_t wrap-around)
#define HIST_MAX_CMDS   16   // quantos comandos ficam "indexados" pra navegação
#define LINE_MAX        64   // tamanho máximo de uma linha de comando

/* --- armazenamento bruto (o que você já tinha) --- */
static uint8_t hist_buf[HIST_SIZE];
static uint8_t hist_head = 0;         // próxima posição livre de escrita (wrap automático)

/* --- índice paralelo: onde cada comando começa e quanto mede --- */
static uint8_t hist_start[HIST_MAX_CMDS];
static uint8_t hist_len[HIST_MAX_CMDS];
static uint8_t hist_count  = 0;       // quantos comandos já indexados (satura em HIST_MAX_CMDS)
static uint8_t hist_newest = 0;       // slot do comando mais recente

/* --- estado da navegação --- */
static int8_t  hist_cursor = -1;      // -1 = editando linha ao vivo; 0 = mais recente; 1 = anterior...
static uint8_t saved_line[LINE_MAX];  // guarda o que o usuário tava digitando antes de apertar UP
static uint8_t saved_len  = 0;

/* --- linha sendo editada agora --- */
static uint8_t line_buf[LINE_MAX];
static uint8_t line_len = 0;

/* ================= chamado quando ENTER é aceito ================= */
void hist_add(const uint8_t *cmd, uint8_t len)
{
    uint8_t i, slot;

    if (len == 0) return;

    for (i = 0; i < len; i++)
        hist_buf[(uint8_t)(hist_head + i)] = cmd[i];      // wrap automático via uint8_t
    hist_buf[(uint8_t)(hist_head + len)] = 0x0D;           // delimitador, como pedido

    slot = hist_newest = (uint8_t)((hist_newest + 1) % HIST_MAX_CMDS);
    hist_start[slot] = hist_head;
    hist_len[slot]   = len;

    hist_head = (uint8_t)(hist_head + len + 1);             // pula o 0x0D também

    if (hist_count < HIST_MAX_CMDS)
        hist_count++;

    hist_cursor = -1;   // qualquer comando novo cancela navegação em andamento
}

/* copia o comando de "idade" age (0=mais novo) para out[]. Retorna 0 se não existir. */
static uint8_t hist_get(int8_t age, uint8_t *out, uint8_t *outlen)
{
    uint8_t slot, start, len, i;

    if (age < 0 || age >= hist_count)
        return 0;

    slot  = (uint8_t)((hist_newest - age + HIST_MAX_CMDS) % HIST_MAX_CMDS);
    start = hist_start[slot];
    len   = hist_len[slot];

    for (i = 0; i < len; i++)
        out[i] = hist_buf[(uint8_t)(start + i)];   // leitura wrap-aware

    *outlen = len;
    return 1;
}

/* ================= apaga visualmente a linha atual ================= */
static void erase_line(void)
{
    uint8_t i;
    for (i = 0; i < line_len; i++) {
        ring_buf_put(0x08);   /* backspace: move cursor pra esquerda    */
        ring_buf_put(' ');    /* sobrescreve o char com espaço em branco */
        ring_buf_put(0x08);   /* backspace de novo: reposiciona cursor  */
    }
    line_len = 0;
}

static void echo_line(const uint8_t *s, uint8_t len)
{
    uint8_t i;
    for (i = 0; i < len; i++)
        ring_buf_put(s[i]);
}

/* ================= tecla UP ================= */
static void fkey_up(void)
{
    uint8_t tmp[LINE_MAX];
    uint8_t tmplen, i;

    if (hist_cursor == -1) {
        saved_len = line_len;
        for (i = 0; i < line_len; i++)
            saved_line[i] = line_buf[i];
    }

    if (!hist_get(hist_cursor + 1, tmp, &tmplen))
        return;   /* já está no comando mais antigo: nada muda na tela */

    hist_cursor++;

    erase_line();               /* apaga exatamente line_len chars, nem 1 a mais */
    echo_line(tmp, tmplen);

    line_len = tmplen;
    for (i = 0; i < line_len; i++)
        line_buf[i] = tmp[i];
}

/* ================= tecla DOWN ================= */
static void fkey_down(void)
{
    uint8_t tmp[LINE_MAX];
    uint8_t tmplen, i;

    if (hist_cursor == -1)
        return;   /* já editando a linha ao vivo, nada abaixo */

    if (hist_cursor == 0) {
        /* volta pra linha que o usuário tava digitando antes do primeiro UP */
        erase_line();
        echo_line(saved_line, saved_len);
        for (i = 0; i < saved_len; i++)
            line_buf[i] = saved_line[i];
        line_len = saved_len;
        hist_cursor = -1;
        return;
    }

    hist_cursor--;
    hist_get(hist_cursor, tmp, &tmplen);   /* garantido existir, pois já veio de lá */

    erase_line();
    echo_line(tmp, tmplen);
    for (i = 0; i < tmplen; i++)
        line_buf[i] = tmp[i];
    line_len = tmplen;
}

uint8_t keyboard_handler(uint8_t scancode)
{
    switch (scancode) {
        case 11:                    /* seta pra cima */
            fkey_up();
            return 1;

        case 12:                    /* seta pra baixo */
            fkey_down();
            return 1;

        case 0x0D:                  /* ENTER */
            break;
            if (line_len > 0)
                hist_add(line_buf, line_len);
            /* comando vazio: cai direto aqui sem chamar hist_add, então
               não polui o histórico — mantém seu comportamento atual */
            ring_buf_put(0x0D);     /* avança linha, como já funciona hoje */
            line_len = 0;
            hist_cursor = -1;
            break;

        case 0x08:                  /* backspace normal, já existente */
            break;
            if (line_len > 0) {
                line_len--;
                ring_buf_put(0x08);
                ring_buf_put(' ');
                ring_buf_put(0x08);
            }
            break;
    }
    return 0;
}