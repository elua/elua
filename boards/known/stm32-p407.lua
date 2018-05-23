-- Olimex STM32-P407 board configuration

return {
  cpu = 'stm32f407vg',
  components = {
    sercon = { uart = 2, speed = 115200 },
    romfs = true,
    shell = { advanced = true },
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    stm32f4_enc = true,
    rpc = { uart = 2, speed = 115200 },
    adc = { buf_size = 2 },
    xmodem = true,
    cints = true, 
    luaints = true
  },
  config = {
    egc = { mode = "alloc" },
    vtmr = { num = 4, freq = 10 },
    ram = { internal_rams = 2 },
    clocks = { external = 25000000, cpu = 168000000 },
    stm32f4_uart_pins = { con_rx_port = 2, con_rx_pin = 11, con_tx_port = 2, con_tx_pin = 10 }
  },
  modules = {
    generic = { 'all', "-i2c", "-net" },
    platform = 'all',
    platform_name = 'stm32'
  },
}

