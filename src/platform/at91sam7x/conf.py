# Configuration file for the AT91SAM7X(256/512) backend

cpumode = ARGUMENTS.get( 'cpumode', 'thumb' ).lower()

specific_files = "board_cstartup.s board_lowlevel.c board_memories.c usart.c pmc.c pio.c platform.c tc.c pwmc.c aic.c platform_int.c pit.c"
if comp[ 'cpu' ] == 'AT91SAM7X256':
  ldscript = "flash256.lds"
  comp.Append(CPPDEFINES = 'at91sam7x256')
elif comp[ 'cpu' ] == 'AT91SAM7X512':
  ldscript = "flash512.lds"
  comp.Append(CPPDEFINES = 'at91sam7x512')
else:
  print "Invalid AT91SAM7X CPU %s" % comp[ 'cpu' ]
  Exit( -1 )
  
comp.Append(CPPDEFINES = ['NOASSERT','NOTRACE'])
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files += " src/platform/arm_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

TARGET_FLAGS = ['-mcpu=arm7tdmi']
if cpumode == 'thumb':
  TARGET_FLAGS += ['-mthumb']
  comp.Append(CPPDEFINES = ['CPUMODE_THUMB'])
else:
  comp.Append(CPPDEFINES = ['CPUMODE_ARM'])

# Configure General Flags for Target
comp.Prepend(CCFLAGS = [TARGET_FLAGS])
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,entry'])
comp.Prepend(ASFLAGS = [TARGET_FLAGS,'-D__ASSEMBLY__'])

# Toolset data
tools[ 'at91sam7x' ] = {}

# Programming function for LPC2888
def progfunc_at91sam7x( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'at91sam7x' ][ 'progfunc' ] = progfunc_at91sam7x

