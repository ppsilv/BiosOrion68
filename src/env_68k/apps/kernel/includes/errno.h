// ============================================================
// errno.h - Códigos de erro para o Orion68DOS
// ============================================================

#ifndef __ERRNO_H__
#define __ERRNO_H__

// Códigos de erro comuns
#define EPERM       1   // Operation not permitted
#define ENOENT      2   // No such file or directory
#define ESRCH       3   // No such process
#define EINTR       4   // Interrupted system call
#define EIO         5   // I/O error
#define ENXIO       6   // No such device or address
#define E2BIG       7   // Argument list too long
#define ENOEXEC     8   // Exec format error
#define EBADF       9   // Bad file number
#define ECHILD      10  // No child processes
#define EAGAIN      11  // Try again
#define ENOMEM      12  // Out of memory
#define EACCES      13  // Permission denied
#define EFAULT      14  // Bad address
#define ENOTBLK     15  // Block device required
#define EBUSY       16  // Device or resource busy
#define EEXIST      17  // File exists
#define EXDEV       18  // Cross-device link
#define ENODEV      19  // No such device
#define ENOTDIR     20  // Not a directory
#define EISDIR      21  // Is a directory
#define EINVAL      22  // Invalid argument
#define ENFILE      23  // File table overflow
#define EMFILE      24  // Too many open files
#define ENOTTY      25  // Not a typewriter
#define ETXTBSY     26  // Text file busy
#define EFBIG       27  // File too large
#define ENOSPC      28  // No space left on device
#define ESPIPE      29  // Illegal seek
#define EROFS       30  // Read-only file system
#define EMLINK      31  // Too many links
#define EPIPE       32  // Broken pipe
#define EDOM        33  // Math argument out of domain of func
#define ERANGE      34  // Math result not representable
#define EDEADLK     35  // Resource deadlock would occur
#define ENAMETOOLONG 36 // File name too long
#define ENOLCK      37  // No record locks available
#define ENOSYS      38  // Function not implemented
#define ENOTEMPTY   39  // Directory not empty
#define ELOOP       40  // Too many symbolic links encountered

// Erros específicos do Orion68DOS
#define ENODRV       100 // No such driver
#define ENODISK      101 // No such disk
#define EFILECORRUPT 102 // File corrupted
#define ENOTSUPPORT  103 // Not supported

// Variável global errno (externa)
extern int errno;

#endif // _ERRNO_H