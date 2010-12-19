import os, sys, platform

sim = ARGUMENTS.get( 'sim', '0' )

flist = ""
cdefs = "-DRFS_STANDALONE_MODE"
socklib = ''
if sim == '0':
  mainname = "main.c"
else:
  mainname = 'main_sim.c'
if platform.system() == "Windows":
  if sim == '1':
    print "SIM target not supported under Windows"
    os.exit( 1 )
  flist = "main.c server.c os_io_win32.c log.c net_win32.c serial_win32.c deskutils.c"
  cdefs = cdefs + " -DWIN32_BUILD"
  exeprefix = ".exe"
  socklib = '-lws2_32'
else:
  flist = "%s server.c os_io_posix.c log.c net_posix.c serial_posix.c deskutils.c" % mainname
  exeprefix = ""

if sim == '0':
  output = 'rfs_server%s' % exeprefix
else:
  output = 'rfs_sim_server%s' % exeprefix
#endif

full_files = " " + " ".join( [ "rfs_server_src/%s" % name for name in flist.split() ] )
full_files = full_files + " src/remotefs/remotefs.c src/eluarpc.c"
local_include = "-Irfs_server_src -Iinc/remotefs -Iinc"

# Compiler/linker options
cccom = "gcc -m32 -O0 -g %s -Wall %s -c $SOURCE -o $TARGET" % ( local_include, cdefs )
linkcom = "gcc -m32 -o $TARGET $SOURCES %s" % socklib

# Env for building the program
comp = Environment( CCCOM = cccom,
                    LINKCOM = linkcom,
                    ENV = os.environ )
Decider( 'MD5' )                  
Default( comp.Program( output, Split( full_files ) ) )

