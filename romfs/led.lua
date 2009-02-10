-- eLua test 

local uartid, invert, ledpin = 0, false
if pd.board() == "SAM7-EX256" then
  ledpin = "PB_20"
elseif pd.board() == "EK-LM3S8962" or pd.board() == "EK-LM3S6965" then
  ledpin = "PF_0"
elseif pd.board() == "STR9-COMSTICK" then
  ledpin = "P9_0"
elseif pd.board() == "LPC-H2888" then
  ledpin = "P2_1"
elseif pd.board() == "MOD711" then
  ledpin = "P1_7"
  uartid = 1
elseif pd.board() == "ATEVK1100" then
  ledpin = "PB_27"
  invert = true
else
  print( "\nError: Unknown board " .. pd.board() .. " !" )
  return
end

function cycle()
  if not invert then pio[ ledpin ] = 1 else pio[ ledpin ] = 0 end
  tmr.delay( 0, 500000 )
  if not invert then pio[ ledpin ] = 0 else pio[ ledpin ] = 1 end
  tmr.delay( 0, 500000 )
end

pio.dir[ ledpin ] = pio.OUTPUT
print( "Hello from eLua on " .. pd.cpu() )
print "Watch your LED blinking :)"
print "Press any key to end this demo.\n"

while uart.recv( uartid, 0, 0 ) < 0 do
  cycle()
end

