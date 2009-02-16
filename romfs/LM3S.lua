local gpio = gpio
  
module(...)

BTN_UP      = "PE_0"
BTN_DOWN    = "PE_1"
BTN_LEFT    = "PE_2"
BTN_RIGHT   = "PE_3"
BTN_SELECT  = "PF_1"
btnpressed = function( button )
  return ( gpio[ button ] == 0 )
end

LED_1 = "PF_0"

gpio.PE_0_3_DIR, gpio.PF_1_DIR = gpio.INPUT, gpio.INPUT
gpio.PE_0_3_PULL, gpio.PF_1_PULL = gpio.PULLUP, gpio.PULLUP
gpio.PF_0_DIR = gpio.OUTPUT

