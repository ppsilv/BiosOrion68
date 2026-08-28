+------------------+
|   PROGRAMA       |  (shell, etc.)
+------------------+
        |
        v (chama open, read, write, close - wrappers)
+------------------+
|   unistd.c       |  (wrappers que chamam vfs_*)
+------------------+
        |
        v (chama vfs_open, vfs_read, vfs_write, vfs_close)
+------------------+
|   VFS            |  (vfs.c)
+------------------+
        |
        v (chama fat_open, fat_read, fat_write, fat_close)
+------------------+
|   vfs_fat.c      |  (wrappers que chamam suas funções)
+------------------+
        |
        v (chama fopen, fread, fwrite, fclose)
+------------------+
|   libfileio.c    |  (SUAS syscalls - trap #12)
+------------------+
        |
        v (trap #12)
+------------------+
|   KERNEL         |  (trata a trap)
+------------------+


PROGRAMA (shell.c)
    |
    | #include "unistd.h"
    |
    v
open("/boot.bat", O_RDONLY)   ← API UNIX (você usa no shell)
    |
    v
vfs_open("/boot.bat", O_RDONLY)   ← VFS (camada de abstração)
    |
    v
fat_open()   ← Driver FATFS (vfs_fat.c)
    |
    v
fopen()   ← SUA SYSCALL (libfileio.c)  ← É AQUI!
    |
    v
trap #12   ← Chama o kernel
    |
    v
Kernel trata a trap
    |
    v
disk_read() → ATA → Hardware