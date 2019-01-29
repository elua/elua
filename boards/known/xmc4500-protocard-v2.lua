
-- Infineon XMC4500 prototype card v2 build configuration

--[[

Notes:

1) This particular target doesn't have any on-board peripherals. The
   board can be used to talk to the Maxon BLDC interface. OR toggle a
   few LEDS from eLua. OR play terminal games.

2) No SDMMC. That makes me sad! :(

3) The fun part about this particular board is that it is very tiny. I
   like to call it "pocket eLua" in the context of this target.

--]]


return {
  cpu = 'xmc4500f144k1024',
  components = {
    sercon = { uart = 0, speed = 115200 },
    xmc45_dts = true,
    xmc45_rtc = true,
    wofs = false,
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    xmodem = false,
    niffs = false,
  },
  config = {
    egc = { mode = "alloc" },
    ram = { internal_rams = 3 },
  },
  modules = {
    generic = { 'all', '-tmr', '-i2c', '-net', '-adc', '-spi', '-uart', '-can', '-pwm', '-rpc' },
    platform = 'all',
  }
}
