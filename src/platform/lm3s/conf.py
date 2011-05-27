# Configuration file for the LM3S microcontroller
specific_files = "startup_gcc.c platform.c uart.c sysctl.c gpio.c ssi.c timer.c pwm.c ethernet.c systick.c flash.c interrupt.c cpu.c adc.c can.c platform_int.c"

if comp[ 'board' ] == 'EK-LM3S1968' or comp[ 'board' ] == 'EK-LM3S6965' or comp[ 'board' ] == 'EK-LM3S8962':
  specific_files = specific_files + " rit128x96x4.c disp.c"
  comp.Append(CPPDEFINES = 'ENABLE_DISP')


# The default for the Eagle 100 board is to start the image at 0x2000,
# so that the built in Ethernet boot loader can be used to upload it
if comp[ 'board' ] == 'EAGLE-100':
  comp.Append(LINKFLAGS = ['-Wl,-Ttext,0x2000'])

if comp[ 'board' ] == 'EK-LM3S9B92':
  ldscript = "lm3s-9b92.ld"
else:
  ldscript = "lm3s.ld"

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
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
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,ResetISR','-Wl,-static'])
comp.Prepend(ASFLAGS = TARGET_FLAGS)

# Toolset data
tools[ 'lm3s' ] = {}

# Programming function
def progfunc_lm3s( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )

tools[ 'lm3s' ][ 'progfunc' ] = progfunc_lm3s
