-- eLua configurations description

module( ..., package.seeall )

local sf = string.format
local at = require "attributes"
local gen = require "generators"

-------------------------------------------------------------------------------
-- Attribute checkers

local function mem_checker( eldesc, vals )
  local startvals = vals.MEM_START_ADDRESS and vals.MEM_START_ADDRESS.value
  local sizevals = vals.MEM_END_ADDRESS and vals.MEM_END_ADDRESS.value
  if not startvals and not sizevals then return true end
  if not startvals then
    return false, "attribute 'start' must also be specified for element 'extmem' of section 'config'"
  elseif not sizevals then
    return false, "attribute 'size' must also be specified for element 'extmem' of section 'config'"
  end
  if #startvals == 0 then
    return false, "attribute 'start' of element 'extmem' in section 'config' must have at least one element"
  elseif #sizevals == 0 then
    return false, "attribute 'size' of element 'extmem' in section 'config' must have at least one element"
  end
  if #startvals ~= #sizevals then
    return false, "attributes 'start' and 'size' of element 'extmem' in section 'config' must have the same number of elements'"
  end
  return true
end

local egc_mode_map = 
{
  disabled = "EGC_NOT_ACTIVE",
  alloc = "EGC_ON_ALLOC_FAILURE",
  limit = "EGC_ON_MEM_LIMIT",
  always = "EGC_ALWAYS"
}

local function egc_checker( eldesc, vals )
  local modev = vals.EGC_INITIAL_MODE.value
  local limv = vals.EGC_INITIAL_MEMLIMIT and vals.EGC_INITIAL_MEMLIMIT.value
  local allmodes = {}
  for w in modev:gmatch( "(%w+)" ) do allmodes[ #allmodes + 1 ] = w:lower() end
  local has_memlimit
  for k, v in pairs( allmodes ) do
    if not egc_mode_map[ v ] then
      return false, sf( "'%s' is not a valid value for attribute 'mode' of element 'egc' in section 'config'", v )
    end
    if v == "limit" then has_memlimit = true end
  end
  if has_memlimit and not limv then
    return false, sf( "you must specify the 'limit' attribute when using 'limit' as a value for attribute 'mode' of element 'egc' in section 'config'" )
  end
  return true
end

-------------------------------------------------------------------------------
-- Specific generators

-- Automatically generates the MEM_START_ADDRESS and MEM_END_ADDRESS macros
-- Assumes that definitions for INTERNAL_RAM_FIRST_FREE and INTERNAL_RAM_LAST_FREE
-- exist (they should come from <cpu>.h)
local function mem_generator( desc, vals, generated )
  if not vals.MEM_START_ADDRESS and not vals.MEM_END_ADDRESS then
    -- Generate configuration only for the internal memory
    local gstr = gen.print_define( "MEM_START_ADDRESS", "{ ( void* )( INTERNAL_RAM_FIRST_FREE ) }" )
    gstr = gstr .. gen.print_define( "MEM_END_ADDRESS", "{ ( void* )( INTERNAL_RAM_LAST_FREE ) }" )
    generated.MEM_START_ADDRESS = true
    generated.MEM_END_ADDRESS = true
    return gstr
  end
  local startvals = vals.MEM_START_ADDRESS.value
  local sizevals = vals.MEM_END_ADDRESS.value
  table.insert( startvals, 1, "INTERNAL_RAM_FIRST_FREE" )
  table.insert( sizevals, 1, "INTERNAL_RAM_LAST_FREE" )
  -- Transform the data in 'sizevals' to 'last address' (the format accepted by eLua)
  for i = 2, #sizevals do
    sizevals[ i ] = tonumber( startvals[ i ] ) + tonumber( sizevals[ i ] ) - 1
  end
  -- Prefix all the values in the 'start' and 'size' arrays with (void*)
  for i = 1, #sizevals do
    startvals[ i ] = "( void* )( " .. tostring( startvals[ i ] ) .. " )"
    sizevals[ i ] = "( void* )( " .. tostring( sizevals[ i ] ) .. " )"
  end
  local gstr = gen.simple_gen( "MEM_START_ADDRESS", vals, generated )
  gstr = gstr .. gen.simple_gen( "MEM_END_ADDRESS", vals, generated )
  -- Fugly hack: hardcode a key in 'vals' which specifies if the build should use a
  -- memory allocator that supports non-contiguous regions
  vals.use_multiple_allocator = #startvals > 1
  return gstr
end

local function egc_generator( desc, vals, generated )
  local modev = vals.EGC_INITIAL_MODE.value
  local limv = vals.EGC_INITIAL_MEMLIMIT and vals.EGC_INITIAL_MEMLIMIT.value
  local allmodes = {}
  local has_memlimit, has_always
  for w in modev:gmatch( "(%w+)" ) do 
    w = w:lower()
    if w == "limit" then has_memlimit = true end
    if w == "always" then has_always = true end
    allmodes[ #allmodes + 1 ] = w:lower()
  end
  if has_always then
    local gstr = gen.print_define( "EGC_INITIAL_MODE", "EGC_ALWAYS" )
    generated.EGC_INITIAL_MODE = true
    return gstr
  end
  local cmodes = {}
  for k, v in pairs( allmodes ) do 
    cmodes[ #cmodes + 1 ] = egc_mode_map[ v ]
  end
  local gstr = gen.print_define( "EGC_INITIAL_MODE", "( " .. table.concat( cmodes, "|" ) .. " )" )
  generated.EGC_INITIAL_MODE = true
  if has_memlimit then gstr = gstr .. gen.simple_gen( "EGC_INITIAL_MEMLIMIT", vals, generated ) end
  return gstr
end

-------------------------------------------------------------------------------
-- Public interface

-- Build the configuration data needed by eLua, save it in the "configs" table
function init()
  local configs = {}

  -- Virtual timers
  configs.vtmr = {
    attrs = {
      num = at.int_attr( 'VTMR_NUM_TIMERS', 1 ),
      freq = at.int_attr( 'VTMR_FREQ_HZ', 1 )
    },
    required = { num = 0, freq = 1 }
  }

  -- EGC
  configs.egc = {
    confcheck = egc_checker,
    gen = egc_generator,
    attrs = {
      mode = at.string_attr( 'EGC_INITIAL_MODE' ),
      limit = at.make_optional( at.int_attr( 'EGC_INITIAL_MEMLIMIT', 1 ) )
    },
  }

  -- Memory configuration generator
  configs.extmem = {
    gen = mem_generator,
    confcheck = mem_checker,
    attrs = {
      start = at.array_of( at.int_attr( 'MEM_START_ADDRESS' ) ),
      size = at.array_of( at.int_attr( 'MEM_END_ADDRESS', 1 ) )
    },
    required = {}
  }

  -- All done
  return configs
end

