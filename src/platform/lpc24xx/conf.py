# Configuration file for the LPC24xx backend

cpumode = ARGUMENTS.get( 'cpumode', 'arm' ).lower()

specific_files = "startup.s irq.c target.c platform.c"
if cputype == 'LPC2468':
  ldscript = "lpc2468.lds"
else:
  print "Invalid CPU %s" % cputype
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
tools[ 'lpc24xx' ] = {}
tools[ 'lpc24xx' ][ 'cccom' ] = "%s -mcpu=arm7tdmi %s %s $_CPPINCFLAGS -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], modeflag, opt, cdefs )
tools[ 'lpc24xx' ][ 'linkcom' ] = "%s -nostartfiles -nostdlib %s -T %s -Wl,--gc-sections -Wl,-e,entry -Wl,--allow-multiple-definition -o $TARGET $SOURCES %s -lc -lgcc -lm" % ( toolset[ 'compile' ], modeflag, ldscript, local_libs )
tools[ 'lpc24xx' ][ 'ascom' ] = "%s -x assembler-with-cpp $_CPPINCFLAGS -mcpu=arm7tdmi %s %s -D__ASSEMBLY__ -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], modeflag, cdefs )

# Programming function for LPC24xx
def progfunc_lpx24xx( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'lpc24xx' ][ 'progfunc' ] = progfunc_lpx24xx

