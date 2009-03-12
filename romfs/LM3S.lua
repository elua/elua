local pio = pio
  
module(...)

BTN_UP      = "PE_0"
BTN_DOWN    = "PE_1"
BTN_LEFT    = "PE_2"
BTN_RIGHT   = "PE_3"
BTN_SELECT  = "PF_1"
btnpressed = function( button )
  return ( pio[ button ] == 0 )
end

LED_1 = "PF_0"

pio.PE_0_3_DIR, pio.PF_1_DIR = pio.INPUT, pio.INPUT
pio.PE_0_3_PULL, pio.PF_1_PULL = pio.PULLUP, pio.PULLUP
pio.PF_0_DIR = pio.OUTPUT

