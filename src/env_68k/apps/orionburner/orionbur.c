/* ============================================================
 * OrionBurn - Gravador de firmware para EEPROM AT28C256 (Low/High)
 * Projeto Orion68
 *
 * VERSAO CORRIGIDA com base no que foi validado no hardware real:
 *
 *   1) Interrupcoes ficam DESABILITADAS durante toda a gravacao.
 *   2) Escrita e feita BYTE A BYTE (8 bits), sequencial, avancando
 *      1 byte por vez a partir do endereco 0x0000, em blocos de
 *      128 bytes lidos do arquivo (64 bytes vao para o chip low,
 *      64 bytes vao para o chip high - os dois juntos formam uma
 *      "pagina" real de 64 bytes por chip).
 *   3) A partir do momento em que a gravacao comeca, NADA de
 *      printf/sprintf e usado para saida - tudo vai direto pro
 *      registrador de video, byte a byte, com formatacao manual.
 *   4) Nenhum uso de %lu ou de snprintf em lugar nenhum do
 *      programa (a lib local nao tem nenhum dos dois). Numeros
 *      grandes sao formatados manualmente (hex ou decimal) antes
 *      de imprimir, tanto antes quanto depois do inicio da
 *      gravacao.
 * ============================================================ */

#include <stdint.h>
#include <stdio.h>
#include "fileio.h"

/* ------------------------------------------------------------
 * CONFIGURACAO DE HARDWARE - EEPROM
 *
 * Espaco de enderecos LINEAR de 8 bits: o byte de endereco par
 * (0, 2, 4, ...) cai em um chip e o impar (1, 3, 5, ...) cai no
 * outro (decisao de hardware/glue logic, nao de software). Por
 * isso a relacao "endereco de chip" -> "endereco de sistema" e
 * sempre: endereco_sistema = endereco_de_chip * 2 (+0 ou +1
 * dependendo de qual dos dois chips).
 * ------------------------------------------------------------ */
#define FLASH_BASE       0x000000UL
#define FLASH            ((volatile uint8_t *) FLASH_BASE)

#define AT28_CHIP_BYTES     32768UL  /* AT28C256 = 32K x 8, por chip */
#define AT28_PAGE_BYTES     64       /* pagina real de CADA chip */
#define AT28_CHUNK_BYTES    128      /* 64 (low) + 64 (high) = 1 pagina dupla */
#define AT28_TOTAL_BYTES    (AT28_CHIP_BYTES * 2UL)  /* 64K de espaco linear */

#define AT28_WRITE_TIMEOUT  400000UL
#define MAX_PAGE_RETRIES    3

/* ------------------------------------------------------------
 * CONFIGURACAO DE HARDWARE - VIDEO
 * (mesma ideia de antes: escreve direto no registrador do
 * controlador de video, sem passar por TRAP/printf, porque a
 * partir da pagina 0 o sistema basico antigo esta sendo
 * sobrescrito e nao pode mais ser considerado confiavel)
 * ------------------------------------------------------------ */
#define VIDEO_BASE        0x00FF8000UL  /* TODO: confirmar valor real */
#define REG_DATA_OUT      0x0001        /* TODO: confirmar offset real */
#define VIDEO_CHAR_REG    ((volatile uint8_t *) (VIDEO_BASE + REG_DATA_OUT))

static void video_putc(char c)
{
    *VIDEO_CHAR_REG = (uint8_t) c;
}

static void video_puts(const char *s)
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
static void video_show_progress(const char *label, uint16_t current, uint16_t total)
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

/* ------------------------------------------------------------
 * CAMADA DE DISCO - usando fileio.h
 * ------------------------------------------------------------ */
typedef struct {
    FIL      fp;
    uint32_t size;
} FILE_HANDLE;

static int fs_open(const char *filename, FILE_HANDLE *fh)
{
    FRESULT res = fopen(&fh->fp, filename, FA_READ);
    if (res != FR_OK) {
        return 0;
    }
    fh->size = (uint32_t) fsize(&fh->fp);
    return 1;
}

static uint32_t fs_read(FILE_HANDLE *fh, uint8_t *buf, uint32_t len)
{
    UINT br = 0;
    FRESULT res = fread(&fh->fp, buf, (UINT) len, &br);
    if (res != FR_OK) {
        return 0;
    }
    return (uint32_t) br;
}

static void fs_close(FILE_HANDLE *fh)
{
    fclose(&fh->fp);
}

/* ------------------------------------------------------------
 * CONTROLE DE INTERRUPCOES
 * ------------------------------------------------------------ */
static uint16_t cpu_disable_interrupts(void)
{
    uint16_t old_sr;
    __asm__ volatile (
        "move.w %%sr, %0\n\t"
        "ori.w  #0x0700, %%sr"
        : "=d" (old_sr)
        :
        : "memory"
    );
    return old_sr;
}

static void cpu_restore_interrupts(uint16_t old_sr)
{
    __asm__ volatile (
        "move.w %0, %%sr"
        :
        : "d" (old_sr)
        : "memory"
    );
}

static void cpu_soft_reset(void)
{
    __asm__ volatile (
        "move.l 0x0.w, %%sp\n\t"
        "move.l 0x4.w, %%a0\n\t"
        "jmp (%%a0)"
        :
        :
        : "memory"
    );
}

/* ------------------------------------------------------------
 * DESBLOQUEIO SDP (Software Data Protection) - AT28C256
 *
 * Endereco de chip 0x5555 -> endereco de sistema 0x5555*2 (chip A)
 * e 0x5555*2+1 (chip B); mesma logica para 0x2AAA. Fazemos os dois
 * byte-writes (um por chip) em vez de um unico word-write.
 * ------------------------------------------------------------ */
static void at28_unlock_byte(uint16_t chip_addr, uint8_t value)
{
    uint32_t sys_addr = (uint32_t) chip_addr * 2UL;
    FLASH[sys_addr]     = value;
    FLASH[sys_addr + 1] = value;
}

static void at28_disable_sdp_page(void)
{
    at28_unlock_byte(0x5555, 0xAA);
    at28_unlock_byte(0x2AAA, 0x55);
    at28_unlock_byte(0x5555, 0xA0);
}

/* ------------------------------------------------------------
 * GRAVACAO BYTE A BYTE, EM BLOCOS DE 128 (64 + 64)
 * ------------------------------------------------------------ */
static uint32_t g_last_fail_addr = 0;

static int flash_write_chunk(uint32_t sys_addr, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint32_t timeout;
    uint8_t last_byte;

    if (len == 0) {
        return 1;
    }

    /* desbloqueia SDP antes de cada bloco - funciona tanto se a
     * protecao ja estava ligada quanto se nao estava */
    at28_disable_sdp_page();

    for (i = 0; i < len; i++) {
        FLASH[sys_addr + i] = data[i];
    }

    last_byte = data[len - 1];
    timeout = AT28_WRITE_TIMEOUT;

    while (FLASH[sys_addr + len - 1] != last_byte) {
        if (--timeout == 0) {
            g_last_fail_addr = sys_addr + len - 1;
            return 0;
        }
    }

    return 1;
}

static int flash_write_buffer_progress(const uint8_t *data, uint32_t len_bytes)
{
    uint32_t offset = 0;
    uint16_t total_chunks = (uint16_t) ((len_bytes + AT28_CHUNK_BYTES - 1) / AT28_CHUNK_BYTES);
    uint16_t chunk_num = 0;

    while (offset < len_bytes) {
        uint16_t chunk = AT28_CHUNK_BYTES;
        uint16_t attempt;
        int ok = 0;

        if (chunk > (len_bytes - offset)) {
            chunk = (uint16_t) (len_bytes - offset);
        }

        for (attempt = 1; attempt <= MAX_PAGE_RETRIES; attempt++) {
            if (flash_write_chunk(offset, &data[offset], chunk)) {
                ok = 1;
                break;
            }
        }

        if (!ok) {
            return 0;
        }

        offset += chunk;
        chunk_num++;
        video_show_progress(" Gravando", chunk_num, total_chunks);
    }

    return 1;
}

static uint32_t flash_verify_buffer_progress(const uint8_t *data, uint32_t len_bytes)
{
    uint32_t offset = 0;
    uint16_t total_chunks = (uint16_t) ((len_bytes + AT28_CHUNK_BYTES - 1) / AT28_CHUNK_BYTES);
    uint16_t chunk_num = 0;
    uint32_t errors = 0;
    uint32_t i;

    while (offset < len_bytes) {
        uint16_t chunk = AT28_CHUNK_BYTES;

        if (chunk > (len_bytes - offset)) {
            chunk = (uint16_t) (len_bytes - offset);
        }

        for (i = 0; i < chunk; i++) {
            if (FLASH[offset + i] != data[offset + i]) {
                errors++;
            }
        }

        offset += chunk;
        chunk_num++;
        video_show_progress("Verificando", chunk_num, total_chunks);
    }

    return errors;
}

/* ------------------------------------------------------------
 * PROGRAMA PRINCIPAL
 * ------------------------------------------------------------ */
#define MAX_FW_SIZE (AT28_CHIP_BYTES * 2UL)  /* low + high juntos, em bytes */

static uint8_t raw_buf[MAX_FW_SIZE];

int main(int argc, char *argv[])
{
    FILE_HANDLE fh;
    uint32_t total_read;
    uint32_t errors;
    uint16_t old_sr;
    char decbuf[6];

    /* Ate aqui, printf normal - sistema basico ainda intacto.
     * Sem %lu em lugar nenhum: convertemos numeros grandes com
     * fmt_dec_u16/fmt_hex32 e imprimimos como string (%s). */
    if (argc < 2) {
        printf("Uso: orionburn <arquivo_firmware>\r\n");
        return 1;
    }

    printf("OrionBurn - gravador de firmware AT28C256 (Orion68)\r\n");
    printf("Arquivo: %s\r\n", argv[1]);

    if (!fs_open(argv[1], &fh)) {
        printf("ERRO: nao foi possivel abrir o arquivo.\r\n");
        return 1;
    }

    if (fh.size == 0 || fh.size > MAX_FW_SIZE) {
        printf("ERRO: tamanho de firmware invalido.\r\n");
        fs_close(&fh);
        return 1;
    }

    total_read = fs_read(&fh, raw_buf, fh.size);
    fs_close(&fh);

    if (total_read != fh.size) {
        printf("ERRO: leitura incompleta do arquivo.\r\n");
        return 1;
    }

    printf("Lidos %u bytes do arquivo.\r\n", (unsigned int) total_read);
    printf("Iniciando gravacao. Interrupcoes serao desabilitadas agora.\r\n");
    printf("A partir daqui, todo o progresso e mostrado direto na tela.\r\n");

    /* A PARTIR DAQUI: nada mais de printf/sprintf/snprintf. Toda
     * saida vai direto pra memoria de video, e as interrupcoes
     * ficam desligadas ate o fim da gravacao (sucesso ou erro). */
    old_sr = cpu_disable_interrupts();

    if (!flash_write_buffer_progress(raw_buf, total_read)) {
        cpu_restore_interrupts(old_sr);
        video_show_hex_addr("ERRO: timeout no endereco", g_last_fail_addr);
        video_puts("NAO REINICIE.\r\n");
        return 1;
    }

    errors = flash_verify_buffer_progress(raw_buf, total_read);

    if (errors != 0) {
        cpu_restore_interrupts(old_sr);
        fmt_dec_u16((uint16_t) errors, decbuf);
        video_puts("\r\nFALHA NA VERIFICACAO: ");
        video_puts(decbuf);
        video_puts(" byte(s) divergente(s). NAO REINICIE.\r\n");
        return 1;
    }

    video_show_status("SUCESSO - firmware gravado e verificado. Reiniciando...");

    cpu_soft_reset();

    /* nunca chega aqui */
    return 0;
}
