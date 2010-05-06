# Configuration file for the AVR32 microcontroller

specific_files = "crt0.s trampoline.s platform.c exception.s intc.c pm.c flashc.c pm_conf_clocks.c usart.c gpio.c tc.c sdramc.c"
ldscript = "at32uc3a0512.ld"
# [TODO] the next line assumes that the board is an ATEVK1100 (see src/platform/avr32/board.h)
cdefs = cdefs + " -DFORAVR32 -DBOARD=1"

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'avr32' ] = {}
tools[ 'avr32' ][ 'cccom' ] = "%s -mpart=uc3a0512 %s $_CPPINCFLAGS -ffunction-sections -fdata-sections %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], opt, cdefs )
tools[ 'avr32' ][ 'linkcom' ] = "%s -nostartfiles -nostdlib -T %s -Wl,--gc-sections -Wl,-e,crt0 -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( toolset[ 'compile' ], ldscript, local_libs )
tools[ 'avr32' ][ 'ascom' ] = "%s -x assembler-with-cpp $_CPPINCFLAGS -mpart=uc3a0512 %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], cdefs )

# Programming function
def progfunc_avr32( comp[ 'target' ], source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )

#  print "Programming..."
#  os.system( "batchisp3.sh -hardware usb -device at32uc3a0512 -operation erase f memory flash blankcheck loadbuffer %s program verify start reset 0" % ( output + ".hex" ) )

tools[ 'avr32' ][ 'progfunc' ] = progfunc_avr32
