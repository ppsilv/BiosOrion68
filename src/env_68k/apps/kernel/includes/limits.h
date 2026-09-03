// ============================================================
// limits.h - Limites para o kernel Orion68DOS (m68k)
// ============================================================
// 
// Este arquivo define os limites para tipos inteiros
// no kernel Orion68DOS para a arquitetura m68k.
// 
// Referência: ISO C99 / POSIX
// ============================================================

#ifndef __LIMITS_H__
#define __LIMITS_H__

// ============================================================
// 1. LIMITES DE TIPOS FUNDAMENTAIS (m68k)
// ============================================================

// --- char ---
#define CHAR_BIT      8           // Bits por byte
#define CHAR_MIN      -128        // Mínimo char (signed)
#define CHAR_MAX      127         // Máximo char (signed)
#define SCHAR_MIN     -128        // Mínimo signed char
#define SCHAR_MAX     127         // Máximo signed char
#define UCHAR_MAX     255         // Máximo unsigned char

// --- short (16 bits no m68k) ---
#define SHRT_MIN      -32768      // Mínimo short
#define SHRT_MAX      32767       // Máximo short
#define USHRT_MAX     65535       // Máximo unsigned short

// --- int (32 bits no m68k) ---
#define INT_MIN       -2147483648 // Mínimo int
#define INT_MAX       2147483647  // Máximo int
#define UINT_MAX      4294967295U // Máximo unsigned int

// --- long (32 bits no m68k) ---
#define LONG_MIN      -2147483648L // Mínimo long
#define LONG_MAX      2147483647L  // Máximo long
#define ULONG_MAX     4294967295UL // Máximo unsigned long

// --- long long (64 bits no m68k) ---
#define LLONG_MIN     -9223372036854775808LL  // Mínimo long long
#define LLONG_MAX     9223372036854775807LL   // Máximo long long
#define ULLONG_MAX    18446744073709551615ULL // Máximo unsigned long long

// ============================================================
// 2. LIMITES DE OUTROS TIPOS
// ============================================================

// --- size_t / ptrdiff_t ---
#define SSIZE_MAX     2147483647  // Máximo signed size_t (32 bits)
#define SIZE_MAX      4294967295U // Máximo size_t

// --- wchar_t (16 bits no m68k) ---
#define WCHAR_MIN     -32768      // Mínimo wchar_t
#define WCHAR_MAX     32767       // Máximo wchar_t
#define WINT_MIN      -2147483648 // Mínimo wint_t
#define WINT_MAX      2147483647  // Máximo wint_t

// ============================================================
// 3. LIMITES DE ARQUIVOS E I/O
// ============================================================

#define MB_LEN_MAX    16          // Máximo tamanho de multibyte char

// --- Limites de arquivo (POSIX) ---
#define NAME_MAX      255         // Máximo tamanho de nome de arquivo
#define PATH_MAX      1024        // Máximo tamanho de caminho completo
#define PIPE_BUF      512         // Tamanho do buffer de pipe
#define LINK_MAX      8           // Máximo de links (para sistemas simples)
#define OPEN_MAX      20          // Máximo de arquivos abertos simultâneos

// ============================================================
// 4. LIMITES PARA TIME
// ============================================================

#define CLOCKS_PER_SEC 1000000    // Clock ticks por segundo (microssegundos)
#define CLK_TCK       1000000     // Clock ticks por segundo (sysconf)

// ============================================================
// 5. LIMITES DE EXECUÇÃO (POSIX)
// ============================================================

#define ARG_MAX       4096        // Máximo tamanho de argumentos (sistema)
#define CHILD_MAX     8           // Máximo de processos filhos
#define STREAM_MAX    20          // Máximo de streams abertas (FOPEN_MAX)

// ============================================================
// 6. LIMITES DE STRING
// ============================================================

#define TMP_MAX       238328      // Máximo de nomes temporários
#define FOPEN_MAX     STREAM_MAX  // Máximo de arquivos abertos com fopen()

// ============================================================
// 7. LIMITES PARA m68k ESPECÍFICOS
// ============================================================

#define WORD_BIT      16          // Bits em uma word (m68k word = 16 bits)
#define LONG_BIT      32          // Bits em um long (m68k long = 32 bits)
#define WORD_SIZE     2           // Bytes em uma word
#define LONG_SIZE     4           // Bytes em um long

// ============================================================
// 8. MACROS ÚTEIS
// ============================================================

// Máximo de dois números
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Mínimo de dois números
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// ============================================================
// 9. VERIFICAÇÃO DE TAMANHOS (para segurança)
// ============================================================

// Compile-time assertions para garantir tamanhos corretos
// (Irá falhar na compilação se os tamanhos estiverem errados)

typedef char assert_char_size[CHAR_BIT == 8 ? 1 : -1];
typedef char assert_short_size[sizeof(short) == 2 ? 1 : -1];
typedef char assert_int_size[sizeof(int) == 4 ? 1 : -1];
typedef char assert_long_size[sizeof(long) == 4 ? 1 : -1];

// ============================================================
// 10. MACROS PARA FORMATAÇÃO
// ============================================================

// Macros para printf/scanf (se você tiver)
#define PRId32  "d"
#define PRIu32  "u"
#define PRIx32  "x"
#define PRIX32  "X"
#define PRId64  "lld"
#define PRIu64  "llu"
#define PRIx64  "llx"
#define PRIX64  "llX"

// ============================================================
// 11. LIMITES PARA O ORION68DOS
// ============================================================

#define ORION_MAX_FILES     32     // Máximo de arquivos abertos
#define ORION_MAX_DEVICES   16     // Máximo de dispositivos no sistema
#define ORION_MAX_PATH      256    // Máximo caminho
#define ORION_MAX_TASKS     16     // Máximo de tarefas
#define ORION_MAX_FIFO_SIZE 4096   // Tamanho máximo de FIFO

#endif // _LIMITS_H