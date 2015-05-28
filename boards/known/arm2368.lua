-- CP-JR ARM2368 board build configuration
-- http://www.futurlec.com/ARM2368_Controller.shtml

return {
  cpu = 'lpc2368',
  components = {
    sercon = { uart = 0, speed = 115200, timer = 0 },
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
    ram = { ext_start = { 0x40000000 }, ext_size = { 32 * 1024 } } 
  },
  modules = {
    generic = { 'all', '-i2c', '-net', '-spi', '-can' }
  }
}

