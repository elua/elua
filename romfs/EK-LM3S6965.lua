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

BTN_UP      = pio.PE_0
BTN_DOWN    = pio.PE_1
BTN_LEFT    = pio.PE_2
BTN_RIGHT   = pio.PE_3
BTN_SELECT  = pio.PF_1

btn_pressed = function( button )
  return pio.pin.getval( button ) == 0
end

LED_1 = pio.PF_0

pio.pin.setdir( pio.INPUT, BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT )
pio.pin.setpull( pio.PULLUP, BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT )
pio.pin.setdir( pio.OUTPUT, LED_1 )

