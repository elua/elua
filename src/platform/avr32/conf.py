# Configuration file for the AVR32 microcontroller

specific_files = "crt0.s trampoline.s platform.c exception.s intc.c pm.c flashc.c pm_conf_clocks.c usart.c gpio.c tc.c sdramc.c"
ldscript = "at32uc3a0512.ld"
# [TODO] the appends assume that the board is an ATEVK1100 (see src/platform/avr32/board.h)
comp.Append(CPPDEFINES = 'FORAVR32')
comp.Append(CPPDEFINES = {'BOARD' : 1})

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-Wl,--gc-sections','-Wl,--allow-multiple-definition','-T',ldscript])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c'])
comp.Append(LIBS = ['c','gcc','m'])

# Target-specific Flags
comp.Prepend(CCFLAGS = ['-mpart=uc3a0512'])
comp.Prepend(ASFLAGS = ['-mpart=uc3a0512'])
comp.Append(LINKFLAGS = ['-Wl,-e,crt0'])

# Toolset data
tools[ 'avr32' ] = {}

# Programming function
def progfunc_avr32( comp[ 'target' ], source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )

#  print "Programming..."
#  os.system( "batchisp3.sh -hardware usb -device at32uc3a0512 -operation erase f memory flash blankcheck loadbuffer %s program verify start reset 0" % ( output + ".hex" ) )

tools[ 'avr32' ][ 'progfunc' ] = progfunc_avr32
