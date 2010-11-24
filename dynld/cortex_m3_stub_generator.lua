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
  f:write( [[
// Cortex M3 dynamic loader stubs
// AUTOMATICALLY GENERATED - DO NOT EDIT!

  .section    .text
  .align      2
  .thumb
  .syntax     unified
  .extern     udl_functable

#define UDL_FTABLE_ADDRESS      0x20000000  

// Common code for all functions
_udl_common:
  // Push new address to stack
  push        {r0}
  // Restore r0 
  ldr.w       r0, [sp, #4]
  // Restore stack
  add         sp, sp, #8
  // Jump directly to function, it will return to the original caller
  ldr.w       pc, [sp, #-8]

]])
  setmetatable( self, { __index = gen } )
  return self
end

gen.add_function = function( self, name, address, index )
  local f = self.f  
  f:write( sf( [[
  .global     %s
  .thumb_func
%s:
  push        {r0}
  ldr         r0, =UDL_FTABLE_ADDRESS
  ldr         r0, [r0]
  ldr.w       r0, [r0, %d]
  b           _udl_common

]], name, name, ( index - 1 ) * 4 ) )
end

gen.finalize = function( self )
  self.f:write( "  .end\n" )
  self.f:close()
end
