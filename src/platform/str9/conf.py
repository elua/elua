# Configuration file for the STR9 backend

cpumode = ARGUMENTS.get( 'cpumode', 'arm' ).lower()

specific_files = "startup912.s startup_generic.s platform.c 91x_scu.c 91x_fmi.c 91x_gpio.c 91x_uart.c 91x_tim.c 91x_vic.c interrupt.c"

# Check CPU
if cputype == 'STR912FAW44':
  ldscript = "str912fw44.lds"
else:
  print "Invalid STR9 CPU %s" % cputype
  sys.exit( -1 )  
  
# Check CPU mode
if cpumode == 'arm':
  modeflag = ''
elif cpumode == 'thumb':
  modeflag = '-mthumb'
else:
  print "Invalid CPU mode %s", cpumode
  sys.exit( -1 )
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'str9' ] = {}
tools[ 'str9' ][ 'cccom' ] = "%s -mcpu=arm966e-s -mfpu=fpa %s $_CPPINCFLAGS %s -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile'], opt, modeflag, cdefs )
tools[ 'str9' ][ 'linkcom' ] = "%s -mcpu=arm966e-s -mfpu=fpa -nostartfiles -nostdlib %s -T %s -Wl,--gc-sections -Wl,-e,_startup -Wl,--allow-multiple-definition -o $TARGET $SOURCES %s -lc -lgcc -lm" % ( toolset[ 'compile' ], modeflag, ldscript, local_libs )
tools[ 'str9' ][ 'ascom' ] = "%s -x assembler-with-cpp $_CPPINCFLAGS -mcpu=arm966e-s -mfpu=fpa %s %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], modeflag, cdefs )

# Programming function for LPC2888
def progfunc_str9( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'str9' ][ 'progfunc' ] = progfunc_str9
