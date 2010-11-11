// Platform dependent network definitions

#ifndef __PNET_H__
#define __PNET_H__

#ifdef WIN32_BUILD

#include <winsock2.h>
#include <windows.h>
typedef int socklen_t;
#define socket_close  closesocket
#define INVALID_SOCKET_VALUE  INVALID_SOCKET

#else // #ifdef WIN32_BUILD

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
typedef int SOCKET;
#define socket_close  close
#define INVALID_SOCKET_VALUE  ( -1 )

#endif // #ifdef WIN32_BUILD

#endif // #ifndef __PNET_H__
