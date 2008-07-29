# Configuration file for the AT91SAM7X(256/512) backend

# 'samtype' can be '7x256' or '7x512' to select chip version
samtype = ARGUMENTS.get( 'samtype', '7x256' )

specific_files = "board_cstartup.s board_lowlevel.c board_memories.c usart.c pmc.c pio.c platform.c tc.c"
if samtype == '7x256':
  ldscript = "flash256.lds"
  cpudef = "-Dat91sam7x256"
  output = output + "256"
elif samtype == '7x512':
  ldscript = "flash512.lds"
  cpudef = "-Dat91sam7x512"
  output = output + "512"
else:
  print "Invalid AT91SAM7X type %s", samtype
  sys.exit( -1 )  
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'at91sam7x' ] = {}
tools[ 'at91sam7x' ][ 'cccom' ] = "arm-elf-gcc -mcpu=arm7tdmi -mthumb %s %s %s -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( opt, local_include, cpudef, cdefs )
tools[ 'at91sam7x' ][ 'linkcom' ] = "arm-elf-gcc -nostartfiles -nostdlib -mthumb -T %s -Wl,--gc-sections -Wl,-e,entry -Wl,--allow-multiple-definition -o $TARGET $SOURCES %s -lc -lgcc -lm" % ( ldscript, local_libs )
tools[ 'at91sam7x' ][ 'ascom' ] = "arm-elf-gcc -x assembler-with-cpp %s -mcpu=arm7tdmi -mthumb %s %s -D__ASSEMBLY__ -Wall -c $SOURCE -o $TARGET" % ( local_include, cpudef, cdefs )

# Programming function for LPC2888
def progfunc_at91sam7x( target, source, env ):
  outname = output + ".elf"
  os.system( "arm-elf-size %s" % outname )
  print "Generating binary image..."
  os.system( "arm-elf-objcopy -O binary %s %s.bin" % ( outname, output ) )
  
tools[ 'at91sam7x' ][ 'progfunc' ] = progfunc_at91sam7x
