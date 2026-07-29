#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "lwip/tcp.h"
#include "orion_bus.pio.h" // Cabeçalho gerado automaticamente pelo pioasm
//#include "orion_bus1.pio.h" 
#include "ringbuffer.h"
#include "ps2_keyboard.h"


#define WIFI_SSID "OpenSoftware4"
#define WIFI_PASSWORD "santos@info09"
#define PORT 4242

// 1. ESTRUTURA PARA CONTROLE DA MÁQUINA DE ESTADOS
typedef enum {
    ESTADO_ESPERA_CABECALHO,
    ESTADO_ESPERA_DADOS,
    ESTADO_CONCLUIDO
} estado_receptor_t;

typedef struct {
    estado_receptor_t estado;
    uint32_t tamanho_total;
    uint32_t bytes_recebidos;
    char nome_arquivo[13]; // Formato 8.3 + \0
    uint8_t *buffer_ram;
    uint32_t crc32;
} conexao_estado_t;

// Variáveis globais de controle do arquivo vindo do Wi-Fi
uint8_t *arquivo_buffer = NULL;
uint8_t arquivo_tamh=0;
uint8_t arquivo_tamL=0;
uint8_t arquivo_ok=0;
uint32_t arquivo_tamanho = 0;
uint32_t ponteiro_leitura = 0;
uint32_t arquivo_crc32 = 0;
bool arquivo_pronto = false;

// Cabeçalho fixo de 16 bytes: 4 bytes (tamanho) + 12 bytes (nome)
#define TAMANHO_CABECALHO 16

extern void gerenciar_barramento_m68k(PIO pio, uint sm);
extern void gerenciar_barramento1_m68k(PIO pio, uint sm);

//crc32 **********************************************************
// Tabela de 1 KB gerada em tempo de execução
static uint32_t crc32_table[256];

// Polinômio padrão IEEE 802.3 (Refletido)
#define CRC32_POLYNOMIAL 0xEDB88320UL

/**
 * Inicializa a tabela de busca do CRC-32.
 * Chame esta função UMA VEZ no início do programa (main).
 */
void crc32_init(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++) {
            c = (c & 1) ? (CRC32_POLYNOMIAL ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
}

/**
 * Atualiza o CRC a cada bloco ou byte recebido.
 * Pode ser chamado iterativamente se você receber o arquivo em partes.
 */
uint32_t crc32_update(uint32_t crc, const uint8_t *buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        crc = crc32_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

/**
 * Função para calcular o CRC-32 final de um buffer completo.
 */
uint32_t crc32_calculate(const uint8_t *buffer, size_t length) {
    // Inicia com 0xFFFFFFFF e faz o XOR final com 0xFFFFFFFF
    return crc32_update(0xFFFFFFFFUL, buffer, length) ^ 0xFFFFFFFFUL;
}

// Callback de recepção modificado com a Máquina de Estados
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    conexao_estado_t *es = (conexao_estado_t *)arg;

    if (!p) {
        printf("Cliente desconectou.\n");
        if (es) {
            if (es->buffer_ram) free(es->buffer_ram);
            free(es);
        }
        tcp_close(tpcb);
        return ERR_OK;
    }

    tcp_recved(tpcb, p->tot_len);

    // Variáveis para navegar pelos dados atuais do pbuf
    uint8_t *dados_pacote = (uint8_t *)p->payload;
    uint32_t tam_pacote = p->len;
    uint32_t indice_dados = 0;

    // --- MÁQUINA DE ESTADOS ---
    if (es->estado == ESTADO_ESPERA_CABECALHO) {
        // Garante que recebemos pelo menos o tamanho do cabeçalho
        if (tam_pacote >= TAMANHO_CABECALHO) {
            // Extrai o tamanho total (primeiros 4 bytes)
            memcpy(&es->tamanho_total, dados_pacote, 4);
            
            // Extrai o nome do arquivo (próximos 12 bytes)
            memcpy(es->nome_arquivo, dados_pacote + 4, 12);
            es->nome_arquivo[12] = '\0'; // Garante finalizador de string

            //printf("Novo arquivo detectado: %s (%d bytes)\n", es->nome_arquivo, es->tamanho_total);

            // Aloca memória na RAM do Pico para o arquivo completo
            es->buffer_ram = (uint8_t *)malloc(es->tamanho_total);
            if (!es->buffer_ram) {
                printf("Erro: Falta de memória RAM no Pico!\n");
                tcp_close(tpcb);
                return ERR_MEM;
            }

            es->bytes_recebidos = 0;
            es->estado = ESTADO_ESPERA_DADOS;

            // Se o pacote trouxe mais dados além do cabeçalho, avança o ponteiro
            indice_dados = TAMANHO_CABECALHO;
        }
    }

    if (es->estado == ESTADO_ESPERA_DADOS) {
        uint32_t bytes_para_copiar = tam_pacote - indice_dados;

        if (bytes_para_copiar > 0) {
            // Copia os bytes da rede direto para a nossa RAM alocada
            memcpy(es->buffer_ram + es->bytes_recebidos, dados_pacote + indice_dados, bytes_para_copiar);
            es->bytes_recebidos += bytes_para_copiar;
            
            printf("Progresso no Pico: %d/%d bytes\n", es->bytes_recebidos, es->tamanho_total);
        }

        // VERIFICAÇÃO DE FINAL DE ARQUIVO
        if (es->bytes_recebidos >= es->tamanho_total) {
            es->estado = ESTADO_CONCLUIDO;
            
            printf("--- ARQUIVO RECEBIDO COM SUCESSO NO PICO! ---\n");
            printf("Arquivo: %s pronto na RAM.\n", es->nome_arquivo);

            // -------------------------------------------------------------
            // AQUI O PICO SABE QUE TERMINOU!
            // Execute sua ação útil aqui (ex: enviar_para_orion68(es->buffer_ram, es->tamanho_total))
            // -------------------------------------------------------------

            // Envia uma confirmação de sucesso de volta para o Linux
            char msg_sucesso[64];// = es->buffer_ram; //"Arq lido\n";
            sprintf(msg_sucesso,"Recebido %s de tamanho %d\0",es->nome_arquivo,es->tamanho_total);
            tcp_write(tpcb, msg_sucesso, sizeof(msg_sucesso), TCP_WRITE_FLAG_COPY);
            tcp_output(tpcb);

            // Opcional: Se o protocolo acabou, você já pode fechar a conexão por aqui
            // tcp_close(tpcb); 
        }
    }
    // Dentro da verificação do final do arquivo no código TCP do Pico:
    if (es->bytes_recebidos >= es->tamanho_total) {
        // Transfere o ponteiro do buffer da rede para o barramento
        arquivo_buffer = es->buffer_ram; 
        arquivo_tamanho = es->tamanho_total;
        ponteiro_leitura = 0;             // Reseta o índice de leitura do m68k

        arquivo_crc32 = crc32_calculate((const uint8_t *)arquivo_buffer, arquivo_tamanho);
        printf("CRC32 do arquivo %s %08X\n",es->nome_arquivo, arquivo_crc32);
        arquivo_pronto = true;            // Libera o Status para o m68k ver 0x01!
        
        printf("Orion68DOS: Arquivo liberado para o barramento.\n");
    }
    pbuf_free(p);
    return ERR_OK;
}

// Callback quando o cliente conecta
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK || newpcb == NULL) {
        return ERR_VAL;
    }
    printf("Cliente conectado!\n");

    // Aloca um bloco de memória de controle para ESTA conexão específica
    conexao_estado_t *es = (conexao_estado_t *)calloc(1, sizeof(conexao_estado_t));
    if (!es) {
        return ERR_MEM;
    }
    es->estado = ESTADO_ESPERA_CABECALHO;

    // Passa a nossa estrutura de estado como o argumento 'arg' para o tcp_recv
    tcp_arg(newpcb, es);
    tcp_recv(newpcb, tcp_server_recv);
    
    return ERR_OK;
}

// (O restante das funções start_tcp_server e main continuam exatamente iguais)


// Initialize and bind the TCP server socket
void start_tcp_server() {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Failed to create PCB\n");
        return;
    }

    // Bind to all available network interfaces on the specified port
    if (tcp_bind(pcb, IP_ADDR_ANY, PORT) != ERR_OK) {
        printf("Failed to bind to port %d\n", PORT);
        return;
    }

    // Start listening for incoming requests
    struct tcp_pcb *listen_pcb = tcp_listen(pcb);
    if (!listen_pcb) {
        printf("Failed to listen\n");
        return;
    }

    // Register our connection acceptance handler
    tcp_accept(listen_pcb, tcp_server_accept);
    printf("TCP Server listening on port %d...\n", PORT);
}
#include "hardware/vreg.h"

extern void configurar_dma_pico(PIO pio, uint sm) ;
extern void core1_entry(void);
int main() {
    stdio_init_all();
    crc32_init();
    kbd_int_off();

    // 1. Aumenta a tensão do núcleo para suportar o overclock (ex: VREG_VOLTAGE_1_20V ou 1_30V)
   vreg_set_voltage(VREG_VOLTAGE_1_30);
   sleep_ms(2); // Dá um tempo para a tensão estabilizar
   set_sys_clock_khz(250000, true);

    sleep_ms(2500);

    // --- DISPARA O CORE 1 PARA CUIDAR DO PS/2 ---
    multicore_launch_core1(core1_entry);

    // --- CONFIGURAÇÃO DA PIO PARA O BARRAMENTO M68K ---
    PIO pio_barramento = pio0;  // Escolhe o bloco PIO 0
    uint sm_m68k = 0;           // Escolhe a State Machine 0
//    uint sm_m68k1 = 1;          // Escolhe a State Machine 1
  
    // Tenta carregar o programa assembly na memória de instruções da PIO
    uint offset_programa = pio_add_program(pio_barramento, &orion_bus_program);
//    uint offset_programa1 = pio_add_program(pio_barramento, &orion_bus1_program);

    // Chama a nossa função auxiliar de inicialização que configurou os pinos
    orion_bus_program_init(pio_barramento, sm_m68k, offset_programa);
//    orion_bus1_program_init(pio_barramento, sm_m68k1, offset_programa1);

    // --- CORREÇÃO DE SEGURANÇA NO BOOT ---
    // Desliga o SM temporariamente, limpa as FIFOs de lixo elétrico e religa
    pio_sm_set_enabled(pio_barramento, sm_m68k, false);
    pio_sm_clear_fifos(pio_barramento, sm_m68k);
    pio_sm_set_enabled(pio_barramento, sm_m68k, true);

    //pio_sm_set_enabled(pio_barramento, sm_m68k1, false);
    //pio_sm_clear_fifos(pio_barramento, sm_m68k1);
    //pio_sm_set_enabled(pio_barramento, sm_m68k1, true);


    printf("PIO Inicializada! Aguardando ciclos de barramento do m68k...\n");

    // Initialize Wi-Fi chip in station architecture mode
    if (cyw43_arch_init()) {
        printf("Wi-Fi initialization failed\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");

    // Connect to your local network using standard timeout settings
 //   if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
 //       printf("Wi-Fi connection failed\n");
 //       //return -1;
 //   }
    printf("Connected! IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
    //Launch the socket setup
    start_tcp_server();

    kb_init();
    initPS2();

    // Main background execution loop
    while (true) {
        // Keep the Wi-Fi architecture driver responsive (polls for network events)
        cyw43_arch_poll();
        // Atende a PIO o mais rápido possível caso o m68k tenha pedido algo
        gerenciar_barramento_m68k(pio_barramento, sm_m68k);
        //gerenciar_barramento_m68k(pio_barramento, sm_m68k1);
    }
}
