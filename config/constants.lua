-- Various constants used by the build descriptor

module( ..., package.seeall )
local sf = string.format

-------------------------------------------------------------------------------
-- UART data

-- UART flow types
uart_flow = 
{
  none = 'PLATFORM_UART_FLOW_NONE',
  rts = 'PLATFORM_UART_FLOW_RTS',
  cts = 'PLATFORM_UART_FLOW_CTS',
  rtscts = '( PLATFORM_UART_FLOW_RTS | PLATFORM_UART_FLOW_CTS )'
}

uart_values = {}

-- Add a sufficient number of virtual and real UARTs
for i = 0, 255 do
  uart_values[ sf( 'vuart%d', i ) ] = sf( '( SERMUX_SERVICE_ID_FIRST + %d )', i )
  uart_values[ tostring( i ) ] = i
end
uart_values.cdc = "CDC_UART_ID"

-------------------------------------------------------------------------------
-- Timer data

-- System timer ID
timer_values = 
{
  systmr = 'PLATFORM_TIMER_SYS_ID'
}

-- Add a sufficient number of virtual timers 
for i = 0, 127 do
  timer_values[ sf( 'vtmr%d', i ) ] = sf( '( VTMR_FIRST_ID + %d )', i )
  timer_values[ tostring( i ) ] = i
end

-------------------------------------------------------------------------------
-- EGC data

egc = 
{
  alloc_failure = 1,
  mem_limit = 2,
  always = 4
}

