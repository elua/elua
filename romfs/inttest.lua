local vtmrid = tmr.VIRT0
local to = 1500000
local uartid = 0 

local function handler( id, resnum )
  print( string.format( "Got interrupt with id %d and resnum %d", id, resnum ) )
  if id == cpu.INT_GPIO_POSEDGE or id == cpu.INT_GPIO_NEGEDGE then
    local port, pin = pio.decode( resnum )
    print( string.format( "Port is %d, pin is %d", port, pin ) )
  elseif id == cpu.INT_TMR_MATCH then
    print "Timer interrupt!"
  end
end

cpu.set_int_handler( handler )
tmr.set_match_int( vtmrid, to, tmr.INT_CYCLIC )
cpu.sei( cpu.INT_GPIO_NEGEDGE, pio.P0_0 )
cpu.sei( cpu.INT_TMR_MATCH, vtmrid )

local tmrid = 0
while true do
  print "Outside interrupt" 
  for i = 1, 1000 do tmr.delay( tmrid, 1000 ) end
  if uart.getchar( uartid, 0 ) ~= "" then break end 
end

