-- eLua-PUC board build configuration

return {
  cpu = 'lpc2468',
  components = {
    sercon = { uart = 0, speed = 115200, buf_size = 128 },
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = true,
    linenoise = { shell_lines = 10, lua_lines = 50 },
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 4 },
    xmodem = true
  },
  config = {
    vtmr = { num = 4, freq = 4 },
    ram = { ext_start = { 0xA0000000 }, ext_size = { 8 * 1048576 } } 
  },
  modules = {
    generic = { 'all', '-i2c', '-net', '-spi', '-can' }
  }
}

