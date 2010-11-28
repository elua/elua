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

#define UDL_MAX_LTR_SYMNAME   ( UDL_MAX_MOD_NAME + 16 )

// Module data structure
typedef struct
{
  char *data;
  u32 offset, total;
  u32 hash;
  u8 refcount;
} udl_module_data;

static udl_module_data udl_modules[ UDL_MAX_MODULES ];
static u32 udl_rotables[ UDL_MAX_MODULES ];
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
    memset( udl_modules + i, 0, sizeof( udl_module_data ) );
    udl_rotables[ i ] = 0;
  }
}

// Find a slot starting from a module name
static int udlh_slot_from_name( const char *modname )
{
  unsigned i;
  udl_module_data *m;

  for( i = 0; i < UDL_MAX_MODULES; i ++ )
  {
    m = udl_modules + i;
    if( m->data && !strcmp( m->data, modname ) )
      return i;
  }
  return -1;
}

// ****************************************************************************
// Public interface

int udl_load( const char *pname )
{
  FILE *fp;
  char *pdata;
  int id;
  u32 temp;
  char ctemp[ UDL_MAX_LTR_SYMNAME + 1 ];
  udl_module_data *m;
  udl_module_data tmpm;

  udl_debug( "Loading module %s\n", pname );
  if( ( fp = fopen( pname, "rb" ) ) == NULL )
  {
    udl_debug( "Cannot read module from file %s\n", pname );
    return UDL_INVALID_MODULE;
  }

  // Read hash
  fread( &tmpm.hash, 1, 4, fp );

  // Read signature
  fread( &temp, 1, 4, fp );
  if( temp != UDL_MOD_SIGN )
  {
    udl_debug( "Invalid module signature\n" );
    fclose( fp );
    return UDL_INVALID_MODULE;
  }

  // Read module data
  fread( &tmpm.total, 1, 4, fp );
  fread( &tmpm.offset, 1, 4, fp );
  udl_debug( "module size: %u offset: %08X\n", ( unsigned )tmpm.total, ( unsigned )tmpm.offset );

  // Read module name (fixed size always)
  fread( ctemp, 1, UDL_MAX_MOD_NAME, fp );
  udl_debug( "module name: %s\n", ctemp );

  // Now it's a good time to check if the module is already loaded
  if( ( id = udlh_slot_from_name( ctemp ) ) != -1 )
  {
    udl_debug( "Module %s already loaded.\n", ctemp );
    if( udl_modules[ id ].hash == tmpm.hash )
    {
      udl_debug( "Modules are identical, incrementing reference count.\n" );
      udl_modules[ id ].refcount ++;
    }
    else
    {
      udl_debug( "Another version of '%s' is already loaded, unable to load the new module.\n", ctemp );
      id = UDL_VERSION_ERROR;
    }
    fclose( fp );
    return id;
  }
  else
  {
    if( ( id = udlh_find_slot() ) == -1 )
    {
      udl_debug( "No slots available\n" );
      fclose( fp );
      return UDL_NO_SLOTS;
    }
  }
  m = udl_modules + id;
  *m = tmpm;
  m->refcount = 1;
  udl_debug( "Will load module at slot %d\n", id );

  // Read all data now
  if( ( pdata = ( char* )malloc( m->total + UDL_MAX_MOD_NAME ) ) == NULL )
  {
    udl_debug( "Not enough memory\n" );
    fclose( fp );
    return UDL_OUT_OF_MEMORY;
  }
  m->data = pdata;
  memcpy( pdata, ctemp, UDL_MAX_MOD_NAME );
  pdata += UDL_MAX_MOD_NAME;
  if( fread( pdata, 1, m->total, fp ) != m->total )
  {
    udl_debug( "Unable to read %u bytes from file\n", ( unsigned )m->total );
    fclose( fp );
    udlh_free_slot( id );
    return UDL_INVALID_MODULE;
  }
  fclose( fp );

  // Is this a LTR-compatible Lua module?
  // It needs two symbols for this: luaopen_<modname> and <modname>_map
  strcpy( ctemp, "luaopen_" );
  strncat( ctemp, ( const char* )m->data, UDL_MAX_LTR_SYMNAME );
  udl_rotables[ id ] = 0;
  if( udl_find_symbol( id, ctemp ) )
  {
    strncpy( ctemp, ( const char* )m->data, UDL_MAX_LTR_SYMNAME );
    strncat( ctemp, "_map", UDL_MAX_LTR_SYMNAME );
    if( ( temp = udl_find_symbol( id, ctemp ) ) != 0 )
    {
      udl_debug( "This is a LTR module\n" );
      // Save the adress of module's rotable in udl_rotables
      udl_rotables[ id ] = temp;
      udl_debug( "ROMTABLE at %X\n", ( unsigned )udl_rotables[ id ] );
    }
  }

#if 0 && defined( UDL_DEBUG )
  // Dump symbol table
  printf( "Symbol table: \n" );
  pdata = m->data + UDL_MAX_MOD_NAME;
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
#endif

  // If the module has an init function, call it now
  if( ( temp = udl_find_symbol( id, UDL_MOD_INIT_FNAME ) ) != 0 )
  {
    p_udl_init_func initf = ( p_udl_init_func )temp;
    if( initf( id ) == 0 )
    {
      udl_debug( "The module init function returned 0, unloading module.\n" );
      udlh_free_slot( id );
      return UDL_INIT_ERROR;
    }
  }
  else
    udl_debug( "the module doesn't have an init function.\n" );

  // Return module slot
  return id;
}

int udl_get_id( u32 pc )
{
  unsigned i;
  udl_module_data *m;

  //udl_debug( "udl_get_id with pc=%08X\n", pc );
  for( i = 0; i < UDL_MAX_MODULES; i ++ )
  {
    m = udl_modules + i;
    if( ( u32 )m->data + UDL_MAX_MOD_NAME < pc && pc < ( u32 )m->data + m->total )
      return i;
  }
  udl_debug( "Unable to find an IDi for addr=%08X\n", ( unsigned )pc );
  return -1;
}

u32 udl_get_offset( int id )
{
  udl_module_data *m = udl_modules + id;

  //udl_debug( "udl_get_offset with id %d: %08X\n", id, ( u32 )m->data + m->offset );
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
  p = m->data + UDL_MAX_MOD_NAME;
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
  udl_module_data *m = udl_modules + id;
  u32 temp;

  udl_debug( "udl_unload called for id=%d\n", id );
  m->refcount --;
  if( m->refcount == 0 )
  {
    udl_debug( "refcount is 0, releasing module.\n" );
    // If the module has a cleanup function, call it now
    if( ( temp = udl_find_symbol( id, UDL_MOD_CLEANUP_FNAME ) ) != 0 )
    {
      p_udl_cleanup_func cleanupf = ( p_udl_cleanup_func )temp;
      if( cleanupf( id ) == 0 )
      {
        // The cleanup function returned 0. This is not necesarily an error, it means
        // that the module can't be unloaded at the moment
        udl_debug( "the module cleanup function returned 0, module can't be unloaded\n" );
        m->refcount = 1;
        return UDL_CANT_UNLOAD;
      }
    }
    else
      udl_debug( "the module doesn't have a cleanup function.\n" );
    udlh_free_slot( id );
  }
  else
    udl_debug( "refcount decreased to %d, module still in memory\n", ( int )m->refcount ); 
  return UDL_OK;
}

void udl_init()
{
  *( u32* )UDL_FTABLE_ADDRESS = ( u32 )udl_functable;
  *( u32* )( UDL_FTABLE_ADDRESS + 4 ) = ( u32 )udl_get_id;
  *( u32* )( UDL_FTABLE_ADDRESS + 8 ) = ( u32 )udl_get_offset;
}

void udl_invalid_symbol()
{
  while( 1 );
}

int udl_ltr_find_next_module( int id )
{
  id ++;
  while( id < UDL_MAX_MODULES )
  {
    if( udl_rotables[ id ] != 0 )
      return id;
    id ++;
  }
  return -1;
}

const char* udl_get_module_name( int id )
{
  if( id >= UDL_MAX_MODULES )
    return NULL;
  return ( const char* )udl_modules[ id ].data;
}

void* udl_ltr_get_rotable( int id )
{
  return id >= UDL_MAX_MODULES ? NULL : ( void* )udl_rotables[ id ];
}


int udl_ltr_is_rotable( void* p )
{
  unsigned i;
  u32 np = ( u32 )p;

  for( i = 0; i < UDL_MAX_MODULES; i ++ )
    if( np == udl_rotables[ i ] )
    {
      //printf( "udl_ltr_is_rotable: table %p found rotable at %d\n", p, i );
      return 1;
    }
  return 0;
}

#else // #ifdef BUILD_UDL

void udl_init()
{
}

int udl_load( const char *pname )
{
  return UDL_NOT_IMPLEMENTED;
}

int udl_ltr_find_next_module( int id )
{
  return -1;
}

const char* udl_get_module_name( int id )
{
  return NULL;
}

void* udl_ltr_get_rotable( int id )
{
  return NULL;
}

int udl_ltr_is_rotable( void* p )
{
  return 0;
}

#endif // #ifdef BUILD_UDL

