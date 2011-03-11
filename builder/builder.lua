-- Set module paths

package.path = package.path .. ";utils/?.lua;builder/?.lua;builder/gen/?.lua"
local utils = require "utils"
local generators = {}

local c = require "builder.platforms.lpc24xx"
c = c.cpu_list.lpc2468

-- First create all I/O generators
local io_gens = { 'gen-io-pio', 'gen-io-pwm', 'gen-io-uart' }
utils.foreach( io_gens, function( k, v ) 
  local m = require( v )
  table.insert( generators, { name = v, gen = m.new( c ) } )
end )

-- Then run them
utils.foreach( generators, function( k, v )
  v.gen:generate( io.stdout )  
end )

