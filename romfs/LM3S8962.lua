local pio = pio
  
module(...)

BTN_UP      = pio.PE_0
BTN_DOWN    = pio.PE_1
BTN_LEFT    = pio.PE_2
BTN_RIGHT   = pio.PE_3
BTN_SELECT  = pio.PF_1

btn_pressed = function( button )
  return pio.pin.getval( button ) == 0
end

LED_1 = pio.PF_0

pio.pin.setdir( pio.INPUT, pio.PE_0, pio.PE_1, pio.PE_2, pio.PE_3, pio.PF_1 )
pio.pin.setpull( pio.PULLUP, pio.PE_0, pio.PE_1, pio.PE_2, pio.PE_3, pio.PF_1 )
pio.pin.setdir( pio.OUTPUT, pio.PF_0 )

