// Network services provided by eLua

#ifndef __ELUA_NET_H__
#define __ELUA_NET_H__

#include "type.h"
#include "lauxlib.h"
#include "platform.h"

// eLua network typedefs
typedef s16 elua_net_size;

// eLua net error codes
enum
{
  ELUA_NET_ERR_OK = 0,
  ELUA_NET_ERR_TIMEDOUT,
  ELUA_NET_ERR_CLOSED,
  ELUA_NET_ERR_ABORTED,
  ELUA_NET_ERR_OVERFLOW,
  ELUA_NET_ERR_LINKDOWN
};

// eLua IP address type
typedef union
{
  u32     ipaddr;
  u8      ipbytes[ 4 ];
  u16     ipwords[ 2 ];
} elua_net_ip;

// eLua network configuration constants
enum
{
  ELUA_NET_CFG_IP,
  ELUA_NET_CFG_NETMASK,
  ELUA_NET_CFG_DNS,
  ELUA_NET_CFG_GW
};

// eLua services ports
#define ELUA_NET_TELNET_PORT          23

// Different constants
#define ELUA_NET_SOCK_STREAM          0
#define ELUA_NET_SOCK_DGRAM           1

// 'no split' indication for recv/recvfrom
#define ELUA_NET_NO_SPLIT             ( -1 )

// Invalid socket value
#define ELUA_NET_INVALID_SOCKET       ( -1 )

// Infinite timeout for timed operations
#define ELUA_NET_INF_TIMEOUT          ( -1 )

// Stack state
#define ELUA_NET_STATE_UP             1
#define ELUA_NET_STATE_DOWN           0

// Receive data callback type
typedef void( *p_elua_net_recv_cb )( int, const u8*, unsigned, elua_net_ip, u16 );
// Stack state changed callback type
typedef void ( *p_elua_net_state_cb )( int state );

// eLua TCP/IP functions
// Generic functions
void elua_net_init( void* pdata );
void elua_net_link_changed();
p_elua_net_state_cb elua_net_set_state_cb( p_elua_net_state_cb plink );
p_elua_net_state_cb elua_net_get_state_cb();
int elua_net_socket( int type );
int elua_net_set_buffer( int s, unsigned bufsize );
int elua_net_set_split( int s, int schar );
int elua_net_close( int s );
void elua_net_set_recv_callback( int s, p_elua_net_recv_cb callback );
elua_net_ip elua_net_lookup( const char* hostname );
int elua_net_get_last_err( int s );
int elua_net_get_telnet_socket();
elua_net_ip elua_net_get_config( int what );
int elua_net_expect( int s, const u8 *str, unsigned len, unsigned timer_id, s32 to_us );
int elua_net_readto( int s, luaL_Buffer *b, const u8 *str, unsigned len, unsigned timer_id, s32 to_us );

// TCP operations
elua_net_size elua_net_recvbuf( int s, luaL_Buffer *buf, elua_net_size maxsize, unsigned timer_id, s32 to_us );
elua_net_size elua_net_recv( int s, void *buf, elua_net_size maxsize, unsigned timer_id, s32 to_us );
elua_net_size elua_net_send( int s, const void* buf, elua_net_size len );
int elua_net_accept( u16 port, unsigned bufsize, unsigned timer_id, s32 to_us, elua_net_ip* pfrom );
int elua_net_connect( int s, elua_net_ip addr, u16 port, unsigned timer_id, s32 to_us );

// UDP operations
unsigned elua_net_sendto( int s, const void* buf, elua_net_size len, elua_net_ip remoteip, u16 port );
elua_net_size elua_net_recvfrombuf( int s, luaL_Buffer *buf, elua_net_size maxsize, elua_net_ip *p_remote_ip, u16 *p_remote_port, unsigned timer_id, s32 to_us );
elua_net_size elua_net_recvfrom( int s, void *buf, elua_net_size maxsize, elua_net_ip *p_remote_ip, u16 *p_remote_port, unsigned timer_id, s32 to_us );

#endif
