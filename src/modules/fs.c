// Module for interfacing with various file systems
// For now this is enabled only if NIFFS is enabled

#include "platform_conf.h"
#if defined( BUILD_NIFFS )

//#include "lua.h"
#include "lualib.h"
#include "auxmods.h"
#include "lrotable.h"
#include "nffs.h"

// Lua: res = niffs_format( linear_size )
static int fs_nffs_format( lua_State *L )
{
  s32 linear_size = -1;

  switch ( lua_gettop( L ) ) {  /* check number of arguments */
    case 1:  /* format filesystem with the given linear size */
      linear_size = luaL_checkinteger ( L, 1 );
      break;

    default:
      return luaL_error( L, "wrong number of arguments" );
  }
  lua_pushinteger( L, nffs_format( linear_size ) );
  return 1;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE fs_map[] =
{
  { LSTRKEY( "nffs_format" ), LFUNCVAL( fs_nffs_format ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_fs( lua_State* L )
{
  LREGISTER( L, AUXLIB_FS, fs_map );
}

#endif // #ifdef BUILD_NIFFS
