-- eLua examples
-- Control LED intensity with PWM

local pwmid1, pwmid2, tmrid = 4, 5, 0

print "Control LED with PWM (fade up/down)"
print "Press any key to exit"
local crtduty, incr = 10, 5
tmr.start( tmrid )
pwm.setup( pwmid1, 50000, crtduty )
pwm.setup( pwmid2, 50000, 100 - crtduty )
pwm.start( pwmid1 )
pwm.start( pwmid2 )

while uart.getchar( 3, 0 ) == "" do
  if crtduty == 95 or crtduty == 5 then
    incr = -incr
  end
  crtduty = crtduty + incr
  pwm.setup( pwmid1, 50000, crtduty )  
  pwm.setup( pwmid2, 50000, 100 - crtduty )  
  tmr.delay( tmrid, 50000 )  
end

pwm.stop( pwmid1 )
pwm.stop( pwmid2 )

