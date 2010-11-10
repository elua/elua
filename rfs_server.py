import os, sys, platform

sim = ARGUMENTS.get( 'sim', '0' )

flist = ""
cdefs = "-DRFS_SERIAL_TRANSPORT -DRFS_STANDALONE_MODE"
socklib = ''
if sim == '0':
  mainname = "main.c"
else:
  mainname = 'main_sim.c'
if platform.system() == "Windows":
  if sim == '1':
    print "SIM target not supported under Windows"
    os.exit( 1 )
  flist = "main.c server.c os_io_win32.c log.c"
  cdefs = cdefs + " -DWIN32_BUILD"
  exeprefix = "exe"
  socklib = 'wsock32'
else:
  flist = "%s server.c os_io_posix.c log.c" % mainname
  exeprefix = "elf"
  socklib = 'socket'

if sim == '0':
  output = 'rfs_server.%s' % exeprefix
else:
  output = 'rfs_sim_server.%s' % exeprefix
#endif

full_files = " " + " ".join( [ "rfs_server/%s" % name for name in flist.split() ] )
full_files = full_files + " src/remotefs/remotefs.c src/eluarpc.c"
if platform.system() == "Windows":
  full_files = full_files + " rfs_server/serial_win32.c"
else:
  full_files = full_files + " rfs_server/serial_posix.c"
local_include = "-Irfs_server -Iinc/remotefs -Iinc"

# Compiler/linker options
cccom = "gcc -m32 -O0 -g %s -Wall %s -c $SOURCE -o $TARGET" % ( local_include, cdefs )
linkcom = "gcc -m32 -o $TARGET $SOURCES -l%s" % socklib

# Env for building the program
comp = Environment( CCCOM = cccom,
                    LINKCOM = linkcom,
                    ENV = os.environ )
Decider( 'MD5' )                  
Default( comp.Program( output, Split( full_files ) ) )

