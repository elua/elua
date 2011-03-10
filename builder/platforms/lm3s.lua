-- Configuration file for eLua platform 'lm3s'

-- List of associated CPUs
cpu_list = { lm3s1968 = {}, lm3s8962 = {}, lm3s6965 = {}, lm3s6918 = {}, lm3s9b92 = {} }

-- LM3S8962 CPU: list of peripherals
local c = cpu_list.lm3s8962
c.pio = { num = 7, prefix = 'letter', pin_array = { 8, 8, 8, 8, 4, 4, 2 } }
c.spi = { num = 1 }
c.uart = { num = 2 } 
c.adc = { num = 4 }
c.timer = { num = 4 }
c.can = { num = 1 }
c.eth = { num = 1 }

-- LM3S1968: based on LM3S8962
local l = cpu_list.lm3s1968
setmetatable( l, { __index = c } )
l.pio = { num = 8, prefix = 'letter', pin_array = { 8, 8, 8, 4, 4, 8, 8, 4 } }
l.eth = nil

-- LM2S6965: based on LM3S8962
l = cpu_list.lm3s6965
setmetatable( l, { __index = c } )
l.uart = { num = 3 }

-- LM3S6918: based on LM3S8962
l = cpu_list.lm3s6918
setmetatable( l, { __index = c } )
l.pwm = nil

-- LM3S9B92: based on LM3S8962
l = cpu_list.lm3s9b92
setmetatable( l, { __index = c } )
l.pio = { num = 10, prefix = 'letter', pin_array = { 8, 8, 8, 8, 8, 6, 8, 8, 0, 8 } }
l.uart = { num = 3 }

