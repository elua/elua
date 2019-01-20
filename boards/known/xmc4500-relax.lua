
-- Infineon XMC4500 Relax boot-kit build configuration

--[[

Notes:

1) The XMC4500 has its own SDMMC peripheral. So, we won't go the
   regular SPI way to talk to the SD card. This also frees up an
   additional USIC (configured as SPI) channel. At the moment, spi,
   cs_port and cs_pin are set to 0. Perhaps there is another way
   to override this requirement?

2) This point is specific to the Relax kit:

   When the Relax board is plugged into a machine, it will not get
   enumerated as a Jlink virtual COM device (OR a ttyACM0) like the
   Hexagon. I presume this has something to do with the Jlink VCOM
   firmware used on the other XMC4500 at the debug port of the kit.

   I use a Silicon Labs CP210x USB to UART bridge to interact with
   eLua. Simply connect the bridge to the pin-outs on the boot-kit.
   UART RX is Pin 1.4, and TX is Pin 1.5. [An Adafruit's FTDI friend
   might work equally well].

   Otherwise, the chip is exactly the same as the one used on the
   Hexagon board. Enjoy eLua!

--]]


return {
  cpu = 'xmc4500f144k1024',
  components = {
    sercon = { uart = 0, speed = 115200 },
    wofs = false,
    romfs = true,
    mmcfs = { spi = 0, cs_port = 0, cs_pin = 0 },
    shell = true,
    term = { lines = 25, cols = 80 },
    linenoise = { shell_lines = 10, lua_lines = 50 },
    xmodem = false,
    niffs = false,
  },
  config = {
    egc = { mode = "alloc" },
    ram = { internal_rams = 3 },
  },
  modules = {
    generic = { 'all', '-tmr', '-i2c', '-net', '-adc', '-spi', '-uart', '-can', '-pwm', '-rpc' },
    platform = 'all',
  }
}
