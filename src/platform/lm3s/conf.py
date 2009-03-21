# Configuration file for the LM3S microcontroller

specific_files = "startup_gcc.c platform.c usart.c sysctl.c gpio.c ssi.c timer.c pwm.c ethernet.c systick.c flash.c interrupt.c cpu.c adc.c"

if boardname == 'EK-LM3S6965' or boardname == 'EK-LM3S8962':
  specific_files = specific_files + " rit128x96x4.c disp.c"
  cdefs = cdefs + " -DENABLE_DISP"

# The default for the Eagle 100 board is to start the image at 0x2000,
# so that the built in Ethernet boot loader can be used to upload it
if boardname == 'EAGLE-100':
  linkopts = "-Wl,-Ttext,0x2000"
else:
  linkopts = ""

ldscript = "lm3s.ld"

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

cdefs = cdefs + " -DFOR" + cputype + " -Dgcc"

# Toolset data
tools[ 'lm3s' ] = {}
tools[ 'lm3s' ][ 'cccom' ] = "%s -mcpu=cortex-m3 -mthumb %s $_CPPINCFLAGS -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], opt, cdefs )
tools[ 'lm3s' ][ 'linkcom' ] = "%s -mthumb -mcpu=cortex-m3 -nostartfiles -T %s %s -Wl,--gc-sections -Wl,-e,ResetISR -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lm %s" % ( toolset[ 'compile' ], ldscript, linkopts, local_libs )
tools[ 'lm3s' ][ 'ascom' ] = "%s -x assembler-with-cpp $_CPPINCFLAGS -mcpu=cortex-m3 -mthumb %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ],  cdefs )

# Programming function
def progfunc_lm3s( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )

tools[ 'lm3s' ][ 'progfunc' ] = progfunc_lm3s
