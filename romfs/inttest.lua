local function handler( id, resnum )
  print( string.format( "Got interrupt with id %d and resnum %d", id, resnum ) )
  local port, pin = pio.decode( resnum )
  print( string.format( "Port is %d, pin is %d", port, pin ) )
end

cpu.set_int_handler( handler )
cpu.sei( cpu.INT_GPIO_NEGEDGE, pio.P0_0 )
local tmrid = 0
while true do
  print "Outside interrupt" 
  for i = 1, 1000 do tmr.delay( tmrid, 1000 ) end
end

