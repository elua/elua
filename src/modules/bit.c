/* Bitwise operations library */
/* (c) Reuben Thomas 2000-2008 */
/* See README for license */

// Modified by BogdanM for eLua

// TH: Fixed various places to do all conversions to/from lua numbers as unsigned
// TH 08.08.2017: Changed the patch now in a way that the bit module behaves similar
// to the bit32 module in lua 5.2: All numbers passed to the bit functions are
// "downscaled" to the 2^32 range.
// In this way the module accepts "unsigned" and "signed" numbers and handle
// them in the right way.
// So print(bit.tohex(2^32-1)), print(bit.tohex(0xffffffff) and
// print(bit.tohex(-1)) will all return the same result "ffffffff"


// In addtion I added the tohex and rotate functions from http://bitop.luajit.org/


#include <limits.h>

//#include "lua.h"
#include "lauxlib.h"
#include "auxmods.h"
#include "type.h"
#include "lrotable.h"

/* FIXME: Assume size_t is an unsigned lua_Integer */
typedef u32 lua_UInteger;
#define LUA_UINTEGER_MAX SIZE_MAX


//TH: Local Push unsigned integer
void  bit_pushuinteger(lua_State *L, lua_UInteger n)
{

  lua_pushnumber( L, ( lua_Number )n);

}

/* Define TOBIT to get a bit value */
#define TOBIT(L, n)                    \
  ( ( (s64)luaL_checknumber((L), (n))) & 0x0ffffffff  )

/* Operations

   The macros MONADIC and VARIADIC only deal with bitwise operations.

   LOGICAL_SHIFT truncates its left-hand operand before shifting so
   that any extra bits at the most-significant end are not shifted
   into the result.

   ARITHMETIC_SHIFT does not truncate its left-hand operand, so that
   the sign bits are not removed and right shift work properly.
   */

#define MONADIC(name, op)                                       \
  static int bit_ ## name(lua_State *L) {                       \
    bit_pushuinteger(L, op TOBIT(L, 1));                        \
    return 1;                                                   \
  }

#define VARIADIC(name, op)                      \
  static int bit_ ## name(lua_State *L) {       \
    int n = lua_gettop(L), i;                   \
    lua_UInteger w = TOBIT(L, 1);               \
    for (i = 2; i <= n; i++)                    \
      w op TOBIT(L, i);                         \
    bit_pushuinteger(L, w);                     \
    return 1;                                   \
  }

#define LOGICAL_SHIFT(name, op)                                         \
  static int bit_ ## name(lua_State *L) {                               \
    bit_pushuinteger(L, (lua_UInteger)TOBIT(L, 1) op                    \
                          (unsigned)luaL_checknumber(L, 2));            \
    return 1;                                                           \
  }

#define ARITHMETIC_SHIFT(name, op)                                      \
  static int bit_ ## name(lua_State *L) {                               \
    bit_pushuinteger(L, (lua_UInteger)TOBIT(L, 1) op                    \
                          (unsigned)luaL_checknumber(L, 2));            \
    return 1;                                                           \
  }

// Added TH
// TH:08.08.2017: The assignments to local vars in the
// macro lead to better code with GCC when expanding brol and bror macros,
// because the compiler cannot now if the function calls to Lua
// have side effects and call them on every occurence
#define BIT_SH(name, fn)                              \
  static int bit_ ## name(lua_State *L) {             \
    lua_UInteger b= (lua_UInteger)TOBIT(L, 1);        \
    unsigned sh = (unsigned)luaL_checknumber(L, 2);   \
    bit_pushuinteger(L, fn(b,sh));                    \
    return 1;                                         \
  }

#define brol(b, n)  ((b << n) | (b >> (32-n)))
#define bror(b, n)  ((b << (32-n)) | (b >> n))

MONADIC(bnot,  ~)
VARIADIC(band, &=)
VARIADIC(bor,  |=)
VARIADIC(bxor, ^=)
ARITHMETIC_SHIFT(lshift,  <<)
LOGICAL_SHIFT(rshift,     >>)
ARITHMETIC_SHIFT(arshift, >>)

BIT_SH(rol, brol) // TH
BIT_SH(ror, bror) // TH



// Lua: res = bit( position )
static int bit_bit( lua_State* L )
{
  bit_pushuinteger( L, ( lua_UInteger )( 1 << luaL_checkinteger( L, 1 ) ) );
  return 1;
}

// Lua: res = isset( value, position )
static int bit_isset( lua_State* L )
{
  lua_UInteger val = ( lua_UInteger )TOBIT( L, 1 );
  unsigned pos = ( unsigned )luaL_checkinteger( L, 2 );

  lua_pushboolean( L, val & ( 1 << pos ) ? 1 : 0 );
  return 1;
}

// Lua: res = isclear( value, position )
static int bit_isclear( lua_State* L )
{
  lua_UInteger val = ( lua_UInteger )TOBIT( L, 1 );
  unsigned pos = ( unsigned )luaL_checkinteger( L, 2 );

  lua_pushboolean( L, val & ( 1 << pos ) ? 0 : 1 );
  return 1;
}

// Lua: res = set( value, pos1, pos2, ... )
static int bit_set( lua_State* L )
{
  lua_UInteger val = ( lua_UInteger )TOBIT( L, 1 );
  unsigned total = lua_gettop( L ), i;

  for( i = 2; i <= total; i ++ )
    val |= 1 << ( unsigned )luaL_checkinteger( L, i );
  bit_pushuinteger( L, ( lua_UInteger )val );
  return 1;
}

// Lua: res = clear( value, pos1, pos2, ... )
static int bit_clear( lua_State* L )
{
  lua_UInteger val = ( lua_UInteger )TOBIT( L, 1 );
  unsigned total = lua_gettop( L ), i;

  for( i = 2; i <= total; i ++ )
    val &= ~( 1 << ( unsigned )luaL_checkinteger( L, i ) );
  bit_pushuinteger( L, ( lua_UInteger )val );
  return 1;
}

//TH: "Borrowed" from luaBitop
static int bit_tohex(lua_State *L)
{
  lua_UInteger b = ( lua_UInteger )TOBIT( L, 1 );
  int n = lua_isnone(L, 2) ? 8 : luaL_checkinteger(L, 2);
  const char *hexdigits = "0123456789abcdef";
  char buf[8];
  int i;
  if (n < 0) { n = -n; hexdigits = "0123456789ABCDEF"; }
  if (n > 8) n = 8;
  for (i = (int)n; --i >= 0; ) { buf[i] = hexdigits[b & 15]; b >>= 4; }
  lua_pushlstring(L, buf, (size_t)n);
  return 1;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE bit_map[] = {
  { LSTRKEY( "bnot" ),    LFUNCVAL( bit_bnot ) },
  { LSTRKEY( "band" ),    LFUNCVAL( bit_band ) },
  { LSTRKEY( "bor" ),     LFUNCVAL( bit_bor ) },
  { LSTRKEY( "bxor" ),    LFUNCVAL( bit_bxor ) },
  { LSTRKEY( "lshift" ),  LFUNCVAL( bit_lshift ) },
  { LSTRKEY( "rshift" ),  LFUNCVAL( bit_rshift ) },
  { LSTRKEY( "arshift" ), LFUNCVAL( bit_arshift ) },
  { LSTRKEY( "bit" ),     LFUNCVAL( bit_bit ) },
  { LSTRKEY( "set" ),     LFUNCVAL( bit_set ) },
  { LSTRKEY( "clear" ),   LFUNCVAL( bit_clear ) },
  { LSTRKEY( "isset" ),   LFUNCVAL( bit_isset ) },
  { LSTRKEY( "isclear" ), LFUNCVAL( bit_isclear ) },
  { LSTRKEY( "tohex" ),   LFUNCVAL( bit_tohex ) }, // TH
  { LSTRKEY("rol"),       LFUNCVAL( bit_rol ) }, // TH
  { LSTRKEY("ror"),       LFUNCVAL( bit_ror ) }, // TH
  { LNILKEY, LNILVAL}
};


LUALIB_API int luaopen_bit (lua_State *L) {
  LREGISTER( L, "bit", bit_map );
}
