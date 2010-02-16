# Configuration file for the LPC17xx backend
import fnmatch
import glob
import os

local_include +=  ['src/platform/%s/drivers/inc' % platform]

fwlib_files = " ".join(glob.glob("src/platform/%s/drivers/src/*.c" % platform))

specific_files = "startup_LPC17xx.c system_LPC17xx.c core_cm3.c platform.c mbed_pio.c"

# Check CPU
if cputype == 'LPC1768':
  ldscript = "lpc17xx.ld"
else:
  print "Invalid LPC17xx CPU %s", cputype
  sys.exit( -1 )  
  

  
# Prepend with path
specific_files = fwlib_files + " " + " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

cdefs = cdefs + " -DFOR" + cputype + " -Dgcc"

# Toolset data
tools[ 'lpc17xx' ] = {}
tools[ 'lpc17xx' ][ 'cccom' ] = "%s -mcpu=cortex-m3 -mthumb -mlittle-endian %s $_CPPINCFLAGS -ffunction-sections -fdata-sections -fno-strict-aliasing %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], opt, cdefs )
tools[ 'lpc17xx' ][ 'linkcom' ] = "%s -mcpu=cortex-m3 -mthumb -Wl,-T -Xlinker %s -u _start -Wl,-e,Reset_Handler -Wl,-static -Wl,--gc-sections -nostartfiles -nostdlib -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( toolset[ 'compile' ], ldscript, local_libs )
tools[ 'lpc17xx' ][ 'ascom' ] = "%s -x assembler-with-cpp $_CPPINCFLAGS -mcpu=cortex-m3 -mthumb %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], cdefs )

# Programming function for LPC17xx
def progfunc_lpc17xx( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'lpc17xx' ][ 'progfunc' ] = progfunc_lpc17xx
