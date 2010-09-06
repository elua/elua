import os, sys, shutil
target = ARGUMENTS.get( 'target', 'lua' ).lower()
cputype = ARGUMENTS.get( 'cpu', '' ).upper()
allocator = ARGUMENTS.get( 'allocator', '' ).lower()
boardname = ARGUMENTS.get( 'board' , '').upper()
toolchain = ARGUMENTS.get( 'toolchain', '')
optram = int( ARGUMENTS.get( 'optram', '1' ) )
boot = ARGUMENTS.get( 'boot', '').lower()

# Helper: "normalize" a name to make it a suitable C macro name
def cnorm( name ):
  name = name.replace( '-', '' )
  name = name.replace( ' ', '' )
  return name.upper()

# List of toolchains
toolchain_list = {
  'arm-gcc' : { 
    'compile' : 'arm-elf-gcc', 
    'link' : 'arm-elf-ld', 
    'asm' : 'arm-elf-as', 
    'bin' : 'arm-elf-objcopy', 
    'size' : 'arm-elf-size' 
  },
  'arm-eabi-gcc' : {
    'compile' : 'arm-eabi-gcc',
    'link' : 'arm-eabi-ld',
    'asm' : 'arm-eabi-as',
    'bin' : 'arm-eabi-objcopy',
    'size' : 'arm-eabi-size'
  },
  'codesourcery' : { 
    'compile' : 'arm-none-eabi-gcc', 
    'link' : 'arm-none-eabi-ld', 
    'asm' : 'arm-none-eabi-as', 
    'bin' : 'arm-none-eabi-objcopy', 
    'size' : 'arm-none-eabi-size' 
  },
  'avr32-gcc' : { 
    'compile' : 'avr32-gcc', 
    'link' : 'avr32-ld', 
    'asm' : 'avr32-as', 
    'bin' : 'avr32-objcopy', 
    'size' : 'avr32-size' 
  },
  'i686-gcc' : { 
    'compile' : 'i686-elf-gcc', 
    'link' : 'i686-elf-ld', 
    'asm' : 'nasm', 
    'bin' : 'i686-elf-objcopy', 
    'size' : 'i686-elf-size' 
  }
}

# Toolchain Aliases
toolchain_list['devkitarm'] = toolchain_list['arm-eabi-gcc']

# List of platform/CPU/toolchains combinations
# The first toolchain in the toolchains list is the default one
# (the one that will be used if none is specified)
platform_list = {  
  'at91sam7x' : { 'cpus' : [ 'AT91SAM7X256', 'AT91SAM7X512' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] },
  'lm3s' : { 'cpus' : [ 'LM3S8962', 'LM3S6965', 'LM3S6918', 'LM3S9B92' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] },
  'str9' : { 'cpus' : [ 'STR912FAW44' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] },
  'i386' : { 'cpus' : [ 'I386' ], 'toolchains' : [ 'i686-gcc' ] },
  'sim' : { 'cpus' : [ 'LINUX' ], 'toolchains' : [ 'i686-gcc' ] },
  'lpc288x' : { 'cpus' : [ 'LPC2888' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] },
  'str7' : { 'cpus' : [ 'STR711FR2' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] },
  'stm32' : { 'cpus' : [ 'STM32F103ZE', 'STM32F103RE' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] },
  'avr32' : { 'cpus' : [ 'AT32UC3A0512' ], 'toolchains' : [ 'avr32-gcc' ] },
  'lpc24xx' : { 'cpus' : [ 'LPC2468' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] }
}

# List of board/CPU combinations
board_list = { 'SAM7-EX256' : [ 'AT91SAM7X256', 'AT91SAM7X512' ],
               'EK-LM3S8962' : [ 'LM3S8962' ],
               'EK-LM3S6965' : [ 'LM3S6965' ],
               'EK-LM3S9B92' : [ 'LM3S9B92' ],
               'STR9-COMSTICK' : [ 'STR912FAW44' ],
               'STR-E912' : [ 'STR912FAW44' ],
               'PC' : [ 'I386' ],
               'SIM' : [ 'LINUX' ],
               'LPC-H2888' : [ 'LPC2888' ],
               'MOD711' : [ 'STR711FR2' ],
               'STM3210E-EVAL' : [ 'STM32F103ZE' ],
               'ATEVK1100' : [ 'AT32UC3A0512' ],
               'ET-STM32' : [ 'STM32F103RE' ],
               'EAGLE-100' : [ 'LM3S6918' ],
               'ELUA-PUC' : ['LPC2468' ]
            }

# ROMFS file list "groups"
# To include a file in a ROMFS build, include it in a group here (or create one
# if you need) and make sure the group is included on your platform's file_list
# definition (right after this).
romfs = { 'bisect' : [ 'bisect.lua' ],
          'hangman' : [ 'hangman.lua' ],
          'lhttpd' : [ 'index.pht', 'lhttpd.lua', 'test.lua' ],
          'pong' : [ 'pong.lua' ],
          'led' : [ 'led.lua' ],
          'piano' : [ 'piano.lua' ],
          'pwmled' : [ 'pwmled.lua' ],
          'tvbgone' : [ 'tvbgone.lua', 'codes.bin' ],
          'hello' : [ 'hello.lua' ],
          'info' : [ 'info.lua' ],
          'morse' : [ 'morse.lua' ],
          'dualpwm' : [ 'dualpwm.lua' ],
          'adcscope' : [ 'adcscope.lua' ],
          'adcpoll' : [ 'adcpoll.lua' ],
          'life' : [ 'life.lua' ],
          'logo' : ['logo.lua', 'logo.bin' ],
          'spaceship' : [ 'spaceship.lua' ],
          'tetrives' : [ 'tetrives.lua' ],
          'inttest' : [ 'inttest.lua' ]
        }

# List of board/romfs data combinations
file_list = { 'SAM7-EX256' : [ 'bisect', 'hangman' , 'led', 'piano', 'hello', 'info', 'morse' ],
              'EK-LM3S8962' : [ 'bisect', 'hangman', 'lhttpd', 'pong', 'led', 'piano', 'pwmled', 'tvbgone', 'hello', 'info', 'morse', 'adcscope', 'adcpoll', 'logo', 'spaceship', 'tetrives' ],
              'EK-LM3S6965' : [ 'bisect', 'hangman', 'lhttpd', 'pong', 'led', 'piano', 'pwmled', 'tvbgone', 'hello', 'info', 'morse', 'adcscope', 'adcpoll', 'logo', 'spaceship', 'tetrives' ],
              'EK-LM3S9B92' : [ 'bisect', 'hangman', 'lhttpd', 'led', 'pwmled', 'hello', 'info', 'adcscope','adcpoll', 'life' ],
              'STR9-COMSTICK' : [ 'bisect', 'hangman', 'led', 'hello', 'info' ],
              'STR-E912' : [ 'bisect', 'hangman', 'led', 'hello', 'info', 'piano' ],
              'PC' : [ 'bisect', 'hello', 'info', 'life', 'hangman' ],
              'SIM' : [ 'bisect', 'hello', 'info', 'life', 'hangman' ],
              'LPC-H2888' : [ 'bisect', 'hangman', 'led', 'hello', 'info' ],
              'MOD711' : [ 'bisect', 'hangman', 'led', 'hello', 'info', 'dualpwm' ],
              'STM3210E-EVAL' : [ 'bisect', 'hello', 'info' ],
              'ATEVK1100' : [ 'bisect', 'hangman', 'led', 'hello', 'info' ],
              'ET-STM32' : [ 'hello', 'hangman', 'info', 'bisect','adcscope','adcpoll', 'dualpwm', 'pwmled' ],
              'EAGLE-100' : [ 'bisect', 'hangman', 'lhttpd', 'led', 'hello', 'info' ],
              'ELUA-PUC' : [ 'bisect', 'hangman', 'led', 'hello', 'info', 'pwmled', 'inttest' ]
}

# Variants: board = <boardname>
#           cpu = <cpuname>
#           board = <boardname> cpu=<cpuname>
if boardname == '' and cputype == '':
  print "Must specifiy board, cpu, or both"
  sys.exit( -1 )
elif boardname != '' and cputype != '':
  # board = <boardname> cpu=<cpuname>
  # Check if the board, cpu pair is correct
  if not board_list.has_key( boardname ):
    print "Unknown board", boardname
    sys.exit( -1 )
  if not cputype in board_list[ boardname ]:
    print "Invalid CPU %s for board %s" % ( cputype, boardname )
    sys.exit( -1 )
elif boardname != '':
  # board = <boardname>
  # Find CPU
  if not board_list.has_key( boardname ):
    print "Unknown board", boardname
    sys.exit( -1 )
  cputype = board_list[ boardname ][ 0 ]
else:
  # cpu = <cputype>
  # Find board name
  for b, v in board_list.items():
    if cputype in v:
      boardname = b
      break
  else:
    print "CPU %s not found" % cputype
    sys.exit( -1 )

# Look for the given CPU in the list of platforms
platform = None
for p, v in platform_list.items():
  if cputype in v[ 'cpus' ]:
    platform = p
    break
else:
  print "Unknown CPU %s" % cputype
  print "List of accepted CPUs: "
  for p, v in platform_list.items():
    print " ", p, "-->",
    for cpu in v[ 'cpus' ]:
      print cpu,
    print
  sys.exit( -1 )

# Check the toolchain
if toolchain != '':
  if not toolchain in platform_list[ platform ][ 'toolchains' ]:
    print "Invalid toolchain '%s' for CPU '%s'" % ( toolchain, cputype )
    sys.exit( -1 )
else:
  toolchain = platform_list[ platform ][ 'toolchains' ][ 0 ]
toolset = toolchain_list[ toolchain ]

# CPU/allocator mapping (if allocator not specified)
if allocator == '':
  if boardname == 'LPC-H2888' or boardname == 'ATEVK1100':
    allocator = 'multiple'
  else:
    allocator = 'newlib'
elif allocator not in [ 'newlib', 'multiple', 'simple' ]:
  print "Unknown allocator", allocator
  print "Allocator can be either 'newlib', 'multiple' or 'simple'"
  sys.exit( -1 )

# Check boot mode selection
if boot == '':
  boot = 'standard'
elif boot not in ['standard', 'luaremote']:
  print "Unknown boot mode: ", boot
  print "Boot mode can be either 'standard' or 'luaremote'"
  sys.exit( -1 );


# User report
if not GetOption( 'clean' ):
  print
  print "*********************************"
  print "Compiling eLua ..."
  print "CPU:         ", cputype
  print "Board:       ", boardname
  print "Platform:    ", platform
  print "Allocator:   ", allocator
  print "Boot Mode:   ", boot
  print "Target:      ", target == 'lua' and 'fplua' or 'target'
  print "Toolchain:   ", toolchain
  print "*********************************"
  print

output = 'elua_' + target + '_' + cputype.lower()
cdefs = '-DELUA_CPU=%s -DELUA_BOARD=%s -DELUA_PLATFORM=%s -D__BUFSIZ__=128' % ( cputype, boardname, platform.upper() )
# Also make the above into direct defines (to use in conditional C code)
cdefs = cdefs + " -DELUA_CPU_%s -DELUA_BOARD_%s -DELUA_PLATFORM_%s" % ( cnorm( cputype ), cnorm( boardname ), cnorm( platform ) )
if allocator == 'multiple':
  cdefs = cdefs + " -DUSE_MULTIPLE_ALLOCATOR"
elif allocator == 'simple':
  cdefs = cdefs + " -DUSE_SIMPLE_ALLOCATOR"

if boot == 'luaremote':
  cdefs += " -DELUA_BOOT_REMOTE"

# Special macro definitions for the SYM target
if platform == 'sim':
  cdefs = cdefs + " -DELUA_SIMULATOR -DELUA_SIM_%s" % cputype

# Lua source files and include path
lua_files = """lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c lrotable.c"""
if target == 'lualong' or target == 'lua':
  lua_full_files = " " + " ".join( [ "src/lua/%s" % name for name in lua_files.split() ] )
  local_include = ['inc', 'inc/newlib', 'src/lua']
  if target == 'lualong':
    cdefs = cdefs + ' -DLUA_NUMBER_INTEGRAL'
else:
  print "Invalid target", target
  sys.exit( 1 )

local_include += ['src/modules', 'src/platform/%s' % platform]
cdefs = cdefs + " -DLUA_OPTIMIZE_MEMORY=%d" % ( optram != 0 and 2 or 0 )

# Additional libraries
local_libs = ''

# Application files
app_files = " src/main.c src/romfs.c src/xmodem.c src/shell.c src/term.c src/common.c src/buf.c src/elua_adc.c src/dlmalloc.c src/salloc.c src/luarpc_elua_uart.c src/elua_int.c "

# Newlib related files
newlib_files = " src/newlib/devman.c src/newlib/stubs.c src/newlib/genstd.c src/newlib/stdtcp.c"

# UIP files
uip_files = "uip_arp.c uip.c uiplib.c dhcpc.c psock.c resolv.c"
uip_files = " src/elua_uip.c " + " ".join( [ "src/uip/%s" % name for name in uip_files.split() ] )
local_include += ['src/uip']

# Lua module files
module_names = "pio.c spi.c tmr.c pd.c uart.c term.c pwm.c lpack.c bit.c net.c cpu.c adc.c can.c luarpc.c bitarray.c"
module_files = " " + " ".join( [ "src/modules/%s" % name for name in module_names.split() ] )

# Optimizer flags (speed or size)
#opt = "-O3"
opt = "-Os -fomit-frame-pointer"
#opt += " -ffreestanding"


# Toolset data (filled by each platform in part)
tools = {}

# We get platform-specific data by executing the platform script
execfile( "src/platform/%s/conf.py" % platform )

# Complete file list
source_files = app_files + specific_files + newlib_files + uip_files + lua_full_files + module_files

# Make ROM File System first
if not GetOption( 'clean' ):
  print "Building ROM File System..."
  romdir = "romfs"
  flist = []
  for sample in file_list[ boardname ]:
    flist += romfs[ sample ]
# Automatically includes the autorun.lua file in the ROMFS
  if os.path.isfile( os.path.join( romdir, 'autorun.lua' ) ):
    flist += [ 'autorun.lua' ]
# Automatically includes platform specific Lua module 
  if os.path.isfile( os.path.join( romdir, boardname + '.lua' ) ):
    flist += [boardname + '.lua']
  import mkfs
  mkfs.mkfs( romdir, "romfiles", flist )
  print
  if os.path.exists( "inc/romfiles.h" ): 
    os.remove( "inc/romfiles.h" )
  shutil.move( "romfiles.h", "inc/" )
  if os.path.exists( "src/fs.o" ): 
    os.remove( "src/fs.o" )

# Env for building the program
comp = Environment( CCCOM = tools[ platform ][ 'cccom' ],
                    ASCOM = tools[ platform ][ 'ascom' ],
                    LINKCOM = tools[ platform ][ 'linkcom' ],
                    OBJSUFFIX = ".o",
                    PROGSUFFIX = ".elf",
                    CPPPATH = local_include,
                    ENV = os.environ )
# comp.TargetSignatures( 'content' )
# comp.SourceSignatures( 'MD5' )
comp[ 'INCPREFIX' ] = "-I"
Default( comp.Program( target = output, source = Split( source_files ) ) )
Decider( 'MD5' )

# Programming target
prog = Environment( BUILDERS = { 'program' : Builder( action = Action ( tools[ platform ][ 'progfunc' ] ) ) }, ENV = os.environ )
prog.program( "prog", output + ".elf" )
