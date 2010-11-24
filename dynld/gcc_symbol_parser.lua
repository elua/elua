-- GCC specific symbol parser (uses ld map files)

module( ..., package.seeall )

local parser = {}

new = function( elfname )
  local cmd = string.format( "arm-none-eabi-readelf --wide -s %s > symtable", elfname )
  local res = os.execute( cmd )
  if res ~=0 then
    print "Error execute arm-none-eabi-objdump"
    return nil
  end
  
  -- Open and parse symbol table
  local f = io.open( "symtable", "rb" )
  assert( f )
  io.write "Parsing symbol table..."
  -- First 4 lines don't have useful information
  for i = 1, 4 do f:read() end
  -- Parse the other lines
  -- Overall format below:
  --  Num:    Value  Size Type    Bind   Vis      Ndx Name
  --   16: 08004a9e     0 NOTYPE  LOCAL  DEFAULT    1 Infinite_Loop 
  --   16: 08004a9e 0 NOTYPE LOCAL DEFAULT 1 Infinite_Loop 
  local syms = {}
  local nsyms = 0
  for l in f:lines() do
    if l:find( "FUNC" ) then
      l = l:gsub( "%s+", " " )
      local _, __, v, n = l:find( "%s%d-:%s(%x+)%s%d-%s[^%d]+%d-%s(.*)%s+$" )
      syms[ n ] = tonumber( "0x" .. v )
      nsyms = nsyms + 1
    end
  end
  f:close()
  print( string.format( "done, found %d function symbols", nsyms ) )
       
  local self = {}
  setmetatable( self, { __index = parser } )
  self.syms = syms
  return self
end

parser.lookup = function( self, symbol )
  return self.syms[ symbol ]
end

