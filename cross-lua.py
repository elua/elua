import os, sys 

output = 'luac.cross'
cdefs = '-DLUA_CROSS_COMPILER'

# Lua source files and include path
lua_files = """lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
   lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
   ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c luac.c print.c lrotable.c"""
lua_full_files = " " + " ".join( [ "src/lua/%s" % name for name in lua_files.split() ] )
local_include = "-Isrc/lua -Iinc/desktop -Iinc"

# Compiler/linker options
cccom = "gcc -O2 %s -Wall %s -c $SOURCE -o $TARGET" % ( local_include, cdefs )
linkcom = "gcc -o $TARGET $SOURCES -lm"

# Env for building the program
comp = Environment( CCCOM = cccom,
                    LINKCOM = linkcom,
                    ENV = os.environ )
if comp['PLATFORM'] == 'win32':
  suffix = ".exe"
else:
  suffix = ".elf"                    
Decider( 'MD5' )                  
Default( comp.Program( output + suffix, Split( lua_full_files ) ) )
