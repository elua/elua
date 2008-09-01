-- TVBGone in eLua
-- Adapted from LadyAda's TVBGone project
-- Currently working only on the LM3S platform

-- Safety checks
if pwm == nil or tmr == nil or pio == nil then
  print "The PWM and/or TMR/PIO modules not found, exiting"
  return
end

-- Check codes file
local codes = io.open( "/rom/codes.bin", "rb" )
if codes == nil then
  print "Unable to open TVBGone codes file"
  return
end 

-- Setup PWM
local pwmid = pwm.PWM2
local tmrid = tmr.TMR0
local led = pio.PF_0
pwm.setclock( pwmid, 25000000 )
pio.output( led )
local _, fstr, freq, timesstr, ontime, offtime

-- Send all the codes in an infinite loop
collectgarbage( "stop" )
while true do
  pio.set( led )
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
    end
    tmr.delay( tmrid, 250000 )
  end
  pio.clear( led )
  tmr.delay( tmrid, 1000000 )
end

codes:close()
