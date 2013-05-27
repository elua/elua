-- MBEd build configuration

return {
  cpu = 'lpc1768',
  components = {
    sercon = { uart = 0, speed = 115200 },
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 4 },
    xmodem = true,
    lpc17xx_semifs = true
  },
  config = {
    egc = { mode = "alloc" },
    ram = { internal_rams = 2 }
  },
  modules = {
    generic = { 'all', "-spi", "-can", "-i2c", "-net" },
    platform = 'all',
  },
}

