#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define W5100_BASE   0xFE0000UL

/* Acesso aos registradores em endereços ímpares: (offset * 2) + 1 */
#define W5100_REG(offset) ((volatile uint8_t *)(W5100_BASE + ((offset) * 2) + 1))

/* Tabela de registradores comuns */
#define W5100_MR    0x0000 /* Mode Register */
#define W5100_GAR   0x0001 /* Gateway Address (4 bytes) */
#define W5100_SUBR  0x0005 /* Subnet Mask (4 bytes) */
#define W5100_SHAR  0x0009 /* MAC Address (6 bytes) */
#define W5100_SIPR  0x000F /* Source IP Address (4 bytes) */
#define W5100_RMSR  0x001A /* RX Memory Size Register */
#define W5100_TMSR  0x001B /* TX Memory Size Register */

/* Registradores do Socket 0 */
#define S0_MR            0x0400
#define S0_CR            0x0401
#define S0_SR            0x0403
#define S0_PORT          0x0404
#define S0_DIPR          0x040C /* IP Destino */
#define S0_DPORT         0x0410 /* Porta Destino */
#define S0_TX_WR         0x0424
#define S0_RX_RSR        0x0426
#define S0_RX_RD         0x0428

#define CR_OPEN          0x01
#define CR_LISTEN        0x02
#define CR_DISCON        0x08
#define CR_SEND          0x20
#define CR_RECV          0x40

#define SOCK_CLOSED      0x00
#define SOCK_INIT        0x13
#define SOCK_LISTEN      0x14
#define SOCK_ESTABLISHED 0x17
#define SOCK_CLOSE_WAIT  0x1C

#define S0_TX_BASE       0x4000
#define S0_RX_BASE       0x6000
#define S0_MASK          0x07FF

/* DHCP message types */
#define DHCP_DISCOVER    1
#define DHCP_OFFER       2
#define DHCP_REQUEST     3
#define DHCP_ACK         5
#define DHCP_NAK         6

/* Offsets do cabeçalho BOOTP/DHCP, conforme RFC 2131.
   Usar offsets numéricos explícitos, em vez de uma struct C,
   garante que o pacote fique EXATAMENTE nesse formato não
   importa como o compilador do seu toolchain trate alinhamento
   ou padding de struct — o que causou o problema anterior. */
#define OFF_OP         0    /* 1 byte  - 1 = BOOTREQUEST                */
#define OFF_HTYPE      1    /* 1 byte  - 1 = Ethernet                   */
#define OFF_HLEN       2    /* 1 byte  - 6 = tamanho do MAC             */
#define OFF_HOPS       3    /* 1 byte  - 0                              */
#define OFF_XID        4    /* 4 bytes - Transaction ID                 */
#define OFF_SECS       8    /* 2 bytes - 0                              */
#define OFF_FLAGS      10   /* 2 bytes - 0x8000 = broadcast             */
#define OFF_CIADDR     12   /* 4 bytes - 0.0.0.0                        */
#define OFF_YIADDR     16   /* 4 bytes - IP oferecido pelo servidor     */
#define OFF_SIADDR     20   /* 4 bytes - IP do servidor DHCP            */
#define OFF_GIADDR     24   /* 4 bytes - 0.0.0.0 (sem relay)            */
#define OFF_CHADDR     28   /* 16 bytes - MAC (6 usados + 10 padding)   */
#define OFF_SNAME      44   /* 64 bytes - 0                             */
#define OFF_FILE       108  /* 128 bytes - 0                            */
#define OFF_MAGIC      236  /* 4 bytes - 0x63825363                     */
#define OFF_OPTIONS    240  /* opções DHCP começam aqui                 */

#define DHCP_PKT_MIN_SIZE 300  /* BOOTP mínimo exigido por muitos servidores */
#define DHCP_PKT_BUF_SIZE 320  /* espaço extra para as opções             */

static uint8_t pkt[DHCP_PKT_BUF_SIZE];

/* ---------------------------------------------------------- */
/* Funções básicas de acesso ao W5100                          */
/* ---------------------------------------------------------- */
static inline uint32_t get_tick_ms(void);

//static inline void w5100_cmd(uint8_t cmd) {
//    *W5100_REG(S0_CR) = cmd;
//    while (*W5100_REG(S0_CR));
//}

static inline void w5100_write16(uint16_t reg, uint16_t val) {
    *W5100_REG(reg)     = (val >> 8) & 0xFF;
    *W5100_REG(reg + 1) = val & 0xFF;
}

static inline uint16_t w5100_read16(uint16_t reg) {
    return (*W5100_REG(reg) << 8) | (*W5100_REG(reg + 1));
}

void w5100_write(uint16_t reg, uint8_t val) {
    *W5100_REG(reg) = val;
}

uint8_t w5100_read(uint16_t reg) {
    return *W5100_REG(reg);
}

/* Grava o MAC do próprio chip no registrador SHAR. */
void w5100_set_mac(const uint8_t *mac) {
    for (int i = 0; i < 6; i++) {
        w5100_write(W5100_SHAR + i, mac[i]);
    }
}
static inline int w5100_cmd(uint8_t cmd) {
    uint32_t start = get_tick_ms();
    *W5100_REG(S0_CR) = cmd;
    while (*W5100_REG(S0_CR)) {
        if ((get_tick_ms() - start) > 500) {
            printf("w5100_cmd(0x%02x) travou! SR=%02x\n", cmd, w5100_read(S0_SR));
            return 0;
        }
    }
    return 1;
}

static void w5100_recv_bytes(uint8_t *buf, uint16_t len) {
    uint16_t ptr = w5100_read16(S0_RX_RD);
    for (uint16_t i = 0; i < len; i++) {
        uint16_t addr = S0_RX_BASE + ((ptr + i) & S0_MASK);
        buf[i] = *W5100_REG(addr);
    }
    w5100_write16(S0_RX_RD, ptr + len);
    w5100_cmd(CR_RECV);
}

/* Le um datagrama UDP inteiro respeitando o tamanho REAL informado
   pelo proprio W5100 no cabecalho de 8 bytes que precede cada
   pacote em modo UDP (4 bytes IP origem + 2 bytes porta origem +
   2 bytes tamanho dos dados, big-endian).

   Isso e' essencial: ler sempre um tamanho fixo (como o codigo
   anterior fazia) faz o RX_RD avancar mais do que o pacote real,
   descolando do RX_WR do hardware. Na proxima espera, o calculo
   de bytes disponiveis (RX_WR - RX_RD) estoura e parece que ja
   ha' dados prontos, quando na verdade sao bytes nao escritos —
   e' exatamente isso que causava o "tipo=0" no ACK.

   Retorna o tamanho real dos dados (pode ser maior que buf_cap,
   caso em que os bytes excedentes sao descartados mas o ponteiro
   e' avancado corretamente mesmo assim). */
static uint16_t w5100_recv_udp_packet(uint8_t *buf, uint16_t buf_cap) {
    uint8_t udp_header[8];
    w5100_recv_bytes(udp_header, 8);

    uint16_t data_len = ((uint16_t)udp_header[6] << 8) | udp_header[7];
    uint16_t to_copy = (data_len < buf_cap) ? data_len : buf_cap;

    if (to_copy > 0) {
        w5100_recv_bytes(buf, to_copy);
    }

    /* Se o pacote real for maior que nosso buffer, descarta o
       restante em pedacos pequenos so' para manter o RX_RD em
       sincronia (nao deveria acontecer com pacotes DHCP normais,
       mas fica a salvaguarda). */
    uint16_t remaining = data_len - to_copy;
    uint8_t scratch[32];
    while (remaining > 0) {
        uint16_t chunk = (remaining < sizeof(scratch)) ? remaining : sizeof(scratch);
        w5100_recv_bytes(scratch, chunk);
        remaining -= chunk;
    }

    return data_len;
}

static void w5100_send_bytes121(const uint8_t *buf, uint16_t len) {
    /* Sincroniza explicitamente TX_WR com TX_RD antes de escrever.
       Isso evita que um valor residual de uma sessão anterior do
       socket desalinhe o início da transmissão. */
    uint16_t tx_rd = w5100_read16(0x0422); /* S0_TX_RD */
    w5100_write16(S0_TX_WR, tx_rd);

    uint16_t ptr = tx_rd;
    for (uint16_t i = 0; i < len; i++) {
        uint16_t addr = S0_TX_BASE + ((ptr + i) & S0_MASK);
        *W5100_REG(addr) = buf[i];
    }
    w5100_write16(S0_TX_WR, ptr + len);
    
   // printf("Antes do SEND: SR=%02x TX_RD=%04x TX_WR=%04x\n",w5100_read(S0_SR), tx_rd, w5100_read16(S0_TX_WR));
    if (!w5100_cmd(CR_SEND)) {
        printf("SEND nao confirmado! SR=%02x\n", w5100_read(S0_SR));
    }
    
    w5100_cmd(CR_SEND);
}

#define S0_TX_RD         0x0422 /* Corrigido para o endereço real do S0_TX_RD */
#define S0_TX_WR         0x0424 /* S0_TX_WR */

static void w5100_send_bytes(const uint8_t *buf, uint16_t len) {
    /* Lê o ponteiro de leitura atual do socket 0 (endereço 0x0422) */
    uint16_t tx_rd = w5100_read16(0x0422); 
    
    /* O W5100 gerencia o envio baseado no TX_WR. Para enviar um novo bloco,
       pegamos o TX_WR atual ou sincronizamos com o TX_RD se o buffer estiver vazio. 
       A forma mais segura no W5100 é ler o TX_WR atual, escrever os dados a partir 
       dele e avançar o TX_WR somando o tamanho (len). */
    uint16_t ptr = w5100_read16(S0_TX_WR);
    
    for (uint16_t i = 0; i < len; i++) {
        uint16_t addr = S0_TX_BASE + ((ptr + i) & S0_MASK);
        *W5100_REG(addr) = buf[i];
    }
    
    /* Avança o ponteiro de escrita do hardware */
    w5100_write16(S0_TX_WR, ptr + len);
    
    if (!w5100_cmd(CR_SEND)) {
        printf("SEND nao confirmado! SR=%02x\n", w5100_read(S0_SR));
    }
}


/* Leitura dupla obrigatória do tamanho de RX (requisito do W5100) */
static uint16_t w5100_get_rx_size(void) {
    uint16_t val1 = 0, val2 = 0;
    do {
        val1 = w5100_read16(S0_RX_RSR);
        if (val1 != 0) {
            val2 = w5100_read16(S0_RX_RSR);
        } else {
            val2 = 0;
        }
    } while (val1 != val2);
    return val1;
}

void w5100_print_ip(void) {
    uint8_t ip[4];
    for (int i = 0; i < 4; i++) {
        ip[i] = w5100_read(W5100_SIPR + i);
    }
    printf("IP obtido: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
}

/* ---------------------------------------------------------- */
/* Espera com base em tempo real (tick), não em contagem de     */
/* iteracoes de loop, que e' pouco confiavel entre compiladores */
/* e velocidades de CPU diferentes.                              */
/*                                                               */
/* O systick do orion68k fica mapeado diretamente na RAM no      */
/* endereco 0x80448, incrementando a cada 10ms.                  */
/* ---------------------------------------------------------- */
#define SYSTICK_ADDR 0x00080064UL     //0x80448UL

static inline uint32_t read_systick(void) {
    return *(volatile uint32_t *)SYSTICK_ADDR;
}

static inline uint32_t get_tick_ms(void) {
    return read_systick() * 10UL;
}

static int wait_rx_at_least(uint16_t min_bytes, uint32_t timeout_ms) {
    uint32_t start = get_tick_ms();
    while (w5100_get_rx_size() < min_bytes) {
        if ((get_tick_ms() - start) >= timeout_ms) {
            return 0; /* timeout */
        }
    }
    return 1; /* dados disponiveis */
}

/* ---------------------------------------------------------- */
/* Montagem e envio de pacotes DHCP (buffer plano, offsets      */
/* explicitos - NAO depende de struct/packing do compilador)    */
/* ---------------------------------------------------------- */

static uint32_t next_xid(void) {
    static uint32_t counter = 0;
    counter++;
    return 0x39000000UL | (counter & 0x00FFFFFFUL);
}

static void put_u32_be(uint8_t *p, uint32_t v) {
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >> 8) & 0xFF;
    p[3] = v & 0xFF;
}

/* xid agora e' passado pelo chamador: DISCOVER e REQUEST de uma
   MESMA transacao devem usar o mesmo xid (RFC 2131), gerar um
   novo a cada chamada (como antes) quebra essa correlacao. */
static void enviar_pacote_dhcp(uint8_t msg_type, uint32_t xid,
                                const uint8_t *mac,
                                const char *hostname,
                                const uint8_t *req_ip,
                                const uint8_t *server_ip) {
    memset(pkt, 0, sizeof(pkt));

    pkt[OFF_OP]    = 1;  /* BOOTREQUEST */
    pkt[OFF_HTYPE] = 1;  /* Ethernet */
    pkt[OFF_HLEN]  = 6;
    pkt[OFF_HOPS]  = 0;
    put_u32_be(&pkt[OFF_XID], xid);
    pkt[OFF_SECS]     = 0x00;
    pkt[OFF_SECS + 1] = 0x00;
    pkt[OFF_FLAGS]     = 0x80; /* flags = 0x8000 (broadcast) */
    pkt[OFF_FLAGS + 1] = 0x00;
    /* ciaddr, yiaddr, siaddr, giaddr já ficam em 0 pelo memset */
    memcpy(&pkt[OFF_CHADDR], mac, 6);
    /* restante do chaddr (10 bytes) e sname/file já em 0 */
    put_u32_be(&pkt[OFF_MAGIC], 0x63825363UL);

    uint8_t *opt = &pkt[OFF_OPTIONS];

    /* Opção 53: DHCP Message Type */
    *opt++ = 53; *opt++ = 1; *opt++ = msg_type;

    /* No REQUEST: opção 50 (IP solicitado) e opção 54 (servidor) */
    if (msg_type == DHCP_REQUEST && req_ip && server_ip) {
        *opt++ = 50; *opt++ = 4;
        memcpy(opt, req_ip, 4);
        opt += 4;

        *opt++ = 54; *opt++ = 4;
        memcpy(opt, server_ip, 4);
        opt += 4;
    }

    /* Opção 12: Hostname */
    uint8_t len = (uint8_t)strlen(hostname);
    *opt++ = 12; *opt++ = len;
    memcpy(opt, hostname, len);
    opt += len;

    /* Opção 55: Parameter Request List */
    *opt++ = 55; *opt++ = 3;
    *opt++ = 1; *opt++ = 3; *opt++ = 6;

    /* Fim das opções */
    *opt++ = 0xFF;

    /* Garante o tamanho mínimo de 300 bytes que muitos servidores
       DHCP/BOOTP exigem (o resto já está zerado pelo memset) */
    uint16_t total_len = (uint16_t)(opt - pkt);
    if (total_len < DHCP_PKT_MIN_SIZE) {
        total_len = DHCP_PKT_MIN_SIZE;
    }

    w5100_send_bytes(pkt, total_len);
}

static uint8_t parse_dhcp_options(const uint8_t *raw, uint16_t raw_len,
                                   uint8_t *out_server_ip) {
    uint8_t msg_type = 0;
    uint16_t i = OFF_OPTIONS;

    while (i < raw_len && raw[i] != 0xFF) {
        uint8_t code = raw[i];
        if (code == 0) { i++; continue; } /* padding */
        uint8_t opt_len = raw[i + 1];

        if (code == 53 && opt_len == 1) {
            msg_type = raw[i + 2];
        } else if (code == 54 && opt_len == 4 && out_server_ip) {
            memcpy(out_server_ip, &raw[i + 2], 4);
        }
        i += 2 + opt_len;
    }
    return msg_type;
}

#define W5100_PHYSTATUS  0x003C  /* bit 0: 1 = link up, 0 = link down */

static inline int w5100_link_up(void) {
    return w5100_read(W5100_PHYSTATUS) & 0x01;
}

/* ---------------------------------------------------------- */
/* Handshake DHCP completo: DISCOVER -> OFFER -> REQUEST -> ACK */
/* ---------------------------------------------------------- */

/* Tempo maximo de espera por OFFER/ACK. O dnsmasq (e a maioria
   dos servidores DHCP) faz um teste de ping (ICMP) no endereco
   candidato antes de oferece-lo, o que pode levar ~1-2s. Por
   isso usamos uma margem generosa aqui. */
#define DHCP_WAIT_TIMEOUT_MS 30000UL

int w5100_dhcp_request(const char *hostname) {
//    uint8_t mac[6] = {0x02, 0x00, 0x00, 0x01, 0x02, 0x03};
    uint8_t mac[6] = {0x00, 0x23, 0x69, 0x44, 0x52, 0x5C};
    uint8_t ip[4]      = {192, 168, 1, 48};
  
    uint8_t rx_buf[DHCP_PKT_BUF_SIZE];
    uint8_t offered_ip[4]  = {0};
    uint8_t server_ip[4]   = {0};
    uint8_t msg_type =0;
    
    /* ---------------- PREPARING FASE 1: DISCOVER ---------------- */
    w5100_set_mac(mac);
    for (int i = 0; i < 4; i++) {
        w5100_write(W5100_SIPR + i, ip[i]);
    }

    *W5100_REG(S0_MR) = 0x02; /* modo UDP */
    w5100_write16(S0_PORT, 68);
    w5100_cmd(CR_OPEN);

    for (int i = 0; i < 4; i++) 
        w5100_write(S0_DIPR + i, 255);
    w5100_write16(S0_DPORT, 67);
    /* Um unico xid para toda a transacao (DISCOVER + REQUEST) */
    uint32_t xid = next_xid();

    /* ---------------- FASE 1: DISCOVER ---------------- */
    printf("Enviando DHCP DISCOVER...xid[%ld]\n",xid);
    enviar_pacote_dhcp(DHCP_DISCOVER, xid, mac, hostname, NULL, NULL);

    if (!wait_rx_at_least(OFF_OPTIONS, DHCP_WAIT_TIMEOUT_MS)) {
        printf("Timeout esperando OFFER\n");
        w5100_cmd(CR_DISCON);
        return 0;
    }

    w5100_recv_udp_packet(rx_buf, sizeof(rx_buf));

    memcpy(offered_ip, &rx_buf[OFF_YIADDR], 4);

    msg_type = parse_dhcp_options(rx_buf, sizeof(rx_buf), server_ip);
    if (msg_type != DHCP_OFFER) {
        printf("Pacote recebido nao e um OFFER (tipo=%d)\n", msg_type);
        w5100_cmd(CR_DISCON);
        return 0;
    }

    if (server_ip[0] == 0 && server_ip[1] == 0 &&
        server_ip[2] == 0 && server_ip[3] == 0) {
        memcpy(server_ip, &rx_buf[OFF_SIADDR], 4);
    }

    printf("OFFER recebido: IP oferecido %d.%d.%d.%d, servidor %d.%d.%d.%d\n",
           offered_ip[0], offered_ip[1], offered_ip[2], offered_ip[3],
           server_ip[0], server_ip[1], server_ip[2], server_ip[3]);

    /* ---------------- PREPARING FASE 2: REQUEST ---------------- */
    w5100_set_mac(mac);
    for (int i = 0; i < 4; i++) {
        w5100_write(W5100_SIPR + i, ip[i]);
    }

    *W5100_REG(S0_MR) = 0x02; /* modo UDP */
    w5100_write16(S0_PORT, 68);
    w5100_cmd(CR_OPEN);

    for (int i = 0; i < 4; i++) 
        w5100_write(S0_DIPR + i, 255);
    w5100_write16(S0_DPORT, 67);
    /* ---------------- FASE 2: REQUEST ---------------- */
    printf("Enviando DHCP REQUEST...xid[%ld] hostname[%s]\n",xid,hostname);
    enviar_pacote_dhcp(DHCP_REQUEST, xid, mac, hostname, offered_ip, server_ip);


    if (!wait_rx_at_least(OFF_OPTIONS, 50000UL )) {
        printf("Timeout esperando ACK\n");
        w5100_cmd(CR_DISCON);
        return 0;
    }

    w5100_recv_udp_packet(rx_buf, sizeof(rx_buf));

    uint8_t server_ip2[4] = {0};
    msg_type = parse_dhcp_options(rx_buf, sizeof(rx_buf), server_ip2);

    if (msg_type == DHCP_NAK) {
        printf("Servidor recusou (NAK). Tente novamente.\n");
        w5100_cmd(CR_DISCON);
        return 0;
    }
    if (msg_type != DHCP_ACK) {
        printf("Pacote recebido nao e um ACK (tipo=%d)\n", msg_type);
        w5100_cmd(CR_DISCON);
        return 0;
    }

    for (int i = 0; i < 4; i++) {
        w5100_write(W5100_SIPR + i, rx_buf[OFF_YIADDR + i]);
    }

    printf("ACK recebido, IP confirmado!\n");

    w5100_cmd(CR_DISCON);
    return 1;
}

int main(void) {
    printf("Calling DHCP\n");
    
    if (w5100_dhcp_request("orion68k")) {
        w5100_print_ip();
    } else {
        printf("No dhcp offer...\n");
    }
    return 0;
}
