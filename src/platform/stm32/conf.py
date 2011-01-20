# Configuration file for the STM32 microcontroller
import fnmatch
import glob
import os

comp.Append(CPPPATH = ['src/platform/%s/FWLib/library/inc' % platform])

fwlib_files = " ".join(glob.glob("src/platform/%s/FWLib/library/src/*.c" % platform))
#print "FWLib: %s " % fwlib_files 

specific_files = "core_cm3.c system_stm32f10x.c startup_stm32f10x_hd.s platform.c stm32f10x_it.c lcd.c lua_lcd.c platform_int.c enc.c"

ldscript = "stm32.ld"
  
# Prepend with path
specific_files = fwlib_files + " " + " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files += " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CPPDEFINES = ["FOR" + cnorm( comp[ 'cpu' ] ),"FOR" + cnorm( comp[ 'board' ] ),'gcc'])
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
tools[ 'stm32' ] = {}

# Programming function
def progfunc_stm32( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'stm32' ][ 'progfunc' ] = progfunc_stm32

