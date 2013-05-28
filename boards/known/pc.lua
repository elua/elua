-- Booatable eLua image build configuration

return {
  cpu = 'i386',
  components = {
    sercon = { uart = 0, speed = 0 },
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
  },
  modules = {
    generic = { 'pd', 'all_lua', 'term' }
  }
}

