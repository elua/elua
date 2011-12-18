//  Module for interfacing With the Quadrature Encoding Interface (QEI)

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "common.h"
#include "auxmods.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "elua_qei.h"

#ifdef BUILD_QEI

//Lua:  qei.init( encoder_id,  phase, swap, index, max_count )
static void qei_init( lua_State *L )
{
    u8 enc_id = ( u8 )luaL_checkinteger( L, 1 );
    u8 phase = ( u8 )luaL_checkinteger( L, 2 );
    u8 swap = ( u8 )luaL_checkinteger( L, 3 );
    u8 index = ( u8 )luaL_checkinteger( L, 4 );
    u32 max_count = ( u32 )luaL_checkinteger( L, 5 );
    elua_qei_init( enc_id, phase, swap, index, max_count );
}

//Lua: qei.velInit( encoder_id, vel_period, ppr, edges )
static void qei_velInit( lua_State *L )
{
    u8 enc_id = ( u8 )luaL_checkinteger( L, 1 );
    u32 vel_period = ( u32 )luaL_checkinteger( L, 2 );
    int ppr = ( int )luaL_checkinteger( L, 3 );
    int edges = ( int )luaL_checkinteger( L, 4 );
    elua_qei_vel_init( enc_id, vel_period, ppr, edges );
}

//Lua:  qei.enable( encoder_id )
static void qei_enable( lua_State *L )
{
    u8 enc_id = ( u8 )luaL_checkinteger( L, 1 );
    elua_qei_enable( enc_id );
}

//Lua:  qei.disable( encoder_id )
static void qei_disable( lua_State *L )
{
    u8 enc_id = ( u8 )luaL_checkinteger( L, 1 );
    elua_qei_disable( enc_id );
}

//Lua:  vel, err = qei.getVelPulses( encoder_id )
static int qei_getVelPulses( lua_State *L )
{
    u8 enc_id = ( u8 )luaL_checkinteger( L, 1 );
    int err = 0;
    if( !(qei_flag & 0x01) )
    {
        err = 1;
        lua_pushinteger( L, -1 );
        lua_pushinteger( L, err );
        return 2;
    }
    else if( (enc_id == ELUA_QEI_CH01) || !(qei_flag & qei_enc_flags[ enc_id ]) )
    {
        err = 2;
        lua_pushinteger( L, -1 );
        lua_pushinteger( L, err );
        return 2;
    }
    u32 pulses = elua_qei_getPulses( enc_id );
    lua_pushinteger( L, pulses );
    lua_pushinteger( L, err );
    return 2;
}

//Lua:  rpm, err = qei.getRPM( encoder_id )
static int qei_getRPM( lua_State *L )
{
    u8 enc_id = ( u8 )luaL_checkinteger( L, 1 );
    int err = 0;
    if( !(qei_flag & 0x01) )
    {
        err = 1;
        lua_pushinteger( L, -1 );
        lua_pushinteger( L, err );
        return 2;
    }
    else if( (enc_id == ELUA_QEI_CH01) || !(qei_flag & qei_enc_flags[ enc_id ]) )
    {
        err = 2;
        lua_pushinteger( L, -1 );
        lua_pushinteger( L, err );
        return 2;
    }
    s32 rpm = elua_qei_getRPM( enc_id );
    lua_pushinteger( L, rpm );
    lua_pushinteger( L, err );
    return 2;
}

//Lua:  pos, err = qei.getPosition( encoder_id )
static int qei_getPosition( lua_State *L )
{
    u8 enc_id = ( u8 )luaL_checkinteger( L, 1 );
    int err = 0;
    if( (enc_id == ELUA_QEI_CH01) || !(qei_flag & qei_enc_flags[ enc_id ]) )
    {
        err = 2;
        lua_pushinteger( L, -1 );
        lua_pushinteger( L, err );
        return 2;
    }
    lua_pushinteger( L, elua_qei_getPosition( enc_id ) );
    lua_pushinteger( L, err );
    return 2;
}


// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE qei_map[] = 
{
    { LSTRKEY( "init" ), LFUNCVAL( qei_init ) },
    { LSTRKEY( "velInit" ), LFUNCVAL( qei_velInit ) },
    { LSTRKEY( "enable" ), LFUNCVAL( qei_enable ) },
    { LSTRKEY( "disable" ), LFUNCVAL( qei_disable ) },
    { LSTRKEY( "getVelPulses" ), LFUNCVAL( qei_getVelPulses ) },
    { LSTRKEY( "getRPM" ), LFUNCVAL( qei_getRPM ) },
    { LSTRKEY( "getPosition" ), LFUNCVAL( qei_getPosition ) },

    { LSTRKEY( "PHA" ), LNUMVAL( ELUA_QEI_PHA ) },
    { LSTRKEY( "PHAB" ), LNUMVAL( ELUA_QEI_PHAB ) },
    { LSTRKEY( "CH0" ), LNUMVAL( ELUA_QEI_CH0 ) },
    { LSTRKEY( "CH01" ), LNUMVAL( ELUA_QEI_CH01 ) },
    { LSTRKEY( "CH1" ), LNUMVAL( ELUA_QEI_CH1 ) },
    { LSTRKEY( "NO_SWAP" ), LNUMVAL( ELUA_QEI_NO_SWAP ) },
    { LSTRKEY( "SWAP" ), LNUMVAL( ELUA_QEI_SWAP ) },
    { LSTRKEY( "NO_INDEX" ), LNUMVAL( ELUA_QEI_NO_INDEX ) },
    { LSTRKEY( "INDEX" ), LNUMVAL( ELUA_QEI_INDEX ) },
    { LSTRKEY( "ERR_OK" ), LNUMVAL( ELUA_QEI_ERR_OK ) },
    { LSTRKEY( "ERR_VELOCITY_NOT_ENABLED" ), LNUMVAL( ELUA_QEI_ERR_VEL_NOT_ENABLED ) },
    { LSTRKEY( "ERR_ENC_NOT_ENABLED" ), LNUMVAL( ELUA_QEI_ERR_ENC_NOT_ENABLED ) },
  
    { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_qei( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
    return 0;
#else
    LREGISTER( L, AUXLIB_QEI, qei_map );
    return 1;
#endif
}

/*endif BUILD_QEI*/
#endif
