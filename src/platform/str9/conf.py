# Configuration file for the STR9 backend

cpumode = ARGUMENTS.get( 'cpumode', 'arm' ).lower()

specific_files = "startup912.s startup_generic.s platform.c 91x_scu.c 91x_fmi.c 91x_gpio.c 91x_uart.c 91x_tim.c 91x_vic.c interrupt.c str9_pio.c 91x_i2c.c"

# Check CPU
if comp[ 'cpu' ] == 'STR912FAW44':
  ldscript = "str912fw44.lds"
else:
  print "Invalid STR9 CPU %s" % comp[ 'cpu' ]
  Exit( -1 )
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CPPDEFINES = ["FOR" + comp[ 'cpu' ],'gcc'])

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS','$_CPPINCFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

# Special Target Configuration
TARGET_FLAGS = ['-mcpu=arm966e-s']
if cpumode == 'thumb':
  TARGET_FLAGS += ['-mthumb']

# toolchain 'arm-gcc' requires '-mfpu=fpa' for some reason
if comp['toolchain'] == 'arm-gcc':
   TARGET_FLAGS += ['-mfpu=fpa']

comp.Prepend(CCFLAGS = TARGET_FLAGS)
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,_startup'])
comp.Prepend(ASFLAGS = [TARGET_FLAGS])

# Toolset data
tools[ 'str9' ] = {}

# Programming function for STR9
def progfunc_str9( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'str9' ][ 'progfunc' ] = progfunc_str9
