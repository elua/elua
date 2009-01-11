# Configuration file for the AVR32 microcontroller

specific_files = "crt0.s trampoline.s platform.c exception.s intc.c pm.c flashc.c pm_conf_clocks.c usart.c gpio.c tc.c"
ldscript = "at32uc3a0512.ld"
cdefs = cdefs + " -DFORAVR32"

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'avr32' ] = {}
tools[ 'avr32' ][ 'cccom' ] = "avr32-gcc -mpart=uc3a0512 %s %s -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( opt, local_include, cdefs )
tools[ 'avr32' ][ 'linkcom' ] = "avr32-gcc -nostartfiles -nostdlib -T %s -Wl,--gc-sections -Wl,-e,crt0 -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( ldscript, local_libs )
tools[ 'avr32' ][ 'ascom' ] = "avr32-gcc -x assembler-with-cpp %s -mpart=uc3a0512 %s -Wall -c $SOURCE -o $TARGET" % ( local_include, cdefs )

# Programming function
def progfunc_avr32( target, source, env ):
  outname = output + ".elf"
  os.system( "avr32-size %s" % outname )
  print "Generating binary image..."
  os.system( "avr32-objcopy -O ihex %s %s.hex" % ( outname, output ) )
#  print "Programming..."
#  os.system( "batchisp3.sh -hardware usb -device at32uc3a0512 -operation erase f memory flash blankcheck loadbuffer %s program verify start reset 0" % ( output + ".hex" ) )

tools[ 'avr32' ][ 'progfunc' ] = progfunc_avr32
