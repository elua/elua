-- Control LED intensity with PWM on two channels

local pwmid1, pwmid2, tmrid = 0, 1, 3

if pd.board() ~= "MOD711" then
  print "Unsopported board"
  return
end
 
print "Control LED with PWM (fade up/down)"
print "Press any key to exit"
local crtduty, incr = 10, 5
pwm.setup( pwmid1, 50000, crtduty )
pwm.setup( pwmid2, 50000, 100 - crtduty )
pwm.start( pwmid1 )
pwm.start( pwmid2 )
while uart.recv( 1, 0, 0 ) < 0 do
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

