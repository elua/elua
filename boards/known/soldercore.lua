-- Soldercore build configuration

return {
  cpu = 'lm3s9d92',
  components = {
    sercon = { uart = "cdc", speed = 115200 },
    cdc = true,
    wofs = true,
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    rpc = { uart = "cdc", speed = 115200 },
    adc = { buf_size = 2 },
    xmodem = true,
    mmcfs = { spi = 1, cs_port = 6, cs_pin = 7 },
    tcpip = { ip = "192.168.1.100", dns = "192.168.1.1", gw = "192.168.1.1", netmask = "255.255.255.0" },
    dns = true,
    dhcp = true,
  },
  config = {
    vtmr = { num = 4, freq = 5 },
  },
  modules = {
    generic = { 'all', '-i2c' },
  }
}


