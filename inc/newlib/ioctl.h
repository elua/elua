// Just a declaration of the "ioctl" function, as it does not appear in the Newlib headers

#ifndef __IOCTL_H__
#define __IOCTL_H__

#include <reent.h>

// **************************  Generic IOCTL calls *****************************

// Seek
#define FDSEEK        0x01
struct fd_seek 
{
  _off_t off;
  int dir;
};

// ***************** Base IOCTRL numbers for other devices *********************
#define IOCTL_BASE_UART     0x100

// Function prototype
int ioctl( int file, unsigned long request, void *ptr );  

#endif
