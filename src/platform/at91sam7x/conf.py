# Configuration file for the AT91SAM7X(256/512) backend

cpumode = ARGUMENTS.get( 'cpumode', 'thumb' ).lower()

specific_files = "board_cstartup.s board_lowlevel.c board_memories.c usart.c pmc.c pio.c platform.c tc.c pwmc.c aic.c"
if cputype == 'AT91SAM7X256':
  ldscript = "flash256.lds"
  cdefs = cdefs + " -Dat91sam7x256"
elif cputype == 'AT91SAM7X512':
  ldscript = "flash512.lds"
  cdefs = cdefs + " -Dat91sam7x512"
else:
  print "Invalid AT91SAM7X CPU %s" % cputype
  sys.exit( -1 )  
  
# Check CPU mode
if cpumode == 'arm':
  modeflag = ''
elif cpumode == 'thumb':
  modeflag = '-mthumb'
else:
  print "Invalid CPU mode %s", cpumode
  sys.exit( -1 )
  
cdefs = cdefs + ' -DNOASSERT -DNOTRACE'
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'at91sam7x' ] = {}
tools[ 'at91sam7x' ][ 'cccom' ] = "%s -mcpu=arm7tdmi %s %s %s -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], modeflag, opt, local_include, cdefs )
tools[ 'at91sam7x' ][ 'linkcom' ] = "%s -nostartfiles -nostdlib %s -T %s -Wl,--gc-sections -Wl,-e,entry -Wl,--allow-multiple-definition -o $TARGET $SOURCES %s -lc -lgcc -lm" % ( toolset[ 'compile' ], modeflag, ldscript, local_libs )
tools[ 'at91sam7x' ][ 'ascom' ] = "%s -x assembler-with-cpp %s -mcpu=arm7tdmi %s %s -D__ASSEMBLY__ -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], local_include, modeflag, cdefs )

# Programming function for LPC2888
def progfunc_at91sam7x( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'at91sam7x' ][ 'progfunc' ] = progfunc_at91sam7x

