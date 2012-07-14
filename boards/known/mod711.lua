-- MOD711 build configuration

return {
  cpu = 'str711fr2',
  components = {
    sercon = { uart = 1, speed = 38400, timer = 0 },
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    xmodem = true
  },
  modules = {
    generic = { 'pio', 'tmr', 'pd', 'pwm', 'uart', 'term', 'pack', 'bit', 'elua', 'cpu', 'all_lua' }
  }
}

