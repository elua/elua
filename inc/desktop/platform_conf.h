// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#ifdef LUA_RPC
#include "auxmods.h"

#define BUILD_RPC
#define LUARPC_ENABLE_SERIAL

#define LUA_PLATFORM_LIBS_REG \
  {LUA_OSLIBNAME, luaopen_os}

#define LUA_PLATFORM_LIBS_ROM \
  _ROM( AUXLIB_RPC, luaopen_rpc, rpc_map )\
  _ROM( AUXLIB_BITARRAY, luaopen_bitarray, bitarray_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )
#endif


#endif // #ifndef __PLATFORM_CONF_H__

