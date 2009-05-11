// host.h -- Defines syscall wrappers to access host OS.

#ifndef _HOST_H
#define _HOST_H

#include <stddef.h>
#include <sys/types.h>

extern int host_errno;

#define EOF (-1)
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

ssize_t host_read( int fd, void * buf, size_t count);
ssize_t host_write( int fd, const void * buf, size_t count);
int host_putchar(int c);
int host_getchar();

#define PROT_READ 0x1   /* Page can be read.  */
#define PROT_WRITE  0x2   /* Page can be written.  */
#define PROT_EXEC 0x4   /* Page can be executed.  */
#define PROT_NONE 0x0   /* Page can not be accessed.  */

#define MAP_SHARED  0x01    /* Share changes.  */
#define MAP_PRIVATE 0x02    /* Changes are private.  */
#define MAP_FIXED 0x10    /* Interpret addr exactly.  */
#define MAP_ANONYMOUS  0x20    /* Don't use a file.  */

#define MAP_FAILED (void *)(-1)

void *host_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset);

#endif // _HOST_H
