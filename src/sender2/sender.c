#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>



#define SERVER_HOST "orion68k"
#define SERVER_PORT 4243

typedef struct st_buf{
    uint32_t tamanho;
    uint32_t crc32;
    char nome_arquivo[13];
    void *dados;
} FILE_RECV;

static FILE_RECV file_data;


extern void crc32_init(void);
extern uint32_t crc32_from_file(const char *filename);

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

    // Resolve o nome de domínio () para IP
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

    // Tenta estabelecer a conexão de fato com o Orion68DOS
    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro ao conectar no servidor");
        close(sock_fd);
        return -1;
    }

    printf("Conectado com sucesso a %s na porta %d!\n", hostname, porta);
    return sock_fd;
}
void dump_hex(const char *label, const void *buf, size_t len) {
    const uint8_t *p = (const uint8_t *)buf;
    printf("--- %s (%zu bytes) ---\n", label, len);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", p[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}
// 2. FUNÇÃO PARA ESCREVER (Enviar dados)
// Retorna a quantidade de bytes enviados, ou -1 se falhar
ssize_t escrever_dados(int sock_fd, const void *buffer, size_t tamanho) {
    size_t total_enviado = 0;
    const char *ptr = buffer;
    //dump_hex("buffer",buffer,313);
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
    // recv() bloqueia e espera até que chegue algum dado do Orion68DOS
    ssize_t bytes_lidos = recv(sock_fd, buffer, tamanho_maximo, 0);
    if (bytes_lidos < 0) {
        perror("Erro ao ler dados");
    } else if (bytes_lidos == 0) {
        printf("O servidor encerrou a conexão.\n");
    }
    return bytes_lidos;
}



// ============================================================================
// Carregar arquivo completo
// ============================================================================

/**
 * Abre um arquivo, aloca memória e carrega todo o seu conteúdo.
 *
 * @param caminho_arquivo Caminho para o arquivo no disco.
 * @param tamanho_saida   Ponteiro para receber o tamanho exato do arquivo lido.
 * @return Ponteiro para o buffer alocado com os dados, ou NULL em caso de erro.
 *
 * NOTA: O chamador da função é responsável por dar free() no buffer retornado.
 */
int carregar_arquivo_completo(const char *caminho_arquivo, size_t *tamanho_saida) {
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
    file_data.dados = malloc(tamanho_arquivo + 21);
    if (!file_data.dados) {
        perror("Erro de falta de memória (malloc falhou)");
        fclose(arquivo);
        return NULL;
    }

    char *ptrfile =file_data.dados;
    // 5. Lê o arquivo inteiro para o file_data.dados de uma vez só
    size_t bytes_lidos = fread(ptrfile+21, 1, tamanho_arquivo, arquivo);

    // Fecha o arquivo imediatamente, pois os dados já estão na RAM
    fclose(arquivo);

    // 6. Verifica se a leitura foi feita por completo
    if (bytes_lidos < tamanho_arquivo) {
        fprintf(stderr, "Erro: Apenas %zu de %zu bytes foram lidos.\n", bytes_lidos, tamanho_arquivo);
        free(file_data.dados);
        return NULL;
    }

    // Garante a finalização da string (útil se for um arquivo de texto)
    //((char*)buffer)[tamanho_arquivo] = '\0';

    // 7. Retorna o tamanho e o ponteiro da memória para o chamador
    *tamanho_saida = tamanho_arquivo;
    file_data.tamanho=tamanho_arquivo;
    return 1;
}


// ============================================================================
// EXEMPLO DE USO NO MAIN
// ============================================================================
int main(int argc, char * argv[]) {
    // 1. Tenta conectar no Orion68DOS
    int tam=0;

    if (argc < 2) {
        printf("Uso: %s <caminho_do_arquivo>\n", argv[0]);
        return 1;
    }
    if( strlen(argv[1]) > 12){
        printf("Filename has more than 12 bytes\n");
        exit(1);
    }
    memset((void*)&file_data,0,sizeof(file_data));

    carregar_arquivo_completo(argv[1],(size_t *)&tam);

    if ( file_data.dados  == NULL ){
        printf("Erro: buffer vazio, provavelmente arquivo não encontrado ou vazio.\n");
        return 1;
    }

    int socket_orion68 = conectar_servidor(SERVER_HOST, SERVER_PORT);
    if (socket_orion68 < 0) {
        return EXIT_FAILURE;
    }

    printf("Enviando o arquivo: %s\n",argv[1]);

    // Preenche o tamanho com o valor numérico puro
    file_data.tamanho = (uint32_t)tam;
    file_data.crc32 = 0;
    // Limpa o campo do nome com zeros e copia o nome (garantindo os espaços ou preenchimento)
    memcpy(file_data.nome_arquivo, argv[1], strlen(argv[1])); // Copia o nome por cima

    // 2. Envia o Arquivo completo (8029 bytes)
    printf("Enviando dados do arquivo...\n");
    printf("Nome do arquivo: %s\n",file_data.nome_arquivo);
    printf("Tam. do arquivo: %d\n",file_data.tamanho);
    printf("Dado do arquivo: %s\n",file_data.dados);

    memcpy(file_data.dados, &file_data.tamanho, 4);
    memcpy(file_data.dados+8, &file_data.nome_arquivo, 13);


    if (escrever_dados(socket_orion68, (void *)file_data.dados, tam) < 0) {
        close(socket_orion68);
        return EXIT_FAILURE;
    }

    // 3. AGORA ESPERA A CONFIRMAÇÃO DO Orion68DOS (Sem loops baseados no tamanho do arquivo)
    char buffer_confirmacao[10032];
    memset(buffer_confirmacao, 0, sizeof(buffer_confirmacao));

    printf("Aguardando confirmação do Orion68DOS...\n");
    ssize_t lidos = ler_dados(socket_orion68, buffer_confirmacao, sizeof(buffer_confirmacao) - 1);

    if (lidos > 0) {
        buffer_confirmacao[lidos] = '\0';
        printf("Resposta do Orion68DOS: %s", buffer_confirmacao); // Deve imprimir "Arq lido"
    }

    printf("\nTransmissão concluída com sucesso!\n");

    // 4. Fecha a conexão limpando o socket
    printf("Fechando conexão.\n");
    close(socket_orion68);

    printf("\n\n");
    return EXIT_SUCCESS;
}
