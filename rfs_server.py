import os, sys, platform

sim = ARGUMENTS.get( 'sim', '0' )

flist = ""
cdefs = ""
if sim == '0':
  mainname = "main.c"
else:
  mainname = 'main_sim.c'
if platform.system() == "Windows":
  if sim == '1':
    print "SIM target not supported under Windows"
    os.exit( 1 )
  flist = "main.c server.c client.c os_io_win32.c serial_win32.c log.c"
  cdefs = "-DWIN32_BUILD"
  exeprefix = "exe"
else:
  flist = "%s server.c client.c os_io_posix.c serial_posix.c log.c" % mainname
  exeprefix = "elf"

if sim == '0':
  output = 'rfs_server.%s' % exeprefix
else:
  output = 'rfs_sim_server.%s' % exeprefix
#endif

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

