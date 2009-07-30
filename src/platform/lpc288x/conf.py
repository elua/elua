# Configuration file for the LPC288x backend

cpumode = ARGUMENTS.get( 'cpumode', 'arm' ).lower()

specific_files = "lpc28xx.s platform.c target.c uart.c"

# Check CPU
if cputype == 'LPC2888':
  ldscript = "lpc2888.lds"
else:
  print "Invalid LPC288x CPU %s", cputype
  sys.exit( -1 )  
  
# Check CPU mode
if cpumode == 'arm':
  modeflag = ''
elif cpumode == 'thumb':
  modeflag = '-mthumb'
else:
  print "Invalid CPU mode %s", cpumode
  sys.exit( -1 )
  
if cpumode == 'thumb':
  print "ERROR: due to a hardware limitation, it is not possible to run Thumb code from the LPC2888 internal flash."  
  print "Compile again, this time with cpumode=arm"
  sys.exit( -1 )
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'lpc288x' ] = {}
tools[ 'lpc288x' ][ 'cccom' ] = "%s -mcpu=arm7tdmi %s $_CPPINCFLAGS %s -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], opt, modeflag, cdefs )
tools[ 'lpc288x' ][ 'linkcom' ] = "%s -mcpu=arm7tdmi -nostartfiles -nostdlib %s -T %s -Wl,--gc-sections -Wl,-e,HardReset -Wl,--allow-multiple-definition -o $TARGET $SOURCES %s -lc -lgcc -lm" % ( toolset[ 'compile' ], modeflag, ldscript, local_libs )
tools[ 'lpc288x' ][ 'ascom' ] = "%s -x assembler-with-cpp $_CPPINCFLAGS -mcpu=arm7tdmi %s %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], modeflag, cdefs )

# Programming function for LPC2888
def progfunc_lpc288x( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'lpc288x' ][ 'progfunc' ] = progfunc_lpc288x
