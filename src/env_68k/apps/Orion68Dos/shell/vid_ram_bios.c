#include <stdint.h>
#include <stdio.h>

#define VIDEO_BASE        0x00FF8000UL  /* TODO: confirmar valor real */
#define REG_DATA_OUT      0x0001        /* TODO: confirmar offset real */
#define VIDEO_CHAR_REG    ((volatile uint8_t *) (VIDEO_BASE + REG_DATA_OUT))

static void video_putc(char c)
{
    *VIDEO_CHAR_REG = (uint8_t) c;
}

void video_puts(const char *s)
{
    while (*s) {
        video_putc(*s++);
    }
}

/* ------------------------------------------------------------
 * FORMATACAO MANUAL DE NUMEROS (sem sprintf/snprintf/%lu)
 * ------------------------------------------------------------ */
static const char HEXDIGITS[] = "0123456789ABCDEF";

/* Escreve 8 digitos hex de v em out (+ '\0'). out precisa ter 9 bytes. */
static void fmt_hex32(uint32_t v, char *out)
{
    int8_t i;
    for (i = 7; i >= 0; i--) {
        out[i] = HEXDIGITS[v & 0xFu];
        v >>= 4;
    }
    out[8] = '\0';
}

/* Escreve v (0 a 65535) em decimal em out. out precisa ter 6 bytes. */
static void fmt_dec_u16(uint16_t v, char *out)
{
    char tmp[5];
    int8_t n = 0;
    int8_t i;

    if (v == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    while (v > 0 && n < 5) {
        tmp[n++] = (char) ('0' + (v % 10));
        v = (uint16_t) (v / 10);
    }

    for (i = 0; i < n; i++) {
        out[i] = tmp[n - 1 - i];
    }
    out[n] = '\0';
}

/* Redesenha a linha de progresso: "<label>: NN% (curr/total)" */
void video_show_progress(const char *label, uint16_t current, uint16_t total)
{
    char numbuf[6];
    uint16_t pct = (total > 0) ? (uint16_t) (((uint32_t) current * 100UL) / total) : 0;

    video_putc('\r');
    video_puts(label);
    video_puts(": ");
    fmt_dec_u16(pct, numbuf);
    video_puts(numbuf);
    video_puts("% (");
    fmt_dec_u16(current, numbuf);
    video_puts(numbuf);
    video_puts("/");
    fmt_dec_u16(total, numbuf);
    video_puts(numbuf);
    video_puts(")      ");
}

static void video_show_status(const char *msg)
{
    video_puts("\r\n");
    video_puts(msg);
    video_puts("\r\n");
}

/* Mostra "<prefixo> 0x<endereco em hex>" direto no video. */
static void video_show_hex_addr(const char *prefixo, uint32_t endereco)
{
    char hexbuf[9];
    fmt_hex32(endereco, hexbuf);
    video_puts("\r\n");
    video_puts(prefixo);
    video_puts(" 0x");
    video_puts(hexbuf);
    video_puts("\r\n");
}

//USO: video_show_progress(" Gravando", chunk_num, total_chunks);
