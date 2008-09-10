import os, sys 
target = ARGUMENTS.get( 'target', 'lua' ).lower() 
cputype = ARGUMENTS.get( 'cpu', '' ).upper()
allocator = ARGUMENTS.get( 'allocator', '' ).lower()
boardname = ARGUMENTS.get( 'board' , '').upper()

# List of platform/CPU combinations
cpu_list = { 'at91sam7x' : [ 'AT91SAM7X256', 'AT91SAM7X512' ], 
              'lm3s' : [ 'LM3S8962', 'LM3S6965' ], 
              'str9' : [ 'STR912FW44' ],
              'i386' : [ 'I386' ],
              'lpc288x' : [ 'LPC2888' ]
            }

# List of board/CPU combinations
board_list = { 'SAM7-EX256' : [ 'AT91SAM7X256', 'AT91SAM7X512' ],        
               'EK-LM3S8962' : [ 'LM3S8962' ],
               'EK-LM3S6965' : [ 'LM3S6965' ],
               'STR9-COMSTICK' : [ 'STR912FW44' ],
               'PC' : [ 'I386' ],
               'LPC-H2888' : [ 'LPC2888' ]
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
  if cputype == 'LPC2888':
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
  print "*********************************"
  print    

output = 'elua_' + target + '_' + cputype.lower()
cdefs = '-DELUA_CPU=%s -DELUA_BOARD=%s -DELUA_PLATFORM=%s' % ( cputype, boardname, platform.upper() )
if allocator == 'multiple':
  cdefs = cdefs + " -DUSE_MULTIPLE_ALLOCATOR"

# Lua source files and include path
lua_files = """lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c"""
if target == 'lualong' or target == 'lua':
  lua_full_files = " " + " ".join( [ "src/lualong/%s" % name for name in lua_files.split() ] )
  local_include = "-Iinc -Iinc/newlib -Isrc/lualong"  
  if target == 'lualong':
    cdefs = cdefs + ' -DLUA_NUMBER_INTEGRAL'
else:
  print "Invalid target", target
  sys.exit( 1 )
local_include = local_include + " -Isrc/modules -Isrc/platform/%s" % platform

# Additional libraries
local_libs = ''
  
# Application files
app_files = " src/romfs.c src/main.c src/xmodem.c src/shell.c src/term.c src/dlmalloc.c"
  
# Newlib related files  
newlib_files = " src/newlib/devman.c src/newlib/stubs.c src/newlib/genstd.c"

# Lua module files
module_files = """ src/modules/pio.c src/modules/spi.c src/modules/tmr.c src/modules/pd.c src/modules/uart.c
                   src/modules/term.c src/modules/pwm.c src/modules/lpack.c src/modules/bit.c"""
  
# Optimizer flags (speed or size)
#opt = "-O3"
opt = "-Os -fomit-frame-pointer"

# Toolset data (filled by each platform in part)
tools = {}

# We get platform-specific data by executing the platform script
execfile( "src/platform/%s/conf.py" % platform )

# Complete file list
source_files = specific_files + newlib_files + app_files + lua_full_files + module_files
  
# Make filesystem first
if not GetOption( 'clean' ):
  print "Building filesystem..."
  import mkfs
  mkfs.mkfs( "romfs", "romfiles" )
  os.system( "mv -f romfiles.h inc/" )
  os.system( "rm -f src/fs.o" )
  
# Env for building the program
comp = Environment( CCCOM = tools[ platform ][ 'cccom' ], 
                    ASCOM = tools[ platform ][ 'ascom' ],
                    LINKCOM = tools[ platform ][ 'linkcom' ],
                    OBJSUFFIX = ".o", 
                    PROGSUFFIX = ".elf",
                    ENV = os.environ )
comp.TargetSignatures( 'content' )
comp.SourceSignatures( 'MD5' )
Default( comp.Program( output, Split( source_files ) ) )

# Programming target
prog = Environment( BUILDERS = { 'program' : Builder( action = Action ( tools[ platform ][ 'progfunc' ] ) ) }, ENV = os.environ )
prog.program( "prog", output + ".elf" )
