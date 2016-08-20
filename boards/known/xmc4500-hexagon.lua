
-- Infineon XMC4500 Hexagon boot-kit build configuration

--[[

Notes:

1) The XMC4500 has its own SDMMC peripheral. So, we won't go the
   regular SPI way to talk to the SD card. This also frees up an
   additional USIC (configured as SPI) channel. At the moment, spi,
   cs_port and cs_pin are set to 0. Perhaps there is another way
   to override this requirement?

--]]


return {
  cpu = 'xmc4500f144k1024',
  components = {
    sercon = { uart = 0, speed = 115200 },
    xmc45_pot = true,
    xmc45_dts = true,
    xmc45_rtc = true,
    xmc45_disp = true,
    wofs = false,
    romfs = true,
    mmcfs = { spi = 0, cs_port = 0, cs_pin = 0 },
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
