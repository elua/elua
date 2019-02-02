
-- Infineon XMC4500 Hexagon SDRAM boot-kit build configuration

--[[

Notes:

1) This XMC4000 board has the XMC4500E144K1024 chip (BGA package).

2) While this XMC4500 also has its own SDMMC peripheral, we can't use
   the hardware SDMMC pins because some of the data lines are used to
   talk to the ISSI SDRAM controller. So, on this board, it is all
   about SDRAM. To have an SD card interface, SPI is the only way.

3) To hook up an external SD card adapter, use the following pin
   configurations:

   MISO pin: #D6 (P3.14) / FLASH_QSPI_IO1
   MOSI pin: #D7 (P3.15) / FLASH_QSPI_IO0
   SCLKOUT pin: #C5 (P0.13) / FLASH_QSPI_CLK
   Slave select 0 pin: #E7 (P3.4)

   I used DAVE4 to generate the boilerplate configuration with DMA
   transfers enabled for SPI.

4) Refer to the XMC4500E144K1024 board reference manual. The external
   SD card adapter should be connected to the HMI interface using the
   KitSat connector.

5) I ran Dimitry Grinberg's uARM on this controller after many years
   using configuration 3). If you choose to do this, there's not a lot
   you can do thereafter. Forget RTC. And please certainly forget the
   custom LCD shield.

6) Clearly, 32GB Samsung cards have a problem with higher SPI bus
   speeds. It works fine at 5000 kHz to 8000 KHz. It however fails
   with the (generated) default of 10000 KHz.

Happy hacking!

--]]


return {
  cpu = 'xmc4500e144k1024',
  components = {
    sercon = { uart = 0, speed = 115200 },
    xmc45_dts = true,
    xmc45_rtc = true,
    wofs = false,
    romfs = true,
    mmcfs = { spi = 0, cs_port = 0, cs_pin = 0 },
    shell = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    xmodem = true
  },
  config = {
    egc = { mode = "alloc" },
    ram = {
      internal_rams = 3,
      ext_start = { 0x60000000 },
      ext_size = { ( 8 * 1048576 ) },
    },
  },
  modules = {
    generic = { 'all', '-tmr', '-i2c', '-net', '-adc', '-spi', '-uart', '-can', '-pwm', '-rpc', '-fs' },
    platform = 'all',
  }
}
