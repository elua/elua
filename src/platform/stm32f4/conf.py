# Configuration file for the STM32 microcontroller
import fnmatch
import glob
import os

comp.Append(CPPPATH = ['src/platform/%s/FWLib/library/inc' % platform])

fwlib_files = " ".join(glob.glob("src/platform/%s/FWLib/library/src/*.c" % platform))
#print "FWLib: %s " % fwlib_files 

#specific_files = "core_cm3.c system_stm32f2xx.c startup_stm32f2xx.s platform.c stm32f2xx_it.c platform_int.c enc.c"
specific_files = "system_stm32f4xx.c startup_stm32f4xx.s stm32f4xx_it.c platform.c platform_int.c cpu.c"

#ldscript = "stm32f4xx_flash.ld" 
ldscript = "stm32.ld"
  
# Prepend with path
specific_files = fwlib_files + " " + " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files += " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CPPDEFINES = ["FOR" + cnorm( comp[ 'cpu' ] ),"FOR" + cnorm( comp[ 'board' ] ),'gcc'])
comp.Append(CPPDEFINES = [ 'USE_STDPERIPH_DRIVER', 'STM32F4XX', 'CORTEX_M4'])

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall','-g'])

#'-nostartfiles',
comp.Append(LINKFLAGS = ['-nostartfiles', '-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition','-Wl,-Map=%s.map' % (output)])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

TARGET_FLAGS = ['-mcpu=cortex-m4', '-mthumb']

# Configure General Flags for Target
comp.Prepend(CCFLAGS = [TARGET_FLAGS,'-mlittle-endian'])
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,Reset_Handler','-Wl,-static'])
comp.Prepend(ASFLAGS = TARGET_FLAGS)

# Toolset data
tools[ 'stm32f4' ] = {}

# Programming function
def progfunc_stm32( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output) )
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'stm32f4' ][ 'progfunc' ] = progfunc_stm32

