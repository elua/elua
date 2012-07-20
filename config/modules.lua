-- Build configuration: module selection

module( ..., package.seeall )
local sf = string.format
local gen = require "generators"

-- List of all Lua modules and their guards
local lua_modules = {
  lua_math = {},
  lua_io = {},
  lua_string = {},
  lua_table = {},
  lua_debug = {},
  lua_package = {},
  lua_co = {}
}

-- List of all eLua generic modules and their build guards
-- (if the module guard evaluates to false at compile time, the module is not included in the build)
-- NOTE: if the guards contain a condition, the condition _MUST_ be specified with spaces next to the operator!
--       For example: NUM_CAN > 0, _NOT_ NUM_CAN>0.
local elua_generic_modules = { 
  adc = { "BUILD_ADC", "NUM_ADC > 0" }, 
  bit = {}, 
  can = { "NUM_CAN > 0"}, 
  cpu = {}, 
  elua = {}, 
  i2c = { "NUM_I2C > 0" }, 
  pack = {}, 
  rpc = { "BUILD_RPC" }, 
  net = { "BUILD_UIP" }, 
  pd = {}, 
  pio = { "NUM_PIO > 0" }, 
  pwm = { "NUM_PWM > 0" },  
  spi = { "NUM_SPI > 0" },  
  term = { "BUILD_TERM" },  
  tmr = { "NUM_TIMER > 0" }, 
  uart = { "NUM_UART > 0" },
}

-- All generic modules (Lua and eLua) in a single table
local all_generic_modules = {}
utils.concat_tables( all_generic_modules, lua_modules )
utils.concat_tables( all_generic_modules, elua_generic_modules )

-- Auxlib names
local auxlibs = {
  lua_math = "LUA_MATHLIBNAME",
  lua_io = "LUA_IOLIBNAME",
  lua_string = "LUA_STRLIBNAME",
  lua_table = "LUA_TABLIBNAME",
  lua_debug = "LUA_DBLIBNAME",
  lua_package = "LUA_LOADLIBNAME",
  lua_co = "LUA_COLIBNAME",
}

-- Open function names
local opennames = {
  lua_math = "luaopen_math",
  lua_io = "luaopen_io",
  lua_string = "luaopen_string",
  lua_table = "luaopen_table",
  lua_debug = "luaopen_debug",
  lua_package = "luaopen_package",
  lua_co = "luaopen_co"
}

-- Map array names. <none> marks the modules that do not have a corresponding
-- map array, which basically means they are not LTR compatible
local mapnames = {
  lua_math = "math_map",
  lua_io = "<none>",
  lua_string = "strlib",
  lua_table = "tab_funcs",
  lua_debug = "dblib",
  lua_package = "<none>",
  lua_co = "co_funcs",
}

-- Return the auxlib name of a given module
local function get_auxlib( m )
  return auxlibs[ m ] or sf( "AUXLIB_%s", m:upper() )
end

-- Return the open function name of a given module
local function get_openf_name( m )
  return opennames[ m ] or sf( "luaopen_%s", m:lower() )
end

-- Return the map array name of a given module
local function get_map_name( m )
  return mapnames[ m ] or sf( "%s_map", m:lower() )
end

local platform_modules
-- Generate a condition string starting from a guard
local function gen_cond_string( g )
  local condition = ''
  for idx, e in pairs( g ) do
    local suffix = idx == #g and "" or " && "
    if e:find( '%s' ) then -- if it has a space, add the condition as it was given, between parantheses
      condition = condition .. "( " .. e .. " )"
    else
      condition = condition .. "defined( " .. e .. " )"
    end
    condition = condition .. suffix
  end
  return condition
end

-- Add/remove a module to the given list
local function process_module( l, name, is_specific )
  local sectname, exclude = is_specific and "platform" or "generic"
  local check = is_specific and platform_modules or all_generic_modules

  -- Handle "+name" / "-name"
  if name:sub( 1, 1 ) == "+" then
    name = name:sub( 2, -1 )
  elseif name:sub( 1, 1 ) == "-" then
    name = name:sub( 2, -1 )
    exclude = true
  end
  local modlist
  -- Handle special cases for 'name'
  if name == "all" then
    modlist = is_specific and utils.table_keys( platform_modules ) or utils.table_keys( all_generic_modules )
  elseif name == "all_lua" then
    if is_specific then return false, "'all_lua' can't be used in attribute 'platform' of section 'modules'" end
    modlist = utils.table_keys( lua_modules )
  elseif name == "all_elua" then
    if is_specific then return false, "'all_elua' can't be used in attribute 'platform' of section 'modules" end
    modlist = utils.table_keys( elua_generic_modules )
  else
    modlist = { name }
  end
  -- For inclusion, check for valid element. For exclusion, check for prior inclusion.
  if exclude then
    for _, m in pairs( modlist ) do
      if not l[ m ] then
        return false, sf( "module '%s' not found in element '%s' of section 'modules'", m, sectname )
      end
      l[ m ] = nil
    end
  else
    for _, m in pairs( modlist ) do
      if not check[ m ] then
        return false, sf( "module '%s' of element '%s' in section 'modules' not found", m, sectname )
      end
      l[ m ] = true
    end
  end
  return true
end

-- Generate the complete module list starting from the board description
function gen_module_list( desc, plconf, platform )
  local mdesc = desc.modules
  if not mdesc then return '' end
  local gen_list_generic, gen_list_platform = {}, {}
  local gstr = string.rep( "/", 80 ) .. "\n" .. "// Module configuration\n\n"
  local ngenmods, nplmods = 0, 0

  platform_modules = plconf.get_platform_modules() or {}
  if mdesc == "all" then -- include all the modules. what a brave, brave soul.
    process_module( gen_list_generic, 'all' )
    process_module( gen_list_platform, 'all', true )
  else
    -- Include only some modules. Validate their names against the corresponding module list
    mdesc.generic = type( mdesc.generic ) == "table" and mdesc.generic or { mdesc.generic }
    mdesc.platform = type( mdesc.platform ) == "table" and mdesc.platform or { mdesc.platform }
    utils.foreach( mdesc.generic, function( k, v ) process_module( gen_list_generic, v ) end )
    utils.foreach( mdesc.platform, function( k, v ) process_module( gen_list_platform, v, true ) end )
  end
  -- Count the notal number of modules
  utils.foreach( gen_list_generic, function( k, v ) ngenmods = ngenmods + 1 end )
  utils.foreach( gen_list_platform, function( k, v ) nplmods = nplmods + 1 end )
  if ngenmods + nplmods == 0 then return '' end

  -- Now build all the module lines, starting from the gen_list and the guards
  -- First define the platform specific line if needed
  if nplmods > 0 then
    -- The (hopefully) proper way to generate this is a bit tricky. We enable the 
    -- platform module if _any_ of the modules in gen_list_platform can be enabled.
    -- In order to do this, we gather their guards in a single, long condition
    -- Count all guards first
    local nguards = 0
    local pltabname = mdesc.platform_name or platform
    for m, _ in pairs( gen_list_platform ) do nguards = nguards + #platform_modules[ m ] end
    if nguards == 0 then -- nothing to guard
      gstr = gstr .. gen.print_define( "PLATFORM_MODULES_LINE", sf( '_ROM( "%s", luaopen_platform, platform_map )', platform ) )
      gstr = gstr .. gen.print_define( "PS_LIB_TABLE_NAME", sf( '"%s"', pltabname ) )
      gstr = gstr .. gen.print_define( "PLATFORM_MODULES_ENABLE" )
    else
      -- Gather the composed condition in 'cond'
      local cond = '\n#if 0' -- the '0' is included here for an easier generation of the composed condition
      for m, _ in pairs( gen_list_platform ) do 
        local g = platform_modules[ m ]
        if #g > 0 then
          cond = cond .. " || ( " .. gen_cond_string( g ) .. " )"
        end
      end
      gstr = gstr .. cond .. "\n"
      gstr = gstr .. gen.print_define( "PLATFORM_MODULES_LINE", sf( '_ROM( "%s", luaopen_platform, platform_map )', platform ) )
      gstr = gstr .. gen.print_define( "PS_LIB_TABLE_NAME", sf( '"%s"', pltabname ) )
      gstr = gstr .. gen.print_define( "PLATFORM_MODULES_ENABLE" )
      gstr = gstr .. "#else\n"
      gstr = gstr .. gen.print_define( sf( "PLATFORM_MODULES_LINE" ) )
      gstr = gstr .. sf( "#warning Unable to include platform modules in the image\n#endif\n\n" )
    end
  else -- no platform modules here, people. move along.
    gstr = gstr .. gen.print_define( "PLATFORM_MODULES_LINE" )
  end
  for m, _ in pairs( gen_list_generic ) do
    local g = all_generic_modules[ m ]
    if get_map_name( m ) ~= "<none>" then
      if #g == 0 then -- no guards
        gstr = gstr .. gen.print_define( sf( "MODULE_%s_LINE", m:upper() ), sf( "_ROM( %s, %s, %s )", get_auxlib( m ), get_openf_name( m ), get_map_name( m ) ) )
      else
        -- Check the guard. If the guard is not satisfied, issue a compile time warning and set the line as empty
        gstr = gstr .. "\n#if " .. gen_cond_string( g ) .. "\n"
        gstr = gstr .. gen.print_define( sf( "MODULE_%s_LINE", m:upper() ), sf( "_ROM( %s, %s, %s )", get_auxlib( m ), get_openf_name( m ), get_map_name( m ) ) )
        gstr = gstr .. "#else\n"
        gstr = gstr .. gen.print_define( sf( "MODULE_%s_LINE", m:upper() ) )
        gstr = gstr .. sf( "#warning Unable to include module '%s' in the image\n#endif\n\n", m )
      end
    end
  end
  gstr = gstr .. "\n"

  -- Finally, generate the acutal list of libraries. Phew.
  gstr = gstr .. "#define LUA_PLATFORM_LIBS_ROM\\\n  PLATFORM_MODULES_LINE"
  for m, _ in pairs( gen_list_generic ) do
    if get_map_name( m ) ~= "<none>" then gstr = gstr .. sf( "\\\n  MODULE_%s_LINE", m:upper() ) end
  end
  gstr = gstr .. "\n\n"

  -- Not quite ready yet. We still need to generate the list of generic modules
  -- that can't be completely ROM'd by the LTR patch in a separate macro that will be 
  -- handled by linit.c
  local noltr = "#define LUA_LIBS_NOLTR\\\n", found
  for m, _ in pairs( gen_list_generic ) do
    if get_map_name( m ) == "<none>" then
      noltr = noltr .. sf( "  { %s, %s },\\\n", get_auxlib( m ), get_openf_name( m ) )
      found = true
    end
  end
  gstr = gstr .. ( found and noltr or "" )

  -- A bit of cosmetic touch ...
  gstr = gstr .. "\n"
  gstr = gstr:gsub( "\n\n\n", "\n\n" )
  return gstr
end

