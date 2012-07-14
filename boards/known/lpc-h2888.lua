-- LPC-H2888 build configuration

return {
  cpu = 'lpc2888',
  components = {
    sercon = { uart = 0, speed = 115200, timer = 0 },
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    xmodem = true,
    rpc = { uart = 0, speed = 115200 }
  },
  config = {
    ram = { ext_start = { "SDRAM_BASE_ADDR" }, ext_size = { "SDRAM_SIZE" } }
  },
  modules = {
    generic = { 'pio', 'tmr', 'pd', 'uart', 'term', 'pack', 'bit', 'elua', 'cpu', 'rpc', 'all_lua' }
  }
}

