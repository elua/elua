local vtmrid = tmr.VIRT0
local to = 1500000
local uartid = 0 
local new_prev_tmr, prev_tmr, prev_gpio

local function tmr_handler( resnum )
  print( string.format( "Timer interrupt for timer %d", resnum ) )
  if prev_tmr then prev_tmr( resnum ) end
end

local function new_tmr_handler( resnum )
  print( string.format( "NEW HANDLER: timer interrupt for timer %d", resnum ) )
  if new_prev_tmr then new_prev_tmr( resnum ) end
end

local function gpio_negedge_handler( resnum )
  local port, pin = pio.decode( resnum )
  print( string.format( "Port is %d, pin is %d", port, pin ) )
  if prev_gpio then prev_gpio( resnum ) end
end

prev_tmr = cpu.set_int_handler( cpu.INT_TMR_MATCH, tmr_handler )
prev_gpio = cpu.set_int_handler( cpu.INT_GPIO_NEGEDGE, gpio_negedge_handler )
tmr.set_match_int( vtmrid, to, tmr.INT_CYCLIC )
cpu.sei( cpu.INT_GPIO_NEGEDGE, pio.P0_0 )
cpu.sei( cpu.INT_TMR_MATCH, vtmrid )

local tmrid, count = 0, 0
while true do
  print "Outside interrupt" 
  for i = 1, 1000 do tmr.delay( tmrid, 1000 ) end
  if uart.getchar( uartid, 0 ) ~= "" then break end 
  count = count + 1
  if count == 5 then
    print "Changing timer interrupt handler"
    new_prev_tmr = cpu.set_int_handler( cpu.INT_TMR_MATCH, new_tmr_handler )
  end
end

tmr.set_match_int( vtmrid, 0, tmr.INT_CYCLIC );
cpu.cli( cpu.INT_GPIO_NEGEDGE, pio.P0_0 )
cpu.cli( cpu.INT_TMR_MATCH, vtmrid )
cpu.set_int_handler( cpu.INT_TMR_MATCH, nil );
cpu.set_int_handler( cpu.INT_GPIO_NEGEDGE, nil );

