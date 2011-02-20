-- eLua build system
module( ..., package.seeall )

local lfs = require "lfs"
local sf = string.format

-- Taken from Lake
local dir_sep = package.config:sub( 1, 1 )
local is_windows = dir_sep == '\\'

-------------------------------------------------------------------------------
-- Various helpers

-- Return the time of the last modification of the file
local function get_ftime( path )
  local t = lfs.attributes( path, 'modification' )
  return t or -1
end

-- Check if a given target name is phony
local function is_phony( target )
  return target:find( "#phony" ) == 1
end

-- Return a string with $(key) replaced with 'value'
local function expand_key( s, key, value )
  local fmt = sf( "%%$%%(%s%%)", key )
  return ( s:gsub( fmt, value ) )
end

-- Return a target name considering phony targets
local function get_target_name( s )
  if not is_phony( s ) then return s end
end

-- 'Liniarize' a file name by replacing its path separators indicators with '_'
local function linearize_fname( s )
  local fmt = "%" .. dir_sep
  return ( s:gsub( fmt, "__" ) )
end

-- Helper: transform a table into a string if needed
local function table_to_string( t )
  if not t then return nil end
  if type( t ) == "table" then t = table.concat( t, " " ) end
  return t
end

-------------------------------------------------------------------------------
-- Public utilities

utils = { dir_sep = dir_sep }

-- Converts a string with items separated by 'sep' into a table
utils.string_to_table = function( s, sep )
  sep = sep or ' '
  if s:sub( -1, -1 ) ~= sep then s = s .. sep end
  local t = {}
  local fmt = sf( "(.-)%s+", sep )
  for w in s:gmatch( fmt ) do table.insert( t, w ) end
  return t
end

-- Replace the extension of a give file name
utils.replace_extension = function( s, newext )
  local pos
  for i = #s, 1, -1 do
    if s:sub( i, i ) == "." then
      pos = i
      break
    end
  end
  if pos then s = s:sub( 1, pos ) .. newext end
  return s
end

-- Return 'true' if building from Windows, false otherwise
utils.is_windows = function()
  return is_windows
end

-- Prepend each component of a 'pat'-separated string with 'prefix'
utils.prepend_string = function( s, prefix, pat )  
  if not s then return "" end
  pat = pat or ' '
  local res = ''
  local st = utils.string_to_table( s, pat )
  for i = 1, #st do
    res = res .. prefix .. st[ i ] .. " "
  end
  return res
end

-- Like above but consider 'prefix' a path
utils.prepend_path = function( s, prefix, pat )
  return utils.prepend_string( s, prefix .. dir_sep, pat )
end

-- full mkdir: create all the paths needed for a multipath
utils.full_mkdir = function( path )
  local ptables = utils.string_to_table( path, dir_sep )
  local p, res = ''
  for i = 1, #ptables do
    p = ( i ~= 1 and p .. dir_sep or p ) .. ptables[ i ]
    res = lfs.mkdir( p )
  end
  return res
end

-- Concatenate the given paths to form a complete path
utils.concat_path = function( paths )
  return table.concat( paths, dir_sep )
end

-------------------------------------------------------------------------------
-- Dummy 'builder': simply checks the date of a file

local _fbuilder = {}

_fbuilder.new = function( target, dep )
  local self = {}
  setmetatable( self, { __index = _fbuilder } )
  self.target = target
  self.dep = dep
  return self
end

_fbuilder.build = function( self )
 -- Doesn't build anything but returns 'true' if the dependency is newer than
 -- the target
 if is_phony( self.target ) then
   return true
 else
   return get_ftime( self.dep ) > get_ftime( self.target )
 end
end

_fbuilder.target_name = function( self )
  return get_target_name( self.dep )
end

-------------------------------------------------------------------------------
-- Target object

local _target = {}

_target.new = function( target, dep, command )
  local self = {}
  setmetatable( self, { __index = _target } )
  self.target = target
  self.command = command
  -- Transform 'dep' into a list of objects that support the 'build' method
  if type( dep ) == 'string' then 
    dep = utils.string_to_table( dep ) 
  elseif type( dep ) ~= 'table' then
    dep = {}
  end
  -- Iterate through 'dep' transforming file names in _fbuilder targets
  for i = 1, #dep do
    if type( dep[ i ] ) == "string" then
      dep[ i ] = _fbuilder.new( target, dep[ i ] )
    end
  end
  self.dep = dep
  self._force_rebuild = #dep == 0
  return self
end

-- Force rebuild
_target.force_rebuild = function( self, flag )
  self._force_rebuild = flag
end

-- Build the given target
_target.build = function( self )
  local docmd = self:target_name() and lfs.attributes( self:target_name(), "mode" ) ~= "file"
  local depends = ''
  local dep = self.dep
  -- Iterate through all dependencies, execute each one in turn
  for i = 1, #dep do
    local res = dep[ i ]:build()
    docmd = docmd or res
    local t = dep[ i ]:target_name()
    if t then depends = depends .. t .. " " end
  end
  docmd = docmd or self._force_rebuild
  -- If at least one dependency is new rebuild the target
  if docmd and self.command then
    local cmd = self.command
    local code
    if type( cmd ) == 'string' then
      cmd = expand_key( cmd, "TARGET", self.target )
      cmd = expand_key( cmd, "DEPENDS", depends )
      cmd = expand_key( cmd, "FIRST", dep[ 1 ]:target_name() )
      print( cmd )
      code = os.execute( cmd )   
    else
      code = cmd( self.target, self.dep )
    end
    if code ~= 0 then return nil, "Error building target " .. self.target end
  end
  return docmd
end

_target.target_name = function( self )
  return get_target_name( self.target )
end

-------------------------------------------------------------------------------
-- Builder public interface

builder = { KEEP_DIR = 0, BUILD_DIR = 1, BUILD_DIR_LINEARIZED = 2 }    

-- Create a new builder object with the output in 'build_dir' and with the 
-- specified compile, dependencies and link command
builder.new = function( build_dir, comp_cmd, dep_cmd, link_cmd )
  self = {}
  setmetatable( self, { __index = builder } )
  self.build_dir = build_dir or ".build"
  self.comp_cmd = comp_cmd
  self.dep_cmd = dep_cmd
  self.link_cmd = link_cmd
  self.exe_extension = utils.is_windows() and "exe" or ""
  -- Create 'builds' directory if needed
  local mode = lfs.attributes( self.build_dir, "mode" )
  if not mode or mode ~= "directory" then
    if not utils.full_mkdir( self.build_dir ) then
      print( "Unable to create directory " .. self.build_dir )
      os.exit( 1 )
    end
  end
  self.clean_mode = false
  self.build_mode = self.KEEP_DIR
  return self
end

-- Initialize builder from the given command line
builder.init = function( self, args )
  self.args = args
  -- Look for '-c' first
  for i = 1, #args do
    local a = args[ i ]:upper()
    if a == "-C" then
      self.clean_mode = true  
    end
  end
  -- Then look for 'build_mode' spec
  local v = self:get_arg( 'build_mode', 'KEEP_DIR' ):upper()
  self.build_mode = self[ v ] or self.KEEP_DIR
end

-- Get the given argument or the default value if not found
-- Arguments are given in the form 'arg=value'
builder.get_arg = function( self, name, default )
  local args = self.args
  name = name:upper()
  for i = 1, #args do
    local arg = args[ i ]
    local si, ei, k, v = arg:find( "([^=]+)=(.*)$" )
    if si and k:upper() == name then return v end
  end
  return default
end

-- Set the compile command
builder.set_compile_cmd = function( self, cmd )
  self.comp_cmd = cmd
end

-- Set the dependencies command
builder.set_dep_cmd = function( self, cmd )
  self.dep_cmd = cmd
end

-- Set the link command
builder.set_link_cmd = function( self, cmd )
  self.link_cmd = cmd
end

-- Set (actually force) the object file extension
builder.set_object_extension = function( self, ext )
  self.obj_extension = ext
end

-- Set (actually force) the executable file extension
builder.set_exe_extension = function( self, ext )
  self.exe_extension = ext
end

-- Set the clean mode
builder.set_clean_mode = function( self, isclean )
  self.clean_mode = isclean
end

-- Is the builder in clean mode?
builder.is_cleaning = function( self )
  return self.clean_mode
end

-- Sets the build mode
builder.set_build_mode = function( self, mode )
  self.build_mode = mode
end

-- Create a return a new C to object target
builder.c_target = function( self, target, deps, comp_cmd )
  return _target.new( target, deps, comp_cmd or self.comp_cmd )
end

-- Return the name of a dependency file name corresponding to a C source
builder.get_dep_filename = function( self, srcname )
  return utils.replace_extension( self.build_dir .. dir_sep .. linearize_fname( srcname ), "d" )
end

-- Create a return a new C dependency target
builder.dep_target = function( self, dep, depdeps, dep_cmd )
  local depname = self:get_dep_filename( dep )
  return _target.new( depname, depdeps, dep_cmd or self.dep_cmd )
end

-- Create and return a new link target
builder.link_target = function( self, out, dep, link_cmd )
  if not out:find( "%." ) and self.exe_extension and #self.exe_extension > 0 then
    out = out .. self.exe_extension
  end
  return _target.new( out, dep, link_cmd or self.link_cmd )
end

-- Create and return a new generic target
builder.target = function( self, dest_target, deps, cmd )
  return _target.new( dest_target, deps, cmd )
end

-- Internal helper
builder._generic_cmd = function( self, args )
  local compcmd = args.compiler or "gcc"
  compcmd = compcmd .. " "
  local flags = table_to_string( args.flags )
  local defines = table_to_string( args.defines )
  local includes = table_to_string( args.includes )
  local comptype = table_to_string( args.comptype ) or "-c"
  compcmd = compcmd .. utils.prepend_string( defines, "-D" )
  compcmd = compcmd .. utils.prepend_string( includes, "-I" )
  return compcmd .. flags .. " " .. comptype .. " -o $(TARGET) $(FIRST)"
end

-- Return a compile command based on the specified args

builder.compile_cmd = function( self, args )
  return self:_generic_cmd( args )
end

-- Return a dependency generation command based on the specified args
builder.dep_cmd = function( self, args )
  args.comptype = "-E -MM"
  return self:_generic_cmd( args )
end

-- Return a link command based on the specified args
builder.link_cmd = function( self, args )
  local flags = table_to_string( args.flags )
  local libraries = table_to_string( args.libraries )
  local linkcmd = args.linker or "gcc"
  linkcmd = linkcmd .. " " .. flags .. " -o $(TARGET) $(DEPENDS)"
  linkcmd = linkcmd .. utils.prepend_string( libraries, "-l" )
  return linkcmd
end

-- Return the object name corresponding to a source file name
builder.obj_name = function( self, name )
  local r = self.obj_extension
  if not r then
    r = utils.is_windows() and "obj" or "o"
  end
  local objname = utils.replace_extension( name, r )
  -- KEEP_DIR: object file in the same directory as source file
  -- BUILD_DIR: object file in the build directory
  -- BUILD_DIR_LINEARIZED: object file in the build directory, linearized filename
  if self.build_mode == self.KEEP_DIR then 
    return objname
  elseif self.build_mode == self.BUILD_DIR_LINEARIZED then
    return self.build_dir .. dir_sep .. linearize_fname( objname )
  else
    local si, ei, path, fname = objname:find( "(.+)/(.-)$" )
    if not si then fname = objname end
    return self.build_dir .. dir_sep .. fname 
  end
end

-- Helper: remove the target (used in clean mode)
builder._clean = function( target, deps )
  print( "Removing " .. target )
  os.remove( target )
end

-- Read and interpret dependencies for each file specified in "ftable"
-- "ftable" is either a space-separated string with all the source files or an array
builder.read_depends = function( self, ftable )
  if type( ftable ) == 'string' then ftable = utils.string_to_table( ftable ) end
  -- Read dependency data
  local dtable = {}
  for i = 1, #ftable do
    local f = io.open( self:get_dep_filename( ftable[ i ] ), "rb" )
    local lines = ftable[ i ]
    if f then
      lines = f:read( "*a" )
      f:close()
      lines = lines:gsub( "\n", " " ):gsub( "\\%s+", " " ):gsub( "%s+", " " ):gsub( "^.-: (.*)", "%1" )
    end
    dtable[ ftable[ i ] ] = lines
  end
  return dtable
end

-- Build and interpret dependencies for the given source files
-- "flable" is either a space-separated string with all the source files or an array
builder.make_depends = function( self, ftable )
  if type( ftable ) == 'string' then ftable = utils.string_to_table( ftable )end

  -- Start with initial dependency data (this might be nil when generated initially)
  local initdep = self:read_depends( ftable )

  -- Build dependencies for all targets
  local deptargets = {}
  for i = 1, #ftable do
    local target = self:dep_target( ftable[ i ], initdep[ ftable[ i ] ], self.clean_mode and builder._clean or nil )
    target:force_rebuild( self.clean_mode )
    table.insert( deptargets, target )
  end
  local t = builder:target( "#phony.deps", deptargets )
  t:build()
  if self.clean_mode then return end

  -- Read dependency data
  self.dtable = self:read_depends( ftable )
end

-- Create compile targets for the given sources
builder.create_compile_targets = function( self, ftable, res )
  if not self.dtable then
    if not self.clean_mode then
      print "Error: call make_depends before compile_targets"
      os.exit( 1 )
    end
  end
  res = res or {}
  if type( ftable ) == 'string' then
    ftable = utils.string_to_table( ftable )
  end

  -- Build dependencies for all targets
  for i = 1, #ftable do
    local target = self:c_target( self:obj_name( ftable[ i ] ), self.dtable and self.dtable[ ftable[ i ] ], self.clean_mode and builder._clean or nil )
    target:force_rebuild( self.clean_mode )
    table.insert( res, target )
  end

  return res
end

-- Build the specified target starting from the given deps
-- This links in regular mode and cleans in clean mode
builder.build_c_target = function( self, out, odeps )
  local t = self:link_target( out, odeps, self.clean_mode and builder._clean or nil )
  t:force_rebuild( self.clean_mode )
  t:build()
end

-------------------------------------------------------------------------------
-- Other exported functions

function new_builder( build_dir, comp_cmd, dep_cmd, link_cmd )
  return builder.new( build_dir, comp_cmd, dep_cmd, link_cmd )
end

