-- TCP/IP support component generator

module( ..., package.seeall )
local sf = string.format
local compgen = require "gen-comp"
local utils = require "utils"
local opt = require "option"

local cgen = {}
local base = compgen.compgen
setmetatable( cgen, { __index = base } )

new = function( ptable )
  local self = {}
  setmetatable( self, { __index = cgen } )
  base.init_instance( self, 'tcpip', ptable )
  self:set_friendly_name( 'TCP/IP support' )
  self:set_enable_macro( 'BUILD_UIP' )
  return self
end

-- Initialization function
cgen.init = function( self, t )
  base.init( self, t )
  self:set_help( 'TCP/IP suport based on the uIP stack.' )
  self:add_deps{ self.IO_ETH }
  self:add_option( opt.ip_option( 'ELUA_CONF_IPADDR', '192.168.1.100', 'Static IP address of this board' ) )
  self:add_option( opt.ip_option( 'ELUA_CONF_NETMASK', '255.255.255.0', 'Network mask' ) )
  self:add_option( opt.ip_option( 'ELUA_CONF_DEFGW', '192.168.1.1', 'Default gateway' ) )
  self:add_option( opt.ip_option( 'ELUA_CONF_DNS', '192.168.1.1', 'DNS server (needed only id the DNS client is used' ) )
end

cgen.__type = function()
  return "gen-comp-tcpip"
end
