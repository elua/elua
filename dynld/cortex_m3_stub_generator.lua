-- Cortex M3 stub generator for the dynamic loader

module( ..., package.seeall )
local sf = string.format

local gen = {}

function new( filename )
  local f = io.open( filename, "wb" )
  if not f then return nil end
  local self = {}
  self.f = f
  -- Write header
  f:write( "// Cortex M3 dynamic loader stubs\n" )
  f:write( "// AUTOMATICALLY GENERATED - DO NOT EDIT!\n\n" )
  f:write( "  .text\n" )
  f:write( "  .code16\n" )
  f:write( "  .syntax unified\n\n" )
  setmetatable( self, { __index = gen } )
  return self
end

gen.add_function = function( self, name, address, index )
  local f = self.f  
  f:write( sf( "  .export %s\n", name ) )
  f:write( "  .thumb\n" )
  f:write( "  .thumb_func\n" )
  f:write( sf( "%s:\n", name ) )
  f:write( sf( "  svc     %d\n", index ) )
  f:write( "  bx      lr\n\n" )
end

gen.finalize = function( self )
  self.f:write( "  .end\n" )
  self.f:close()
end
