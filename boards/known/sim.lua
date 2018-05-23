-- eLua simulator running on linux

return {
  cpu = 'linux',
  components = {
    sercon = { uart = 0, speed = 0 },
    wofs = true,
    romfs = true,
    shell = { advanced = true },
    term = { lines = 25, cols = 80 },
    mmcfs = { spi = 0, cs_port = 0, cs_pin = 0 },
  },
  modules = {
    generic = { 'pd', 'all_lua', 'term', 'elua' }
  }
}

