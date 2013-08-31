-- EK-LM3S9D92 build configuration

return {
  cpu = 'lm3s9d92',
  components = {
    sercon = { uart = 0, speed = 115200 },
    wofs = true,
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 2 },
    xmodem = true,
    linenoise = { shell_lines = 10, lua_lines = 30 } 
  },
  config = {
    vtmr = { num = 4, freq = 4 },
  },
  modules = {
    generic = { 'all', '-i2c', '-net' },
  }
}

