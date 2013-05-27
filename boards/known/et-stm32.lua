-- ET-STM32 build configuration

return {
  cpu = 'stm32f103re',
  components = {
    sercon = { uart = 0, speed = 115200, buf_size = 128 },
    wofs = true,
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = true,
    linenoise = { shell_lines = 10, lua_lines = 50 },
    stm32_enc = true,
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 4 },
    xmodem = true
  },
  config = {
    egc = { mode = "alloc" },
    vtmr = { num = 4, freq = 10 },
  },
  modules = {
    generic = { 'all', '-i2c', '-net' },
    platform = 'all',
  }
}

