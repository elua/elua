# Configuration file for the AVR32 microcontrollers

specific_files = "crt0.s trampoline.s platform.c exception.s intc.c pm.c flashc.c pm_conf_clocks.c usart.c gpio.c tc.c spi.c platform_int.c adc.c pwm.c i2c.c ethernet.c lcd.c usb-cdc.c"
comp.Append(CPPDEFINES = 'FORAVR32')

# See board.h for possible BOARD values.
if comp[ 'board' ]  == "ATEVK1100":
    specific_files += " sdramc.c"
    comp.Append(CPPDEFINES = {'BOARD' : 1})
elif comp[ 'board' ]  == "ATEVK1101":
    comp.Append(CPPDEFINES = {'BOARD' : 2})
elif comp[ 'board' ]  == "MIZAR32":
    specific_files += " sdramc.c"
    comp.Append(CPPDEFINES = {'BOARD' : 98})
else:
    print "Invalid board for %s platform (%s)" %( platform, comp[ 'board' ] )
    sys.exit( -1 )

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )

# Choose ldscript according to choice of bootloader
if comp[ 'bootloader' ] == "none":
    print "Compiling for FLASH execution"
    ldscript = "src/platform/%s/%s.ld" % ( platform, comp[ 'cpu' ].lower() )
else :
    print "Compiling for SDRAM execution"
    ldscript = "src/platform/%s/%s_%s.ld" % ( platform, comp[ 'cpu' ].lower(), comp[ 'bootloader'] )

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall','-DBOOTLOADER_%s' %comp['bootloader'].upper()])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-Wl,--gc-sections','-Wl,--allow-multiple-definition','-Wl,--relax','-Wl,--direct-data','-T',ldscript])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c'])
comp.Append(LIBS = ['c','gcc','m'])

# Target-specific Flags
comp.Prepend(CCFLAGS = ['-mpart=%s' % (comp[ 'cpu' ][4:].lower())])
comp.Prepend(ASFLAGS = ['-mpart=%s' % (comp[ 'cpu' ][4:].lower())])
comp.Append(LINKFLAGS = ['-Wl,-e,crt0'])

# Toolset data
tools[ 'avr32' ] = {}

# Programming function
def progfunc_avr32( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )

  # print "Programming..."
  # os.system( "batchisp -hardware usb -device %s -operation erase f memory flash blankcheck loadbuffer %s program verify start reset 0" % ( comp[ 'cpu' ].lower(), output + ".hex" ) )

tools[ 'avr32' ][ 'progfunc' ] = progfunc_avr32
