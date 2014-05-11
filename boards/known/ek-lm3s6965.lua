-- EK-LM3S6965 build configuration

return {
  cpu = 'lm3s6965',
  components = {
    sercon = { uart = 0, speed = 115200 },
    wofs = true,
    romfs = true,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    lm3s_disp = true,
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 2 },
    tcpip = { ip = "192.168.1.100", dns = "192.168.1.1", gw = "192.168.1.1", netmask = "255.255.255.0" },
    dns = true,
    dhcp = true,
    xmodem = true,
    mmcfs = { spi = 0, cs_port = 3, cs_pin = 0 },
    lm3s_pio = true,
  },
  config = {
    vtmr = { num = 4, freq = 4 },
  },
  modules = {
    generic = { 'all', '-i2c', '-can' },
    platform = 'all',
  }
}

