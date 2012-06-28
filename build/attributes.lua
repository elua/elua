-- Attributes are used by various parts of the board description
-- system (for example configs and components)

module( ..., package.seeall )
local ct = require "constants"
local sf = string.format

-- Validator for a 'choice' attribute
-- Returns the value if OK, (false, errmsg) for error
-- Needs: attrvals - list of permitted values for this attribute
local function validate_choice( adesc, aname, aval, comp )
  aval = aval:lower()
  for k, v in pairs( adesc.attrvals ) do
    if v == aval then return v end
  end
  return false, sf( "invalid value '%s' for attribute '%s' of component '%s'\n", aval, aname, comp )
end

-- Validator for a 'number' attribute
-- Returns the value if OK, (false, errmsg) for error
-- Needs: attrtype - the number type ('int' or 'float')
--        attrmin - minimum value (no minimum check will be performed if not specified)
--        attrmax - maximum value (no maximum check will be performed if not specified)
local function validate_number( adesc, aname, aval, comp )
  aval = tonumber( aval )
  if not aval then return false, sf( "value of attribute '%s' for component '%s' must be a number", aname, comp ) end
  if adesc.attrtype == 'int' and math.floor( aval ) ~= aval then
    return false, sf( "value of attribute '%s' for component '%s' must be an integer", aname, comp )
  end
  local minval = adesc.attrmin or aval
  local maxval = adesc.attrmax or aval
  if aval < minval then
    return false, sf( "value of attribute '%s' for component '%s' must be larger than '%s'\n", aname, comp, tostring( minval ) )
  end
  if aval > maxval then
    return false, sf( "value of attribute '%s' for component '%s' must be smaller than '%s'\n", aname, comp, tostring( maxval ) )
  end
  return aval
end

-- Validator for a log2 number attribute
-- Works like number, but additionaly checks that the value is a power of 2
-- Also changes the attribute value to its log2 
local function validate_log2( adesc, aname, aval, comp )
  local res, err = validate_number( adesc, aname, aval, comp )
  if not res then return res, err end
  if aval <= 0 then
    return false, sf( "value of attribute '%s' for component '%s' must be larger than 0\n", aname, comp )
  end
  local thelog = math.log( res ) / math.log( 2 )
  if thelog ~= math.floor( thelog ) then
    return false, sf( "value of attribute '%s' for component '%s' must be a power of 2\n", aname, comp )
  end
  return thelog
end

-- Validator for a string attribute
-- Return the string if OK, (false, errmsg) for error
-- Needs: attrmaxsize - maximum size of the string
local function validate_string( adesc, aname, aval, comp )
  aval = tostring( aval )
  if type( aval ) ~= "string" then
    return false, sf( "value of attribute '%s' for component '%s' must be a string", aname, comp )
  end
  maxsize = adesc.attrmaxsize or math.huge
  if #aval > adesc.attrmaxsize then
    return false, sf( "value of attribute '%s' for component '%s' must be less than %d chars in length", aname, comp, maxsize )
  end
  return aval
end

-------------------------------------------------------------------------------
-- Public interface

-- Returns a new timer attribute with the given macro and default (systmr if not specified)
function timer_attr( macro, default )
  default = default or ct.systmr
  return { macro = macro, default = default }
end

-- Returns a new integer number attribute with the given limits
function int_attr( macro, minv, maxv, default )
  return { macro = macro, default = default, validator = validate_number, attrtype = 'int', attrmin = minv, attrmax = maxv }
end

-- Returns a new integer number attribute with the given limits
-- The generated value will be the integer's base 2 logarithm
function int_log2_attr( macro, minv, maxv, default )
  local t = int_attr( macro, minv, maxv, default )
  t.validator = validate_log2
  return t
end

-- Returns a new choice attribute with the given possible values
function choice_attr( macro, values, default )
  return { macro = macro, default = default, validator = validate_choice, attrvals = values }
end

-- Returns a new flow control attribute
function flow_control_attr( macro, default )
  default = default or ct.uart_flow.none
  return choice_attr( macro, ct.uart_flow_vals, default )
end

-- Returns a new string attribute
function string_attr( macro, maxsize, dfault )
  return { macro = macro, default = default, validator = validate_string, attrmaxsize = maxsize }
end

-- Make the given attribute optional
function make_optional( attr )
  attr.optional = true
  return attr
end

