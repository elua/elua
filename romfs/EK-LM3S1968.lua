--   This auxiliar platform dependent module defines some hardware devices
-- available in the specific development kit.
--   It will be automatically require()d when eLua detects it is running on the
-- respective platform, exposing auxiliar constants and functions to facilitate
-- and keep portable the access to the underlying hardware.
--   The code configures the MCU to interface with the platform devices and
-- exposes the following  objects, constants and fuctions:
-- Onboard Buttons:
--    BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT
-- Onboard LED:
-- Auxiliar Function:
--    btn_pressed( button )
--    returns true if the arg button is pressed, false otherwise

local pio = pio
  
module(...)

BTN_UP      = pio.PG_3
BTN_DOWN    = pio.PG_4
BTN_LEFT    = pio.PG_5
BTN_RIGHT   = pio.PG_6
BTN_SELECT  = pio.PG_7

btn_pressed = function( button )
  return pio.pin.getval( button ) == 0
end

LED_1 = pio.PG_2

pio.pin.setdir( pio.INPUT, BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT )
pio.pin.setpull( pio.PULLUP, BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT )
pio.pin.setdir( pio.OUTPUT, LED_1 )

