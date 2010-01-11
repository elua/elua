import os, sys, platform

flist = ""
cdefs = ""
if platform.system() == "Windows":
  flist = "main.c server.c client.c os_io_win32.c serial_win32.c log.c"
  cdefs = "-DWIN32_BUILD"
  exeprefix = "exe"
else:
  flist = "main.c server.c client.c os_io_posix.c serial_posix.c log.c"
  exeprefix = "elf"

output = 'rfs_server.%s' % exeprefix

full_files = " " + " ".join( [ "rfs_server/%s" % name for name in flist.split() ] )
full_files = full_files + " src/remotefs/remotefs.c"
local_include = "-Irfs_server -Iinc/remotefs"

# Compiler/linker options
cccom = "gcc -O0 -g %s -Wall %s -c $SOURCE -o $TARGET" % ( local_include, cdefs )
linkcom = "gcc -o $TARGET $SOURCES"

# Env for building the program
comp = Environment( CCCOM = cccom,
                    LINKCOM = linkcom,
                    ENV = os.environ )
Decider( 'MD5' )                  
Default( comp.Program( output, Split( full_files ) ) )

