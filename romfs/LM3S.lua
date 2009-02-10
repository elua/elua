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

LED_1 = pio.PF_0

pio.dir[ BTN_UP ], pio.dir[ BTN_DOWN ], pio.dir[ BTN_LEFT ], pio.dir[ BTN_RIGHT ], pio.dir[ BTN_SELECT ] = 
  pio.INPUT, pio.INPUT, pio.INPUT, pio.INPUT, pio.INPUT
pio.pull[ BTN_UP ], pio.pull[ BTN_DOWN ], pio.pull[ BTN_LEFT ], pio.pull[ BTN_RIGHT ], pio.pull[ BTN_SELECT ] = 
  pio.PULLUP, pio.PULLUP, pio.PULLUP, pio.PULLUP, pio.PULLUP
pio.dir[ LED_1 ] = pio.OUTPUT

