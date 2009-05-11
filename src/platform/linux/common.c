// common.c -- Defines some global functions.
//             From JamesM's kernel development tutorials.

#include "common.h"

// Write a byte out to the specified port.
void outb(u16int port, u8int value)
{
#if 0
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
#endif
}

u8int inb(u16int port)
{
#if 0
    u8int ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
#endif
	return 0;
}

u16int inw(u16int port)
{
#if 0
    u16int ret;
    asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
#endif
	return 0;
}

