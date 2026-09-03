#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#define SERVER_HOST "picow.lab.lan"
#define SERVER_PORT 4242
uint64_t t0 = time_us_64();
extern void crc32_init(void);
extern uint32_t crc32_from_file(const char *filename);
extern void* carregar_arquivo_completo(const char *caminho_arquivo, size_t *tamanho_saida);

// 1. Defina a estrutura exatamente com 16 bytes
// O atributo packed garante que o compilador não coloque "espaços vazios" na memória
struct __attribute__((packed)) CabecalhoProtocolo {
    uint32_t tamanho;      // 4 bytes puros
    char nome_arquivo[12]; // 12 bytes de texto
};


// 1. FUNÇÃO PARA CONECTAR
// Retorna o descritor do socket (ID) se sucesso, ou -1 se falhar
int conectar_servidor(const char *hostname, int porta) {
    int sock_fd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    // Cria o socket TCP
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Erro ao criar o socket");
        return -1;
    }

    // Resolve o nome de domínio (pico2.lab.lan) para IP
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Erro: Não foi possível resolver o host %s\n", hostname);
        close(sock_fd);
        return -1;
    }

    // Configura a estrutura de endereço
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(porta);

    // Tenta estabelecer a conexão de fato com o Pico
    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro ao conectar no servidor");
        close(sock_fd);
        return -1;
    }

    printf("Conectado com sucesso a %s na porta %d!\n", hostname, porta);
    return sock_fd;
}

// 2. FUNÇÃO PARA ESCREVER (Enviar dados)
// Retorna a quantidade de bytes enviados, ou -1 se falhar
ssize_t escrever_dados(int sock_fd, const void *buffer, size_t tamanho) {
    size_t total_enviado = 0;
    const char *ptr = buffer;

    // Garante que todo o bloco de dados seja transmitido pelo SO
    while (total_enviado < tamanho) {
        ssize_t enviado = send(sock_fd, ptr + total_enviado, tamanho - total_enviado, 0);
        if (enviado <= 0) {
            perror("Erro ao enviar dados");
            return -1;
        }
        total_enviado += enviado;
    }
    return total_enviado;
}



// 3. FUNÇÃO PARA LER (Receber dados)
// Retorna os bytes lidos, 0 se o servidor fechou a conexão, ou -1 se falhar
ssize_t ler_dados(int sock_fd, void *buffer, size_t tamanho_maximo) {
    // recv() bloqueia e espera até que chegue algum dado do Pico
    ssize_t bytes_lidos = recv(sock_fd, buffer, tamanho_maximo, 0);
    if (bytes_lidos < 0) {
        perror("Erro ao ler dados");
    } else if (bytes_lidos == 0) {
        printf("O servidor encerrou a conexão.\n");
    }
    return bytes_lidos;
}

// ============================================================================
// EXEMPLO DE USO NO MAIN
// ============================================================================
int main(int argc, char * argv[]) {
    // 1. Tenta conectar no Pico 2 W
    int tam=0;
    unsigned char * buffer;
    struct CabecalhoProtocolo cabecalho;

    if (argc < 2) {
        printf("Uso: %s <caminho_do_arquivo>\n", argv[0]);
        return 1;
    }

    if( strlen(argv[1]) > 12){
        printf("Filename has more than 12 bytes\n");
        exit(1);
    }
    crc32_init();

    uint32_t crc = crc32_from_file(argv[1]);
    printf("CRC32: 0x%08X\n", crc);

    buffer = (unsigned char *)carregar_arquivo_completo(argv[1],(size_t *)&tam);

    if ( buffer == NULL ){
        printf("Erro: buffer vazio, provavelmente arquivo não encontrado ou vazio.\n");
        return 1;
    }

    int socket_pico = conectar_servidor(SERVER_HOST, SERVER_PORT);
    if (socket_pico < 0) {
        return EXIT_FAILURE;
    }

    // Preenche o tamanho com o valor numérico puro
    cabecalho.tamanho = (uint32_t)tam; 

    printf("Enviando o arquivo: %s\n",argv[1]);

    // Limpa o campo do nome com zeros e copia o nome (garantindo os espaços ou preenchimento)
    memset(cabecalho.nome_arquivo, ' ', 12); // Preenche com espaços como você fez no seu exemplo
    memcpy(cabecalho.nome_arquivo, argv[1], strlen(argv[1])); // Copia o nome por cima

    // Envia os 16 bytes estruturados direto da memória
    if (escrever_dados(socket_pico, &cabecalho, sizeof(cabecalho)) < 0) {
        close(socket_pico);
        return EXIT_FAILURE;
    }

    // 2. Envia o Arquivo completo (8029 bytes)
    printf("Enviando dados do arquivo...\n");
    if (escrever_dados(socket_pico, buffer, tam) < 0) {
        close(socket_pico);
        return EXIT_FAILURE;
    }

    // 3. AGORA ESPERA A CONFIRMAÇÃO DO PICO (Sem loops baseados no tamanho do arquivo)
    char buffer_confirmacao[10032];
    memset(buffer_confirmacao, 0, sizeof(buffer_confirmacao));

    printf("Aguardando confirmação do Pico...\n");
    ssize_t lidos = ler_dados(socket_pico, buffer_confirmacao, sizeof(buffer_confirmacao) - 1);

    if (lidos > 0) {
        buffer_confirmacao[lidos] = '\0';
        printf("Resposta do Pico: %s", buffer_confirmacao); // Deve imprimir "Arq lido"
    }

    printf("\nTransmissão concluída com sucesso!\n");

    // 4. Fecha a conexão limpando o socket
    printf("Fechando conexão.\n");
    close(socket_pico);

    printf("\n\n");
    return EXIT_SUCCESS;
}



/*

Carga completa de arquivo

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/**
 * Abre um arquivo, aloca memória e carrega todo o seu conteúdo.
 * 
 * @param caminho_arquivo Caminho para o arquivo no disco.
 * @param tamanho_saida   Ponteiro para receber o tamanho exato do arquivo lido.
 * @return Ponteiro para o buffer alocado com os dados, ou NULL em caso de erro.
 * 
 * NOTA: O chamador da função é responsável por dar free() no buffer retornado.
 */
void* carregar_arquivo_completo(const char *caminho_arquivo, size_t *tamanho_saida) {
    FILE *arquivo = NULL;
    struct stat st;
    void *buffer = NULL;

    // 1. Garante que os parâmetros passados são válidos
    if (!caminho_arquivo || !tamanho_saida) {
        return NULL;
    }
    *tamanho_saida = 0;

    // 2. Abre o arquivo em modo binário ("rb") para evitar conversões de quebra de linha
    arquivo = fopen(caminho_arquivo, "rb");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    // 3. Obtém o tamanho do arquivo usando o descritor de arquivos do sistema
    if (fstat(fileno(arquivo), &st) != 0) {
        perror("Erro ao obter o tamanho do arquivo");
        fclose(arquivo);
        return NULL;
    }

    size_t tamanho_arquivo = st.st_size;

    // 4. Aloca a memória. Adicionamos +1 byte para o terminador '\0' caso seja texto
    buffer = malloc(tamanho_arquivo + 1);
    if (!buffer) {
        perror("Erro de falta de memória (malloc falhou)");
        fclose(arquivo);
        return NULL;
    }


    // 5. Lê o arquivo inteiro para o buffer de uma vez só
    size_t bytes_lidos = fread(buffer, 1, tamanho_arquivo, arquivo);
    
    // Fecha o arquivo imediatamente, pois os dados já estão na RAM
    fclose(arquivo);

    // 6. Verifica se a leitura foi feita por completo
    if (bytes_lidos < tamanho_arquivo) {
        fprintf(stderr, "Erro: Apenas %zu de %zu bytes foram lidos.\n", bytes_lidos, tamanho_arquivo);
        free(buffer);
        return NULL;
    }

    // Garante a finalização da string (útil se for um arquivo de texto)
    ((char*)buffer)[tamanho_arquivo] = '\0';

    // 7. Retorna o tamanho e o ponteiro da memória para o chamador
    *tamanho_saida = tamanho_arquivo;

    return buffer;
}


/*

int main() {
    // 1. Tenta conectar no Pico 2 W
    int socket_pico = conectar_servidor(SERVER_HOST, SERVER_PORT);
    if (socket_pico < 0) {
        return EXIT_FAILURE;
    }
    int tam=0;
    unsigned char * buffer;
    struct CabecalhoProtocolo cabecalho;


    buffer = (unsigned char *)carregar_arquivo_completo("/home/pdsilva/project/Orion68/src/sender/sender.c",(size_t *)&tam);


    // Preenche o tamanho com o valor numérico puro
    cabecalho.tamanho = (uint32_t)tam; 

    // Limpa o campo do nome com zeros e copia o nome (garantindo os espaços ou preenchimento)
    memset(cabecalho.nome_arquivo, ' ', 12); // Preenche com espaços como você fez no seu exemplo
    memcpy(cabecalho.nome_arquivo, "sender.c", strlen("sender.c")); // Copia o nome por cima

    // Envia os 16 bytes estruturados direto da memória
    if (escrever_dados(socket_pico, &cabecalho, sizeof(cabecalho)) < 0) {
        close(socket_pico);
        return EXIT_FAILURE;
    }

    // Teste enviando uma string para o seu Echo Server do Pico
    //    const char *mensagem = "Ola Pico 2 W! Preparado para o Orion68DOS?";
    printf("Enviando: mensagem de tamanho [%03d]\n", tam);
    
    if (escrever_dados(socket_pico, buffer, tam) < 0) {
        close(socket_pico);
        return EXIT_FAILURE;
    }

    // Buffer para guardar a resposta do Eco do Pico
    char resposta[512];
    memset(resposta, 0, sizeof(resposta));

    // Como o seu código no Pico devolve o eco, vamos ler a resposta
    ssize_t lidos=1;
    size_t tamanho_total_esperado = tam; // Esse valor viria do seu cabeçalho
    size_t total_recebido = 0;

    while (total_recebido < tamanho_total_esperado) {
        memset(resposta, 0, sizeof(resposta));
        
        // Calcula quanto ainda falta para não ler mais do que deve
        size_t a_ler = tamanho_total_esperado - total_recebido;
        if (a_ler > sizeof(resposta) - 1) {
            a_ler = sizeof(resposta) - 1;
        }

        lidos = ler_dados(socket_pico, resposta, a_ler);
        if (lidos <= 0) {
            break; // Conexão caiu ou fechou antes da hora
        }

        total_recebido += lidos;
        resposta[lidos] = '\0';
        printf("Recebido ACK: %ld bytes. Progresso: %zu/%zu\n", lidos, total_recebido, tamanho_total_esperado);
        //printf("%s\n",resposta);
    }
    printf("Transmissão concluída com sucesso! Saindo do loop de leitura.\n");

    // 4. Fecha a conexão limpando o socket
    printf("Fechando conexão.\n");
    close(socket_pico);

    printf("\n\n");
    return EXIT_SUCCESS;
}


*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct __attribute__((packed)) {
    uint32_t tamanho_total;
    char nome_arquivo[13]; // <--- O [13] CORRIGIDO AQUI!
    uint32_t crc32;
    char conteudo[];
} file_header_t;

int main2(){
    file_header_t *pfh;

    // Aloca a struct + 1000 bytes para o conteúdo dinâmico
    pfh = (file_header_t *) malloc(sizeof(file_header_t) + 1000);

    if (pfh == NULL) {
        printf("Erro ao alocar memoria!\n");
        return 1;
    }

    pfh->tamanho_total = __builtin_bswap32(0x45464748); // 0x02B5B428
    pfh->crc32 = __builtin_bswap32(0x41424344);      // A B C D

    // Grava com segurança nos limites corretos da struct
    memcpy(pfh->nome_arquivo, "paulo.sil\0", 10);
    memcpy(pfh->conteudo, "paulo da silva\0", 15);

    uint8_t *ponteiro_byte = (uint8_t*)pfh;

    // Tamanho total para exibir: 21 bytes do cabeçalho + 15 bytes do texto
    int tamanho_total_bytes = sizeof(file_header_t) + 15;

    // 1. Mostrando o dump em HEXADECIMAL
    printf("Dump Hexadecimal:\n");
    for(int i = 0; i < tamanho_total_bytes; i++){
        printf("%02X|", *(ponteiro_byte + i));
    }

    // 2. Mostrando em CARACTERE
    printf("\n\nDump em Caracteres (Letras):\n");
    for(int i = 0; i < tamanho_total_bytes; i++){
        uint8_t byte = *(ponteiro_byte + i);
        if (byte >= 32 && byte <= 126) {
            printf("%c|", byte);
        } else {
            printf(".|");
        }
    }
    printf("\n");

    free(pfh);
    return 0;
}
