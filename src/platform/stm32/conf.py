# Configuration file for the STM32 microcontroller
import fnmatch
import glob
import os

local_include +=  ['src/platform/%s/FWLib/library/inc' % platform]

fwlib_files = " ".join(glob.glob("src/platform/%s/FWLib/library/src/*.c" % platform))
#print "FWLib: %s " % fwlib_files 

specific_files = "core_cm3.c systick.c system_stm32f10x.c startup_stm32f10x_hd.s platform.c stm32f10x_it.c lcd.c lua_lcd.c"

ldscript = "stm32.ld"
  
# Prepend with path
specific_files = fwlib_files + " " + " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

if boardname == 'STM3210E-EVAL':
  cdefs = cdefs + " -DFORSTM3210E_EVAL"

cdefs = cdefs + " -Dgcc"

# Toolset data
tools[ 'stm32' ] = {}
tools[ 'stm32' ][ 'cccom' ] = "%s -mcpu=cortex-m3 -mthumb -mlittle-endian %s $_CPPINCFLAGS -ffunction-sections -fdata-sections -fno-strict-aliasing %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], opt, cdefs )
tools[ 'stm32' ][ 'linkcom' ] = "%s -mcpu=cortex-m3 -mthumb -Wl,-T -Xlinker %s -u _start -Wl,-e,Reset_Handler -Wl,-static -Wl,--gc-sections -nostartfiles -nostdlib -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( toolset[ 'compile' ], ldscript, local_libs )
tools[ 'stm32' ][ 'ascom' ] = "%s -x assembler-with-cpp $_CPPINCFLAGS -mcpu=cortex-m3 -mthumb %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], cdefs )

# Programming function
def progfunc_stm32( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'stm32' ][ 'progfunc' ] = progfunc_stm32

