-- TVBGone in eLua
-- Adapted from LadyAda's TVBGone project

-- Check codes file
local codes = io.open( "/rom/codes.bin", "rb" )
if codes == nil then
  print "Unable to open TVBGone codes file"
  return
end 

local pwmid, tmrid
if pd.board() == 'EK-LM3S8962' or pd.board() == 'EK-LM3S6965' then
  pwmid, tmrid = 2, 1
  pwm.setclock( pwmid, 25000000 )
  led, startpin, exitpin = "PF_0", "PF_1", "PE_1"
else
  print( pd.board() .. " not supported with this example" )
  return
end

-- Setup PIO
gpio.dir[ led ] = gpio.OUTPUT
gpio.dir[ startpin ], gpio.dir[ exitpin  ] = gpio.INPUT, gpio.INPUT
gpio.pull[ startpin  ], gpio.pull[ exitpin ] = gpio.PULLUP, gpio.PULLUP

-- Local variables
local _, fstr, freq, timesstr, ontime, offtime, runme

-- Send all the codes in an infinite loop
collectgarbage( "stop" )
runme = true
while runme do
  while gpio[ startpin ] == 1 do 
    if gpio[ exitpin ] == 0 then
      runme = false 
      break 
    end
  end
  if not runme then break end
  gpio[ led ] = 1
  codes:seek( "set", 0 )
  while true do
    fstr = codes:read( 4 )
    if fstr == nil then break end
    _, freq = pack.unpack( fstr, "<L" )
    pwm.setup( pwmid, freq, 50 )
    while true do
      timesstr = codes:read( 4 )
      _, ontime = pack.unpack( timesstr, "<H" )
      _, offtime = pack.unpack( timesstr, "<H", 3 )
      pwm.start( pwmid )
      tmr.delay( tmrid, ontime * 10 )
      pwm.stop( pwmid )
      if offtime == 0 then break end
      tmr.delay( tmrid, offtime * 10 )
      if gpio[ exitpin ] == 0 then
        runme = false
        break 
      end          
    end
    if not runme then break end
    tmr.delay( tmrid, 250000 )
  end
  gpio[ led ] = 0
  if not runme then break end  
  tmr.delay( tmrid, 500000 )
end

codes:close()

