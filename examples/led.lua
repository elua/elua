-- eLua test 

if pd.platform() == "AT91SAM7X" then
  ledpin = pio.PB_20
elseif pd.platform() == "LM3S" then
  ledpin = pio.PF_0
elseif pd.platform() == "STR9" then
  ledpin = pio.P9_0
else
  print( "Unknown platform " .. pd.platform() )
  return
end

function cycle()
  pio.set( ledpin )
  tmr.delay( tmr.TMR0, 500000 )
  pio.clear( ledpin )
  tmr.delay( tmr.TMR0, 500000 )
end

pio.output( ledpin )
print( "Hello from eLua on " .. pd.name() )
print "Watch your LED blinking :)"
print "Press any key to end this demo.\n"

while uart.recv( uart.UART0, 0, 0 ) < 0 do
  cycle()
end
