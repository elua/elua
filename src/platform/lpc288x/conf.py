# Configuration file for the LPC288x backend

cpumode = ARGUMENTS.get( 'cpumode', 'arm' ).lower()

specific_files = "lpc28xx.s platform.c target.c uart.c"

# Check CPU
if comp[ 'cpu' ] == 'LPC2888':
  ldscript = "lpc2888.lds"
else:
  print "Invalid LPC288x CPU %s", comp[ 'cpu' ]
  Exit( -1 )  
  
if cpumode == 'thumb':
  print "ERROR: due to a hardware limitation, it is not possible to run Thumb code from the LPC2888 internal flash."  
  print "Compile again, this time with cpumode=arm"
  Exit( -1 )
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files += " src/platform/arm_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CPPDEFINES = ["FOR" + comp[ 'cpu' ],'gcc'])

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

# Special Target Configuration
TARGET_FLAGS = ['-mcpu=arm7tdmi']
if cpumode == 'thumb':
  TARGET_FLAGS += ['-mthumb']
  comp.Append(CPPDEFINES = ['CPUMODE_THUMB'])
else:
  comp.Append(CPPDEFINES = ['CPUMODE_ARM'])
  
comp.Prepend(CCFLAGS = TARGET_FLAGS)
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,HardReset'])
comp.Prepend(ASFLAGS = [TARGET_FLAGS,'-D__ASSEMBLY__'])

# Toolset data
tools[ 'lpc288x' ] = {}

# Programming function for LPC2888
def progfunc_lpc288x( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'lpc288x' ][ 'progfunc' ] = progfunc_lpc288x
