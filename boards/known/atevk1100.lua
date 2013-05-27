-- ATEVK1100 board configuration

return {
  cpu = 'at32uc3a0512',
  components = {
    sercon = { uart = 0, speed = 115200, buf_size = 2048 },
    mmcfs = { spi = 5, cs_port = 0, cs_pin = "SD_MMC_SPI_NPCS_PIN" },
    adc = { buf_size = 2 },
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = true,
    shell = true,
    xmodem = true,
    romfs = true
  },
  config = {
    vtmr = { num = 4, freq = 10 },
    ram = {
      ext_start = { "SDRAM" },
      ext_size = { "SDRAM_SIZE" },
    }
  },
  modules = {
    generic = { 'all', "-rpc", "-can", "-net" }
  },
  macros = {
    { "BOARD_SPI0_SCK_PIN", "AVR32_PIN_PA13" },
    { "BOARD_SPI0_SCK_PIN_FUNCTION", 0 },
    { "BOARD_SPI0_MISO_PIN", "AVR32_PIN_PA11" },
    { "BOARD_SPI0_MISO_PIN_FUNCTION", 0 }, 
    { "BOARD_SPI0_MOSI_PIN", "AVR32_PIN_PA12" },
    { "BOARD_SPI0_MOSI_PIN_FUNCTION", 0 },
    { "BOARD_SPI0_CS_PIN", "AVR32_PIN_PA10" },
    { "BOARD_SPI0_CS_PIN_FUNCTION", 0 },
    { "BOARD_SPI1_SCK_PIN", "AVR32_PIN_PA15" },
    { "BOARD_SPI1_SCK_PIN_FUNCTION", 1 },
    { "BOARD_SPI1_MISO_PIN", "AVR32_PIN_PA17" },
    { "BOARD_SPI1_MISO_PIN_FUNCTION", 1 }, 
    { "BOARD_SPI1_MOSI_PIN", "AVR32_PIN_PA16" },
    { "BOARD_SPI1_MOSI_PIN_FUNCTION", 1 },
    { "BOARD_SPI1_CS_PIN", "AVR32_PIN_PA14" },
    { "BOARD_SPI1_CS_PIN_FUNCTION", 1 },
  },
}

