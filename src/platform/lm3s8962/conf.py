# Configuration file for the LM3S8962 microcontroller

specific_files = "startup_gcc.c platform.c usart.c sysctl.c gpio.c ssi.c timer.c"
ldscript = "lm3s8962.ld"
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

cdefs = cdefs + " -Dgcc"

# Toolset data
tools[ 'lm3s8962' ] = {}
tools[ 'lm3s8962' ][ 'cccom' ] = "arm-elf-gcc -mcpu=cortex-m3 -mthumb %s %s -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( opt, local_include, cdefs )
tools[ 'lm3s8962' ][ 'linkcom' ] = "arm-elf-gcc -nostartfiles -nostdlib -T %s -Wl,--gc-sections -Wl,-e,ResetISR -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( ldscript, local_libs )
tools[ 'lm3s8962' ][ 'ascom' ] = "arm-elf-gcc -x assembler-with-cpp %s -mcpu=cortex-m3 -mthumb %s -Wall -c $SOURCE -o $TARGET" % ( local_include, cdefs )

# Programming function for LPC2888
def progfunc_lm3s8962( target, source, env ):
  outname = output + ".elf"
  os.system( "arm-elf-size %s" % outname )
  print "Generating binary image..."
  os.system( "arm-elf-objcopy -O binary %s %s.bin" % ( outname, output ) )
  
tools[ 'lm3s8962' ][ 'progfunc' ] = progfunc_lm3s8962
