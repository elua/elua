-- eLua test 

if pd.board() == "SAM7-EX256" then
  ledpin = pio.PB_20
elseif pd.board() == "EK-LM3S8962" or pd.board() == "EK-LM3S6965" then
  ledpin = pio.PF_0
elseif pd.board() == "STR9-comStick" then
  ledpin = pio.P9_0
else
  print( "\nError: Unknown board " .. pd.board() .. " !\n" )
  return
end

function cycle()
  pio.set( ledpin )
  tmr.delay( 0, 500000 )
  pio.clear( ledpin )
  tmr.delay( 0, 500000 )
end

pio.output( ledpin )
print( "Hello from eLua on " .. pd.cpu() )
print "Watch your LED blinking :)"
print "Press any key to end this demo.\n"

while uart.recv( 0, 0, 0 ) < 0 do
  cycle()
end
