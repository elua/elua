-- LPCXpresso LPC1769 build configuration

return {
  cpu = 'lpc1769',
  components = {
    sercon = { uart = "cdc", speed = 115200 },
    romfs = true,
    shell = true,
    cdc = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 4 },
    xmodem = true
  },
  config = {
    egc = { mode = "alloc" },
    ram = { internal_rams = 2 },
    clocks = { external = 12000000, cpu = 120000000 },
    -- P0.0 UART3 TX, P0.1 UART3 RX, P0.29 USB D+, P0.30 USB D-, P1.18 USB_UP_LED, P2.9 USB CONNECT
    map_pins = {
       port        = { 0, 0,  0,  0,  1, 2 },
       pin         = { 0, 1, 29, 30, 18, 9 },
       pinfunction = { 2, 2,  1,  1,  1, 1 }
    }
  },
  modules = {
    generic = { 'all', "-spi", "-i2c", "-net" },
    platform = 'all',
    platform_name = 'lpcxpresso'
  },
}

