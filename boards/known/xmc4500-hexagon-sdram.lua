
-- Infineon XMC4500 Hexagon SDRAM boot-kit build configuration

--[[

Notes:

1) This XMC4000 board has the XMC4500E144K1024 chip (BGA package).

2) While this XMC4500 also has its own SDMMC peripheral, we can't use
   the hardware SDMMC pins because some of the data lines are used to
   talk to the ISSI SDRAM controller. So, on this board, it is all
   about SDRAM. To have an SD card interface, SPI is the only way.

--]]


return {
  cpu = 'xmc4500e144k1024',
  components = {
    sercon = { uart = 0, speed = 115200 },
    xmc45_dts = true,
    xmc45_rtc = true,
    wofs = false,
    romfs = true,
    mmcfs = { spi = 0, cs_port = 0, cs_pin = 0 },
    shell = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    xmodem = true
  },
  config = {
    egc = { mode = "alloc" },
    ram = {
      internal_rams = 3,
      ext_start = { 0x60000000 },
      ext_size = { ( 8 * 1048576 ) },
    },
  },
  modules = {
    generic = { 'all', '-tmr', '-i2c', '-net', '-adc', '-spi', '-uart', '-can', '-pwm', '-rpc', '-fs' },
    platform = 'all',
  }
}
