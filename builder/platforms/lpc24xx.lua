-- Configuration file for eLua platform 'lpc24xx'

module( ..., package.seeall )

-- List of associated CPUs
cpu_list = { lpc2468 = {} }

-- LPC24xx CPU: list of peripherals
local c = cpu_list.lpc2468
c.pio = { num = 5, prefix = 'number', pins_per_port = 32 }
c.uart = { num = 4 } 
c.pwm = { num = 12 }
c.adc = { num = 8 }
c.timer = { num = 4 }

