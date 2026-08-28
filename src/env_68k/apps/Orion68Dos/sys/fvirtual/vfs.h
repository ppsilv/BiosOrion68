// ============================================
// SISTEMA DE ARQUIVOS VIRTUAL (VFS)
// ============================================

// Estrutura que representa um "arquivo" (qualquer coisa)
typedef struct {
    char *name;                    // Nome (ex: "/dev/tty", "/proc/mem")
    int type;                      // FILE_TYPE_REGULAR, FILE_TYPE_DEVICE, FILE_TYPE_PROC
    
    // Ponteiros para funções (operação polimórfica)
    int (*open)(struct File *file);
    int (*read)(struct File *file, void *buffer, size_t size);
    int (*write)(struct File *file, const void *buffer, size_t size);
    int (*close)(struct File *file);
    int (*ioctl)(struct File *file, int cmd, void *arg);
    
    // Dados específicos do dispositivo/arquivo
    void *private_data;            // Pode apontar para FIL (FATFS), UART, etc.
    size_t position;               // Posição atual de leitura/escrita
    int ref_count;                 // Quantos processos estão usando
} File;

// Tabela de descritores (cada processo tem uma)
typedef struct {
    File *files[256];              // Max 256 arquivos abertos por processo
    int count;
} FileDescriptorTable;