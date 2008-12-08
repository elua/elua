local pio = pio
  
module(...)

BTN_UP      = pio.PE_0
BTN_DOWN    = pio.PE_1
BTN_LEFT    = pio.PE_2
BTN_RIGHT   = pio.PE_3
BTN_SELECT  = pio.PF_1
btnpressed = function( button )
  return ( pio.get ( button ) == 0 )
end

LED_1 = pio.PF_0

pio.input( BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT )
pio.pullup( BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT )
pio.output ( LED_1 )

                
