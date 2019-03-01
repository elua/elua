
-- Infineon XMC4300 EtherCat kit - Version 1.1

--[[

Notes:

1) Again, No SDMMC. That makes me sad! :( We have to kludge on
   existing boilerplate in gen/ to get SPI working with an external SD
   card adapter.

   TODO: SPI pin details?

2) The board simply looks beautiful! It certainly reminds me of the
   EK-LM3S8962 days with Edelstoff! :)

3) DFRobot LCD keypad shield: LCD_DATA7: To get an Arduino 16x2 LCD
   working with the XMC4300 Relax kit, you must short the pins P15_9
   and P2_4. For pin allocation, see xmc4000/lcd.h.

--]]


return {
  cpu = 'xmc4300f100k256',
  components = {
    sercon = { uart = 0, speed = 115200 },
    wofs = false,
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    xmodem = false,
    niffs = false,
    xmc43_dac = true,
    xmc43_lcd = true,
    xmc43_capture = true,
  },
  config = {
    egc = { mode = "alloc" },
    ram = { internal_rams = 1 },
  },
  modules = {
    generic = { 'all', '-tmr', '-i2c', '-net', '-adc', '-spi', '-uart', '-can', '-pwm', '-rpc', '-fs' },
    platform = 'all',
  }
}
