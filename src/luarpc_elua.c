#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "platform_conf.h"
#include "luarpc_rpc.h"

#ifdef BUILD_LUARPC

int uart_timeout = PLATFORM_UART_INFINITE_TIMEOUT;

/* Setup Transport */
void transport_init (Transport *tpt)
{
	tpt->fd = INVALID_TRANSPORT;
}

/* Open Listener / Server */
void transport_open_listener(lua_State *L, ServerHandle *handle)
{
	/* Get args & Set up connection */
	
	handle->ltpt.fd = CON_UART_ID;
}

/* Open Connection / Client */
int transport_open_connection(lua_State *L, Handle *handle)
{
	handle->tpt.fd = CON_UART_ID;
	return 0;
}

/* Accept Connection */
void transport_accept (Transport *tpt, Transport *atpt)
{
	struct exception e;
	TRANSPORT_VERIFY_OPEN;
	atpt->fd = tpt->fd;
}

/* Read & Write to Transport */
void transport_read_buffer_low (Transport *tpt, u8 *buffer, int length)
{
	int n = 0;
	int c;
	struct exception e;
	
	while( n < length )
	{
		TRANSPORT_VERIFY_OPEN;
		c = platform_uart_recv( CON_UART_ID, CON_TIMER_ID, uart_timeout );
				
    if( c < 0 )
		{
			e.errnum = ERR_NODATA;
			e.type = nonfatal;
			Throw( e );
		}
		else
		{
			buffer[ n ] = ( u8 ) c;
			n++;
		}
  }
}

void transport_read_buffer (Transport *tpt, u8 *buffer, int length)
{
	u8 tmp;
	struct exception e;
	TRANSPORT_VERIFY_OPEN;
	
	/* Read Header */
	transport_read_buffer_low( tpt, &tmp, 1);
	if ( tmp != HEAD_BYTE )
	{
		uart_timeout = 1000000;
		e.errnum = ERR_PROTOCOL;
		e.type = nonfatal;
		Throw( e );
	}
	
	/* Read Data */
	transport_read_buffer_low( tpt, buffer, length);
	
	/* Read Trailer */
	transport_read_buffer_low( tpt, &tmp, 1);
	if ( tmp != TAIL_BYTE )
	{
		uart_timeout = 1000000;
		e.errnum = ERR_PROTOCOL;
		e.type = nonfatal;
		Throw( e );
	}
	
	uart_timeout = PLATFORM_UART_INFINITE_TIMEOUT;
}

void transport_write_buffer_low (Transport *tpt, const u8 *buffer, int length)
{
	int i;
	for( i = 0; i < length; i ++ )
    platform_uart_send( CON_UART_ID, buffer[ i ] );
}

void transport_write_buffer( Transport *tpt, const u8 *buffer, int length )
{
	u8 tmp;
	struct exception e;
	TRANSPORT_VERIFY_OPEN;
	
	tmp = HEAD_BYTE;
	transport_write_buffer_low( tpt, (const u8 *)&tmp, 1 );
	
	transport_write_buffer_low( tpt, buffer, length );
	
	tmp = TAIL_BYTE;
	transport_write_buffer_low( tpt, (const u8 *)&tmp, 1 );
}

/* Check if data is available on connection without reading:
 		- 1 = data available, 0 = no data available */
int transport_readable (Transport *tpt)
{
	return 1;
}

/* Check if transport is open:
		- 1 = connection open, 0 = connection closed */
int transport_is_open (Transport *tpt)
{
	return (tpt->fd != INVALID_TRANSPORT);
}

/* Shut down connection */
void transport_close (Transport *tpt)
{
	tpt->fd = INVALID_TRANSPORT;
}

#endif