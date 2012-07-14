-- STR-E912 build configuration

return {
  cpu = 'str912faw44',
  components = {
    sercon = { uart = 0, speed = 115200 },
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = true,
    linenoise = { shell_lines = 10, lua_lines = 50 },
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 2, first_timer = 0, num_timers = 0 },
    xmodem = true
  },
  config = {
    vtmr = { num = 4, freq = 16 },
  },
  modules = {
    generic = 'all',
    exclude_generic = { "net", "can" },
    platform = 'all'
  }
}

