-- Control LED intensity with PWM on two channels

local pwmid1, pwmid2, tmrid

if pd.board() == 'MOD711' or pd.board() == 'ET-STM32' then
  pwmid1, pwmid2, tmrid = 0, 1, 3
else
  print( pd.board() .. " not supported by this example" )
  return
end
 
print "Control LED with PWM (fade up/down)"
print "Press any key to exit"
local crtduty, incr = 10, 5
tmr.start( tmrid )

pwm.setup( pwmid1, 50000, crtduty )
pwm.setup( pwmid2, 50000, 100 - crtduty )

pwm.start( pwmid1 )
pwm.start( pwmid2 )

while uart.getchar( 1, 0 ) == "" do
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

