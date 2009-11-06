-- Control LED intensity with PWM

local pwmid, tmrid, ledpin
if pd.board() == 'EK-LM3S8962' or pd.board() == 'EK-LM3S6965' or pd.board() == 'ET-STM32' or pd.board() == 'EAGLE-100' then
  pwmid, tmrid = 0, 1
  pwm.setclock( pwmid, 25000000 )
elseif pd.board() == 'ELUA-PUC' then
  pwmid, tmrid = 7, 0
  local psel = cpu.r32( cpu.IO_PINSEL3 )
  psel = bit.bor( bit.band( psel, 0xFFFFFCFF ), 0x00000200 )
  cpu.w32( cpu.IO_PINSEL3, psel )
else
  print( pd.board() .. " not supported by this example" )
  return
end

print "Control LED with PWM (fade up/down)"
print "Press any key to exit"
local crtduty, incr = 10, 5
tmr.start( tmrid )
pwm.setup( pwmid, 50000, crtduty )
pwm.start( pwmid )
while uart.getchar( 0, 0 ) == "" do
  if crtduty == 95 or crtduty == 5 then
    incr = -incr
  end
  crtduty = crtduty + incr
  pwm.setup( pwmid, 50000, crtduty )  
  tmr.delay( tmrid, 50000 )  
end

pwm.stop( pwmid )
