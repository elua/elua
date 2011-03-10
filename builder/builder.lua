-- Set module paths

package.path = package.path .. ";utils/?.lua;builder/?.lua;builder/gen/?.lua"
local utils = require "utils"
local gen = require "gen-pio"

local c = require "builder.platforms.lpc24xx"
c = c.cpu_list.lpc2468
local g = gen.new( c )
g:generate( io.stdout )
gen = require "gen-uart"
g = gen.new( c )
g:generate( io.stdout )
