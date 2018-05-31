-- Build configuration: module selection

module( ..., package.seeall )
local sf = string.format
local gen = require "generators"

-- List of all Lua modules, their guards (if applicable), their library name,
-- their open function and their LTR map. "<none>" in the LTR map field marks
-- the modules that do not have a corresponding LTR map, which basically means
-- they are not LTR compatible
local lua_modules = {
  lua_math = { lib = "LUA_MATHLIBNAME", open = "luaopen_math", map = "math_map" },
  lua_io = { lib = "LUA_IOLIBNAME", open = "luaopen_io", map = "<none>" },
  lua_string = { lib = "LUA_STRLIBNAME", open = "luaopen_string", map = "strlib" },
  lua_table = { lib = "LUA_TABLIBNAME", open = "luaopen_table", map = "tab_funcs" },
  lua_debug = { lib = "LUA_DBLIBNAME", open = "luaopen_debug", map = "dblib" },
  lua_package = { lib = "LUA_LOADLIBNAME", open = "luaopen_package", map = "<none>" },
  lua_co = { lib = "LUA_COLIBNAME", open = false, map = "co_funcs" }
}

-- List of all eLua generic modules, in the same format as above
-- (if the module guard evaluates to false at compile time, the module is not included in the build)
-- NOTE: if the guards contain a condition, the condition _MUST_ be specified with spaces next to the operator!
--       For example: NUM_CAN > 0, _NOT_ NUM_CAN>0.
local elua_generic_modules = { 
  adc = { guards = { "BUILD_ADC", "NUM_ADC > 0" } }, 
  bit = {}, 
  can = { guards = { "NUM_CAN > 0" } }, 
  cpu = {}, 
  elua = {}, 
  i2c = { guards = { "NUM_I2C > 0" } },
  pack = {}, 
  rpc = { guards = { "BUILD_RPC" } },
  net = { guards = { "BUILD_UIP" } },
  pd = {}, 
  pio = { guards = { "NUM_PIO > 0" } },
  pwm = { guards = {"NUM_PWM > 0" } }, 
  spi = { guards = { "NUM_SPI > 0" } },
  term = { guards = { "BUILD_TERM" } },
  tmr = { guards = { "NUM_TIMER > 0" } },
  uart = { guards = { "NUM_UART > 0" } },
  fs = { guards = { "BUILD_NIFFS" } }
}

-- All generic modules (Lua and eLua) in a single table
local all_generic_modules = {}
utils.concat_tables( all_generic_modules, lua_modules )
utils.concat_tables( all_generic_modules, elua_generic_modules )

function add_extra_modules( exmodules )
  utils.concat_tables( all_generic_modules, exmodules )
end

-- Return the auxlib name of a given module
local function get_auxlib( m, t )
  t = t or all_generic_modules
  return t[ m ].lib or sf( "AUXLIB_%s", m:upper() )
end

-- Return the open function name of a given module
local function get_openf_name( m, t )
  t = t or all_generic_modules
  if t[ m ].open == false then
    return "luaopen_dummy"
  else
    return t[ m ].open or sf( "luaopen_%s", m:lower() )
  end
end

-- Return the map array name of a given module
local function get_map_name( m, t )
  t = t or all_generic_modules
  return t[ m ].map or sf( "%s_map", m:lower() )
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

-- Generates module-specific data for the given component (generic or platform)
local function generate_data( t, is_platform )
  local prefix = is_platform and "PL_" or ""
  local desc = is_platform and platform_modules or all_generic_modules
  local s = ""

  -- Generate the proper line for each module in turn
  for m, _ in pairs( t ) do
    if get_map_name( m, desc ) ~= "<none>" then
      local g = desc[ m ].guards or {}
      if #g == 0 then -- no guards
        s = s .. gen.print_define( sf( "%sMODULE_%s_LINE", prefix, m:upper() ), sf( "_ROM( %s, %s, %s )", get_auxlib( m, desc ), get_openf_name( m, desc ), get_map_name( m, desc ) ) )
      else
        -- Check the guard. If the guard is not satisfied, issue a compile time warning and set the line as empty
        s = s .. "\n#if " .. gen_cond_string( g ) .. "\n"
        s = s .. gen.print_define( sf( "%sMODULE_%s_LINE", prefix, m:upper() ), sf( "_ROM( %s, %s, %s )", get_auxlib( m, desc ), get_openf_name( m, desc ), get_map_name( m, desc ) ) )
        s = s .. "#else\n"
        s = s .. gen.print_define( sf( "%sMODULE_%s_LINE", prefix, m:upper() ) )
        s = s .. sf( "#warning Unable to include %s module '%s' in the image\n#endif\n\n", is_platform and "platform specific" or "generic", m )
      end
    end
  end
  s = s .. "\n"

  -- Finally, generate the acutal list of libraries. 
  if is_platform then
    s = s .. "#define PLATFORM_MODULES_LIBS_ROM"
  else
    s = s .. "#define LUA_PLATFORM_LIBS_ROM\\\n  PLATFORM_MODULES_LINE"
  end
  for m, _ in pairs( t ) do
    if get_map_name( m, desc ) ~= "<none>" then s = s .. sf( "\\\n  %sMODULE_%s_LINE", prefix, m:upper() ) end
  end
  return s .. "\n\n"
end

-- Generate the complete module list starting from the board description
function gen_module_list( desc, plconf, platform, boardname )
  local mdesc = desc.modules
  if not mdesc then return '' end
  local gen_list_generic, gen_list_platform = {}, {}
  local gstr = string.rep( "/", 80 ) .. "\n" .. "// Module configuration\n\n"
  local ngenmods, nplmods = 0, 0

  platform_modules = plconf.get_platform_modules( boardname, desc.cpu ) or {}
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
    local nguards, nmodules = 0, 0
    local pltabname = mdesc.platform_name or platform
    for m, _ in pairs( gen_list_platform ) do
      nmodules = nmodules + 1
      nguards = nguards + #( platform_modules[ m ].guards or {} )
    end
    if nguards == 0 or nguards < nmodules then -- nothing to guard or not all have guards
      gstr = gstr .. gen.print_define( "PLATFORM_MODULES_LINE", sf( '_ROM( "%s", luaopen_platform, platform_map )', pltabname ) )
      gstr = gstr .. gen.print_define( "PS_LIB_TABLE_NAME", sf( '"%s"', pltabname ) )
      gstr = gstr .. gen.print_define( "PLATFORM_MODULES_ENABLE" )
    else
      -- Gather the composed condition in 'cond'
      local cond = '\n#if 0' -- the '0' is included here for an easier generation of the composed condition
      for m, _ in pairs( gen_list_platform ) do 
        local g = platform_modules[ m ].guards or {}
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
  if ngenmods > 0 then gstr = gstr .. generate_data( gen_list_generic ) end
  if nplmods > 0 then gstr = gstr .. generate_data( gen_list_platform, true ) end

  -- Not quite ready yet. We still need to generate the list of generic modules
  -- that can't be completely ROM'd by the LTR patch in a separate macro that will be 
  -- handled by linit.c
  local noltr, found = "#define LUA_LIBS_NOLTR\\\n", false
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

