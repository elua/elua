import os, sys, platform

flist = "main.c"
cdefs = ""
if platform.system() == "Windows":
  cdefs = "-DWIN32_BUILD"
  exeprefix = "exe"
else:
  exeprefix = "elf"

output = "mux.%s" % exeprefix

full_files = " " + " ".join( [ "mux/%s" % name for name in flist.split() ] )
if platform.system() == "Windows":
  full_files = full_files + " rfs_server/serial_win32.c"
else:
  full_files = full_files + " rfs_server/serial_posix.c"
full_files = full_files + " rfs_server/log.c"
local_include = "-Imux -Irfs_server -Iinc"

# Compiler/linker options
cccom = "gcc -m32 -O0 -g %s -Wall %s -c $SOURCE -o $TARGET" % ( local_include, cdefs )
linkcom = "gcc -m32 -pthread -o $TARGET $SOURCES"

# Env for building the program
comp = Environment( CCCOM = cccom,
                    LINKCOM = linkcom,
                    ENV = os.environ )
Decider( 'MD5' )                  
Default( comp.Program( output, Split( full_files ) ) )

