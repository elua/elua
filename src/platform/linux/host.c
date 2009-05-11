#include "host.h"

#define __NR_read     3
#define __NR_write      4
#define __NR_mmap2    192

int host_errno = 0;

#define __syscall_return(type, res) do { \
	if((unsigned long)(res) >= (unsigned long)(-125)) { \
		host_errno = -(res); \
		res = -1; \
	} \
	return (type) (res); \
} while(0)

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type host_##name(type1 arg1,type2 arg2,type3 arg3) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
                  "d" ((long)(arg3))); \
__syscall_return(type,__res); \
}

#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
          type5,arg5,type6,arg6) \
type host_##name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5,type6 arg6) \
{ \
long __res; \
__asm__ volatile ("push %%ebp ; movl %%eax,%%ebp ; movl %1,%%eax ; int $0x80 ; pop %%ebp" \
        : "=a" (__res) \
        : "i" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)),"S" ((long)(arg4)),"D" ((long)(arg5)), \
          "0" ((long)(arg6))); \
	__syscall_return(type, __res); \
}

_syscall3(ssize_t, read, int, fd, void *, buf, size_t, count);
_syscall3(ssize_t, write, int, fd, const void *, buf, size_t, count);
_syscall6(void *,mmap2, void *,addr, size_t, length, int, prot, int, flags, int, fd, off_t, offset);

#if 0
void *host_mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset) {
	long res;
	__asm__ __volatile__ (
		"  push %%ebp\n"
		"  movl %%eax, %%ebp\n"
		"  movl %1, %%eax\n"
  	"  int  $0x80\n"
  	"  pop %%ebp\n"
		: "=a" (res)
		: "i" (__NR_mmap2), "b" (addr), "c" (length), "d" (prot), "S" (flags),
		"D" (fd), "0" (pgoffset)
	);
	return (void *)res;
}
#endif

int host_putchar(int c) {
	unsigned char ch = (unsigned char)c;
	if(host_write(STDOUT_FILENO, &ch, 1) != 1) {
		return EOF;
	}
	return (int)ch;
}

int host_getchar() {
	unsigned char ch = 0;
	if(host_read(STDIN_FILENO, &ch, 1) != 1) {
		return EOF;
	}
	return (int)ch;
}

