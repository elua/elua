-- eLua test 

local uartid, invert, ledpin = 0, false
if pd.board() == "SAM7-EX256" then
  ledpin = pio.PB_20
elseif pd.board() == "EK-LM3S8962" or pd.board() == "EK-LM3S6965" then
  ledpin = pio.PF_0
elseif pd.board() == "EAGLE-100" then
  ledpin = pio.PE_1
elseif pd.board() == "STR9-COMSTICK" then
  ledpin = pio.P9_0
elseif pd.board() == "LPC-H2888" then
  ledpin = pio.P2_1
elseif pd.board() == "MOD711" then
  ledpin = pio.P1_7
  uartid = 1
elseif pd.board() == "ATEVK1100" then
  ledpin = pio.PB_27
  invert = true
else
  print( "\nError: Unknown board " .. pd.board() .. " !" )
  return
end

function cycle()
  if not invert then pio.pin.sethigh( ledpin ) else pio.pin.setlow( ledpin ) end
  tmr.delay( 0, 500000 )
  if not invert then pio.pin.setlow( ledpin ) else pio.pin.sethigh( ledpin ) end
  tmr.delay( 0, 500000 )
end

pio.pin.setdir( pio.OUTPUT, ledpin )
print( "Hello from eLua on " .. pd.cpu() )
print "Watch your LED blinking :)"
print "Press any key to end this demo.\n"

while uart.getchar( uartid, 0 ) == "" do
  cycle()
end

