--------------------------------------------------------------------------------
--
-- A Simon game for the Kinitis K60 Freescale Tower
--
--------------------------------------------------------------------------------

ledpins = { pio.PA_11, pio.PA_28, pio.PA_29, pio.PA_10 }
padpins = { }
seq = {}

local TMR_DLY_ID = 1


-- Auxiliar functions
function leds_on()
  for i=1, #ledpins do
    pio.pin.setlow( ledpins[ i ] )
  end
end



function leds_off()
  for i=1, #ledpins do
    pio.pin.sethigh( ledpins[ i ] )
  end
end

 

function play_valid_sequence()
  io.write( "Valid sequence is: " )
  leds_off()
  for i = 1, #seq do
    io.write( seq[ i ] )
    pio.pin.setlow( ledpins[ seq[ i ] ] )  
    tmr.delay( TMR_DLY_ID, 500000 )
    pio.pin.sethigh( ledpins[ seq[ i ] ] )
    tmr.delay( TMR_DLY_ID, 500000 )
  end
  io.write( "\n" )
  leds_on()
end      



function touched_pad()
  local tp
  tp = tonumber( io.read() )
  io.write( tp, "\n")
--  tmr.delay( tmr.VIRT0, 1000000 )
--  io.write( "I touched the pad #", tp, "\n" )
--  print(" Type of tp is ", type( tp ) )
  return tp
end


-- Initializations
for i = 1, #ledpins do
  pio.pin.sethigh( i )
  pio.pin.setdir( pio.OUTPUT, ledpins[ i ] )
end
    
local wrong_guess = false

  print("\n\n** Starting a new game **" )

-- Main Loop --
while true do
  table.insert( seq, math.random( #ledpins ) )   -- add an led to the valid sequence
  play_valid_sequence()
  
  for i=1, #seq do
    io.write( "Please enter a pad [1-4] for sequence item #", i, ": " )
    local pad = touched_pad()
    if ( pad ~= seq[ i ] ) then
      io.write( "Wrong pad entered, correct would be ", seq[ i ], "\n" )
      wrong_guess = true
      break
    end
  end      
  if wrong_guess then
    io.write( "Sorry, you've lost\n" )
    seq = {}
    collectgarbage("collect")
    wrong_guess = false
    print("\n\n** Starting a new game **" )
  end  
end

