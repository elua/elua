--   This auxiliary platform dependent module defines some hardware devices
-- available in the specific development kit.
--   It will be automatically require()d when eLua detects it is running on the
-- respective platform, exposing auxiliary constants and functions to facilitate
-- and keep portable the access to the underlying hardware.
--   The code configures the MCU to interface with the platform devices and
-- exposes the following  objects, constants and functions:
-- On-board LEDs:
--  Led1, Led2, Led3 and Led4

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