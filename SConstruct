import os, sys
target = ARGUMENTS.get( 'target', 'lua' ).lower()
cputype = ARGUMENTS.get( 'cpu', '' ).upper()
allocator = ARGUMENTS.get( 'allocator', '' ).lower()
boardname = ARGUMENTS.get( 'board' , '').upper()
optram = int( ARGUMENTS.get( 'optram', '1' ) )

# ROMFS file list
romfs = { 'bisect' : [ 'bisect.lua' ],
          'hangman' : [ 'hangman.lua' ],
          'lhttpd' : [ 'index.pht', 'lhttpd.lua', 'test.lua' ],
          'pong' : [ 'pong.lua', 'LM3S.lua', 'logo.rit', 'img.rit'],
          'led' : [ 'led.lua' ],
          'piano' : [ 'piano.lua' ],
          'pwmled' : [ 'pwmled.lua' ],
          'tvbgone' : [ 'tvbgone.lua', 'codes.bin' ],
          'hello' : [ 'hello.lua' ],
          'info' : [ 'info.lua' ],
          'morse' : [ 'morse.lua' ],
          'dualpwm' : [ 'dualpwm.lua' ],
          'adcscope' : [ 'adcscope.lua' ],
          'life' : [ 'life.lua' ]
        }

# List of platform/CPU combinations
cpu_list = { 'at91sam7x' : [ 'AT91SAM7X256', 'AT91SAM7X512' ],
              'lm3s' : [ 'LM3S8962', 'LM3S6965' ],
              'str9' : [ 'STR912FW44' ],
              'i386' : [ 'I386' ],
              'lpc288x' : [ 'LPC2888' ],
              'str7' : [ 'STR711FR2' ],
              'stm32' : [ 'STM32F103ZE' ],
              'avr32' : [ 'AT32UC3A0512' ]
            }

# List of board/CPU combinations
board_list = { 'SAM7-EX256' : [ 'AT91SAM7X256', 'AT91SAM7X512' ],
               'EK-LM3S8962' : [ 'LM3S8962' ],
               'EK-LM3S6965' : [ 'LM3S6965' ],
               'STR9-COMSTICK' : [ 'STR912FW44' ],
               'PC' : [ 'I386' ],
               'LPC-H2888' : [ 'LPC2888' ],
               'MOD711' : [ 'STR711FR2' ],
               'STM3210E-EVAL' : [ 'STM32F103ZE' ],
               'ATEVK1100' : [ 'AT32UC3A0512' ]
            }

# List of board/romfs data combinations
file_list = { 'SAM7-EX256' : [ 'bisect', 'hangman' , 'led', 'piano', 'hello', 'info', 'morse' ],
              'EK-LM3S8962' : [ 'bisect', 'hangman', 'lhttpd', 'pong', 'led', 'piano', 'pwmled', 'tvbgone', 'hello', 'info', 'morse', 'adcscope' ],
              'EK-LM3S6965' : [ 'bisect', 'hangman', 'lhttpd', 'pong', 'led', 'piano', 'pwmled', 'tvbgone', 'hello', 'info', 'morse', 'adcscope' ],
              'STR9-COMSTICK' : [ 'bisect', 'hangman', 'led', 'hello', 'info' ],
              'PC' : [ 'bisect', 'hello', 'info', 'life' ],
              'LPC-H2888' : [ 'bisect', 'hangman', 'led', 'hello', 'info' ],
              'MOD711' : [ 'bisect', 'hangman', 'led', 'hello', 'info', 'dualpwm' ],
              'STM3210E-EVAL' : [ 'bisect', 'hello', 'info' ],
              'ATEVK1100' : [ 'bisect', 'hangman', 'led', 'hello', 'info' ]
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

platform = None
# Look for the given CPU in the list of platforms
for p, v in cpu_list.items():
  if cputype in v:
    platform = p
    break
else:
  print "Unknown CPU %s" % cputype
  print "List of accepted CPUs: "
  for p, v in cpu_list.items():
    print " ", p, "-->",
    for cpu in v:
      print cpu,
    print
  sys.exit( -1 )

# CPU/allocator mapping (if allocator not specified)
if allocator == '':
  if boardname == 'LPC-H2888' or boardname == 'ATEVK1100':
    allocator = 'multiple'
  else:
    allocator = 'newlib'
elif allocator not in [ 'newlib', 'multiple' ]:
  print "Unknown allocator", allocator
  print "Allocator can be either 'newlib' or 'multiple'"
  sys.exit( -1 )


# User report
if not GetOption( 'clean' ):
  print
  print "*********************************"
  print "Compiling eLua ..."
  print "CPU:         ", cputype
  print "Board:       ", boardname
  print "Platform:    ", platform
  print "Allocator:   ", allocator
  print "Target:      ", target
  print "*********************************"
  print

output = 'elua_' + target + '_' + cputype.lower()
cdefs = '-DELUA_CPU=%s -DELUA_BOARD=%s -DELUA_PLATFORM=%s -D__BUFSIZ__=128' % ( cputype, boardname, platform.upper() )
if allocator == 'multiple':
  cdefs = cdefs + " -DUSE_MULTIPLE_ALLOCATOR"

# Lua source files and include path
lua_files = """lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c lrotable.c"""
if target == 'lualong' or target == 'lua':
  lua_full_files = " " + " ".join( [ "src/lua/%s" % name for name in lua_files.split() ] )
  local_include = "-Iinc -Iinc/newlib -Isrc/lua"
  if target == 'lualong':
    cdefs = cdefs + ' -DLUA_NUMBER_INTEGRAL'
else:
  print "Invalid target", target
  sys.exit( 1 )
local_include = local_include + " -Isrc/modules -Isrc/platform/%s" % platform
cdefs = cdefs + " -DLUA_OPTIMIZE_MEMORY=%d" % ( optram != 0 and 2 or 0 )

# Additional libraries
local_libs = ''

# Application files
app_files = " src/main.c src/romfs.c src/xmodem.c src/shell.c src/term.c src/common.c src/buf.c src/dlmalloc.c "

# Newlib related files
newlib_files = " src/newlib/devman.c src/newlib/stubs.c src/newlib/genstd.c src/newlib/stdtcp.c"

# UIP files
uip_files = "uip_arp.c uip.c uiplib.c dhcpc.c psock.c resolv.c"
uip_files = " src/elua_uip.c " + " ".join( [ "src/uip/%s" % name for name in uip_files.split() ] )
local_include = local_include + " -Isrc/uip"

# Lua module files
module_names = "pio.c spi.c tmr.c pd.c uart.c term.c pwm.c lpack.c bit.c net.c cpu.c adc.c"
module_files = " " + " ".join( [ "src/modules/%s" % name for name in module_names.split() ] )

# Optimizer flags (speed or size)
#opt = "-O3"
opt = "-Os -fomit-frame-pointer"

# Toolset data (filled by each platform in part)
tools = {}

# We get platform-specific data by executing the platform script
execfile( "src/platform/%s/conf.py" % platform )

# Complete file list
source_files = app_files + specific_files + newlib_files + uip_files + lua_full_files + module_files

# Make filesystem first
if not GetOption( 'clean' ):
  print "Building filesystem..."
  flist = []
  for sample in file_list[ boardname ]:
    flist = flist + romfs[ sample ]
  import mkfs
  mkfs.mkfs( "romfs", "romfiles", flist )
  print
  os.system( "mv -f romfiles.h inc/" )
  os.system( "rm -f src/fs.o" )

# Env for building the program
comp = Environment( CCCOM = tools[ platform ][ 'cccom' ],
                    ASCOM = tools[ platform ][ 'ascom' ],
                    LINKCOM = tools[ platform ][ 'linkcom' ],
                    OBJSUFFIX = ".o",
                    PROGSUFFIX = ".elf",
                    ENV = os.environ )
# comp.TargetSignatures( 'content' )
# comp.SourceSignatures( 'MD5' )
Default( comp.Program( output, Split( source_files ) ) )
Decider( 'MD5' )

# Programming target
prog = Environment( BUILDERS = { 'program' : Builder( action = Action ( tools[ platform ][ 'progfunc' ] ) ) }, ENV = os.environ )
prog.program( "prog", output + ".elf" )
