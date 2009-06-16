/*****************************************************************************
* Lua-RPC library, Copyright (C) 2001 Russell L. Smith. All rights reserved. *
*   Email: russ@q12.org   Web: www.q12.org                                   *
* For documentation, see http://www.q12.org/lua. For the license agreement,  *
* see the file LICENSE that comes with this distribution.                    *
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifndef LUA_CROSS_COMPILER
#include "platform.h"
#include "platform_conf.h"
#endif
#include "lrotable.h"

#include "luarpc_rpc.h"

#ifdef BUILD_LUARPC

/* Support for Compiling with rotables */
#ifdef LUA_OPTIMIZE_MEMORY
#define LUA_ISCALLABLE(state, idx) ( lua_isfunction( state, idx ) || lua_islightfunction( state, idx ) )
#else
#define LUA_ISCALLABLE(state, idx) lua_isfunction( state, idx )
#endif

struct exception_context the_exception_context[ 1 ];

static void errorMessage (const char *msg, va_list ap)
{
  fflush (stdout);
  fflush (stderr);
  fprintf (stderr,"\nError: ");
  vfprintf (stderr,msg,ap);
  fprintf (stderr,"\n\n");
  fflush (stderr);
}


DOGCC(static void panic (const char *msg, ...)
      __attribute__ ((noreturn,unused));)
static void panic (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg);
  errorMessage (msg,ap);
  exit (1);
}


DOGCC(static void rpcdebug (const char *msg, ...)
      __attribute__ ((noreturn,unused));)
static void rpcdebug (const char *msg, ...)
{
  va_list ap;
  va_start (ap,msg);
  errorMessage (msg,ap);
  abort();
}

/* return a string representation of an error number */

static const char * errorString (int n)
{
  switch (n) {
  case ERR_EOF: return "connection closed unexpectedly (\"end of file\")";
  case ERR_CLOSED: return "operation requested on a closed transport";
  case ERR_PROTOCOL: return "error in the received LuaRPC protocol";
  case ERR_COMMAND: return "undefined RPC command";
  case ERR_DATALINK: return "transmission error at data link level";
  case ERR_NODATA: return "no data received when attempting to read";
  case ERR_BADFNAME: return "function name is too long";
  default: return transport_strerror (n);
  }
}


/****************************************************************************/
/* transport layer generics */

/* read from the transport into a string buffer. */

static void transport_read_string( Transport *tpt, const char *buffer, int length )
{
  transport_read_buffer( tpt, ( u8 * )buffer, length );
}


/* write a string buffer to the transport */

static void transport_write_string( Transport *tpt, const char *buffer, int length )
{
  transport_write_buffer( tpt, ( u8 * )buffer, length );
}


/* read a u8 from the transport */

static u8 transport_read_u8( Transport *tpt )
{
  u8 b;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  transport_read_buffer( tpt, &b, 1 );
  return b;
}


/* write a u8 to the transport */

static void transport_write_u8( Transport *tpt, u8 x )
{
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  transport_write_buffer (tpt,&x,1);
}


/* read a u32 from the transport */

static u32 transport_read_u32( Transport *tpt )
{
  u8 b[4];
  u32 i;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  transport_read_buffer ( tpt, b, 4 );
  i = ( b[ 0 ] << 24 ) | (b[ 1 ] << 16 ) | ( b[ 2 ] << 8) | b[ 3 ];
  return i;
}


/* write a u32 to the transport */

static void transport_write_u32 (Transport *tpt, u32 x)
{
  u8 b[4];
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  b[0] = x >> 24;
  b[1] = x >> 16;
  b[2] = x >> 8;
  b[3] = x;
  transport_write_buffer( tpt, b, 4 );
}


/* Represent doubles as byte string */
union DoubleBytes {
  double d;
  u8 b[ sizeof( double ) ];
};

/* read a double from the transport */

static double transport_read_double (Transport *tpt)
{
  union DoubleBytes double_bytes;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  /* @@@ handle endianness */
  transport_read_buffer ( tpt,double_bytes.b, sizeof( double ) );
  return double_bytes.d;
}


/* write a double to the transport */

static void transport_write_double (Transport *tpt, double x)
{
  union DoubleBytes double_bytes;
  struct exception e;
  TRANSPORT_VERIFY_OPEN;
  /* @@@ handle endianness */
  double_bytes.d = x;
  transport_write_buffer( tpt,double_bytes.b, sizeof( double ) );
}



/****************************************************************************/
/* lua utility */

/* replacement for lua_error that resets the exception stack before leaving
 * Lua-RPC.
 */

void my_lua_error( lua_State *L, const char *errmsg )
{
  lua_pushstring( L, errmsg );
  lua_error( L );
}

int check_num_args( lua_State *L, int desired_n )
{
  int n = lua_gettop( L );   /* number of arguments on stack */
  if ( n != desired_n )
  {
    char s[ 100 ]; /* @@@ can we cut this down? */
    sprintf( s, "must have %d argument%c", desired_n,
       ( desired_n == 1 ) ? '\0' : 's' );
    my_lua_error( L, s );
  }
  return n;
}

static int ismetatable_type( lua_State *L, int ud, const char *tname )
{
  if( lua_getmetatable( L, ud ) ) {  /* does it have a metatable? */
    lua_getfield( L, LUA_REGISTRYINDEX, tname );  /* get correct metatable */
    if( lua_rawequal( L, -1, -2 ) ) {  /* does it have the correct mt? */
      lua_pop( L, 2 );  /* remove both metatables */
      return 1;
    }
  }
  return 0;
}



/****************************************************************************/
/* read and write lua variables to a transport.
 * these functions do little error handling of their own, but they call transport
 * functions which may throw exceptions, so calls to these functions must be
 * wrapped in a Try block.
 */

enum {
  RPC_NIL=0,
  RPC_NUMBER,
  RPC_BOOLEAN,
  RPC_STRING,
  RPC_TABLE,
  RPC_TABLE_END,
  RPC_FUNCTION,
  RPC_FUNCTION_END
};

enum
{
  RPC_CMD_CALL = 1,
  RPC_CMD_GET,
  RPC_CMD_CON,
  RPC_CMD_NEWINDEX
};

enum { RPC_PROTOCOL_VERSION = 3 };

/* write a table at the given index in the stack. the index must be absolute
 * (i.e. positive).
 */

static void write_variable( Transport *tpt, lua_State *L, int var_index );
static int read_variable( Transport *tpt, lua_State *L );

static void write_table( Transport *tpt, lua_State *L, int table_index )
{
  lua_pushnil( L );  /* push first key */
  while ( lua_next( L, table_index ) ) 
  {
    /* next key and value were pushed on the stack */
    write_variable( tpt, L, lua_gettop( L ) - 1 );
    write_variable( tpt, L, lua_gettop( L ) );
    /* remove value, keep key for next iteration */
    lua_pop( L, 1 );
  }
}

static int writer( lua_State *L, const void* b, size_t size, void* B ) {
  (void)L;
  luaL_addlstring((luaL_Buffer*) B, (const char *)b, size);
  return 0;
}

static void write_function( Transport *tpt, lua_State *L, int var_index )
{
  luaL_Buffer b;
  
  /* push function onto stack, serialize to string */
  lua_pushvalue( L, var_index );
  luaL_buffinit( L, &b );
  lua_dump( L, writer, &b ); /* deal with errors thrown at this level? */
  /* put string representation on stack and send it */
  luaL_pushresult( &b );
  write_variable( tpt, L, lua_gettop( L ) );
  
  /* Remove function & dumped string from stack*/
  lua_pop( L, 2 );
}



/* write a variable at the given index in the stack. the index must be absolute
 * (i.e. positive).
 */

static void write_variable( Transport *tpt, lua_State *L, int var_index )
{
  int stack_at_start = lua_gettop( L );
  
  switch( lua_type( L, var_index ) )
  {
    case LUA_TNUMBER:
      transport_write_u8( tpt, RPC_NUMBER );
      transport_write_double( tpt, lua_tonumber( L, var_index ) );
      break;

    case LUA_TSTRING:
    {
      const char *s;
      u32 len;
      transport_write_u8( tpt, RPC_STRING );
      s = lua_tostring( L, var_index );
      len = lua_strlen( L, var_index );
      transport_write_u32( tpt, len );
      transport_write_string( tpt, s, len );
      break;
    }

    case LUA_TTABLE:
      transport_write_u8( tpt, RPC_TABLE );
      write_table( tpt, L, var_index );
      transport_write_u8( tpt, RPC_TABLE_END );
      break;

    case LUA_TNIL:
      transport_write_u8( tpt, RPC_NIL );
      break;

    case LUA_TBOOLEAN:
      transport_write_u8( tpt,RPC_BOOLEAN );
      transport_write_u8( tpt, ( u8 )lua_toboolean( L, var_index ) );
      break;

    case LUA_TFUNCTION:
      transport_write_u8( tpt, RPC_FUNCTION );
      write_function( tpt, L, var_index );
      transport_write_u8( tpt, RPC_FUNCTION_END );
      break;

    case LUA_TUSERDATA:
      my_lua_error( L, "can't pass user data to a remote function" );
      break;

    case LUA_TTHREAD:
      my_lua_error( L, "can't pass threads to a remote function" );
      break;

    case LUA_TLIGHTUSERDATA:
      my_lua_error( L, "can't pass light user data to a remote function" );
      break;
  }
  MYASSERT( lua_gettop( L ) == stack_at_start );
}


/* read a table and push in onto the stack */

static void read_table( Transport *tpt, lua_State *L )
{
  int table_index;
  lua_newtable( L );
  table_index = lua_gettop( L );
  for ( ;; ) 
  {
    if( !read_variable( tpt, L ) )
      return;
    read_variable( tpt, L );
    lua_rawset( L, table_index );
  }
}

static void read_function( Transport *tpt, lua_State *L )
{
  const char *b;
  size_t len;
  
  for( ;; )
  {
    if( !read_variable( tpt, L ) )
      return;

    b = luaL_checklstring( L, -1, &len );
    luaL_loadbuffer( L, b, len, b );
    lua_insert( L, -2 );
    lua_pop( L, 1 );
  }
}


/* read a variable and push in onto the stack. this returns 1 if a "normal"
 * variable was read, or 0 if an end-table marker was read (in which case
 * nothing is pushed onto the stack).
 */

static int read_variable( Transport *tpt, lua_State *L )
{
  struct exception e;
  u8 type = transport_read_u8( tpt );

  switch( type )
  {
    case RPC_NIL:
      lua_pushnil( L );
      break;

    case RPC_BOOLEAN:
      lua_pushboolean( L, transport_read_u8( tpt ) );
      break;

    case RPC_NUMBER:
      lua_pushnumber( L, transport_read_double( tpt ) );
      break;

    case RPC_STRING:
    {
      u32 len = transport_read_u32( tpt );
      char *s = ( char * )alloca( len + 1 );
      transport_read_string( tpt, s, len );
      s[ len ] = 0;
      lua_pushlstring( L, s, len );
      break;
    }

    case RPC_TABLE:
      read_table( tpt, L );
      break;

    case RPC_TABLE_END:
      return 0;

    case RPC_FUNCTION:
      read_function( tpt, L );
      break;
    
    case RPC_FUNCTION_END:
      return 0;

    default:
      e.errnum = ERR_PROTOCOL;
      e.type = fatal;
      Throw( e );
  }
  return 1;
}


/****************************************************************************/
/* rpc utility */

/* functions for sending and receving headers
 */

static void write_header( Transport *tpt )
{
  char header[ 5 ];

  /* write the protocol header */
  header[0] = 'L';
  header[1] = 'R';
  header[2] = 'P';
  header[3] = 'C';
  header[4] = RPC_PROTOCOL_VERSION;

  transport_write_string( tpt, header, sizeof( header ) );
}

static void read_header( Transport *tpt )
{
  struct exception e;
  char header[ 5 ];
  
  /* check that the header is ok */
  transport_read_string( tpt, header, sizeof( header ) );
  if( header[0] != 'L' ||
      header[1] != 'R' ||
      header[2] != 'P' ||
      header[3] != 'C' ||
      header[4] != RPC_PROTOCOL_VERSION )
  {
    e.errnum = ERR_PROTOCOL;
    e.type = nonfatal;
    Throw( e );
  }
}

/****************************************************************************/
/* client side handle and handle helper userdata objects.
 *
 * a handle userdata (handle to a RPC server) is a pointer to a Handle object.
 * a helper userdata is a pointer to a Helper object.
 *
 * helpers let us make expressions like:
 *    handle.funcname (a,b,c)
 * "handle.funcname" returns the helper object, which calls the remote
 * function.
 */

/* global error handling */
static int global_error_handler = LUA_NOREF;  /* function reference */

/* handle a client or server side error. NOTE: this function may or may not
 * return. the handle `h' may be 0.
 */

void deal_with_error(lua_State *L, Handle *h, const char *error_string)
{ 
  if( global_error_handler !=  LUA_NOREF )
  {
    lua_getref( L, global_error_handler );
    lua_pushstring( L, error_string );
    lua_pcall( L, 1, 0, 0 );
  }
  else
    my_lua_error( L, error_string );
}


Handle *handle_create( lua_State *L )
{
  Handle *h = ( Handle * )lua_newuserdata( L, sizeof( Handle ) );
  luaL_getmetatable( L, "rpc.handle" );
  lua_setmetatable( L, -2 );
  h->error_handler = LUA_NOREF;
  h->async = 0;
  h->read_reply_count = 0;
  return h;
}

static Helper *helper_create( lua_State *L, Handle *handle, const char *funcname )
{
  Helper *h = ( Helper * )lua_newuserdata( L, sizeof( Helper ) );
  luaL_getmetatable( L, "rpc.helper" );
  lua_setmetatable( L, -2 );
  h->handle = handle;
  h->parent = NULL;
  h->nparents = 0;
  strncpy ( h->funcname, funcname, NUM_FUNCNAME_CHARS );
  return h;
}


/* indexing a handle returns a helper */
static int handle_index (lua_State *L)
{
  const char *s;
  Helper *h;

  MYASSERT( lua_gettop( L ) == 2 );
  MYASSERT( lua_isuserdata( L, 1 ) && ismetatable_type( L, 1, "rpc.handle" ) );

  if( lua_type( L, 2 ) != LUA_TSTRING )
    my_lua_error( L, "can't index a handle with a non-string" );
  s = lua_tostring( L, 2 );
  if ( strlen( s ) > NUM_FUNCNAME_CHARS - 1 )
    my_lua_error( L, "function name is too long" );
    
  h = helper_create( L, ( Handle * )lua_touserdata( L, 1 ), s );

  /* return the helper object */
  return 1;
}

static int helper_newindex( lua_State *L );

/* indexing a handle returns a helper */
static int handle_newindex( lua_State *L )
{
  const char *s;
  Helper *h;

  MYASSERT( lua_gettop( L ) == 3 );
  MYASSERT( lua_isuserdata( L, 1 ) && ismetatable_type( L, 1, "rpc.handle" ) );

  if( lua_type( L, 2 ) != LUA_TSTRING )
    my_lua_error( L, "can't index a handle with a non-string" );
  s = lua_tostring( L, 2 );
  if ( strlen( s ) > NUM_FUNCNAME_CHARS - 1 )
    my_lua_error( L, "function name is too long" );
  
  h = helper_create( L, ( Handle * )lua_touserdata( L, 1 ), "" );
	lua_replace(L, 1);

  helper_newindex( L );

  return 0;
}

/* replays series of indexes to remote side as a string */
static void helper_remote_index( Helper *helper )
{
  int i, len;
  Helper **hstack;
  Transport *tpt = &helper->handle->tpt;
  
  /* get length of name & make stack of helpers */
  len = strlen( helper->funcname );
  if( helper->nparents > 0 )
  {
    hstack = ( Helper ** )alloca( sizeof( Helper * ) * helper->nparents );
    hstack[ helper->nparents - 1 ] = helper->parent;
    len += strlen( hstack[ helper->nparents - 1 ]->funcname ) + 1;
  
    for(i = helper->nparents - 1 ; i > 0 ; i -- )
    {
      hstack[ i - 1 ] = hstack[ i ]->parent;
      len += strlen( hstack[ i ]->funcname ) + 1;
    }
  }
  
  transport_write_u32( tpt, len );
  /* replay helper key names */     
  if( helper->nparents > 0)
  {
    for( i = 0 ; i < helper->nparents ; i ++ )
    {
     transport_write_string( tpt, hstack[ i ]->funcname, strlen( hstack[ i ]->funcname ) );
     transport_write_string( tpt, ".", 1 ); 
    }
  }
  transport_write_string( tpt, helper->funcname, strlen( helper->funcname ) );
}


static int helper_get(lua_State *L, Helper *helper )
{
  struct exception e;
  int freturn = 0;
  Transport *tpt = &helper->handle->tpt;
  
  Try
  {
    int len;        
    /* write function name */
    len = strlen( helper->funcname );
    transport_write_u8( tpt, RPC_CMD_GET );
    helper_remote_index( helper );

    /* read variable back */
    read_variable( tpt, L );

    freturn = 1;
  }
  Catch( e )
  {
    switch( e.type )
    {
      case fatal:
        if ( e.errnum == ERR_CLOSED )
          my_lua_error( L, "can't refer to a remote function after the handle has been closed" );
        deal_with_error( L, helper->handle, errorString( e.errnum ) );
        transport_close( tpt );
        break;
      case nonfatal:
        deal_with_error( L, helper->handle, errorString( e.errnum ) );
        lua_pushnil( L );
        return 1;
        break;
      default:
        deal_with_error( L, helper->handle, errorString( e.errnum ) );
        transport_close( tpt );
        break;
    }
  }
  return freturn;
}


static int helper_call (lua_State *L)
{
  struct exception e;
  int freturn = 0;
  Helper *h;
  Transport *tpt;
  MYASSERT( lua_gettop( L ) >= 1 );
  MYASSERT( lua_isuserdata( L, 1 ) && ismetatable_type( L, 1, "rpc.helper" ) );
  
  /* get helper object and its transport */
  h = ( Helper * )lua_touserdata( L, 1 );
  tpt = &h->handle->tpt;
  
  
  /* @@@ ugly way to capture get calls, should find another way */
  if( strcmp("get", h->funcname ) == 0 )
  {
    helper_get( L, h->parent );
    freturn = 1;
  }
  else
  {
    Try
    {
      int i,len,n;
      u32 nret,ret_code;

      /* first read out any pending return values for old async calls */
      for (; h->handle->read_reply_count > 0; h->handle->read_reply_count--) {
        ret_code = transport_read_u8 (tpt);   /* return code */
        if( ret_code == 0 )
        {
          /* read return arguments, ignore everything we read */
          nret = transport_read_u32( tpt );
        
          for (i=0; i < ( ( int ) nret ); i++)
            read_variable (tpt,L);
        
          lua_pop (L,nret);
        }
        else
        {
          /* read error and handle it */
          u32 code = transport_read_u32( tpt );
          u32 len = transport_read_u32( tpt );
          char *err_string = ( char * )alloca( len + 1 );
          transport_read_string( tpt, err_string, len );
          err_string[ len ] = 0;

          deal_with_error( L, h->handle, err_string );
          freturn = 0;
        }
      }

      /* write function name */
      transport_write_u8( tpt, RPC_CMD_CALL );
      helper_remote_index( h );

      /* write number of arguments */
      n = lua_gettop( L );
      transport_write_u32( tpt, n - 1 );
    
      /* write each argument */
      for( i = 2; i <= n; i ++ )
        write_variable( tpt, L, i );

      /* if we're in async mode, we're done */
      if ( h->handle->async )
      {
        h->handle->read_reply_count++;
        freturn = 0;
      }

      /* read return code */
      ret_code = transport_read_u8( tpt );

      if ( ret_code== 0 )
      {
        /* read return arguments */
        nret = transport_read_u32( tpt );
      
        for ( i = 0; i < ( (int ) nret ); i ++ )
          read_variable( tpt, L );
      
        freturn = ( int )nret;
      }
      else
      {
        /* read error and handle it */
        u32 code = transport_read_u32( tpt );
        u32 len = transport_read_u32( tpt );
        char *err_string = ( char * )alloca( len + 1 );
        transport_read_string( tpt, err_string, len );
        err_string[ len ] = 0;

        deal_with_error( L, h->handle, err_string );
        freturn = 0;
      }
    }
    Catch( e )
    {
      switch( e.type )
      {
        case fatal:
          if ( e.errnum == ERR_CLOSED )
            my_lua_error( L, "can't refer to a remote function after the handle has been closed" );
          deal_with_error( L, h->handle, errorString( e.errnum ) );
          transport_close( tpt );
          break;
        case nonfatal:
          deal_with_error( L, h->handle, errorString( e.errnum ) );
          lua_pushnil( L );
          return 1;
          break;
        default:
          deal_with_error( L, h->handle, errorString( e.errnum ) );
          transport_close( tpt );
          break;
      }
    }
  }
  return freturn;
}


static int helper_newindex( lua_State *L )
{
  struct exception e;
  int freturn = 0;
  Helper *h;
  Transport *tpt;
  MYASSERT( lua_isuserdata( L, -3 ) && ismetatable_type( L, -3, "rpc.helper" ) );
  MYASSERT( lua_isstring( L, -2 ) );
  
  /* get helper object and its transport */
  h = ( Helper * )lua_touserdata( L, -3 );
  tpt = &h->handle->tpt;
  
  Try
  {
    int len;
        
    /* write function name */
    len = strlen( h->funcname );
    transport_write_u8( tpt, RPC_CMD_NEWINDEX );
    helper_remote_index( h );

    write_variable( tpt, L, lua_gettop( L ) - 1 );
    write_variable( tpt, L, lua_gettop( L ) );

    freturn = 0;
  }
  Catch( e )
  {
    switch( e.type )
    {
      case fatal:
        if ( e.errnum == ERR_CLOSED )
          my_lua_error( L, "can't refer to a remote function after the handle has been closed" );
        deal_with_error( L, h->handle, errorString( e.errnum ) );
        transport_close( tpt );
        break;
      case nonfatal:
        deal_with_error( L, h->handle, errorString( e.errnum ) );
        lua_pushnil( L );
        return 1;
        break;
      default:
        deal_with_error( L, h->handle, errorString( e.errnum ) );
        transport_close( tpt );
        break;
    }
  }
  return freturn;
}


static Helper *helper_append( lua_State *L, Helper *helper, const char *funcname )
{
  Helper *h = ( Helper * )lua_newuserdata( L, sizeof( Helper ) );
  luaL_getmetatable( L, "rpc.helper" );
  lua_setmetatable( L, -2 );
  h->handle = helper->handle;
  h->parent = helper;
  h->nparents = helper->nparents + 1;
  strncpy ( h->funcname, funcname, NUM_FUNCNAME_CHARS );
  return h;
}

/* indexing a handle returns a helper */
static int helper_index (lua_State *L)
{
  const char *s;
  Helper *h;

  MYASSERT( lua_gettop( L ) == 2 );
  MYASSERT( lua_isuserdata( L, 1 ) && ismetatable_type( L, 1, "rpc.helper" ) );

  if( lua_type( L, 2 ) != LUA_TSTRING )
    my_lua_error( L, "can't index a handle with a non-string" );
  s = lua_tostring( L, 2 );
  if ( strlen( s ) > NUM_FUNCNAME_CHARS - 1 )
    my_lua_error( L, "function name is too long" );
  
  h = helper_append( L, ( Helper * )lua_touserdata( L, 1 ), s );

  return 1;
}

/****************************************************************************/
/* server side handle userdata objects. */

static ServerHandle *server_handle_create( lua_State *L )
{
  ServerHandle *h = ( ServerHandle * )lua_newuserdata( L, sizeof( ServerHandle ) );
  luaL_getmetatable( L, "rpc.server_handle" );
  lua_setmetatable( L, -2 );

  h->link_errs = 0;

  transport_init( &h->ltpt );
  transport_init( &h->atpt );
  return h;
}


static void server_handle_shutdown( ServerHandle *h )
{
  transport_close( &h->ltpt );
  transport_close( &h->atpt );
}


static void server_handle_destroy( ServerHandle *h )
{
  server_handle_shutdown( h );
}

/****************************************************************************/
/* remote function calling (client side) */

/* rpc_connect (ip_address, port)
 *     returns a handle to the new connection, or nil if there was an error.
 *     if there is an RPC error function defined, it will be called on error.
 */

static int rpc_connect( lua_State *L )
{
  struct exception e;
  Handle *handle = 0;
  
  Try
  {
    handle = handle_create ( L );
    transport_open_connection( L, handle );
    
    transport_write_u8( &handle->tpt, RPC_CMD_CON );
    write_header( &handle->tpt );
    read_header( &handle->tpt );
  }
  Catch( e )
  {     
    deal_with_error( L, 0, errorString( e.errnum ) );
    lua_pushnil( L );
  }
  return 1;
}


/* rpc_close (handle)
 *     this closes the transport, but does not free the handle object. that's
 *     because the handle will still be in the user's name space and might be
 *     referred to again. we'll let garbage collection free the object.
 *     it's a lua runtime error to refer to a transport after it has been closed.
 */

static int rpc_close( lua_State *L )
{
  check_num_args( L, 1 );

  if( lua_isuserdata( L, 1 ) )
  {
    if( ismetatable_type( L, 1, "rpc.handle" ) )
    {
      Handle *handle = ( Handle * )lua_touserdata( L, 1 );
      transport_close( &handle->tpt );
      return 0;
    }
    if( ismetatable_type( L, 1, "rpc.server_handle" ) )
    {
      ServerHandle *handle = ( ServerHandle * )lua_touserdata( L, 1 );
      server_handle_shutdown( handle );
      return 0;
    }
  }

  my_lua_error(L,"argument must be an RPC handle");
  return 0;
}


/* rpc_async (handle,)
 *     this sets a handle's asynchronous calling mode (0/nil=off, other=on).
 *     (this is for the client only).
 */
/* @@@ This should probably be adjusted to be in line with our new multiple command architecture */
static int rpc_async (lua_State *L)
{
  Handle *handle;
  check_num_args( L, 2 );

  if ( !lua_isuserdata( L, 1 ) || !ismetatable_type( L, 1, "rpc.handle" ) )
    my_lua_error( L, "first argument must be an RPC client handle" );

  handle = ( Handle * )lua_touserdata( L, 1 );

  if ( lua_isnil( L, 2 ) || ( lua_isnumber( L, 2 ) && lua_tonumber( L, 2 ) == 0) )
    handle->async = 0;
  else
    handle->async = 1;

  return 0;
}

/****************************************************************************/
/* lua remote function server */

/* read function call data and execute the function. this function empties the
 * stack on entry and exit. This sets a custom error handler to catch errors 
 * around the function call.
 */

static void read_cmd_call( Transport *tpt, lua_State *L )
{
  int i, stackpos, good_function, nargs;
  u32 len;
  char *funcname;
  char *token = NULL;

  /* read function name */
  len = transport_read_u32( tpt ); /* function name string length */ 
  funcname = ( char * )alloca( len + 1 );
  transport_read_string( tpt, funcname, len );
  funcname[ len ] = 0;
    
  /* get function */
  /* @@@ perhaps handle more like variables instead of using a long string? */
  /* @@@ also strtok is not thread safe */
  token = strtok( funcname, "." );
  lua_getglobal( L, token );
  token = strtok( NULL, "." );
  while( token != NULL )
  {
    lua_getfield( L, -1, token );
    token = strtok( NULL, "." );
  }
  stackpos = lua_gettop( L ) - 1;
  good_function = LUA_ISCALLABLE( L, -1 );

  /* read number of arguments */
  nargs = transport_read_u32( tpt );

  /* read in each argument, leave it on the stack */
  for ( i = 0; i < nargs; i ++ ) 
    read_variable( tpt, L );

  /* call the function */
  if( good_function )
  {
    int nret, error_code;
    error_code = lua_pcall( L, nargs, LUA_MULTRET, 0 );
    
    /* handle errors */
    if ( error_code )
    {
      size_t len;
      const char *errmsg;
      errmsg = lua_tolstring (L, -1, &len);
      transport_write_u8( tpt, 1 );
      transport_write_u32( tpt, error_code );
      transport_write_u32( tpt, len );
      transport_write_string( tpt, errmsg, len );
    }
    else
    {
      /* pass the return values back to the caller */
      transport_write_u8( tpt, 0 );
      nret = lua_gettop( L ) - stackpos;
      transport_write_u32( tpt, nret );
      for ( i = 0; i < nret; i ++ )
        write_variable( tpt, L, stackpos + 1 + i );
    }
  }
  else
  {
    /* bad function */
    const char *msg = "undefined function: ";
    int errlen = strlen( msg ) + len;
    transport_write_u8( tpt, 1 );
    transport_write_u32( tpt, LUA_ERRRUN );
    transport_write_u32( tpt, errlen );
    transport_write_string( tpt, msg, strlen( msg ) );
    transport_write_string( tpt, funcname, len );
  }
  /* empty the stack */
  lua_settop ( L, 0 );
}


static void read_cmd_get( Transport *tpt, lua_State *L )
{
  u32 len;
  char *funcname;
  char *token = NULL;

  /* read function name */
  len = transport_read_u32( tpt ); /* function name string length */ 
  funcname = ( char * )alloca( len + 1 );
  transport_read_string( tpt, funcname, len );
  funcname[ len ] = 0;

  /* get function */
  /* @@@ perhaps handle more like variables instead of using a long string? */
  /* @@@ also strtok is not thread safe */
  token = strtok( funcname, "." );
  lua_getglobal( L, token );
  token = strtok( NULL, "." );
  while( token != NULL )
  {
    lua_getfield( L, -1, token );
    token = strtok( NULL, "." );
  }

  /* return top value on stack */
  write_variable( tpt, L, lua_gettop( L ) );

  /* empty the stack */
  lua_settop ( L, 0 );
}


static void read_cmd_newindex( Transport *tpt, lua_State *L )
{
  u32 len;
  char *funcname;
  char *token = NULL;

  /* read function name */
  len = transport_read_u32( tpt ); /* function name string length */ 
  funcname = ( char * )alloca( len + 1 );
  transport_read_string( tpt, funcname, len );
  funcname[ len ] = 0;

  /* get function */
  /* @@@ perhaps handle more like variables instead of using a long string? */
  /* @@@ also strtok is not thread safe */
  if( strlen( funcname ) > 0 ) /*  */
  {
    token = strtok( funcname, "." );
    lua_getglobal( L, token );
    token = strtok( NULL, "." );
    while( token != NULL )
    {
      lua_getfield( L, -1, token );
      token = strtok( NULL, "." );
    }
    read_variable( tpt, L ); /* key */
    read_variable( tpt, L ); /* value */  
    lua_settable( L, -3 ); /* set key to value on indexed table */
  }
  else
  {
    read_variable( tpt, L ); /* key */
    read_variable( tpt, L ); /* value */  
    lua_setglobal( L, lua_tostring( L, -2 ) );
  }
  


  /* empty the stack */
  lua_settop ( L, 0 );
}


static ServerHandle *rpc_listen_helper( lua_State *L )
{
  struct exception e;
  ServerHandle *handle = 0;

  Try
  {
    /* make server handle */
    handle = server_handle_create( L );

    /* make listening transport */
    transport_open_listener( L, handle );
  }
  Catch( e )
  {
    if( handle )
      server_handle_destroy( handle );
    
    deal_with_error( L, 0, errorString( e.errnum ) );
    return 0;
  }
  return handle;
}


/* rpc_listen (port) --> server_handle */
static int rpc_listen( lua_State *L )
{
  ServerHandle *handle;

  handle = rpc_listen_helper( L );
  if ( handle == 0 )
    printf( "Bad Handle!" );
    
  return 1;
}


/* rpc_peek (server_handle) --> 0 or 1 */
static int rpc_peek( lua_State *L )
{
  ServerHandle *handle;

  check_num_args( L, 1 );
  if ( !( lua_isuserdata( L, 1 ) && ismetatable_type( L, 1, "rpc.server_handle" ) ) )
    my_lua_error( L, "argument must be an RPC server handle" );

  handle = ( ServerHandle * )lua_touserdata( L, 1 );

  /* if accepting transport is open, see if there is any data to read */
  if ( transport_is_open( &handle->atpt ) )
  {
    if ( transport_readable( &handle->atpt ) )
      lua_pushnumber( L, 1 );
    else 
      lua_pushnil( L );
      
    return 1;
  }

  /* otherwise, see if there is a new connection on the listening transport */
  if ( transport_is_open( &handle->ltpt ) )
  {
    if ( transport_readable( &handle->ltpt ) )
      lua_pushnumber ( L, 1 );
    else
      lua_pushnil( L );
      
    return 1;
  }

  lua_pushnumber( L, 0 );
  return 1;
}


static void rpc_dispatch_helper( lua_State *L, ServerHandle *handle )
{  
  struct exception e;

  Try 
  {
    /* if accepting transport is open, read function calls */
    if ( transport_is_open( &handle->atpt ) )
    {
      Try
      {
        switch ( transport_read_u8( &handle->atpt ) )
        {
          case RPC_CMD_CALL:
            read_cmd_call( &handle->atpt, L );
            break;
          case RPC_CMD_GET:
            read_cmd_get( &handle->atpt, L );
            break;
          case RPC_CMD_CON: /*  @@@ allow client to "reconnect", should support better mechanism */
            read_header( &handle->atpt );
            write_header( &handle->atpt );
            break;
          case RPC_CMD_NEWINDEX:
            read_cmd_newindex( &handle->atpt, L );
            break;
          default:
            e.type = nonfatal;
            e.errnum = ERR_COMMAND;
            Throw( e );
        }
        
        handle->link_errs = 0;
      }
      Catch( e )
      {
        /* if the client has closed the connection, close our side
         * gracefully too.
         */
        transport_close( &handle->atpt );

        switch( e.type )
        {
          case fatal:
            Throw( e );
            
          case nonfatal:
            handle->link_errs++;
            if ( handle->link_errs > MAX_LINK_ERRS )
            {
              handle->link_errs = 0;
              Throw( e );
            }
            break;
            
          default: 
            Throw( e );
        }
      }
    }
    else
    {
      /* if accepting transport is not open, accept a new connection from the
       * listening transport.
       */
      transport_accept( &handle->ltpt, &handle->atpt );
      
      switch ( transport_read_u8( &handle->atpt ) )
      {
        case RPC_CMD_CON:
          read_header( &handle->atpt );
          write_header( &handle->atpt );
          break;
        default: /* connection must be established to issue any other commands */
          e.type = nonfatal;
          e.errnum = ERR_COMMAND;
          Throw( e );
      }
    }
  }
  Catch( e )
  {
    switch( e.type )
    {
      case fatal:
        server_handle_shutdown( handle );
        deal_with_error( L, 0, errorString( e.errnum ) );
        break;
      case nonfatal:
        transport_close( &handle->atpt );
        break;
      default:
        Throw( e );
    }
  }
}


/* rpc_dispatch (server_handle) */

static int rpc_dispatch( lua_State *L )
{
  ServerHandle *handle;
  check_num_args( L, 1 );

  if ( ! ( lua_isuserdata( L, 1 ) && ismetatable_type( L, 1, "rpc.server_handle" ) ) )
    my_lua_error( L, "argument must be an RPC server handle" );

  handle = ( ServerHandle * )lua_touserdata( L, 1 );

  rpc_dispatch_helper( L, handle );
  return 0;
}


/* lrf_server (port) */

static int rpc_server( lua_State *L )
{
  ServerHandle *handle = rpc_listen_helper( L );
  while ( transport_is_open( &handle->ltpt ) )
    rpc_dispatch_helper( L, handle );
  
  printf( "LT: %d, AT: %d\n", handle->ltpt.fd, handle->atpt.fd );
  
  server_handle_destroy( handle );
  return 0;
}

/****************************************************************************/
/* more error handling stuff */

/* rpc_on_error ([handle,] error_handler)
 */

static int rpc_on_error( lua_State *L )
{
  check_num_args( L, 1 );

  if( global_error_handler !=  LUA_NOREF )
    lua_unref (L,global_error_handler);
  
  global_error_handler = LUA_NOREF;

  if ( LUA_ISCALLABLE( L, 1 ) )
    global_error_handler = lua_ref( L, 1 );
  else if ( lua_isnil( L, 1 ) )
    { ;; }
  else
    my_lua_error( L, "bad arguments" );

  /* @@@ add option for handle */
  /* Handle *h = (Handle*) lua_touserdata (L,1); */
  /* if (lua_isuserdata (L,1) && ismetatable_type(L, 1, "rpc.handle")); */

  return 0;
}

/****************************************************************************/
/* register RPC functions */



#ifndef LUARPC_STANDALONE

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"

const LUA_REG_TYPE rpc_handle[] =
{
  { LSTRKEY( "__index" ), LFUNCVAL( handle_index ) },
  { LSTRKEY( "__newindex"), LFUNCVAL( handle_newindex )},
  { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE rpc_helper[] =
{
  { LSTRKEY( "__call" ), LFUNCVAL( helper_call ) },
  { LSTRKEY( "__index" ), LFUNCVAL( helper_index ) },
  { LSTRKEY( "__newindex" ), LFUNCVAL( helper_newindex ) },
  { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE rpc_server_handle[] =
{
  { LNILKEY, LNILVAL }
};

const LUA_REG_TYPE rpc_map[] =
{
  {  LSTRKEY( "connect" ), LFUNCVAL( rpc_connect ) },
  {  LSTRKEY( "close" ), LFUNCVAL( rpc_close ) },
  {  LSTRKEY( "server" ), LFUNCVAL( rpc_server ) },
  {  LSTRKEY( "on_error" ), LFUNCVAL( rpc_on_error ) },
  {  LSTRKEY( "listen" ), LFUNCVAL( rpc_listen ) },
  {  LSTRKEY( "peek" ), LFUNCVAL( rpc_peek ) },
  {  LSTRKEY( "dispatch" ), LFUNCVAL( rpc_dispatch ) },
  {  LSTRKEY( "rpc_async" ), LFUNCVAL( rpc_async ) },
#if LUA_OPTIMIZE_MEMORY > 0
/*  {  LSTRKEY("mode"), LSTRVAL( LUARPC_MODE ) }, */
#endif // #if LUA_OPTIMIZE_MEMORY > 0
  { LNILKEY, LNILVAL }
};


LUALIB_API int luaopen_luarpc(lua_State *L)
{
#if LUA_OPTIMIZE_MEMORY > 0
  luaL_rometatable(L, "rpc.helper", (void*)rpc_helper);
  luaL_rometatable(L, "rpc.handle", (void*)rpc_handle);
  luaL_rometatable(L, "rpc.server_handle", (void*)rpc_server_handle);
#else
  luaL_register( L, "rpc", rpc_map );
  lua_pushstring( L, LUARPC_MODE );
  lua_setfield(L, -2, "mode");

  luaL_newmetatable( L, "rpc.helper" );
  luaL_register( L, NULL, rpc_helper );
  
  luaL_newmetatable( L, "rpc.handle" );
  luaL_register( L, NULL, rpc_handle );
  
  luaL_newmetatable( L, "rpc.server_handle" );
#endif
  return 1;
}

#else

static const luaL_reg rpc_handle[] =
{
  { "__index", handle_index },
  { "__newindex", handle_newindex },
  { NULL, NULL }
};

static const luaL_reg rpc_helper[] =
{
  { "__call", helper_call },
  { "__index", helper_index },
  { "__newindex", helper_newindex },
  { NULL, NULL }
};

static const luaL_reg rpc_server_handle[] =
{
  { NULL, NULL }
};

static const luaL_reg rpc_map[] =
{
  { "connect", rpc_connect },
  { "close", rpc_close },
  { "server", rpc_server },
  { "on_error", rpc_on_error },
  { "listen", rpc_listen },
  { "peek", rpc_peek },
  { "dispatch", rpc_dispatch },
  { "rpc_async", rpc_async },
  { NULL, NULL }
};


LUALIB_API int luaopen_luarpc(lua_State *L)
{
  luaL_register( L, "rpc", rpc_map );
  lua_pushstring(L, LUARPC_MODE);
  lua_setfield(L, -2, "mode");

  luaL_newmetatable( L, "rpc.helper" );
  luaL_register( L, NULL, rpc_helper );
  
  luaL_newmetatable( L, "rpc.handle" );
  luaL_register( L, NULL, rpc_handle );
  
  luaL_newmetatable( L, "rpc.server_handle" );

  return 1;
}

#endif

#endif
