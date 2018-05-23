-- Attributes are used by various parts of the board description
-- system (for example configs and components)

module( ..., package.seeall )
local ct = require "constants"
local sf = string.format

-- Validator for a 'choice' attribute
-- Returns the value if OK, (false, errmsg) for error
-- Needs: attrvals - list of permitted values for this attribute
local function _validate_choice( adesc, aname, aval, elname, sectname )
  aval = tostring( aval ):lower()
  for k, v in pairs( adesc.attrvals ) do
    if v == aval then return v end
  end
  return false, sf( "invalid value '%s' for attribute '%s' of element '%s' in section '%s'", aval, aname, elname, sectname )
end

-- Validator for a 'number' attribute
-- Returns the value if OK, (false, errmsg) for error
-- Needs: attrtype - the number type ('int' or 'float')
--        attrmin - minimum value (no minimum check will be performed if not specified)
--        attrmax - maximum value (no maximum check will be performed if not specified)
--        allow_string - allows this number to also be a string
local function _validate_number( adesc, aname, _aval, elname, sectname )
  local aval = tonumber( _aval )
  if not aval then
    if type( _aval ) == "string" and adesc.allow_string then
      return _aval
    else
      return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be a number", aname, elname, sectname ) end
    end
  if adesc.attrtype == 'int' and math.floor( aval ) ~= aval then
    return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be an integer", aname, elname, sectname )
  end
  local minval = adesc.attrmin or aval
  local maxval = adesc.attrmax or aval
  if aval < minval then
    return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be larger than '%s'", aname, elname, sectname, tostring( minval ) )
  end
  if aval > maxval then
    return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be smaller than '%s'", aname, elname, sectname, tostring( maxval ) )
  end
  return aval
end

-- Validator for a log2 number attribute
-- Works like number, but additionaly checks that the value is a power of 2
-- Also changes the attribute value to its log2 
local function _validate_log2( adesc, aname, aval, elname, sectname )
  local res, err = _validate_number( adesc, aname, aval, elname, sectname )
  if not res then return res, err end
  if adesc.allow_string and not tonumber( res ) and type( res ) == "string" then return res end
  if aval <= 0 then
    return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be larger than 0", aname, elname, sectname )
  end
  local thelog = math.log( res ) / math.log( 2 )
  if thelog ~= math.floor( thelog ) then
    return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be a power of 2", aname, elname, sectname )
  end
  return thelog
end

-- Validator for a string attribute
-- Return the string if OK, (false, errmsg) for error
-- Needs: attrmaxsize - maximum size of the string
local function _validate_string( adesc, aname, aval, elname, sectname )
  if type( aval ) ~= "string" then
    return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be a string", aname, elname, sectname )
  end
  maxsize = adesc.attrmaxsize or math.huge
  if #aval > maxsize then
    return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be less than %d chars in length", aname, elname, sectname, maxsize )
  end
  return aval
end

-- Validator for an IP attribute
-- Return the IP as a table of numbers if OK, (false, errmsg) for error
local function _validate_ip( adesc, aname, aval, elname, sectname )
  if type( aval ) == "string" then -- transform this to table
    if aval:sub( -1, -1 ) ~= "." then aval = aval .. "." end -- add a final dot to make parsing easier
    local t = {}
    for w in aval:gmatch( "(%w+)%." ) do t[ #t + 1 ] = tonumber( w ) end
    aval = t
  elseif type( aval ) ~= "table" then
    return false, sf( "attribute '%s' of element '%s' in section '%s' must be a string or a table", aname, elname, sectname )
  end
  if #aval ~= 4 then return false, sf( "invalid IP for attribute '%s' of element '%s' in section '%s'", aname, elname, sectname ) end
  for i = 1, 4 do
    local e = aval[ i ]
    if type( e ) ~= "number" or math.floor( e ) ~= e or e < 0 or e > 255 then
      return false, sf( "invalid IP for attribute '%s' of element '%s' in section '%s'", aname, elname, sectname )
    end
  end
  return aval
end

-- Validator for a boolean value
local function _validate_bool( adesc, aname, aval, elname, sectname )
  if type( aval ) ~= "boolean" then
    return false, sf( "attribute '%s' of element '%s' in section '%s' must be a boolean", aname, elname, sectname)
  end
  return aval and 1 or 0
end

-- Builds a validator with the given array element checker
local function build_validator( realvname )
  return function( adesc, aname, aval, elname, sectname )
    if not adesc.is_array then return realvname( adesc, aname, aval, elname, sectname ) end
    if type( aval ) ~= "table" then
      if adesc.allow_single then
        aval = { aval }
      else
        return false, sf( "value of attribute '%s' for element '%s' in section '%s' must be an array", aname, elname, sectname )
      end
    end
    local newv = {}
    for i = 1, #aval do
      local res, err = realvname( adesc, aname, aval[ i ], elname, sectname )
      if not res then
        return false, sf( "error at index %d: %s", i, err )
      else
        newv[ #newv + 1 ] = res
      end
    end
    return newv
  end
end

local validate_number = build_validator( _validate_number )
local validate_choice = build_validator( _validate_choice )
local validate_log2 = build_validator( _validate_log2 ) 
local validate_string = build_validator( _validate_string )
local validate_ip = build_validator( _validate_ip )
local validate_bool = build_validator( _validate_bool )

-- Composite validator: run each validator in turn
local function composite_validator( adesc, aname, aval, elname, sectname )
  for _, onedesc in pairs( adesc.attrs ) do
    onedesc.is_array, onedesc.allow_single = adesc.is_array, onedesc.allow_single
    local res, err = onedesc.validator( onedesc, aname, aval, elname, sectname )
    if res then return res end
  end
  return false, sf( "Invalid value '%s' for attribute '%s' of element '%s' in section '%s'", tostring( aval ), aname, elname, sectname )
end

-------------------------------------------------------------------------------
-- Public interface

-- Returns a new choice attribute with the given possible values
function choice_attr( macro, values, default )
  return { macro = macro, default = default, validator = validate_choice, attrvals = values }
end

-- Returns a new timer attribute with the given macro and default (systmr if not specified)
function timer_attr( macro, default )
  default = default or 'systmr'
  local t = choice_attr( macro, utils.table_keys( ct.timer_values ), default )
  t.is_timer, t.mapping = true, ct.timer_values
  return t
end

-- Returns a new integer number attribute with the given limits
function int_attr( macro, minv, maxv, default )
  return { macro = macro, default = default, validator = validate_number, attrtype = 'int', attrmin = minv, attrmax = maxv, allow_string = true }
end

-- Returns a new UART attribute
function uart_attr( macro, default )
  local t = choice_attr( macro, utils.table_keys( ct.uart_values ), default )
  t.is_uart, t.mapping = true, ct.uart_values
  return t
end

-- Returns a new integer number attribute with the given limits
-- The generated value will be the integer's base 2 logarithm
function int_log2_attr( macro, minv, maxv, default )
  local t = int_attr( macro, minv, maxv, default )
  t.validator = validate_log2
  return t
end

-- Returns a new flow control attribute
function flow_control_attr( macro, default )
  default = default or 'none'
  local t = choice_attr( macro, utils.table_keys( ct.uart_flow ), default )
  t.mapping = ct.uart_flow
  return t
end

-- Returns a new string attribute
function string_attr( macro, maxsize, default )
  return { macro = macro, default = default, validator = validate_string, attrmaxsize = maxsize }
end

-- Returns a new IP attribute
function ip_attr( macro, default )
  return { macro = macro, default = default, validator = validate_ip, is_ip = true }
end

-- Returns a new combined attribute
function combine_attr( macro, attrs )
  return { macro = macro, validator = composite_validator, attrs = attrs }
end

-- Make the given attribute optional
function make_optional( attr )
  attr.optional = true
  return attr
end

-- Mark the given attribute as an array of element of the same type
function array_of( attr, allow_single )
  attr.is_array = true
  attr.allow_single = allow_single
  return attr
end

-- Bypass the validator of a given attribute
function dont_validate( attr )
  attr.validator = function( adesc, aname, aval, elname, sectname ) return aval end
  return attr
end

-- Removes the string option from the given number attribute
function nostr( attr )
  attr.allow_string = false
  return attr
end

-- Returns a new boolean attr (can be either true or false)
function bool_attr( macro, default )
    return { macro = macro, validator = validate_bool, default = default }
end

