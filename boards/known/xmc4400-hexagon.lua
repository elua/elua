
-- Infineon XMC4400 Hexagon boot-kit build configuration

--[[

Notes:

1) The XMC4400 hexagon uses a different set of pins for RX and TX;
   RX = 1.5, TX = 1.7.

--]]


return {
  cpu = 'xmc4400f100x512',
  components = {
    sercon = { uart = 0, speed = 115200 },
    wofs = false,
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    xmodem = false
  },
  config = {
    egc = { mode = "alloc" },
    ram = { internal_rams = 3 },
  },
  modules = {
    generic = { 'all', '-i2c', '-net', '-adc', '-spi', '-uart', '-can', '-pwm', '-rpc' },
    platform = 'all',
  }
}
