-- ATEVK1101 board configuration

return {
  cpu = 'at32uc3b0256',
  components = {
    sercon = { uart = 1, speed = 115200, buf_size = 128 },
    adc = { buf_size = 2 },
    mmcfs = { spi = 1, cs_port = 0, cs_pin = "SD_MMC_SPI_NPCS_PIN" },
    term = { lines = 25, cols = 80 },
    cints = true,
    shell = true,
    xmodem = true,
    romfs = true
  },
  config = {
    vtmr = { num = 4, freq = 10 },
  },
  modules = {
    generic = { 'all_lua', 'pd', 'uart', 'pio', 'spi', 'tmr', 'term', 'cpu', 'adc', 'bit', 'pack' }
  },
  macros = {
    { "BOARD_SPI0_SCK_PIN", "AVR32_PIN_PA15" },
    { "BOARD_SPI0_SCK_PIN_FUNCTION", 0 },
    { "BOARD_SPI0_MISO_PIN", "AVR32_PIN_PA25" },
    { "BOARD_SPI0_MISO_PIN_FUNCTION", 0 }, 
    { "BOARD_SPI0_MOSI_PIN", "AVR32_PIN_PA14" },
    { "BOARD_SPI0_MOSI_PIN_FUNCTION", 0 },
    { "BOARD_SPI0_CS_PIN", "AVR32_PIN_PA10" },
    { "BOARD_SPI0_CS_PIN_FUNCTION", 0 },
  },
}

