-- Build configuration: module selection

module( ..., package.seeall )
local sf = string.format
local gen = require "generators"

-- List of all generic modules and their build guards
-- (if the module guard evaluates to false at compile time, the module is not included in the build)
local generic_modules = { 
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
  math = {}
}

-- Auxlib names
local auxlibs = { math = "LUA_MATHLIBNAME" }

-- Open function names
local opennames = {}

-- Map array names
local mapnames = {}

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

-- Generate a condition string starting from a guard
local function gen_cond_string( g )
  local condition = ''
  for idx, e in pairs( g ) do
    local suffix = idx == #g and "" or " && "
    if e:find( '%s' ) then -- if it has a string, add the condition as it was given, between parantheses
      condition = condition .. "( " .. e .. " )"
    else
      condition = condition .. "defined( " .. e .. " )"
    end
    condition = condition .. suffix
  end
  return condition
end

-- Generate the complete module list starting from the board description
function gen_module_list( desc, plconf, platform )
  local mdesc = desc.modules
  if not mdesc then return '' end
  local gen_list_generic, gen_list_platform = {}, {}
  local platform_modules = plconf.get_platform_modules() or {}
  local gstr = string.rep( "/", 80 ) .. "\n" .. "// Module configuration\n\n"
  local ngenmods, nplmods = 0, 0

  if mdesc == "all" then -- include all the modules. what a brave, brave soul.
    for m, _ in pairs( generic_modules ) do 
      gen_list_generic[ m ] = true
      ngenmods = ngenmods + 1
    end
    for m, _ in pairs( platform_modules ) do
      gen_list_platform[ m ] = true
      nplmods = nplmods + 1
    end
  else
    -- Include only some modules. Validate their names against the corresponding module list
    if mdesc.generic == "all" then -- all generic modules
      for m, _ in pairs( generic_modules ) do
        gen_list_generic[ m ] = true
        ngenmods = ngenmods + 1
      end
    elseif mdesc.generic then
      for _, m in pairs( mdesc.generic ) do
        if not generic_modules[ m ] then return false, sf( "unknown generic module '%s' in section 'modules'", m ) end
        gen_list_generic[ m ] = true
        ngenmods = ngenmods + 1
      end
    end
    if mdesc.platform == "all" then -- all platform modules
      for m, _ in pairs( platform_modules ) do
        gen_list_platform[ m ] = true
        nplmods = nplmods + 1
      end
    elseif mdesc.platform then
       for _, m in pairs( mdesc.platform ) do
        if not platform_modules[ m ] then return false, sf( "unknown platform module '%s' in section 'modules'", m ) end
        gen_list_platform[ m ] = true
        nplmods = nplmods + 1
      end
    end
  end
  -- Need to exclude anything?
  if type( mdesc.exclude_generic ) == "table" then
    for _, m in pairs( mdesc.exclude_generic ) do
      if not gen_list_generic[ m ] then
        return false, sf( "module '%s' in exclude_generic not found in the generic module list", m )
      end
      gen_list_generic[ m ] = nil
      ngenmods = ngenmods - 1
    end
  end
  if type( mdesc.exclude_platform ) == "table" then
    for _, m in pairs( mdesc.exclude_platform ) do
      if not gen_list_platform[ m ] then
        return false, sf( "module '%s' in exclude_platform not found in the platform specific module list", m )
      end
      gen_list_platform[ m ] = nil
      nplmods = nplmods - 1
    end
  end
  if ngenmods + nplmods == 0 then return '' end

  -- Now build all the module lines, starting from the gen_list and the guards
  -- First define the platform specific line if needed
  if nplmods > 0 then
    -- The (hopefully) proper way to generate this is a bit tricky. We enable the 
    -- platform module if _any_ of the modules in gen_list_platform can be enabled.
    -- In order to do this, we gather their guards in a single, long condition
    -- Count all guards first
    local nguards = 0
    for m, _ in pairs( gen_list_platform ) do nguards = nguards + #platform_modules[ m ] end
    if nguards == 0 then -- nothing to guard
      gstr = gstr .. gen.print_define( "PLATFORM_MODULES_LINE", sf( '_ROM( "%s", luaopen_platform, platform_map )', platform ) )
      gstr = gstr .. gen.print_define( "PS_LIB_TABLE_NAME", sf( '"%s"', platform ) )
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
      gstr = gstr .. gen.print_define( "PS_LIB_TABLE_NAME", sf( '"%s"', platform ) )
      gstr = gstr .. gen.print_define( "PLATFORM_MODULES_ENABLE" )
      gstr = gstr .. "#else\n"
      gstr = gstr .. gen.print_define( sf( "PLATFORM_MODULES_LINE" ) )
      gstr = gstr .. sf( "#warning Unable to include platform modules in the image\n#endif\n\n" )
    end
  else -- no platform modules here, people. move along.
    gstr = gstr .. gen.print_define( "PLATFORM_MODULES_LINE" )
  end
  for m, _ in pairs( gen_list_generic ) do
    local g = generic_modules[ m ]
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
  gstr = gstr .. "\n"

  -- Finally, generate the acutal list of libraries. Phew.
  gstr = gstr .. "#define LUA_PLATFORM_LIBS_ROM\\\n  PLATFORM_MODULES_LINE"
  for m, _ in pairs( gen_list_generic ) do  
    gstr = gstr .. sf( "\\\n  MODULE_%s_LINE", m:upper() )
  end
  gstr = gstr .. "\n"

  -- A bit of cosmetic touch ...
  gstr = gstr .. "\n"
  gstr = gstr:gsub( "\n\n\n", "\n\n" )
  return gstr
end

