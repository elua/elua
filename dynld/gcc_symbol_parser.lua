-- GCC specific symbol parser (uses ld map files)

module( ..., package.seeall )

parser = {}

new = function( mapname )
  local f = io.open( mapname, "rb" ) 
  assert(f)
  local gp = {}
  setmetatable( gp, { __index = parser } )
  gp.data = f:read( "*a" )
  f:close()
  return gp
end

parser.lookup = function( self, symbol )
  local pattern = string.format( "%%s-(0x%%x-)%%s-%s%%s-\n", symbol )
  return self.data:match( pattern )
end

