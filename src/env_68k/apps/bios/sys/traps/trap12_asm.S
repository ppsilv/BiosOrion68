
.section .text
.align 2


.global file_open
.global file_close
.global file_read
.global file_write
.global file_sync
.global file_unlink
.global file_lseek
.global file_tell
.global file_size
.global file_eof
.global file_rename
.global file_stat
.global file_opendir
.global file_readdir
.global file_closedir
.global file_mkdir
.global file_mount
.global file_unmount
.global file_list
.global file_save
.global file_load
.global file_findfirst
.global file_findnext



.extern fstat
.extern fopendir
.extern freaddir
.extern fclosedir
.extern fmkdir
.extern fmount
.extern funmount


.extern do_load_basic

/* =========================================================================
 * 1. FILE OPEN: f_open(fp, path, mode) -> 3 argumentos
 * Regs: A0 = fp, A1 = path, D0 = mode
 * ========================================================================= */
file_open:
    move.l  %d0,-(%sp)          /* argv[2] = mode */
    move.l  %a1,-(%sp)          /* argv[1] = path */
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #3,-(%sp)           /* 1º param: argc = 3 */

    jsr     do_open

    lea     20(%sp),%sp         /* Limpa 5 longs (20 bytes) */
    bra     trapC_exit

/* =========================================================================
 * 2. FILE CLOSE: f_close(fp) -> 1 argumento
 * Regs: A0 = fp
 * ========================================================================= */
file_close:
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_close

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit

/* =========================================================================
 * 3. FILE READ: f_read(fp, buff, btr, br) -> 4 argumentos
 * Regs: A0 = fp, A1 = buff, D0 = btr, A2 = br
 * ========================================================================= */
file_read:
    move.l  %a2,-(%sp)          /* argv[3] = br */
    move.l  %d0,-(%sp)          /* argv[2] = btr */
    move.l  %a1,-(%sp)          /* argv[1] = buff */
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #4,-(%sp)           /* 1º param: argc = 4 */

    jsr     do_read

    lea     24(%sp),%sp         /* Limpa 6 longs (24 bytes) */
    bra     trapC_exit

/* =========================================================================
 * 4. FILE WRITE: f_write(fp, buff, btw, bw) -> 4 argumentos
 * Regs: A0 = fp, A1 = buff, D0 = btw, A2 = bw
 * ========================================================================= */
file_write:
    move.l  %a2,-(%sp)          /* argv[3] = bw */
    move.l  %d0,-(%sp)          /* argv[2] = btw */
    move.l  %a1,-(%sp)          /* argv[1] = buff */
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #4,-(%sp)           /* 1º param: argc = 4 */

    jsr     do_write

    lea     24(%sp),%sp         /* Limpa 6 longs (24 bytes) */
    bra     trapC_exit

file_sync:
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_sync

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit


/* =========================================================================
 * 5. FILE LSEEK: f_lseek(fp, offset) -> 2 argumentos
 * Regs: A0 = fp, D0 = offset
 * ========================================================================= */
file_lseek:
    move.l  %d0,-(%sp)          /* argv[1] = offset */
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #2,-(%sp)           /* 1º param: argc = 2 */

    jsr     do_lseek

    lea     16(%sp),%sp         /* Limpa 4 longs (16 bytes) */
    bra     trapC_exit

/* =========================================================================
 * 6. FILE UNLINK/REMOVE: f_unlink(path) -> 1 argumento
 * Regs: A0 = path
 * ========================================================================= */
file_unlink:
    move.l  %a0,-(%sp)          /* argv[0] = path */
    
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_unlink

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit

/* =========================================================================
 * 1. INFORMACÕES E POSICIONAMENTO (1 argumento: A0 = fp)
 * ========================================================================= */

file_tell:
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_tell

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit

file_size:
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_size

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit

file_eof:
    move.l  %a0,-(%sp)          /* argv[0] = fp */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_eof

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit

/* =========================================================================
 * 2. RENOMEAR E STAT (2 argumentos: A0 = path_old/path, A1 = path_new/fno)
 * ========================================================================= */

file_rename:
    move.l  %a1,-(%sp)          /* argv[1] = path_new */
    move.l  %a0,-(%sp)          /* argv[0] = path_old */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #2,-(%sp)           /* 1º param: argc = 2 */

    jsr     do_rename

    lea     16(%sp),%sp         /* Limpa 4 longs (16 bytes) */
    bra     trapC_exit

file_stat:
    move.l  %a1,-(%sp)          /* argv[1] = fno */
    move.l  %a0,-(%sp)          /* argv[0] = path */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #2,-(%sp)           /* 1º param: argc = 2 */

    jsr     do_stat

    lea     16(%sp),%sp         /* Limpa 4 longs (16 bytes) */
    bra     trapC_exit

/* =========================================================================
 * 3. DIRETÓRIOS
 * ========================================================================= */

/* f_opendir: A0 = dp, A1 = path (2 argumentos) */
file_opendir:
    move.l  %a1,-(%sp)          /* argv[1] = path */
    move.l  %a0,-(%sp)          /* argv[0] = dp */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #2,-(%sp)           /* 1º param: argc = 2 */

    jsr     do_opendir

    lea     16(%sp),%sp         /* Limpa 4 longs (16 bytes) */
    bra     trapC_exit

/* f_readdir: A0 = dp, A1 = fno (2 argumentos) */
file_readdir:
    move.l  %a1,-(%sp)          /* argv[1] = fno */
    move.l  %a0,-(%sp)          /* argv[0] = dp */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #2,-(%sp)           /* 1º param: argc = 2 */

    jsr     do_readdir

    lea     16(%sp),%sp         /* Limpa 4 longs (16 bytes) */
    bra     trapC_exit

/* f_closedir: A0 = dp (1 argumento) */
file_closedir:
    move.l  %a0,-(%sp)          /* argv[0] = dp */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_closedir

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit

/* f_mkdir: A0 = path (1 argumento) */
file_mkdir:
    move.l  %a0,-(%sp)          /* argv[0] = path */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_mkdir

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit

/* =========================================================================
 * 4. MONTAGEM E DESMONTAGEM
 * ========================================================================= */

/* f_mount: A0 = fs, A1 = path, D0 = opt (3 argumentos) */
file_mount:
    move.l  %d0,-(%sp)          /* argv[2] = opt */
    move.l  %a1,-(%sp)          /* argv[1] = path */
    move.l  %a0,-(%sp)          /* argv[0] = fs */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #3,-(%sp)           /* 1º param: argc = 3 */

    jsr     do_mount

    lea     20(%sp),%sp         /* Limpa 5 longs (20 bytes) */
    bra     trapC_exit

/* f_unmount: A0 = path (1 argumento) */
file_unmount:
    move.l  %a0,-(%sp)          /* argv[0] = path */
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #1,-(%sp)           /* 1º param: argc = 1 */

    jsr     do_unmount

    lea     12(%sp),%sp         /* Limpa 3 longs (12 bytes) */
    bra     trapC_exit    

file_list:
    |; 1. Monta a tabela argv[] na própria pilha (argv[0], argv[1], argv[2])
    move.l  %a0,-(%sp)    |; argv[0] = Ponteiro FIL / "teste.bas"
    
    move.l  %sp,%a2       |; A2 = Endereço do início do vetor argv[]

    |; 2. Empilha os parâmetros para do_save(argc, argv) - da direita para a esquerda
    move.l  %a2,-(%sp)   | ; 2º parâmetro: char *argv[]
    move.l  #1,-(%sp)    | ; 1º parâmetro: int argc (3 argumentos)

    | ; 3. Chama a função
    jsr     do_ls

    | ; 4. Limpa a pilha (8 bytes dos parâmetros + 12 bytes da tabela argv = 20 bytes)
    lea     12(%sp),%sp
    bra     trapC_exit

file_save:
    | 1. Monta a tabela argv[] na própria pilha (argv[0], argv[1], argv[2])
    move.l  %A2,-(%sp)    |; argv[3] = bytes_escritos
    move.l  %d0,-(%sp)    |; argv[2] = Quantidade de bytes
    move.l  %a1,-(%sp)    |; argv[1] = Ponteiro do buffer
    move.l  %a0,-(%sp)    |; argv[0] = Ponteiro FIL / "teste.bas"
    
    move.l  %sp,%a2       |; A2 = Endereço do início do vetor argv[]

    |; 2. Empilha os parâmetros para do_save(argc, argv) - da direita para a esquerda
    move.l  %a2,-(%sp)    |; 2º parâmetro: char *argv[]
    move.l  #4,-(%sp)     |; 1º parâmetro: int argc (3 argumentos)

    |; 3. Chama a função
    jsr     do_save_basic

    |; 4. Limpa a pilha (8 bytes dos parâmetros + 12 bytes da tabela argv = 20 bytes)
    lea     24(%sp),%sp
    bra     trapC_exit

file_load:
    | 1. Monta a tabela argv[] na própria pilha (argv[0], argv[1], argv[2])
    move.l  %A2,-(%sp)    |; argv[3] = bytes_escritos
    move.l  %d0,-(%sp)    |; argv[2] = Quantidade de bytes
    move.l  %a1,-(%sp)    |; argv[1] = Ponteiro do buffer de dados
    move.l  %a0,-(%sp)    |; argv[0] = "teste.bas"
    
    move.l  %sp,%a2       |; A2 = Endereço do início do vetor argv[]

    |; 2. Empilha os parâmetros para do_save(argc, argv) - da direita para a esquerda
    move.l  %a2,-(%sp)    |; 2º parâmetro: char *argv[]
    move.l  #4, -(%sp)    |; 1º parâmetro: int argc (3 argumentos)
    |; 3. Chama a funç
    jsr     do_load_basic

    |; 4. Limpa a pilha (8 bytes dos parâmetros + 12 bytes da tabela argv = 20 bytes)
    lea     24(%sp),%sp
    bra     trapC_exit

file_findfirst:
    | 1. Monta a tabela argv[] na própria pilha (argv[0], argv[1], argv[2])
    move.l  %A2,-(%sp)    |; argv[3] = pattern
    move.l  %d0,-(%sp)    |; argv[2] = path/filename
    move.l  %a1,-(%sp)    |; argv[1] = Ponteiro FILINFO
    move.l  %a0,-(%sp)    |; argv[0] = Ponteiro DIR*"
    
    move.l  %sp,%a2       |; A2 = Endereço do início do vetor argv[]

    |; 2. Empilha os parâmetros para do_save(argc, argv) - da direita para a esquerda
    move.l  %a2,-(%sp)    |; 2º parâmetro: char *argv[]
    move.l  #4,-(%sp)     |; 1º parâmetro: int argc (3 argumentos)

    |; 3. Chama a função
    jsr     do_findfirst

    |; 4. Limpa a pilha (8 bytes dos parâmetros + 12 bytes da tabela argv = 20 bytes)
    lea     24(%sp),%sp
    bra     trapC_exit

file_findnext:
    move.l  %a1,-(%sp)          /* argv[1] = FILINFO */
    move.l  %a0,-(%sp)          /* argv[0] = DIR*/
    move.l  %sp,%a2             /* A2 = &argv[0] */

    move.l  %a2,-(%sp)          /* 2º param: argv */
    move.l  #2,-(%sp)           /* 1º param: argc = 2 */

    jsr     do_findnext

    lea     16(%sp),%sp         /* Limpa 4 longs (16 bytes) */
    bra     trapC_exit

trapC_exit:
        /* Restaura o mundo (D0 contém o retorno de cconin se foi chamada) */
        movem.l (%sp)+,%d1-%d7/%a0-%a6
        rte
