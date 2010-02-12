local pio = pio
  
module(...)

LED_1 = pio.P1_0
LED_2 = pio.P1_1
LED_3 = pio.P1_2
LED_4 = pio.P1_3

pio.pin.setdir( pio.OUTPUT, LED_1 )
pio.pin.setdir( pio.OUTPUT, LED_2 )
pio.pin.setdir( pio.OUTPUT, LED_3 )
pio.pin.setdir( pio.OUTPUT, LED_4 )