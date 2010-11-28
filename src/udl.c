// eLua micro dynamic loader (udl)

#include "platform_conf.h"
#ifdef BUILD_UDL

#include "udl.h"
#include "type.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ***************************************************************************
// Internal data structures and variables

#define UDL_NAME_OFFSET       8

typedef struct
{
  char *data;
  const char* psymtable;
  u32 offset, total;
} udl_module_data;

static udl_module_data udl_modules[ UDL_MAX_MODULES ];
extern u32 udl_functable[];

#define UDL_DEBUG

#ifdef UDL_DEBUG
#define udl_debug printf
#else
#define udl_debug dummy
static void dummy( const char* p, ... )
{
}
#endif

// Find a free slot
static int udlh_find_slot()
{
  unsigned i;

  for( i = 0; i < UDL_MAX_MODULES; i ++ )
    if( udl_modules[ i ].data == NULL )
      return i;
  return -1;      
}

// Free a slot
static void udlh_free_slot( unsigned i )
{
  if( udl_modules[ i ].data )
  {
    free( udl_modules[ i ].data );
    udl_modules[ i ].data = NULL;
    udl_modules[ i ].total = 0;
  }
}

// ****************************************************************************
// Public interface

int udl_load( const char *pname )
{
  FILE *fp;
  long size;
  char *pdata, *orig;
  int id;
  udl_module_data *m;
  u32 temp;

  udl_debug( "Loading module %s\n", pname );

  // Find a free slot
  if( ( id = udlh_find_slot() ) == -1 )
  {
    udl_debug( "No slots available\n" );
    return UDL_NO_SLOTS;
  }
  udl_debug( "Will read module on slot %d\n", id );
  m = udl_modules + id;

  // Read module from file
  // [UDLTODO] pseudo-mmap for XIP!
  fp = fopen( pname, "rb" );
  fseek( fp, 0, SEEK_END );
  size = ftell( fp );
  fseek( fp, 0, SEEK_SET );
  if( ( pdata = ( char* )malloc( size ) ) == NULL )
  {
    udl_debug( "Not enough memory\n" );
    return UDL_OUT_OF_MEMORY;
  }
  fread( pdata, size, 1, fp );
  fclose( fp );
  orig = pdata;
  m->data = pdata;
  m->total = size;
  udl_debug( "Memory range: %08X to %08X\n", ( unsigned int )pdata, ( unsigned int )pdata + size );
  
  // Check if the module is valid
  temp = *( u32* )pdata;
  if( temp != UDL_MOD_SIGN )
  {
    udl_debug( "Invalid module signature\n" );
    return UDL_INVALID_MODULE;
  }
  udl_debug( "Got signature\n" );

  // Read program offset
  pdata += 4;
  temp = *( u32 *)pdata;
  udl_debug( "Got program offset %08X\n", ( unsigned )temp );
  m->offset = temp;

  // Read module name
  pdata += 4;
  udl_debug( "Module name is %s\n", pdata );
  pdata += strlen( pdata ) + 1;
  m->psymtable = pdata;

#ifdef UDL_DEBUG 
  // Dump symbol table
  printf( "Symbol table: \n" );
  while( 1 )
  {
    if( *pdata == '\0' )
    {
      pdata ++;
      break;
    }
    printf( "  name: %s\t\t ", pdata );
    pdata += strlen( pdata ) + 1;
    pdata = ( char* )( ( ( u32 )pdata + 3 ) & ~3 );
    printf( "offset: %08X\n", ( unsigned )*( u32* )pdata );
    pdata += 4;
  }
  pdata = ( char* )( ( ( u32 )pdata + 3 ) & ~3 );
  printf( "Offset of binary image: %d\n", pdata - orig );
#endif

  return UDL_OK;
}

int udl_get_id( u32 pc )
{
  unsigned i;
  udl_module_data *m;

  //udl_debug( "udl_get_id with pc=%08X\n", pc );
  for( i = 0; i < UDL_MAX_MODULES; i ++ )
  {
    m = udl_modules + i;
    if( ( u32 )m->data < pc && pc < ( u32 )m->data + m->total )
      return i;
  }
  //udl_debug( "Unable to find an ID\n" );
  return -1;
}

u32 udl_get_offset( int id )
{
  udl_module_data *m = udl_modules + id;

  udl_debug( "udl_get_offset with id %d: %08X\n", id, ( u32 )m->data + m->offset );
  return ( u32 )m->data + m->offset;
}

/*u32 udl_find_symbol( const char *pname, const char *symname )
{
  unsigned i;
  udl_module_data *m;
  const char *p;
  int found = 0;

  for( i = 0; i < UDL_MAX_MODULES; i ++ )
  {
    m = udl_modules + i;
    if( m->data && !strcmp( pname, ( const char *)m->data + UDL_NAME_OFFSET ) )
    {
      p = m->psymtable;
      while( 1 )
      {
        if( *p == '\0' )
          return 0;
        found = !strcmp( p, symname );
        p += strlen( p ) + 1;
        p = ( const char* )( ( ( u32 )p + 3 ) & ~3 );
        if( found )
          return *( u32* )p + udl_get_offset( i );
        p += 4;
      }
    }
  }
  return 0;
}*/

u32 udl_find_symbol( int id, const char *symname )
{
  int found = 0;
  udl_module_data *m = udl_modules + id;
  const char *p;

  if( m->data == 0 )
    return 0;
  p = m->psymtable;
  while( 1 )
  {
    if( *p == '\0' )
      return 0;
    found = !strcmp( p, symname );
    p += strlen( p ) + 1;
    p = ( const char* )( ( ( u32 )p + 3 ) & ~3 );
    if( found )
      return *( u32* )p + udl_get_offset( id );
    p += 4;
  }
  return 0;
}

int udl_unload( int id )
{
  udl_debug( "udl_unload called for id=%d\n", id );
  udlh_free_slot( id );
  return UDL_OK;
}

void udl_init()
{
  *( u32* )UDL_FTABLE_ADDRESS = ( u32 )udl_functable;
  *( u32* )( UDL_FTABLE_ADDRESS + 4 ) = ( u32 )udl_get_id;
  *( u32* )( UDL_FTABLE_ADDRESS + 8 ) = ( u32 )udl_get_offset;
}

// Loookup a symbol in the eLua symbol table
u32 udl_get_elua_symbol( u32 idx )
{
  return udl_functable[ idx ];
}

void udl_invalid_symbol()
{
  while( 1 );
}

#else // #ifdef BUILD_UDL

void udl_init()
{
}

int udl_load( const char *pname )
{
  return UDL_NOT_IMPLEMENTED;
}

#endif // #ifdef BUILD_UDL

