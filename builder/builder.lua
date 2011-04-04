-- Set module paths

package.path = package.path .. ";utils/?.lua;builder/?.lua;builder/gen/?.lua;builder/ui/?.lua"
local utils = require "utils"
local gen = require "gen"
local sf = string.format
local generators = {}

--local cf = require "builder.platforms.lpc24xx"
--local c = cf.cpu_list.lpc2468
local cf = require "builder.platforms.lm3s"
local c = cf.cpu_list.lm3s8962

-- First create all I/O generators
local io_gens = { 'gen-io-pio', 'gen-io-pwm', 'gen-io-uart', 'gen-io-timer' }
utils.foreach( io_gens, function( k, v ) require( v ).new( c ) end )

-- Then run them
-- utils.foreach( gen.get_gen_array(), function( k, v ) v.gen:generate( io.stdout ) end )

-------------------------------------------------------------------------------
-- Dependency-related functions

local function _gen( comp )  
  return assert( gen.find_generator( comp ) )
end

-- Compute all the dependencies of a component
local function get_all_deps( comp )
  local deps = {}
  local explored = {}
  local function dfs( c )
    for i = 1, #c:get_deps() do
      local d = c:get_deps()[ i ]
      if not explored[ d ] then
        explored[ d ] = true
        dfs( _gen( d ) )
      end
    end
  end
  dfs( comp )
  utils.foreach( explored, function( k, v ) table.insert( deps, k ) end )
  return deps
end

-------------------------------------------------------------------------------
-- Components initialization

local comp_gens = { 'gen-comp-vtmr' }
utils.foreach( comp_gens, function( k, v ) require( v ).new( cf ) end )

-- Initialize all generators
utils.foreach( comp_gens, function( k, v ) _gen( v ):init() end )

-------------------------------------------------------------------------------
-- Configurator functions

local ui = require "builder.ui.ui-cmdline".new_ui()

-- Build a list of components and their respective state
local function build_component_list( clist )
  local all = {}
  local ctable = { configured = "[C]", enabled = "[E]", disabled = "[D]" }
  -- Iterate through clist and get all the components and their state
  utils.foreach( clist, function( k, v )
    local g = _gen( v )
    table.insert( all, ctable[ g:get_state() ] .. " " .. v )
  end ) 
  return all
end

-- Builder action: show list of dependencies
local function act_show_deps( g )
  local s = ''
  -- First show own dependencies
  table.foreach( get_all_deps( g ), function( k, depname )
    local dg = _gen( depname )
    s = s .. sf( 'Component depends on "%s" (%s) which is currently %s.\n', depname, dg:get_friendly_name(), dg:get_state() )
  end )
  return s:sub( 1, -2 )
end

-- Builder action: enable
local function act_enable( g )
  local can_enable = g:can_enable()
  local reason = "ERROR: the component can't be enabled\n\n"
  if not can_enable then
    return reason .. "This is most likely a configurator error, check the generator implementation."
  end
  local deps = get_all_deps( g )

  -- No dependencies? That makes me a happy bunny
  if #deps == 0 then
    g:enable( true )
    return true
  end

  -- Check deps
  reason = ""
  for k, depname in pairs( deps ) do
    local dg = _gen( depname )
    if not dg:is_enabled() then 
      reason = reason .. sf( "Error: component '%s' (%s) (needed by '%s') is not enabled\n", depname, dg:get_friendly_name(), dg:get_name() )
    end
  end
  if reason ~= "" then
    return reason .. "\nError: the component can't be enabled"
  end

  -- Everything OK, enable component and notify the rest of the components of this change
  g:enable( true )
  g.notify_all( g:get_name(), true )
  return true
end

-- Build a list of actions corresponding to the state of a component
local function build_action_list( cgen )
  local act_show_conf = { action = 'show', text = 'Show configuration' }
  local act_disable = { action = 'disable', text = 'Disable component' }
  local act_enable = { func = act_enable, text = 'Enable component' }
  local act_configure = { action = 'configure', text = 'Configure component' }
  local act_show_deps = #cgen:get_deps() > 0 and { func = act_show_deps, text = "Show dependencies" }
  local acttable = { 
    configured = { act_show_conf, act_configure, act_disable, act_show_deps },
    enabled = { act_configure, act_disable, act_show_deps },
    disabled = { act_enable, act_show_deps }
  }
  return acttable[ cgen:get_state() ]
end

-- Enter the configurator loop
while true do
  -- Show the list of components and their state, ask for an action
  local l = build_component_list( comp_gens )
  local res = ui:choice( l, 1, "Main configurator menu", "Choose a component to configure:\n" )
  if res == 'exit' then break end -- [TODO] warn user if the current configuration is not complete
  
  -- We have a component, now find what we can do with it
  local g = _gen( comp_gens[ res ] ) 
  while true do
    l = build_action_list( g )
    res = ui:choice( l, 1, "Component: " .. g:get_component(), g:get_help() .. "\nChoose an action:\n" )
    if res == "exit" then break end
    local func = l[ res ].func
    res = func( g )
    if type( res ) == "string" then ui:info( res, "Component: " .. g:get_component() ) end
    if func ~= act_show_deps then break end
  end
end


