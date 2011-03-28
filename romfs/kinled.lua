local ledpins = { pio.PA_11, pio.PA_28, pio.PA_29, pio.PA_10 }
local tmrid, tmrondelay, tmroffdelay = 0, 100000, 1000

local function ledoff( i )
  pio.pin.sethigh( ledpins[ i ] )
end

local function ledon( i )
  pio.pin.setlow( ledpins[ i ] )
end

for i = 1, #ledpins do
  ledoff( i )
  pio.pin.setdir( pio.OUTPUT, ledpins[ i ] )
end

local i = 1
while true do
  ledon( i )
  tmr.delay( tmrid, tmrondelay )
  ledoff( i )
  tmr.delay( tmrid, tmroffdelay )
  i = i + 1
  if i > #ledpins then i = 1 end
end
