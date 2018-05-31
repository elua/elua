#! /usr/bin/env lua

--[[
   build_elua.lua: A build script for eLua written in Lua.

   The command line syntax is the same as for the old scons/SConstruct system.
   See http://www.eluaproject.net/en_building.html

   The only required option is the target board or CPU. e.g.:
     lua build_elua.lua board=MIZAR32

   This script requires some well-known Lua libraries to run.
   To install them on Ubuntu/Debian, go (as root):
       apt-get install luarocks
       luarocks install luafilesystem
       luarocks install lpack
       luarocks install md5
--]]

local args = { ... }
local b = require "utils.build"
local mkfs = require "utils.mkfs"
local bconf = require "config.config"
local board_base_dir = "boards"
local bd = require "build_data"

builder = b.new_builder()
utils = b.utils
sf = string.format

-------------------------------------------------------------------------------
-- Build configuration 'shortcuts'

cdefs, cflags, includes, lflags, asflags, libs = {}, {}, {}, {}, {}, {}

-- "Normalize" a name to make it a suitable C macro name
function cnorm( name )
  name = name:gsub( "[%-%s]*", '' )
  return name:upper()
end

-- Add a macro defition
function addm( data )
  table.insert( cdefs, data )
end

-- Add an include directory
function addi( data )
  table.insert( includes, data )
end

-- Add a compiler flag
function addcf( data )
  table.insert( cflags, data )
end

-- Delete a compiler flag
function delcf( data )
  cflags = utils.linearize_array( cflags )
  for _, v in pairs( data ) do
    local i = utils.array_element_index( cflags, v )
    if i then table.remove( cflags, i ) end
  end
end

-- Add a linker flag
function addlf( data )
  table.insert( lflags, data )
end

-- Add an assembler flag
function addaf( data )
  table.insert( asflags, data )
end

-- Add a library
function addlib( data )
  table.insert( libs, data )
end

-------------------------------------------------------------------------------

-- Return the full path of a board configuration file
local function get_conf_file_path( bname )
  local known_board_name = utils.concat_path( { board_base_dir, "known", bname .. ".lua" } )
  local custom_board_name = utils.concat_path( { board_base_dir, "custom", bname .. ".lua" } )
  if utils.is_file( custom_board_name ) then return custom_board_name end
  if utils.is_file( known_board_name ) then return known_board_name end
  assert( sf( "board configuration file for board '%s' not found!", bname ) )
end

-- Automatically build the list of available boards by scanning the board_base_dir directory
local board_flist = utils.linearize_array( utils.string_to_table( utils.get_files( board_base_dir, "%.lua$" ) ) )
local board_list = {}
for k, v in pairs( board_flist ) do
  local temp = utils.replace_extension( v, '' )
  temp = utils.string_to_table( temp, utils.dir_sep )
  temp = temp[ #temp ] -- now 'temp' contains the actual name of the board
  if not utils.array_element_index( board_list, temp ) then board_list[ #board_list + 1 ] = temp end
end

-- Check a single command line option against the corresponding value in the build configuration
local function check_cmdline_vs_conf( argname, comp, bd )
  if bd[ argname ] then
    if comp[ argname ] == "auto" then
      comp[ argname ] = bd[ argname ]
    elseif bd[ argname ] ~= comp[ argname ] then
      if builder:is_user_option( argname ) then
        print( utils.col_yellow( sf( "[CONFIG] WARNING: changing '%s' from '%s' to '%s' as specified in the command line", 
               argname, tostring( bd[ argname ] ), tostring( comp[ argname ] ) ) ) )
      else
        comp[ argname ] = bd[ argname ]
      end
    end
  end
end

builder:add_option( 'target', 'build "regular" float lua, 32 bit integer-only "lualong" or 64-bit integer only lua "lualonglong"', 'lua', { 'lua', 'lualong', 'lualonglong' } )
builder:add_option( 'allocator', 'select memory allocator', 'auto', { 'newlib', 'multiple', 'simple', 'auto' } )
builder:add_option( 'board', 'selects board for target (cpu will be inferred)', nil, board_list )
builder:add_option( 'toolchain', 'specifies toolchain to use (auto=search for usable toolchain)', 'auto', { bd.get_all_toolchains(), 'auto' } )
builder:add_option( 'optram', 'enables Lua Tiny RAM enhancements', true )
builder:add_option( 'boot', 'boot mode, standard will boot to shell, luarpc boots to an rpc server', 'standard', { 'standard' , 'luarpc' } )
builder:add_option( 'romfs', 'ROMFS compilation mode', 'verbatim', { 'verbatim' , 'compress', 'compile' } )
builder:add_option( 'cpumode', 'ARM CPU compilation mode (only affects certain ARM targets)', nil, { 'arm', 'thumb' } )
builder:add_option( 'bootloader', 'Build for bootloader usage (AVR32 only)', 'none', { 'none', 'emblod' } )
builder:add_option( 'debug', 'Enable debug build', false )
builder:add_option( 'extras', 'Path to directory containing build extras', '' )
builder:add_option( 'extrasconf', 'Config file for build extras, defaults to conf.lua', '' )
builder:add_option( "output_dir", "choose executable directory", "." )
builder:add_option( "romfs_dir", 'choose ROMFS directory', 'romfs' )
builder:add_option( "board_config_file", "choose board configuration file", "" )
builder:add_option( "skip_conf", "skip board configuration step, use pre-generated header file directly", false )
builder:add_option( "config_only", "execute only the configurator, then exit", false )
builder:init( args )
builder:set_build_mode( builder.BUILD_DIR_LINEARIZED )

-- Build the 'comp' target which will 'redirect' all the requests
-- for its fields to builder:get_option
comp = {}
setmetatable( comp, { __index = function( t, key ) return builder:get_option( key ) end } )

local function dprint( ... )
  if comp.disp_mode ~= "minimal" then
    print( ... )
  end
end

if not comp.board then
  print "You must specify the board"
  os.exit( -1 )
end

-- Interpret the board definition file
local bfopt, bfname = builder:get_option( "board_config_file" )
if #bfopt > 0 then
  if not utils.is_file( bfopt ) then
    print( utils.col_red( sf( "[CONFIG] Error: board configuration file '%s' not found.", bfopt ) ) )
    os.exit( -1 )
  end
  bfname = bfopt 
else
  bfname = get_conf_file_path( comp.board )
  if not bfname then
    print( utils.col_red( sf( "[CONFIG] Error: board configuration file for board '%s' not found in '%s'.", comp.board, board_base_dir ) ) )
    os.exit( -1 )
  end
end
dprint( utils.col_blue( "[CONFIG] Found board description file at " .. bfname ) )
local bdata, err = bconf.compile_board( bfname, comp.board )
if not bdata then
  print( utils.col_red( "[CONFIG] Error compiling board description file: " .. err ) )
  return
end
-- Check if the file has changed. If not, do not rewrite it. This keeps the compilation time sane.
local bhname = utils.concat_path( { board_base_dir, "headers", "board_" .. comp.board:lower() .. ".h" } )
if builder:get_option( "skip_conf" ) then
  dprint( utils.col_blue( "[CONFIG] skipping generation of configuration file" ) )
else
  if ( utils.get_hash_of_string( bdata.header ) ~= utils.get_hash_of_file( bhname ) or not utils.is_file( bhname ) ) then
    -- Save the header file
    local f = assert( io.open( bhname, "wb" ) )
    f:write( bdata.header )
    f:close()
    dprint( utils.col_blue( "[CONFIG] Generated board header file at " .. bhname ) )
  else
    dprint( utils.col_blue( "[CONFIG] Board header file is unchanged." ) )
  end
end
if comp.config_only then return end
-- Define the correct CPU header for inclusion in the platform_conf.h file
addm( 'ELUA_CPU_HEADER="\\"cpu_' .. bdata.cpu:lower() .. '.h\\""' )
-- Define the correct board header for inclusion in the platform_conf.h file
addm( 'ELUA_BOARD_HEADER="\\"board_' .. comp.board:lower() .. '.h\\""' )
-- Make available the board directory for the generated header files
addi( utils.concat_path{ board_base_dir, "headers" } )
-- Force compilation flags if needed
if bdata.build then
 utils.foreach( { 'target', 'allocator', 'optram', 'boot', 'romfs', 'cpumode', 'bootloader' }, function( k, v )
    check_cmdline_vs_conf( v, comp, bdata.build )
 end )
end
-- Automatically set the allocator to 'multiple' if needed
if bdata.multi_alloc and comp.allocator == "newlib" then
  io.write( utils.col_yellow( "[CONFIG] WARNING: your board has non-contigous RAM areas, but you specified an allocator ('newlib') that can't handle this configuration." ) )
  print( utils.col_yellow( "Rebuild with another allocator ('multiple' or 'simple')" ) )
end
if comp.allocator == "auto" then comp.allocator = bdata.multi_alloc and "multiple" or "newlib" end
comp.cpu = bdata.cpu:upper()
if not comp.optram then
  print( utils.col_yellow( "[CONFIG] WARNING: you have disabled Lua Tiny RAM (LTR). You might experience compilation issues. Also, some modules might not work correctly." ) )
end

platform = bd.get_platform_of_cpu( comp.cpu )
if not platform then
  print( "Unable to find platform (this shouldn't happen, check the build script for errors)" )
  os.exit( -1 )
end

-- Check the toolchain
local usable_chains = bd.get_toolchains_of_platform( platform )
if comp.toolchain ~= 'auto' then
  if utils.array_element_index( usable_chains, comp.toolchain ) == nil then
    print( sf( "Invalid toolchain '%s' for CPU '%s'", comp.toolchain, comp.cpu ) )
    print( sf( "List of accepted toolchains (for %s): %s", comp.cpu, table.concat( usable_chains, "," ) ) )
    os.exit( -1 )
  end
  toolset = bd.get_toolchain_data( comp.toolchain )
  comp.CC = toolset.compile
  comp.AS = toolset.compile
else
  -- If 'auto' try to match a working toolchain with target
  -- Try to execute all compilers, exit when one found
  local chain
  for i = 1, #usable_chains do
    local c = usable_chains[ i ]
    local t = bd.get_toolchain_data( c )
    local res = utils.check_command( t.compile .. " " .. t.version )
    if res == 0 or res == true then chain = c break end
  end
  if chain then
    comp.toolchain = chain
    toolset = bd.get_toolchain_data( chain )
    comp.CC = toolset.compile
    comp.AS = comp.CC
  else
    print "Unable to find an usable toolchain in your path."
    print( sf( "List of accepted toolchains (for %s): %s", comp.cpu, table.concat( usable_chains, "," ) ) )
    os.exit( -1 )
  end
end

-- Build the compilation command now
local fscompcmd = ''
if comp.romfs == 'compile' then
  if comp.target == 'lualonglong' then
    print "Cross-compilation is not yet supported for 64-bit integer-only Lua (lualonglong)."
    os.exit( -1 )
  end
  local suffix = ''
  if utils.is_windows() then
    suffix = '.exe'
  end
  -- First check for luac.cross in the current directory
  if not utils.is_file( "luac.cross" .. suffix ) then
    print "The eLua cross compiler was not found."
    print "Build it by running 'lua cross-lua.lua'"
    os.exit( -1 )
  end
  local cmdpath = { lfs.currentdir(), sf( 'luac.cross%s -ccn %s -cce %s -o %%s -s %%s', suffix, toolset[ "cross_" .. comp.target:lower() ], toolset.cross_cpumode:lower() ) }
  dprint( "Cross compile command: " .. cmdpath[ 2 ] )
  fscompcmd = table.concat( cmdpath, utils.dir_sep )
elseif comp.romfs == 'compress' then
  if comp.target == 'lualong' or comp.target == 'lualonglong' then fscompoptnums = '--noopt-numbers' else fscompoptnums = '--opt-numbers' end
  fscompcmd = 'lua luasrcdiet.lua --quiet --maximum --opt-comments --opt-whitespace --opt-emptylines --opt-eols --opt-strings ' .. fscompoptnums .. ' --opt-locals -o %s %s'
end

-- Determine build version
if utils.check_command('git describe --always') == 0 then
  addm( "USE_GIT_REVISION" )
  elua_vers = utils.exec_capture('git describe --always')
  -- If purely hexadecimal (no tag reference) prepend 'dev-'
  if string.find(elua_vers, "^[+-]?%x+$") then
     elua_vers = 'dev-' .. elua_vers
  end
  local sver = utils.gen_header_string( 'git_version', { elua_version = elua_vers, elua_str_version = ("\"" .. elua_vers .. "\"" ) } )
  if utils.get_hash_of_string( sver ) ~= utils.get_hash_of_file( utils.concat_path{ 'inc', 'git_version.h' } ) then 
    utils.gen_header_file( 'git_version', { elua_version = elua_vers, elua_str_version = ("\"" .. elua_vers .. "\"" ) } )
  end
else
  print "WARNING: unable to determine version from repository"
  elua_vers = "unknown"
end

-- Create the output directory if it is not created yet
local outd = builder:get_option( "output_dir" )
if not utils.is_dir( outd ) then
  if not utils.full_mkdir( outd ) then
    print( "[builder] Unable to create directory " .. outd )
    os.exit( 1 )
  end
end

-- Output file
output = outd .. utils.dir_sep .. 'elua_' .. comp.target .. '_' .. comp.board:lower()
builder:set_build_dir( builder:get_build_dir() .. utils.dir_sep .. comp.board:lower() )

-- User report
dprint ""
dprint "*********************************"
dprint "Compiling eLua ..."
dprint( "CPU:            ", comp.cpu )
dprint( "Board:          ", comp.board )
dprint( "Platform:       ", platform )
dprint( "Allocator:      ", comp.allocator )
dprint( "Boot Mode:      ", comp.boot )
dprint( "Target:         ", comp.target  )
dprint( "Toolchain:      ", comp.toolchain )
dprint( "ROMFS mode:     ", comp.romfs )
dprint( "Debug:          ", comp.debug )
if comp.extras ~= '' then
  dprint( "Extras:         ", comp.extras )
end
if comp.extrasconf ~= '' then
  dprint( "Extras conf:         ", comp.extrasconf )
end
dprint( "Version:        ", elua_vers )
dprint "*********************************"
dprint ""

-- Build list of source files, include directories, macro definitions
addm( "ELUA_CPU=" .. comp.cpu:upper() )
addm( "ELUA_BOARD=" .. comp.board:upper() )
addm( "ELUA_PLATFORM=" .. platform:upper() )
addm( "__BUFSIZ__=128" )

-- Also make the above into direct defines (to use in conditional C code)
addm( "ELUA_CPU_" .. cnorm( comp.cpu ) )
addm( "ELUA_BOARD_" .. cnorm( comp.board ) )
addm( "ELUA_PLATFORM_" .. cnorm( platform ) )

if comp.allocator == 'multiple' then
   addm( "USE_MULTIPLE_ALLOCATOR" )
elseif comp.allocator == 'simple' then
   addm( "USE_SIMPLE_ALLOCATOR" )
end
if comp.boot == 'luarpc' then addm( "ELUA_BOOT_RPC" ) end
if comp.target == 'lualong' or comp.target == 'lualonglong' then addm( "LUA_NUMBER_INTEGRAL" ) end
if comp.target == 'lualonglong' then addm( "LUA_INTEGRAL_LONGLONG" ) end
if comp.target ~= 'lualong' and comp.target ~= "lualonglong" then addm( "LUA_PACK_VALUE" ) end
if bd.get_endianness_of_platform( platform ) == "big" then addm( "ELUA_ENDIAN_BIG" ) else addm( "ELUA_ENDIAN_LITTLE" ) end

-- Special macro definitions for the SIM target
if platform == 'sim' then addm( { "ELUA_SIMULATOR", "ELUA_SIM_" .. cnorm( comp.cpu ) } ) end

-- Lua source files and include path
exclude_patterns = { "^src/platform", "^src/uip", "^src/serial", "^src/luarpc_desktop_serial.c", "^src/linenoise_posix.c", "^src/lua/print.c", "^src/lua/luac.c" }
local source_files = utils.get_files( "src", function( fname )
  fname = fname:gsub( "\\", "/" )
  local include = fname:find( ".*%.c$" )
  if include then
    utils.foreach( exclude_patterns, function( k, v ) if fname:match( v ) then include = false end end )
  end
  return include
end )
-- Add uIP files manually because not all of them are included in the build ([TODO] why?)
local uip_files = " " .. utils.prepend_path( "uip_arp.c uip.c uiplib.c dhcpc.c psock.c resolv.c uip-neighbor.c", "src/uip" )

addi{ { 'inc', 'inc/newlib',  'inc/remotefs', 'src/platform', 'src/lua' }, { 'src/modules', 'src/platform/' .. platform, 'src/platform/' .. platform .. '/cpus' }, "src/uip", "src/fatfs", "inc/niffs" }
addm( "LUA_OPTIMIZE_MEMORY=" .. ( comp.optram and "2" or "0" ) )
addcf( { '-Os','-fomit-frame-pointer' } )

if comp.debug == true then
   addcf( { '-g' } )
end

-- Toolset data (filled by each platform in part)
tools = {}
specific_files = ''
extras_files = ''

-- We get platform-specific data by executing the platform script
dofile( sf( "src/platform/%s/conf.lua", platform ) )

-- Read the extra configuration if needed. This can set
-- the extra files to compile in the 'extras_files' variable.
if comp.extras ~= '' then
  if comp.extrasconf ~= '' then
    dofile( sf( "%s/%s", comp.extras, comp.extrasconf ) )
  else
    dofile( sf( "%s/conf.lua", comp.extras ) )
  end
end

-- Complete file list
source_files = source_files .. uip_files .. specific_files .. extras_files

-------------------------------------------------------------------------------
-- Create compiler/linker/assembler command lines and build

-- ROM file system builder

romfs_exclude_patterns = { '%.DS_Store', '%.gitignore' }

function match_pattern_list( item, list )
  for k, v in pairs( list ) do
    if item:find(v) then return true end
  end
end

local function make_romfs( target, deps )
  print "Building ROM file system ..."
  local romdir = builder:get_option( "romfs_dir" )
  local flist = {}
  flist = utils.string_to_table( utils.get_files( romdir, function( fname ) return not match_pattern_list( fname, romfs_exclude_patterns ) end ) )
  flist = utils.linearize_array( flist )
  for k, v in pairs( flist ) do
    flist[ k ] = v:gsub( romdir .. utils.dir_sep, "" )
  end

  if not mkfs.mkfs( romdir, "romfiles", flist, comp.romfs, fscompcmd ) then return -1 end
  if utils.is_file( "inc/romfiles.h" ) then
    -- Read both the old and the new file
    local oldfile = io.open( "inc/romfiles.h", "rb" )
    assert( oldfile )
    local newfile = io.open( "romfiles.h", "rb" )
    assert( newfile )
    local olddata, newdata = oldfile:read( "*a" ), newfile:read( "*a" )
    oldfile:close()
    newfile:close()
    -- If content is similar return '1' to builder to indicate that the target didn't really
    -- produce a change even though it ran
    if olddata == newdata then
      os.remove( "romfiles.h" )
      return 1
    end
    os.remove( "inc/romfiles.h" )
  end
  os.rename( "romfiles.h", "inc/romfiles.h" )
  return 0
end

-- Generic 'prog' action function
local function genprog( target, deps )
  local outname = deps[ 1 ]:target_name()
  local outtype = target:find( "%.hex$" ) and "ihex" or "binary"
  print( sf( "Generating binary image %s...", target ) )
  os.execute( sf( "%s %s", toolset.size, outname ) )
  os.execute( sf( "%s -O %s %s %s", toolset.bin, outtype, outname, target ) )
  return 0
end

-- Generic 'size' action function
local function sizefunc( target, deps )
  local outname = deps[ 1 ]:target_name()
  os.execute( sf( "%s %s", toolset.size, outname ) )
  return 0
end

-- Command lines for the tools (compiler, linker, assembler)
compcmd = compcmd or builder:compile_cmd{ flags = cflags, defines = cdefs, includes = includes, compiler = toolset.compile }
linkcmd = linkcmd or builder:link_cmd{ flags = lflags, libraries = libs, linker = toolset.compile }
ascmd = ascmd or builder:asm_cmd{ flags = asflags, defines = cdefs, includes = includes, assembler = toolset.asm }
builder:set_exe_extension( ".elf" )
builder:set_compile_cmd( compcmd )
builder:set_link_cmd( linkcmd )
builder:set_asm_cmd( ascmd )

-- Create the ROMFS target
local romfs_target = builder:target( "#phony:romfs", nil, make_romfs )
romfs_target:force_rebuild( true )

-- Create executable targets
odeps = builder:create_compile_targets( source_files )
exetarget = builder:link_target( output, { romfs_target, odeps } )
-- This is also the default target
builder:default( builder:add_target( exetarget, 'build eLua executable' ) )

-- Create 'prog' target(s)
local ptargets = {}
local progfunc = tools[ platform ].progfunc or genprog
utils.foreach( tools[ platform ].prog_flist, function( _, t )
  local target = builder:target( t, { exetarget }, progfunc )
  table.insert( ptargets, target )
end )
if #ptargets > 0 then
  progtarget = builder:target( "#phony:prog", ptargets )
  builder:add_target( progtarget, "build eLua firmware image", { "prog" } )
end

-- Create generic 'size' target
local size_target = builder:target( "#phony:size", { exetarget }, sizefunc )
size_target:force_rebuild( true )
builder:add_target( size_target, "shows the size of the eLua firmware", { "size" } )

-- If the backend needs to do more processing before the build starts, do it now
if tools[ platform ].pre_build then
  tools[ platform ].pre_build()
end

-- Finally build everything
builder:build()

