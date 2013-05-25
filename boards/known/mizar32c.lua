-- Mizar32 C board configuration

return {
  cpu = 'at32uc3a0128',
  components = {
    sercon = { uart = 0, speed = 115200, buf_size = 2048 },
    mmcfs = { spi = 4, cs_port = 0, cs_pin = "SD_MMC_SPI_NPCS_PIN" },
    cints = true,
  },
  config = {
    vtmr = { num = 4, freq = 10 },
    ram = {
      internal_rams = 0,
      ext_start = { "SDRAM + ELUA_FIRMWARE_SIZE" },
      ext_size = { "SDRAM_SIZE - ELUA_FIRMWARE_SIZE" },
    }
  },
  modules = {
    generic = { 'all_lua', '-lua_debug', 'pd', 'pio', 'tmr' }
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
  build = {
    target = "lualong",
    allocator = "simple"
  }
}

