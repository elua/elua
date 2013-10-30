// A very simple, quite inneficient, yet very small memory allocator
// It can do both fixed block and variable block allocation

#ifdef USE_SIMPLE_ALLOCATOR

#include <stddef.h>
#include <string.h>
#include "platform.h"
#include "platform_conf.h"

// Macros for the dynamic size allocator
// Dynamic structure: pointer to next, pointer to prev
// First bit of pointer is 0 if block free, 1 if block taken
// Pointer must be multiplied by DYN_SIZE_MULT to get actual address
// There are two 'guards' (at the beginning and at the end) 
#define DYN_SIZE_MULT           8
#define DYN_SIZE_MULT_SHIFT     3
#define DYN_HEADER_SIZE         8
#define DYN_MIN_SPLIT_SIZE      16

static u8 s_initialized;

// ****************************************************************************
// Utility functions for the dynamic memory allocator

// Get actual size
static size_t s_act_size( size_t size )
{
  if( size & ( DYN_SIZE_MULT - 1 ) )
    size = ( ( size >> DYN_SIZE_MULT_SHIFT ) + 1 ) << DYN_SIZE_MULT_SHIFT;
  return size;
}

// Get next block
static char* s_get_next_block( char* ptr )
{
  return ( char* )( ( *( u32* )ptr & 0x7FFFFFFF ) << DYN_SIZE_MULT_SHIFT );
}

// Set next block
static void s_set_next_block( char* ptr, char* next )
{
  u32 *temp = ( u32* )ptr;
  
  *temp = ( *temp & 0x80000000 ) | ( ( u32 )next >> DYN_SIZE_MULT_SHIFT );
}

// Get prev block
static char* s_get_prev_block( char* ptr )
{
  return ( char* )( ( *( ( u32* )ptr + 1 ) & 0x7FFFFFFF ) << DYN_SIZE_MULT_SHIFT );
}

// Set prev block
static void s_set_prev_block( char* ptr, char* prev )
{
  u32 *temp = ( u32* )ptr + 1;
   
  *temp = ( *temp & 0x80000000 ) | ( ( u32 )prev >> DYN_SIZE_MULT_SHIFT );
}

// Get block size
static size_t s_get_block_size( char* ptr )
{
  char* next = s_get_next_block( ptr );
  
  return next != NULL ? ( size_t )( next - ptr ) : 0;
}

// Mark block as taken
static void s_mark_block_taken( char* where )
{
  *( u32* )where |= 0x80000000;
}

// Mark block as free
static void s_mark_block_free( char* where )
{
  *( u32* )where &= 0x7FFFFFFF;
}

// Is the block free?
static int s_is_block_free( char* where )
{
  return ( *( u32* )where & 0x80000000 ) == 0;
}

// Create a new block with the given neighbours
static void s_create_new_block( char* where, char* next, char* prev )
{
  u32* temp = ( u32* )where;
  
  *temp ++ = ( u32 )next >> DYN_SIZE_MULT_SHIFT;
  *temp = ( u32 )prev >> DYN_SIZE_MULT_SHIFT;
}

// Tries to compact free blocks
static void s_compact_free( char* ptr )
{
  char *temp1, *temp2;
  
  s_mark_block_free( ptr );  
  // Look for free blocks before and after, concatenate if possible
  temp1 = temp2 = ptr;
  while( s_is_block_free( temp1 ) )
    temp1 = s_get_prev_block( temp1 );
  temp1 = s_get_next_block( temp1 );      
  while( s_is_block_free( temp2 ) )
    temp2 = s_get_next_block( temp2 );    
  if( temp1 != ptr || s_get_prev_block( temp2 ) != ptr )
  {
    s_set_next_block( temp1, temp2 );
    s_set_prev_block( temp2, temp1 );
  }
}

// Utility function: find a free block in the dynamic memory part
// Returns pointer to block for success, NULL for error
static void* s_get_free_block( size_t size, void* pstart )
{
  char *temp, *pblock = NULL, *next;
  size_t minsize = ( size_t )~0, bsize;
  
  if( !size )
    return NULL;
  size = s_act_size( size + DYN_HEADER_SIZE );  
  temp = s_get_next_block( pstart );
  // Best-fit only for now
  while( temp )
  {
    if( s_is_block_free( temp ) )
    {
      bsize = s_get_block_size( temp );      
      if( ( size <= bsize ) && ( bsize < minsize ) )
      {
        minsize = bsize;
        pblock = temp;
      }
    }
    temp = s_get_next_block( temp );
  }
  if( pblock == NULL )
    return NULL;
  s_mark_block_taken( pblock );
  if( minsize > size && ( minsize - size ) >= DYN_MIN_SPLIT_SIZE )
  {
    temp = pblock + size;
    next = s_get_next_block( pblock );
    s_set_prev_block( temp, pblock );
    s_set_next_block( temp, next );
    s_set_prev_block( next, temp );
    s_set_next_block( pblock, temp );
    s_compact_free( temp );
  }
  return pblock + DYN_HEADER_SIZE;
}

// Utility function: free a memory block
static void s_free_block( char* ptr )
{
  ptr -= DYN_HEADER_SIZE;
  s_compact_free( ptr );
}

// Get 'real' block size
static size_t s_get_actual_block_size( char* ptr )
{
  return s_get_block_size( ptr - DYN_HEADER_SIZE ) - DYN_HEADER_SIZE;
}

// Shrinks the given block to its new size
static void s_shrink_block( char* pblock, size_t size )
{
  char *temp, *next;
  
  pblock -= DYN_HEADER_SIZE;
  size = s_act_size( size + DYN_HEADER_SIZE );  
  if( size >= s_get_block_size( pblock ) || ( s_get_block_size( pblock ) - size ) < DYN_MIN_SPLIT_SIZE )
    return;
  temp = pblock + size;
  next = s_get_next_block( pblock );
  s_set_prev_block( temp, pblock );
  s_set_next_block( temp, next );
  s_set_prev_block( next, temp );
  s_set_next_block( pblock, temp );    
  s_compact_free( temp );
}

static void s_init()
{
  unsigned i = 0;
  size_t memspace;
  char *crt, *g1, *g2, *pstart;

  while( ( pstart = platform_get_first_free_ram( i ) ) != NULL )
  {
    memspace = ( u32 )platform_get_last_free_ram( i ) - ( u32 )pstart;
    g1 = ( char* )pstart;
    crt = g1 + DYN_SIZE_MULT;
    g2 = g1 + memspace - DYN_SIZE_MULT;
    s_create_new_block( g1, crt, NULL );
    s_create_new_block( crt, g2, g1 );
    s_create_new_block( g2, NULL, crt );
    s_mark_block_taken( g1 );
    s_mark_block_taken( g2 );    
    s_mark_block_free( crt );
    i ++;
  }   
  s_initialized = 1;
}

// ****************************************************************************

void* smalloc( size_t size )
{
  unsigned i = 0;
  void *ptr = NULL, *pstart;

  if( !s_initialized )
    s_init();
  while( ( pstart = platform_get_first_free_ram( i ++ ) ) != NULL )
    if( ( ptr = s_get_free_block( size, pstart ) ) != NULL )
      break;
  return ptr;
}

void sfree( void* ptr )
{
  if( !ptr || !s_initialized )
    return;
  s_free_block( ptr );
}

void* scalloc( size_t nmemb, size_t size )
{
  void* ptr;

  if( !s_initialized )
    s_init();
  if( ( ptr = smalloc( nmemb * size ) ) != NULL )
    memset( ptr, 0, nmemb * size );
  return ptr;
}

void* srealloc( void* ptr, size_t size )
{
  void* newptr = NULL;

  if( !s_initialized )
    s_init();
  // Special cases:
  // realloc with ptr == NULL -> malloc
  // realloc with size == 0 -> free
  if( ptr == NULL )
    return smalloc( size );
  else if( size == 0 )
  {
    sfree( ptr );
    return NULL;
  }

  // Test new size versus the old size
  if( s_get_actual_block_size( ptr ) == size )
    return ptr;
  else if( size < s_get_actual_block_size( ptr ) )
  {
    s_shrink_block( ptr, size );
    return ptr;
  }
  else
  {
    if( ( newptr = smalloc( size ) ) == NULL )
      return NULL;
    memmove( newptr, ptr, s_get_actual_block_size( ptr ) );
    sfree( ptr );
  }
  return newptr;
}

#endif // #ifdef USE_SIMPLE_ALLOCATOR

