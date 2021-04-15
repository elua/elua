
--[[

Zeisig Labortechnik: eLua board file for the cryogenic machine's
processor board

Notes:

1) This machine controls the internal temperature of specifically
designed viscometry baths with a stability of +/- 0.01 degree C. As
much as I love to see the measurements of a viscometer (usually, the
computation of Billmeyer viscosity of ploymers dissolved in Formic
acid), I personally also love to see eLua on every target platform I
work with. What is fun without SpaceShip OR Game of life?

2) User LEDs (LED0 and LED1) are mapped to pins PB.0 and PB.1
respectively.

R

--]]

return {
  cpu = 'stm32f103vct6',
  components = {
    mmcfs = { spi = 0, cs_port = 0, cs_pin = 4 },
    sercon = { uart = 0, speed = 115200, buf_size = 128 },
    wofs = true,
    romfs = false,
    shell = true,
    term = { lines = 25, cols = 80 },
    cints = true,
    luaints = true,
    linenoise = { shell_lines = 10, lua_lines = 50 },
    rpc = { uart = 0, speed = 115200 },
    adc = { buf_size = 4 },
    stm32_buzzer = true,
    stm32_tft = true,
    xmodem = true,
    niffs = { linear_area = true },
  },
  config = {
    egc = { mode = "alloc" },
    vtmr = { num = 4, freq = 10 },
  },
  modules = {
    generic = { 'all', '-i2c', '-net', '-can' },
    platform = 'all',
  }
}
