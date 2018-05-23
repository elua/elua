-- Sections (collection of elements) management

module( ..., package.seeall )
local sf = string.format
local gen = require "generators"
local utils = require "utils"

conf, enabled, required = {}, {}, {}

-- Enables and configures an element (a collection of attributes)
-- section - the section of the element
-- sectname - the name of the section that contains the element (components, config ...)
-- name - name of the element
-- data - values of attributes in the element
-- req: required mode. true if this element's value are forced to its required values, false otherwise
-- Returns true if OK, (false, err) for error
function config_element( section, sectname, name, data, req )
  local desc = section[ name ]
  local attrs = desc.attrs or {}

  -- Process each element in 'data' in turn
  for attr, v in pairs( data ) do
    local attrmeta = attrs[ attr ]
    if not attrmeta then return false, sf( "attribute '%s' is not defined for element '%s' in section '%s'", attr, name, sectname ) end
    if attrmeta.validator and not req then
      local res, err = attrmeta:validator( attr, v, name, sectname )
      if not res then
        return false, err  
      else
        -- The validator can also change the attribute's value
        v = res
      end
    end
    if not req or not conf[ attrmeta.macro ] then
      if conf[ attrmeta.macro ] and tostring( conf[ attrmeta.macro ].value ) ~= tostring( v ) and not conf[ attrmeta.macro ].from_default then
        print( utils.col_yellow( sf( "[CONFIG] WARNING: overriding value of attribute '%s' in element '%s' from '%s' to '%s' in section '%s'", 
               attr, name, conf[ attrmeta.macro ].value, v, sectname ) ) )
      end
      conf[ attrmeta.macro ] = { name = attr, desc = attrmeta, value = v, sectname = sectname, elname = name, from_default = false }
    end
  end
  -- Set default values where needed
  for name, data in pairs( attrs ) do
    if not conf[ data.macro ] and ( data.default ~= nil ) then
      conf[ data.macro ] = { name = name, desc = data, value = data.default, sectname = sectname, elname = name, from_default = true }
    end
  end
  -- Mark this component as configured
  enabled[ name ] = true
  if req then required[ name ] = true end
  return true
end

-- Configures the given section
-- section: the section that will be compiled
-- sectname: the name of the section
-- data: the data corresponding to the section
-- Returns true if OK, (false, errmsg) for error
function configure_section( section, sectname, data )
  conf, enabled, required = {}, {}, {}

  -- Check if any part of the section needs to be automatically enabled
  for elname, elval in pairs( data ) do
    if elval and section[ elname ] and section[ elname ].autoenable then
      local auto = section[ elname ].autoenable
      local t = type( auto ) == "table" and auto or { auto }
      for _, v in pairs( t ) do data[ v ] = true end
    end
  end

  -- Configure each element in turn, doing validation if required
  for elname, elval in pairs( data ) do
    if not section[ elname ] then return nil, sf( "unknown element '%s' in section '%s'", elname, sectname ) end
    -- Handle the special situation <elname> = true (or anything else that is not false)
    if type( elval ) ~= "table" and elval then
      data[ elname ] = {}
      elval = data[ elname ]
    end
    if elval then 
      local cres, cerr = config_element( section, sectname, elname, elval )
      if not cres then return false, cerr end
    end
  end

  -- We also need to generated required elements. A required element is an element that
  -- is generated every time, even if it was not specified in the configuration file.
  for elname, eldesc in pairs( section ) do
    if eldesc.required then
      config_element( section, sectname, elname, eldesc.required, true )
    end
  end

  -- Step 2: basic consistency check
  -- For each element, we check that all its required attributes (the ones that don't have
  -- an 'optional' key set to true) have a value. An element can overwrite this default 
  -- verification by specifying its own 'confcheck' function. There is another function called
  -- 'auxcheck' that is called AFTER the basic consistency check (if it exists)
  for elname, _ in pairs( enabled ) do
    if not required[ elname ] then
      local desc = section[ elname ]
      local attrs = desc.attrs or {}
      if desc.confcheck then
        local d, err = desc:confcheck( conf, enabled )
        if not d then return false, err end
      else
        for attr, adesc in pairs( attrs ) do
          if not conf[ adesc.macro ] and not adesc.optional then
            return false, sf( "required attribute '%s' of component '%s' in section '%s' not specified", attr, elname, sectname )
          end
        end
        if desc.auxcheck then
          local d, err = desc:auxcheck( conf, enabled )
          if not d then return false, err end
        end
      end
    end
  end

  return true
end

-- Generate configuration data for the given section (must be called after configure_section!)
-- section: the section that will be compiled
-- sectname: the name of the section
-- data: the data corresponding to the section
-- Returns the generated header if OK, (false, errmsg) for error
function generate_section( section, sectname, data )
  -- Actual generation of code
  -- The default generator simply adds '#define KEY VALUE' pairs. An element can overwrite this
  -- default generation by specifying its own 'gen' function. If the element has an 'auxgen'
  -- function, it will be called after the default attribute generation
  -- Also, we never generate the same key twice. We ensure this by keeping a table of the
  -- keys that were already generated
  local generated = {}
  local genstr = string.rep( "/", 80 ) .. "\n" .. sf( "// Configuration for section '%s'\n\n", sectname ) 
  for elname, _ in pairs( enabled ) do
    local desc = section[ elname ]
    local attrs = desc.attrs or {}
    genstr = genstr .. sf( "// Configuration for element '%s'\n", elname )
    if desc.gen then
      genstr = genstr .. desc:gen( conf, generated )
    else
      for aname, adesc in pairs( attrs ) do 
        genstr = genstr .. gen.simple_gen( adesc.macro, conf, generated )
      end
      if desc.auxgen then genstr = genstr .. desc:auxgen( conf, generated ) end
    end
    -- Add the "build enable" macro
    if desc.macro then 
      genstr = genstr .. gen.print_define( desc.macro ) .. "\n" 
    else
      genstr = genstr .. "\n"
    end
  end

  -- Finally, check for dependencies
  -- For each attribute that has a 'needs' element, check if the dependency is met
  -- The attribute can also define the 'depcheck' function for a default dependency check
  for elname, _ in pairs( enabled ) do
    local desc = section[ elname ]
    if desc.depcheck then
      local res, err = desc:depcheck( conf, generated )
      if not res then return false, err end
    elseif desc.needs then
      local needs = type( desc.needs ) == "table" and desc.needs or { desc.needs }
      for _, v in pairs( needs ) do
        -- Look for negative expressions (not enabled)
        local neg
        if v:sub( 1, 1 )  == "!" then
          neg = true
          v = v:sub( 2 )
        end
        if not neg and not enabled[ v ] then
          return false, sf( "element '%s' in section '%s' needs element '%s' to be enabled", elname, sectname, v )
        elseif neg and enabled[ v ] then
          return false, sf( "element '%s' in section '%s' needs element '%s' to be disabled", elname, sectname, v )
        end
      end
    end
  end

  -- All done
  return genstr
end

