-- NETDUINO-2 build configuration

return {
  cpu = 'stm32f205rf',
  components = {
    -- sercon = { uart = 1, speed = 115200, buf_size = 128 },
    sercon = { uart = "cdc", speed = 115200 },
    cdc = { buf_size = 128 },
    wofs = true,
    romfs = true,
    shell = { advanced = true },
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = true,
    -- linenoise = { shell_lines = 10, lua_lines = 50 },
    rpc = { uart = 1, speed = 115200 },
    adc = { buf_size = 2 },
    xmodem = true,
    -- mmcfs = { cs_port = 1, cs_pin = 10, spi = 0 }
  },
  config = {
    egc = { mode = "alloc" },
    vtmr = { num = 4, freq = 10 },
    clocks = { external = 25000000, cpu = 120000000 }
  },
  modules = {
    generic = { 'all', "-i2c", "-net" },
  }
}

