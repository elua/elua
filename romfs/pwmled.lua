-- Control LED intensity with PWM

local pwmid, tmrid, ledpin
if pd.board() == 'EK-LM3S8962' or pd.board() == 'EK-LM3S6965' then
  pwmid, tmrid = 0, 1
  pwm.setclock( pwmid, 25000000 )
else
  print( pd.board() .. " not supported by this example" )
  return
end

print( tmr.mindelay( tmrid ), tmr.maxdelay( tmrid ), tmr.getclock( tmrid ) )

print "Control LED with PWM (fade up/down)"
print "Press any key to exit"
local crtduty, incr = 10, 5
pwm.setup( pwmid, 50000, crtduty )
pwm.start( pwmid )
while uart.recv( 0, 0, 0 ) < 0 do
  if crtduty == 95 or crtduty == 5 then
    incr = -incr
  end
  crtduty = crtduty + incr
  pwm.setup( pwmid, 50000, crtduty )  
  tmr.delay( tmrid, 50000 )  
end

pwm.stop( pwmid )
