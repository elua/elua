local args = { ... }
local b = require "utils.build"
local mkfs = require "utils.mkfs"
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
-- Build data

-- List of toolchains
local toolchain_list = 
{
  [ 'arm-gcc' ] = { 
    compile = 'arm-elf-gcc', 
    link = 'arm-elf-ld', 
    asm = 'arm-elf-as', 
    bin = 'arm-elf-objcopy', 
    size = 'arm-elf-size',
    cross_cpumode = 'little',
    cross_lua = 'float_arm 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  [ 'arm-eabi-gcc' ] = {
    compile = 'arm-eabi-gcc',
    link = 'arm-eabi-ld',
    asm = 'arm-eabi-as',
    bin = 'arm-eabi-objcopy',
    size = 'arm-eabi-size',
    cross_cpumode = 'little',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  codesourcery = { 
    compile = 'arm-none-eabi-gcc', 
    link = 'arm-none-eabi-ld', 
    asm = 'arm-none-eabi-as', 
    bin = 'arm-none-eabi-objcopy', 
    size = 'arm-none-eabi-size',
    cross_cpumode = 'little',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  [ 'avr32-gcc' ] = { 
    compile = 'avr32-gcc', 
    link = 'avr32-ld', 
    asm = 'avr32-as', 
    bin = 'avr32-objcopy', 
    size = 'avr32-size',
    cross_cpumode = 'big',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  },
  [ 'i686-gcc' ] = { 
    compile = 'i686-elf-gcc', 
    link = 'i686-elf-ld', 
    asm = 'nasm', 
    bin = 'i686-elf-objcopy', 
    size = 'i686-elf-size',
    cross_cpumode = 'little',
    cross_lua = 'float 64',
    cross_lualong = 'int 32',
    version = '--version'
  }
}

-- Toolchain Aliases
toolchain_list[ 'devkitarm' ] = toolchain_list[ 'arm-eabi-gcc' ]

-- List of platform/CPU/toolchains combinations
-- The first toolchain in the toolchains list is the default one
-- (the one that will be used if none is specified)
local platform_list = 
{  
  at91sam7x = { cpus = { 'AT91SAM7X256', 'AT91SAM7X512' }, toolchains = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' } },
  lm3s = { cpus = { 'LM3S1968', 'LM3S8962', 'LM3S6965', 'LM3S6918', 'LM3S9B92' }, toolchains = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' } },
  str9 = { cpus = { 'STR912FAW44' }, toolchains = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' } },
  i386 = { cpus = { 'I386' }, toolchains = { 'i686-gcc' } },
  sim = { cpus = { 'LINUX' }, toolchains = { 'i686-gcc' } },
  lpc288x = { cpus = { 'LPC2888' }, toolchains = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' } },
  str7 = { cpus = { 'STR711FR2' }, toolchains = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' } },
  stm32 = { cpus = { 'STM32F103ZE', 'STM32F103RE' }, toolchains = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' } },
  avr32 = { cpus = { 'AT32UC3A0512', 'AT32UC3A0128', 'AT32UC3B0256' }, toolchains = { 'avr32-gcc' } },
  lpc24xx = { cpus = { 'LPC2468' }, toolchains = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' } },
  lpc17xx = { cpus = { 'LPC1768' }, toolchains = { 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' } }
}

-- List of board/CPU combinations
local board_list = 
{ 
  [ 'SAM7-EX256' ]      = { 'AT91SAM7X256', 'AT91SAM7X512' },
  [ 'EK-LM3S1968' ]     = { 'LM3S1968' },
  [ 'EK-LM3S8962' ]     = { 'LM3S8962' },
  [ 'EK-LM3S6965' ]     = { 'LM3S6965' },
  [ 'EK-LM3S9B92' ]     = { 'LM3S9B92' },
  [ 'STR9-COMSTICK' ]   = { 'STR912FAW44' },
  [ 'STR-E912' ]        = { 'STR912FAW44' },
  [ 'PC' ]              = { 'I386' },
  [ 'SIM' ]             = { 'LINUX' },
  [ 'LPC-H2888' ]       = { 'LPC2888' },
  [ 'MOD711' ]          = { 'STR711FR2' },
  [ 'STM3210E-EVAL' ]   = { 'STM32F103ZE' },
  [ 'ATEVK1100' ]       = { 'AT32UC3A0512' },
  [ 'ATEVK1101' ]       = { 'AT32UC3B0256' },
  [ 'ET-STM32' ]        = { 'STM32F103RE' },
  [ 'EAGLE-100' ]       = { 'LM3S6918' },
  [ 'ELUA-PUC' ]        = { 'LPC2468' },
  [ 'MBED' ]            = { 'LPC1768' },
  [ 'MIZAR32' ]         = { 'AT32UC3A0128' },
  [ 'NETDUINO' ]        = { 'AT91SAM7X512' },
}

-- Build the CPU list starting from the above list
local cpu_list = {}
for k, v in pairs( board_list ) do
  local clist = v
  for i = 1, #clist do
    if not utils.array_element_index( cpu_list, clist[ i ] ) then      
      table.insert( cpu_list, clist[ i ] )
    end
  end
end

-- ROMFS file list "groups"
-- To include a file in a ROMFS build, include it in a group here (or create one
-- if you need) and make sure the group is included on your platform's file_list
-- definition (right after this).

-- The following table will be left here just as an example
-- eLua examples were removed from the distro since v0.8
--[[
local romfs = 
{
  bisect = { 'bisect.lua' },
  hangman = { 'hangman.lua' },
  lhttpd = { 'index.pht', 'lhttpd.lua', 'test.lua' },
  led = { 'led.lua' },
  piano = { 'piano.lua' },
  pwmled = { 'pwmled.lua' },
  tvbgone = { 'tvbgone.lua', 'codes.bin' },
  hello = { 'hello.lua' },
  info = { 'info.lua' },
  morse = { 'morse.lua' },
  dualpwm = { 'dualpwm.lua' },
  adcscope = { 'adcscope.lua' },
  adcpoll = { 'adcpoll.lua' },
  life = { 'life.lua' },
  logo = {'logo.lua', 'logo.bin' },
  pong = { 'pong.lua' },
  spaceship = { 'spaceship.lua' },
  tetrives = { 'tetrives.lua' },
  snake = { 'snake.lua' },
  dataflash = { 'dataflash.lua' },
  pachube = { 'pachube_demo.lua' },
  inttest = { 'inttest.lua' }
}
--]]

local romfs = {}

-- List of board/romfs data combinations
-- The following table will be left here just as an example
-- eLua examples were removed from the distro since v0.8
--[[
local file_list = 
{ 
  [ 'SAM7-EX256' ] = { 'bisect', 'hangman' , 'led', 'piano', 'hello', 'info', 'morse' },
  [ 'EK-LM3S1968' ]  = { 'bisect', 'hangman', 'pong', 'led', 'piano', 'pwmled', 'hello', 'info', 'morse', 'adcscope', 'adcpoll', 'logo', 'spaceship', 'tetrives', 'snake' },
  [ 'EK-LM3S8962' ]  = { 'lhttpd','bisect', 'led', 'pachube' },
  [ 'EK-LM3S6965' ]  = { 'bisect', 'hangman', 'pong', 'led', 'piano', 'pwmled', 'hello', 'info', 'morse', 'adcscope', 'adcpoll', 'logo', 'tetrives' },
  [ 'EK-LM3S9B92' ]  = { 'bisect', 'hangman', 'led', 'pwmled', 'hello', 'info', 'adcscope','adcpoll', 'life' },
  [ 'STR9-COMSTICK' ]  = { 'bisect', 'hangman', 'led', 'hello', 'info' },
  [ 'STR-E912' ]  = { 'bisect', 'hangman', 'led', 'hello', 'info', 'piano', 'adcscope' },
  [ 'PC' ]  = { 'bisect', 'hello', 'info', 'life', 'hangman' },
  [ 'SIM' ]  = { 'bisect', 'hello', 'info', 'life', 'hangman' },
  [ 'LPC-H2888' ]  = { 'bisect', 'hangman', 'led', 'hello', 'info' },
  [ 'MOD711' ]  = { 'bisect', 'hangman', 'led', 'hello', 'info', 'dualpwm' },
  [ 'STM3210E-EVAL' ]  = { 'bisect', 'hello', 'info' },
  [ 'ATEVK1100' ]  = { 'bisect', 'hangman', 'led', 'hello', 'info', 'dataflash' },
  [ 'ATEVK1101' ]  = { 'bisect', 'led', 'hello', 'info', 'dataflash' },
  [ 'ET-STM32' ]  = { 't' },
  [ 'EAGLE-100' ]  = { 'bisect', 'hangman', 'lhttpd', 'led', 'hello', 'info' },
  [ 'ELUA-PUC' ]  = { 'bisect', 'hangman', 'led', 'hello', 'info', 'pwmled', 'adcscope', 'adcpoll', 'inttest' },
  [ 'MBED' ]  = { 'bisect', 'hangman', 'hello', 'info', 'led', 'pwmled', 'dualpwm', 'life', 'adcscope', 'adcpoll' },
  [ 'MIZAR32' ]  = { },
  [ 'NETDUINO' ] = { },
}
--]]

local file_list = 
{ 
  [ 'SAM7-EX256' ]      = {},
  [ 'EK-LM3S1968' ]     = {},
  [ 'EK-LM3S8962' ]     = {},
  [ 'EK-LM3S6965' ]     = {},
  [ 'EK-LM3S9B92' ]     = {},
  [ 'STR9-COMSTICK' ]   = {},
  [ 'STR-E912' ]        = {},
  [ 'PC' ]              = {},
  [ 'SIM' ]             = {},
  [ 'LPC-H2888' ]       = {},
  [ 'MOD711' ]          = {},
  [ 'STM3210E-EVAL' ]   = {},
  [ 'ATEVK1100' ]       = {},
  [ 'ATEVK1101' ]       = {},
  [ 'ET-STM32' ]        = {},
  [ 'EAGLE-100' ]       = {},
  [ 'ELUA-PUC' ]        = {},
  [ 'MBED' ]            = {},
  [ 'MIZAR32' ]         = {},
  [ 'NETDUINO']         = {},
}

builder:add_option( 'target', 'build "regular" float lua or integer-only "lualong"', 'lua', { 'lua', 'lualong' } )
builder:add_option( 'cpu', 'build for the specified CPU (board will be inferred, if possible)', 'auto', { cpu_list, 'auto' } )
builder:add_option( 'allocator', 'select memory allocator', 'auto', { 'newlib', 'multiple', 'simple', 'auto' } )
builder:add_option( 'board', 'selects board for target (cpu will be inferred)', 'auto', { utils.table_keys( board_list ), 'auto' } )
builder:add_option( 'toolchain', 'specifies toolchain to use (auto=search for usable toolchain)', 'auto', { utils.table_keys( toolchain_list ), 'auto' } )
builder:add_option( 'optram', 'enables Lua Tiny RAM enhancements', true )
builder:add_option( 'boot', 'boot mode, standard will boot to shell, luarpc boots to an rpc server', 'standard', { 'standard' , 'luarpc' } )
builder:add_option( 'romfs', 'ROMFS compilation mode', 'verbatim', { 'verbatim' , 'compress', 'compile' } )
builder:init( args )
builder:set_build_mode( builder.BUILD_DIR_LINEARIZED )

-- Build the 'comp' target which will 'redirect' all the requests
-- for its fields to builder:get_option
comp = {}
setmetatable( comp, { __index = function( t, key ) return builder:get_option( key ) end } )

-- Variants: board = <board>
--           cpu = <cpuname>
--           board = <board> cpu=<cpuname>
if comp.board == 'auto' and comp.cpu == 'auto' then
  print "Must specifiy board, cpu, or both"
  os.exit( -1 )
elseif comp.board ~= 'auto' and comp.cpu ~= 'auto' then
  -- Check if the board, cpu pair is correct
  if utils.array_element_index( board_list[ comp.board:upper() ], comp.cpu:upper() ) == nil then
    print( sf( "Invalid CPU '%s' for board '%s'" , comp.cpu, comp.board ) )
    os.exit( -1 )
  end
elseif comp.board ~= 'auto' then
  -- Find CPU
  comp.cpu = board_list[ comp.board:upper() ][ 1 ]
else
  -- cpu = <cputype>
  -- Find board name
  for b, v in pairs( board_list ) do
    if utils.array_element_index( v, comp.cpu:upper() ) then
      comp.board = b
      break
    end
  end
  if comp.board == 'auto' then
    print( sf( "CPU '%s' not found", comp.cpu ) )
    os.exit( -1 )
  end
end

-- Look for the given CPU in the list of platforms
for p, v in pairs( platform_list ) do
  if utils.array_element_index( v.cpus, comp.cpu:upper() ) then
    platform = p
    break
  end
end
if not platform then
  print( "Unable to find platform (this shouldn't happen, check the build script for errors)" )
  os.exit( -1 )
end

-- Check the toolchain
if comp.toolchain ~= 'auto' then
  if utils.array_element_index( platform_list[ platform ].toolchains, comp.toolchain ) == nil then
    print( sf( "Invalid toolchain '%s' for CPU '%s'", comp.toolchain, comp.cpu ) )
    os.exit( -1 )
  end
  toolset = toolchain_list[ comp.toolchain ]
  comp.CC = toolset.compile
  comp.AS = toolset.compile
else
  -- If 'auto' try to match a working toolchain with target
  local usable_chains = platform_list[ platform ].toolchains
  -- Try to execute all compilers, exit when one found
  local chain
  for i = 1, #usable_chains do
    local c = usable_chains[ i ]
    local t = toolchain_list[ c ]
    local res = utils.check_command( t.compile .. " " .. t.version )
    if res == 0 then chain = c break end
  end
  if chain then
    comp.toolchain = chain
    comp.CC = toolchain_list[ chain ].compile
    comp.AS = comp.CC
    toolset = toolchain_list[ chain ]
  else
    print "Unable to find an usable toolchain in your path."
    print( sf( "List of accepted toolchains (for %s): %s", comp.cpu, table.concat( usable_chains ) ) )
    os.exit( -1 )
  end
end    

-- CPU/allocator mapping (if allocator not specified)
if comp.allocator == 'auto' then
  if utils.array_element_index( comp.board:upper(), { 'LPC-H2888', 'ATEVK1100', 'MBED' } ) then
    comp.allocator = 'multiple'
  else
    comp.allocator = 'newlib'
  end
end    

-- Build the compilation command now
local compcmd = ''
if comp.romfs == 'compile' then
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
  compcmd = table.concat( cmdpath, utils.dir_sep )
  print( compcmd )
elseif comp.romfs == 'compress' then
  compcmd = 'lua luasrcdiet.lua --quiet --maximum --opt-comments --opt-whitespace --opt-emptylines --opt-eols --opt-strings --opt-numbers --opt-locals -o %s %s'
end

-- Output file
output = 'elua_' .. comp.target .. '_' .. comp.cpu:lower()
builder:set_output_dir( ".build" .. utils.dir_sep .. comp.board:lower() )

-- User report
print ""
print "*********************************"
print "Compiling eLua ..."
print( "CPU:            ", comp.cpu )
print( "Board:          ", comp.board )
print( "Platform:       ", platform )
print( "Allocator:      ", comp.allocator )
print( "Boot Mode:      ", comp.boot )
print( "Target:         ", comp.target  )
print( "Toolchain:      ", comp.toolchain )
print( "ROMFS mode:     ", comp.romfs )
print "*********************************"
print ""

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

if comp.boot == 'luarpc' then
  addm( "ELUA_BOOT_RPC" )
end

-- Special macro definitions for the SYM target
if platform == 'sim' then
  addm( { "ELUA_SIMULATOR", "ELUA_SIM_" .. cnorm( comp.cpu ) } )
end

-- Lua source files and include path
local lua_files = ([[lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
  lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
  ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c lrotable.c legc.c]]):gsub( "\n", "" )
local lua_full_files = utils.prepend_path( lua_files, "src/lua" )

addi( { 'inc', 'inc/newlib',  'inc/remotefs', 'src/platform', 'src/lua' } )

if comp.target == 'lualong' then
  addm( "LUA_NUMBER_INTEGRAL" )
end

addi( { 'src/modules', 'src/platform/' .. platform } )
addm( "LUA_OPTIMIZE_MEMORY=" .. ( comp.optram and "2" or "0" ) )

-- Application files
local app_files = ([[ src/main.c src/romfs.c src/semifs.c src/xmodem.c src/shell.c src/term.c src/common.c src/common_tmr.c src/buf.c src/elua_adc.c src/dlmalloc.c 
                src/salloc.c src/luarpc_elua_uart.c src/elua_int.c src/linenoise.c src/common_uart.c src/eluarpc.c ]]):gsub( "\n", "" )

-- Newlib related files
local newlib_files = " src/newlib/devman.c src/newlib/stubs.c src/newlib/genstd.c src/newlib/stdtcp.c"

-- UIP files
local uip_files = "uip_arp.c uip.c uiplib.c dhcpc.c psock.c resolv.c"
uip_files = " src/elua_uip.c " .. utils.prepend_path( uip_files, "src/uip" )
addi( "src/uip" )

-- FatFs files
app_files = app_files .. "src/elua_mmc.c src/mmcfs.c src/fatfs/ff.c src/fatfs/ccsbcs.c "
addi( "src/fatfs" )

-- Lua module files
local module_names = "pio.c spi.c tmr.c pd.c uart.c term.c pwm.c lpack.c bit.c net.c cpu.c adc.c can.c luarpc.c bitarray.c elua.c i2c.c"
local module_files = " " .. utils.prepend_path( module_names, 'src/modules' )

-- Remote file system files
local rfs_names = "remotefs.c client.c elua_os_io.c elua_rfs.c"
local rfs_files = " " .. utils.prepend_path( rfs_names, "src/remotefs" )

-- Optimizer flags (speed or size)
addcf( { '-Os','-fomit-frame-pointer' } )

-- Toolset data (filled by each platform in part)
tools = {}
specific_files = ''

-- We get platform-specific data by executing the platform script
dofile( sf( "src/platform/%s/conf.lua", platform ) )

-- Complete file list
local source_files = app_files .. specific_files .. newlib_files .. uip_files .. lua_full_files .. module_files .. rfs_files 

-------------------------------------------------------------------------------
-- Create compiler/linker/assembler command lines and build

-- ROM file system builder
local function make_romfs()
  print "Building ROM file system ..."
  local romdir = "romfs"
  local flist = {}
  for _, sample in pairs( file_list[ comp.board:upper() ] ) do
    table.insert( flist, romfs[ sample ] )
  end
  flist = utils.linearize_array( flist )  
  if not mkfs.mkfs( romdir, "romfiles", flist, comp.romfs, compcmd ) then return -1 end
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

-- Command lines for the tools (compiler, linker, assembler)
compcmd = builder:compile_cmd{ flags = cflags, defines = cdefs, includes = includes, compiler = toolset.compile }
linkcmd = builder:link_cmd{ flags = lflags, libraries = libs, linker = toolset.compile }
ascmd = builder:asm_cmd{ flags = asflags, defines = cdefs, includes = includes, assembler = toolset.asm }
builder:set_compile_cmd( compcmd )
builder:set_link_cmd( linkcmd )
builder:set_asm_cmd( ascmd )
builder:set_exe_extension( ".elf" )

-- Create executable target 
romtarget = builder:target( "inc/romfiles.h", nil, make_romfs )
builder:make_depends( source_files, { romtarget } )
odeps = builder:create_compile_targets( source_files )
exetarget = builder:link_target( output, odeps )
-- This is also the default target
builder:default( builder:add_target( exetarget, 'build eLua executable' ) )

-- Create 'prog' target
progtarget = builder:target( "#phony:prog", { exetarget }, tools[ platform ].progfunc )
builder:add_target( progtarget, "build eLua firmware image", { "prog" } )

-- If the backend needs to do more processing before the build starts, do it now
if tools[ platform ].pre_build then
  tools[ platform ].pre_build()
end

-- Finally build everything
builder:build()

