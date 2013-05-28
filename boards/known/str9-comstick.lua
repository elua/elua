-- STR9-comStick board build configuration

return {
  cpu = 'str912faw44',
  components = {
    sercon = { uart = 1, speed = 115200 },
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = true,
    linenoise = { shell_lines = 10, lua_lines = 50 },
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 2 },
    xmodem = true
  },
  config = {
    vtmr = { num = 4, freq = 16 },
  },
  modules = {
    generic = { 'all', "-net", "-can" },
    platform = 'all'
  }
}

