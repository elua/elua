-- Mizar32 B board configuration

return {
  cpu = 'at32uc3a0256',
  components = {
    sercon = { uart = "cdc", speed = 115200 },
    cdc = true,
    mmcfs = { spi = 4, cs_port = 0, cs_pin = "SD_MMC_SPI_NPCS_PIN" },
    adc = { buf_size = 2 },
    term = { lines = 25, cols = 80 },
    tcpip = { ip = "192.168.1.10", netmask = "255.255.255.0", gw = "192.168.1.1", dns = "192.168.1.1" },
    dhcp = true,
    dns = true,
    cints = true,
    luaints = true,
    shell = true,
    xmodem = true,
    avr32_lcd = true,
    avr32_rtc = true,
  },
  config = {
    vtmr = { num = 4, freq = 10 },
    ram = {
      ext_start = { "SDRAM + ELUA_FIRMWARE_SIZE" },
      ext_size = { "SDRAM_SIZE - ELUA_FIRMWARE_SIZE" },
    }
  },
  modules = {
    generic = { 'all', "-rpc", "-can" },
    platform = { 'all' },
    platform_name = 'mizar32'
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

