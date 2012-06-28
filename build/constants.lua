-- Various constants used by the build descriptor

module( ..., package.seeall )
local sf = string.format

-- UART flow types
uart_flow = 
{
  none = 'PLATFORM_UART_FLOW_NONE',
  rts = 'PLATFORM_UART_FLOW_RTS',
  cts = 'PLATFORM_UART_FLOW_CTS',
  rtscts = '( PLATFORM_UART_FLOW_RTS | PLATFORM_UART_FLOW_CTS )'
}

uart_flow_vals = {}
for k, v in pairs( uart_flow ) do
  uart_flow_vals[ #uart_flow_vals + 1 ] = v
end

-- System timer ID
systmr = 'PLATFORM_TIMER_SYS_ID'

-- Add a sufficient number of virtual timers 
for i = 0, 31 do
  _G[ sf( 'vtmr%d', i ) ] = sf( '( VTMR_FIRST_ID + %d )', i )
end

-- Add a sufficient number of virtual UARTs
for i = 0, 31 do
  _G[ sf( 'vuart%d', i ) ] = sf( '( SERMUX_SERVICE_ID_FIRST + %d )', i )
end

