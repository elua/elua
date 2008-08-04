import os, sys 
target = ARGUMENTS.get( 'target', 'lua' ).lower() 
cputype = ARGUMENTS.get( 'cpu', 'at91sam7x256' ).lower()

# List of platform/CPU combinations
cpu_list = { 'at91sam7x' : [ 'at91sam7x256', 'at91sam7x512' ], 
              'lm3s' : [ 'lm3s8962' ], 
              'str9' : [ 'str912fw44' ],
              'i386' : [ 'i386' ]
            }
            
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
    
output = 'elua_' + target + '_' + cputype 
cdefs = '-D%s' % cputype

# Lua source files and include path
lua_files = """lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c"""
if target == 'lualong':
  lua_full_files = " " + " ".join( [ "src/lualong/%s" % name for name in lua_files.split() ] )
  local_include = "-Iinc -Iinc/newlib -Isrc/lualong"  
  cdefs = cdefs + ' -DLUA_INTONLY'
elif target == 'lua':
  lua_full_files = " " + " ".join( [ "src/lua/%s" % name for name in lua_files.split() ] )   
  local_include = "-Iinc -Iinc/newlib -Isrc/lua"   
else:
  print "Invalid target", target
  sys.exit( 1 )
local_include = local_include + " -Isrc/modules -Isrc/platform/%s" % platform

# Additional libraries
local_libs = ''
  
# Application files
app_files = " src/romfs.c src/main.c src/xmodem.c src/shell.c"
  
# Newlib related files  
newlib_files = " src/newlib/devman.c src/newlib/stubs.c src/newlib/genstd.c"

# Lua module files
module_files = " src/modules/pio.c src/modules/spi.c src/modules/tmr.c src/modules/pd.c src/modules/uart.c"
  
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
  mkfs.mkfs( "files", "luatest" )
  os.system( "mv -f luatest.h inc/" )
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
