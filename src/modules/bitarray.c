  // Module that implements a fixed size bit array

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "type.h"
#include "auxmods.h"
#include "lrotable.h"
#include <string.h>

#define META_NAME                 "eLua.bitarray"
#define bitarray_check( L )      ( bitarray_t* )luaL_checkudata( L, 1, META_NAME )
#define ROUND_SIZE(s)            ( ( ( s ) >> 3 ) + ( ( s ) & 7 ? 1 : 0 ) )

// Unpack modes
enum
{
  BITARRAY_UNPACK_RAW = 0,
  BITARRAY_UNPACK_SEQ
};
 
// Structure that describes our array
typedef struct
{
  u32 capacity;
  u8 elsize;
  u8 values[ 1 ];
} bitarray_t;

// Index shift values/masks
static const u8 bitarray_index_shift[] = { 0, 3, 2, 0, 1 };
static const u8 bitarray_index_mask[] = { 0, 0x01, 0x03, 0, 0x0F };

// Lua: array = bitarray.new( capacity, [element_size_bits], [fill] ), or
//      array = bitarray.new( "string", [element_size_bits] ), or
//      array = bitarray.new( lua_array, [element_size_bits] )
static int bitarray_new( lua_State *L )
{
  u32 total, capacity;
  u8 elsize, fill = 0, fromarray = 0;
  const char *buf = NULL;
  bitarray_t *pa;
  size_t temp;
   
  if( lua_isnumber( L, 1 ) )
  {
    // capacity, [element_size_bits], [fill]
    capacity = ( u32 )luaL_checkinteger( L, 1 );
    if( lua_isnumber( L, 2 ) )
      elsize = luaL_checkinteger( L, 2 );
    else
      elsize = 8;
    if( lua_isnumber( L, 3 ) )
      fill = luaL_checkinteger( L, 3 );  
  }
  else if( lua_isstring( L, 1 ) || lua_istable( L, 1 ) || lua_isrotable( L, 1 ) )
  {
    // string, [element_size_bits] OR (ro)table, [element_size_bits]
    if( lua_isstring( L, 1 ) )
      buf = lua_tolstring( L, 1, &temp );
    else
    {
      temp = lua_objlen( L, 1 );
      fromarray = 1;
    }
    if( lua_isnumber( L, 2 ) )
      elsize = luaL_checkinteger( L, 2 );
    else
      elsize = 8;
    if( elsize == 0 )
      return luaL_error( L, "length is zero." );
    if( ( temp << 3 ) % elsize )
      return luaL_error( L, "length is not a multiple of element size." );
    capacity = ( u32 )( temp << 3 ) / elsize;  
  }
  else 
    return luaL_error( L, "invalid arguments." );
    
  if( elsize <= 0 || ( elsize > 32 ) || ( elsize & ( elsize - 1 ) ) )
    return luaL_error( L, "invalid element size." );
  total = ROUND_SIZE( capacity * elsize );
  if( total <= 0 )
    return luaL_error( L, "invalid arguments.");
  pa = ( bitarray_t* )lua_newuserdata( L, sizeof( bitarray_t ) + total - 1 );
  pa->capacity = capacity;
  pa->elsize = elsize;
  
  if( buf )
    memcpy( pa->values, buf, temp );
  else if( fromarray )
  {
    for( total = 1; total <= temp; total ++ )
    {
      lua_rawgeti( L, 1, total );
      pa->values[ total - 1 ] = lua_tointeger( L, -1 );
      lua_pop( L, 1 );
    }
  }
  else
    memset( pa->values, fill, total );
  luaL_getmetatable( L, META_NAME );
  lua_setmetatable( L, -2 );
  return 1;
}

// Helper: get the value at the given index
static u32 bitarray_getval( bitarray_t *pa, u32 idx )
{
  u32 shift, val = 0;
  u8 rest, mask;
  
  idx --;
  if( pa->elsize < 8 )        // sub-byte elements
  {
    shift = idx >> bitarray_index_shift[ pa->elsize ];
    mask = 1 << bitarray_index_shift[ pa->elsize ];
    rest = idx & ( mask - 1 );
    val = pa->values[ shift ];
    val = ( val >> ( ( mask - 1 - rest ) * pa->elsize ) ) & bitarray_index_mask[ pa->elsize ]; 
  }
  else      // one byte or more elements
    switch( pa->elsize )
    {
      case 8:
        val = pa->values[ idx ];
        break;
        
      case 16:
        val = *( ( u16* )pa->values + idx );
        break;
        
      case 32:
        val = *( ( u32* )pa->values + idx );
        break;  
    }
  return val;
}

// Lua: value = array[ idx ]
static int bitarray_get( lua_State *L )
{
  bitarray_t *pa;
  u32 idx;
   
  pa = bitarray_check( L );
  idx = ( u32 )luaL_checkinteger( L, 2 );
  if( ( idx <= 0 ) || ( idx > pa->capacity ) )
    return luaL_error( L, "invalid index." );
  lua_pushinteger( L, bitarray_getval( pa, idx ) );    
  return 1;
}

// Lua: array[ key ] = value
static int bitarray_set( lua_State *L )
{
  bitarray_t *pa;
  u32 idx, shift, newval, val = 0;
  u8 rest, mask;
   
  pa = bitarray_check( L );
  idx = ( u32 )luaL_checkinteger( L, 2 );
  newval = ( u32 )luaL_checkinteger( L, 3 );
  if( ( idx <= 0 ) || ( idx > pa->capacity ) )
    return luaL_error( L, "invalid index." );
  idx --;
  if( pa->elsize < 8 )        // sub-byte elements
  {
    shift = idx >> bitarray_index_shift[ pa->elsize ];
    mask = 1 << bitarray_index_shift[ pa->elsize ];
    rest = idx & ( mask - 1 );
    val = pa->values[ shift ];
    val &= ~( bitarray_index_mask[ pa->elsize ] << ( ( mask - 1 - rest ) * pa->elsize ) );
    val |= newval << ( ( mask - 1 - rest ) * pa->elsize );
    pa->values[ shift ] = val; 
  }
  else      // one byte or more elements
    switch( pa->elsize )
    {
      case 8:
        pa->values[ idx ] = ( u8 )newval;
        break;
        
      case 16:
        *( ( u16* )pa->values + idx ) = ( u16 )newval;
        break;
        
      case 32:
        *( ( u32* )pa->values + idx ) = ( u32 )newval;
        break;  
    }    
  return 0;
}

// Lua : size = #array
static int bitarray_len( lua_State *L )
{
  bitarray_t *pa;
  
  pa = bitarray_check( L );
  lua_pushinteger( L, pa->capacity );
  return 1;
}

// Lua iterator
static int bitarray_iter( lua_State *L )
{
  bitarray_t *pa;
  u32 idx;
  
  pa = bitarray_check( L );
  idx = ( u32 )luaL_checkinteger( L, 2 ) + 1;
  if( idx <= pa->capacity )
  {
    lua_pushinteger( L, idx );
    lua_pushinteger( L, bitarray_getval( pa, idx ) );
    return 2;
  }
  else
    return 0;
}

// Lua iterator "factory"
static int bitarray_pairs( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  lua_pushlightfunction( L, bitarray_iter );
#else
  lua_pushcclosure( L, bitarray_iter, 0 );
#endif
  lua_pushvalue( L, 1 );
  lua_pushinteger ( L, 0 );
  return 3;
}

// Lua: string = bitarray.tostring( array, ["raw"|"seq"] )
static int bitarray_tostring( lua_State *L )
{
  luaL_Buffer b;
  bitarray_t *pa;
  u32 idx;
  u8 mode = BITARRAY_UNPACK_SEQ;
  const char *ptextmode;
   
  pa = bitarray_check( L );
  if( lua_isstring( L, 2 ) )
  {
    ptextmode = lua_tostring( L, 2 );
    if( !strcmp( ptextmode, "raw" ) )
      mode = BITARRAY_UNPACK_RAW;
    else if( !strcmp( ptextmode, "seq" ) )
      mode = BITARRAY_UNPACK_SEQ;
    else
      return luaL_error( L, "invalid mode string." );
  }
  if( ( mode == BITARRAY_UNPACK_SEQ ) && ( pa->elsize > 8 ) )
    return luaL_error( L, "element size too large." );
  luaL_buffinit( L, &b );
  if( mode == BITARRAY_UNPACK_SEQ )
    for( idx = 1; idx <= pa->capacity; idx  ++ )
      luaL_addchar( &b, bitarray_getval( pa, idx ) );
  else
    luaL_addlstring( &b, ( char* )pa->values, ROUND_SIZE( pa->capacity * pa->elsize ) );
  luaL_pushresult( &b );
  return 1;  
}

// Lua: table = bitarray.totable( array, ["raw"|"seq"] )
static int bitarray_totable( lua_State *L )
{
  bitarray_t *pa;
  u32 idx;
  u8 mode = BITARRAY_UNPACK_SEQ;
  const char *ptextmode;
   
  pa = bitarray_check( L );
  if( lua_isstring( L, 2 ) )
  {
    ptextmode = lua_tostring( L, 2 );
    if( !strcmp( ptextmode, "raw" ) )
      mode = BITARRAY_UNPACK_RAW;
    else if( !strcmp( ptextmode, "seq" ) )
      mode = BITARRAY_UNPACK_SEQ;
    else
      return luaL_error( L, "invalid mode string." );
  }
  if( ( mode == BITARRAY_UNPACK_SEQ ) && ( pa->elsize > 8 ) )
    return luaL_error( L, "element size too large." );
  lua_newtable( L );    
  if( mode == BITARRAY_UNPACK_SEQ )
    for( idx = 1; idx <= pa->capacity; idx ++ )
    {
      lua_pushinteger( L, bitarray_getval( pa, idx ) );
      lua_rawseti( L, -2, idx );
    }      
  else
    for( idx = 0; idx < ROUND_SIZE( pa->capacity * pa->elsize ); idx ++ )
    {
      lua_pushinteger( L, pa->values[ idx ] );
      lua_rawseti( L, -2, idx + 1 );
    }
  return 1;  
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE bitarray_map[] = 
{
  { LSTRKEY( "new" ), LFUNCVAL( bitarray_new ) },
  { LSTRKEY( "pairs" ), LFUNCVAL( bitarray_pairs ) },
  { LSTRKEY( "tostring" ), LFUNCVAL( bitarray_tostring ) },
  { LSTRKEY( "totable" ), LFUNCVAL( bitarray_totable ) },
  { LNILKEY, LNILVAL } 
};

static const LUA_REG_TYPE bitarray_mt_map[] = 
{
  { LSTRKEY( "__index" ), LFUNCVAL( bitarray_get ) },
  { LSTRKEY( "__newindex" ), LFUNCVAL( bitarray_set ) },
  { LSTRKEY( "__len" ), LFUNCVAL( bitarray_len ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_bitarray( lua_State* L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  luaL_rometatable( L, META_NAME, ( void* )bitarray_mt_map );
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_newmetatable( L, META_NAME );
  luaL_register( L, NULL, bitarray_mt_map );
  luaL_register( L, AUXLIB_BITARRAY, bitarray_map );  
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
