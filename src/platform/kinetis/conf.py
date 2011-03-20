# Configuration file for the kinetis microcontroller
specific_files = ""

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files += " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CPPDEFINES = ["FOR" + comp[ 'cpu' ],'gcc'])
comp.Append(CPPDEFINES = ['CORTEX_M4'])

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

TARGET_FLAGS = ['-mcpu=cortex-m4','-mthumb']

# Configure General Flags for Target
comp.Prepend(CCFLAGS = [TARGET_FLAGS,'-mlittle-endian'])
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,ResetISR','-Wl,-static'])
comp.Prepend(ASFLAGS = TARGET_FLAGS)

# Toolset data
tools[ 'kinetis' ] = {}

# Programming function
def progfunc_kinetis( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )

tools[ 'kinetis' ][ 'progfunc' ] = progfunc_kinetis
