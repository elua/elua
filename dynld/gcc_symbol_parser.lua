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
  local symmap = {}
  for l in f:lines() do
    l = l:gsub( "%s+", " " )
    local _, __, v, t, g, n = l:find( "%s%d-:%s(%x+)%s%d-%s(.-)%s(.-)%s[^%d]+%d-%s(.*)%s*$" )
    if #n > 0 and t ~= "NOTYPE" then
      syms[ n ] = { address = tonumber( "0x" .. v ), isfunction = t == 'FUNC', isglobal = g == 'GLOBAL' }
      symmap[ #symmap + 1 ] = { n, syms[ n ] }
    end
  end
  f:close()
  print( string.format( "done, found %d symbols", #symmap ) )
       
  local self = {}
  setmetatable( self, { __index = parser } )
  self.syms = syms
  self.symmap = symmap
  return self
end

parser.lookup = function( self, symbol )
  return self.syms[ symbol ]
end

parser._iterfunc = function( self, i )
  i = i + 1
  local v = self.symmap[ i ]
  if v then return i, { v[ 1 ], v[ 2 ].address, v[ 2 ].isfunction, v[ 2 ].isglobal } end
end

parser.iter = function( self )
  return parser._iterfunc, self, 0
end
