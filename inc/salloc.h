// A very simple, quite inneficient, yet very small memory allocator

#ifndef __SALLOC_H__
#define __SALLOC_H__

#include <stddef.h>

void* smalloc( size_t size );
void sfree( void* ptr );
void* scalloc( size_t nmemb, size_t size );
void* srealloc( void* ptr, size_t size );

#endif // #ifndef __SALLOC_H__

