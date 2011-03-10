-- Generator interface for the I/O subsystem configuration

module( ..., package.seeall )
local sf = string.format
local gen = require "gen"
local utils = require "utils"

iogen = {}
local base = gen.gentable
setmetatable( iogen, { __index = base } )

iogen.new = function( component, ctable )
  local self = {}
  setmetatable( self, { __index = iogen } )
  self:init_instance( component, ctable )
  return self
end

iogen.init_instance = function( self, component, ctable )
  base.init_instance( self, 'gen-io-' .. component )
  self.component = component
  self.ctable = ctable
  -- IO components are always enabled
  self.is_enabled = true
end

-- Generic IO subsystem initialization: check if the component is available
-- Return the component's initialization table or nil
iogen.init = function( self, t )
  self.ctable = t or self.ctable
  self.is_available = false
  if not utils.tget( self.ctable, self.component, "table" ) then return end
  t = self.ctable[ self.component ]
  self.num = utils.tget( t, "num", "number" )
  if not self.num or self.num <= 0 then return end
  self.is_available = true
  return t
end

-- Simple generator: just write the "num" component
iogen.generate = function( self, dest, moredata )
  dest:write( sf( "// %s configuration\n", self.component:upper() ) )
  self.strout( dest, sf( "#define NUM_%s", self.component:upper() ), sf( "%d\n", self.is_available and self.num or 0 ) )
  if not moredata then dest:write( "\n" ) end
end

iogen.__type = function()
  return "gen-io"
end

