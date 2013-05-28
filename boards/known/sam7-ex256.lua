-- SAM7-EX256 build configuration

return {
  cpu = 'at91sam7x256',
  components = {
    sercon = { uart = 0, speed = 115200, buf_size = 128 },
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = true,
    linenoise = { shell_lines = 10, lua_lines = 50 },
    rpc = { uart = 0, speed = 115200 },
    xmodem = true
  },
  config = {
    egc = { mode = "alloc" },
    vtmr = { num = 4, freq = 4 },
  },
  modules = {
    generic = { 'all', '-i2c', '-net', '-adc', '-spi', '-can' }
  }
}

