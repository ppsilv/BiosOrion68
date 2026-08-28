## App flux to read something on disk drive


+------------------+
|   SUA APLICAÇÃO   |  (seu programa)
+------------------+
        |
        v (chama fopen, fread, etc.)
+------------------+
|   libfileio.a    |  (suas syscalls)
+------------------+
        |
        v (trap #12)
+------------------+
|   FATFS (ff.c)   |  (elm-chan FatFS)
+------------------+
        |
        v (precisa de disk_read/disk_write)
+------------------+
|   diskio.c       |  (interface com hardware)
+------------------+
        |
        v (chama ata_read/ata_write)
+------------------+
|   ata.c          |  (driver ATA/IDE)
+------------------+
        |
        v
+------------------+
|   HARDWARE       |  (disco/SD/CF)
+------------------+


lib/
├── libdrivers.a      # Drivers de hardware (ATA, USB, etc.)
│   ├── ata.c         # Driver ATA/IDE
│   ├── sdcard.c      # Driver SD Card
│   └── ...
│
├── libdiskio.a       # Interface FatFS com hardware
│   └── diskio.c      # disk_read/disk_write (chama drivers)
│
├── libfatfs.a        # FatFS puro (elm-chan)
│   └── ff.c          # f_open, f_read, f_write, etc.
│
├── libfileio.a       # Suas syscalls (trap #12)
│   └── fileio.c      # fopen, fread, fwrite (chama FatFS)
│
└── libvfs.a          # VFS (sua camada de abstração)
    ├── vfs.c
    ├── vfs_fat.c
    ├── vfs_tty.c
    └── ...
