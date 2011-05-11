--------------------------------------------------------------------------------
--
-- A Touch Demo for the Kinitis K60 Freescale Tower
--
--------------------------------------------------------------------------------

local ledpins = { pio.PA_11, pio.PA_28, pio.PA_29, pio.PA_10 }
local touchchans = {5, 8, 7, 9}
local baselines = {}
local ledstate = {1,1,1,1}
local laststate = {0,0,0,0}

tmr_id = 1
delay_time = 50000

-- Initializations
tmr.start(tmr_id)

for i = 1, #ledpins do
  pio.pin.setdir( pio.OUTPUT, ledpins[ i ] )
  pio.pin.sethigh( ledpins[ i ] )
end

for i = 1, #touchchans do -- collect baselines
  baselines[i] = kin.tsi.init(touchchans[i])
end

print("Press ESC to exit")

while true do
  for i = 1, #touchchans do
    if kin.tsi.read(touchchans[i],baselines[i]) == 1 then
       if laststate[i] == 0 then -- toggle pin only when detecting new press
         ledstate[i] = bit.bxor(1,ledstate[i])
         pio.pin.setval(ledstate[i],ledpins[i])
       end
       laststate[i] = 1
    else
       laststate[i] = 0
    end
  end
  tmr.delay ( tmr_id, delay_time )

  -- Exit if user hits Escape
  key = term.getchar( term.NOWAIT )
  if key == term.KC_ESC then break end 
end