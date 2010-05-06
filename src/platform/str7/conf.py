# Configuration file for the STR7 backend

cpumode = ARGUMENTS.get( 'cpumode', 'thumb' ).lower()

specific_files = "platform.c crt0.s 71x_rccu.c 71x_uart.c 71x_apb.c 71x_gpio.c 71x_tim.c"
if comp[ 'cpu' ] == 'STR711FR2':
  ldscript = "str711fr2.lds"
else:
  print "Invalid STR7 CPU %s" % comp[ 'cpu' ]
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
tools[ 'str7' ] = {}
tools[ 'str7' ][ 'cccom' ] = "%s -mcpu=arm7tdmi %s %s $_CPPINCFLAGS -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], modeflag, opt, cdefs )
tools[ 'str7' ][ 'linkcom' ] = "%s -nostartfiles -nostdlib %s -T %s -Wl,--gc-sections -Wl,-e,entry -Wl,--allow-multiple-definition -o $TARGET $SOURCES %s -lc -lgcc -lm" % ( toolset[ 'compile' ], modeflag, ldscript, local_libs )
tools[ 'str7' ][ 'ascom' ] = "%s -x assembler-with-cpp $_CPPINCFLAGS -mcpu=arm7tdmi %s %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], modeflag, cdefs )

# Programming function for LPC2888
def progfunc_str7( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'str7' ][ 'progfunc' ] = progfunc_str7
