-- Generic utility functions

module( ..., package.seeall )

local lfs = require "lfs"
local sf = string.format

-- Taken from Lake
dir_sep = package.config:sub( 1, 1 )
is_os_windows = dir_sep == '\\'

-- Converts a string with items separated by 'sep' into a table
string_to_table = function( s, sep )
  if type( s ) ~= "string" then return end
  sep = sep or ' '
  if s:sub( -1, -1 ) ~= sep then s = s .. sep end
  s = s:gsub( sf( "^%s*", sep ), "" )
  local t = {}
  local fmt = sf( "(.-)%s+", sep )
  for w in s:gmatch( fmt ) do table.insert( t, w ) end
  return t
end

-- Split a file name into 'path part' and 'extension part'
split_path = function( s )
  local pos
  for i = #s, 1, -1 do
    if s:sub( i, i ) == "." then
      pos = i
      break
    end
  end
  if pos then return s:sub( 1, pos - 1 ), s:sub( pos ) end
  return s
end

-- Replace the extension of a give file name
replace_extension = function( s, newext )
  local p, e = split_path( s )
  if e then s = p .. "." .. newext end
  return s
end

-- Return 'true' if building from Windows, false otherwise
is_windows = function()
  return is_os_windows
end

-- Prepend each component of a 'pat'-separated string with 'prefix'
prepend_string = function( s, prefix, pat )  
  if not s or #s == 0 then return "" end
  pat = pat or ' '
  local res = ''
  local st = string_to_table( s, pat )
  foreach( st, function( k, v ) res = res .. prefix .. v .. " " end )
  return res
end

-- Like above but consider 'prefix' a path
prepend_path = function( s, prefix, pat )
  return prepend_string( s, prefix .. dir_sep, pat )
end

-- full mkdir: create all the paths needed for a multipath
full_mkdir = function( path )
  local ptables = string_to_table( path, dir_sep )
  local p, res = ''
  for i = 1, #ptables do
    p = ( i ~= 1 and p .. dir_sep or p ) .. ptables[ i ]
    res = lfs.mkdir( p )
  end
  return res
end

-- Concatenate the given paths to form a complete path
concat_path = function( paths )
  return table.concat( paths, dir_sep )
end

-- Return true if the given array contains the given element, false otherwise
array_element_index = function( arr, element )
  for i = 1, #arr do
    if arr[ i ] == element then return i end
  end
end

-- Linearize an array with (possibly) embedded arrays into a simple array
_linearize_array = function( arr, res, filter )
  if type( arr ) ~= "table" then return end
  for i = 1, #arr do
    local e = arr[ i ]
    if type( e ) == 'table' and filter( e ) then
      _linearize_array( e, res, filter )
    else
      table.insert( res, e )
    end
  end 
end

linearize_array = function( arr, filter )
  local res = {}
  filter = filter or function( v ) return true end
  _linearize_array( arr, res, filter )
  return res
end

-- Return an array with the keys of a table
table_keys = function( t )
  local keys = {}
  foreach( t, function( k, v ) table.insert( keys, k ) end )
  return keys
end

-- Returns true if 'path' is a regular file, false otherwise
is_file = function( path )
  return lfs.attributes( path, "mode" ) == "file"
end

-- Return a list of files in the given directory matching a given mask
get_files = function( path, mask, norec )
  local t = ''
  for f in lfs.dir( path ) do
    local fname = path .. dir_sep .. f
    if lfs.attributes( fname, "mode" ) == "file" then
      local include
      if type( mask ) == "string" then
        include = fname:find( mask )
      else
        include = mask( fname )
      end
      if include then t = t .. ' ' .. fname end
    elseif lfs.attributes( fname, "mode" ) == "directory" and not fname:find( "%.+$" ) and not norec then
      t = t .. " " .. get_files( fname, mask, norec )
    end
  end
  return t
end

-- Check if the given command can be executed properly
check_command = function( cmd )
  local res = os.execute( cmd .. " > .build.temp 2>&1" )
  os.remove( ".build.temp" )
  return res
end

-- Execute the given command for each value in a table
foreach = function ( t, cmd )
  if type( t ) ~= "table" then return end
  for k, v in pairs( t ) do cmd( k, v ) end
end

---------------------------------------
-- Color-related funtions
-- Currently disabled when running in Windows

local dcoltable = { 'black', 'red', 'green', 'yellow', 'blue', 'magenta', 'cyan', 'white' }
local coltable = {}
foreach( dcoltable, function( k, v ) coltable[ v ] = k - 1 end )

local _col_builder = function( col )
  local _col_maker = function( s )
    if is_os_windows then
      return s
    else
      return( sf( "\027[%dm%s\027[m", coltable[ col ] + 30, s ) )
    end
  end
  return _col_maker
end

col_funcs = {}
foreach( coltable, function( k, v ) 
  local fname = "col_" .. k
  _G[ fname ] = _col_builder( k ) 
  col_funcs[ k ] = _G[ fname ]
end )

