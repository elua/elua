# Configuration file for the kinetis microcontroller
specific_files = "startup_kinetis.c platform.c cpu/arm_cm4.c cpu/sysinit.c cpu/kinetis_it.c drivers/uart/uart.c drivers/mcg/mcg.c drivers/wdog/wdog.c tsi.c"
specific_includes = 'cpu cpu/headers platforms drivers/mcg drivers/uart drivers/wdog'
ldscript = "mk60n512md100.ld"
                                         
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files += " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CPPDEFINES = ["FOR" + comp[ 'cpu' ],'gcc'])
comp.Append(CPPDEFINES = ['CORTEX_M4'])
comp.Append(CPPDEFINES = ['TOWER', 'TWR_K60N512'])
comp.Append(CPPPATH = [ "src/platform/%s/%s" % ( platform, f ) for f in specific_includes.split() ])

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

TARGET_FLAGS = ['-mcpu=cortex-m4', '-march=armv7e-m', '-mthumb']

# Configure General Flags for Target
comp.Prepend(CCFLAGS = [TARGET_FLAGS,'-mlittle-endian'])
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,Reset_Handler','-Wl,-static'])
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
