# Configuration file for the LM3S microcontroller

specific_files = "startup_gcc.c platform.c usart.c sysctl.c gpio.c ssi.c timer.c pwm.c ethernet.c systick.c flash.c interrupt.c cpu.c adc.c"

if boardname == 'EK-LM3S6965' or boardname == 'EL-LM3S8962':
  specific_files = specific_files + " rit128x96x4.c disp.c"

ldscript = "lm3s.ld"

# Use default toolchain prefix if one isn't provided
if cprefix == '':
  cprefix= "arm-elf-"
else:
  cprefix = cprefix + '-'

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

cdefs = cdefs + " -DFOR" + cputype + " -Dgcc"

# Toolset data
tools[ 'lm3s' ] = {}
tools[ 'lm3s' ][ 'cccom' ] = cprefix + "gcc -mcpu=cortex-m3 -mthumb  %s %s -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( opt, local_include, cdefs )

if cprefix == 'arm-elf-':
  tools[ 'lm3s' ][ 'linkcom' ] = cprefix + "gcc -nostartfiles -nostdlib -T %s -Wl,--gc-sections -Wl,-e,ResetISR -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( ldscript, local_libs )
else:
  tools[ 'lm3s' ][ 'linkcom' ] = cprefix + "gcc -mthumb -mcpu=cortex-m3 -nostartfiles -T %s -Wl,--gc-sections -Wl,-e,ResetISR -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lm %s" % ( ldscript, local_libs )

tools[ 'lm3s' ][ 'ascom' ] = cprefix + "gcc -x assembler-with-cpp %s -mcpu=cortex-m3 -mthumb %s -Wall -c $SOURCE -o $TARGET" % ( local_include, cdefs )

# Programming function
def progfunc_lm3s( target, source, env ):
  outname = output + ".elf"
  os.system( cprefix + "size %s" % outname )
  print "Generating binary image..."
  os.system( cprefix + "objcopy -O binary %s %s.bin" % ( outname, output ) )

tools[ 'lm3s' ][ 'progfunc' ] = progfunc_lm3s
