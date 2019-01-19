
-- Infineon XMC4700 Relax boot-kit build configuration

--[[

Notes:

1) This kit has no SDMMC slot! I have to solder my own slot. :( It is
   impossible that this kit does not come with an SDMMC interface! :(

2) I now have an XMC47 (!lite) relax kit with a uSD slot and Ethernet.

--]]


return {
  cpu = 'xmc4700f144k2048',
  components = {
    sercon = { uart = 0, speed = 115200 },
    wofs = false,
    romfs = true,
    mmcfs = { spi = 0, cs_port = 0, cs_pin = 0 },
    shell = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    xmodem = false,
    niffs = false,
    xmc47_dac = true,
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
