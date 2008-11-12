# Configuration file for the STM32 microcontroller
import fnmatch
import glob
import os

local_include = local_include + " -Isrc/platform/%s/FWLib/library/inc" % platform

fwlib_files = " ".join(glob.glob("src/platform/%s/FWLib/library/src/*.c" % platform))
#print "FWLib: %s " % fwlib_files 

specific_files = "cortexm3_macro.s stm32f10x_vector.c systick.c platform.c stm32f10x_it.c lcd.c lua_lcd.c lstm32_mod.c"

ldscript = "stm32.ld"
  
# Prepend with path
specific_files = fwlib_files + " " + " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

cdefs = cdefs + " -Dgcc"

# Toolset data
tools[ 'stm32' ] = {}
tools[ 'stm32' ][ 'cccom' ] = "arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mlittle-endian %s %s -ffunction-sections -fdata-sections -fno-strict-aliasing %s -Wall -c $SOURCE -o $TARGET" % ( opt, local_include, cdefs )
#tools[ 'stm32' ][ 'linkcom' ] = "arm-none-eabi-gcc -nostartfiles -nostdlib -T %s -Wl,--gc-sections -Wl,-e,ResetISR -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( ldscript, local_libs )
tools[ 'stm32' ][ 'linkcom' ] = "arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Wl,-T -Xlinker %s -u _start -Wl,-e,Reset_Handler -Wl,-static -Wl,--gc-sections -nostartfiles -nostdlib -Wl,-Map -Xlinker project.map -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( ldscript, local_libs )
tools[ 'stm32' ][ 'ascom' ] = "arm-none-eabi-gcc -x assembler-with-cpp %s -mcpu=cortex-m3 -mthumb %s -Wall -c $SOURCE -o $TARGET" % ( local_include, cdefs )

# Programming function
def progfunc_stm32( target, source, env ):
  outname = output + ".elf"
  os.system( "arm-none-eabi-size %s" % outname )
  print "Generating binary image..."
  os.system( "arm-none-eabi-objcopy -O binary %s %s.bin" % ( outname, output ) )
  os.system( "arm-none-eabi-objcopy -O ihex %s %s.hex" % ( outname, output ) )
  
tools[ 'stm32' ][ 'progfunc' ] = progfunc_stm32
