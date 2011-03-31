# Configuration file for the LPC17xx backend
import fnmatch
import glob
import os


comp.Append(CPPPATH = ['src/platform/%s/drivers/inc' % platform])

fwlib_files = " ".join(glob.glob("src/platform/%s/drivers/src/*.c" % platform))

specific_files = "startup_LPC17xx.c system_LPC17xx.c core_cm3.c platform.c mbed_pio.c"

# Check CPU
if comp[ 'cpu' ] == 'LPC1768':
  ldscript = "LPC17xx.ld"
else:
  print "Invalid LPC17xx CPU %s", comp[ 'cpu' ]
  Exit( -1 )  
  
# Prepend with path
specific_files = fwlib_files + " " + " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files += " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CPPDEFINES = ["FOR" + comp[ 'cpu' ],'gcc'])
comp.Append(CPPDEFINES = ['CORTEX_M3'])

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

TARGET_FLAGS = ['-mcpu=cortex-m3','-mthumb']

# Configure General Flags for Target
comp.Prepend(CCFLAGS = [TARGET_FLAGS,'-mlittle-endian'])
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,Reset_Handler','-Wl,-static'])
comp.Prepend(ASFLAGS = TARGET_FLAGS)


# Toolset data
tools[ 'lpc17xx' ] = {}

# Programming function for LPC17xx
def progfunc_lpc17xx( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'lpc17xx' ][ 'progfunc' ] = progfunc_lpc17xx
