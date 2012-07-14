-- eLua simulator running on linux

return {
  cpu = 'linux',
  components = {
    sercon = { uart = 0, speed = 0 },
    wofs = true,
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
  },
  modules = {
    generic = { 'pd', 'math', 'term', 'elua' }
  }
}

