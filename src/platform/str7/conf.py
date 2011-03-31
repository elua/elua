# Configuration file for the STR7 backend

cpumode = ARGUMENTS.get( 'cpumode', 'thumb' ).lower()

specific_files = "platform.c crt0.s 71x_rccu.c 71x_uart.c 71x_apb.c 71x_gpio.c 71x_tim.c"
if comp[ 'cpu' ] == 'STR711FR2':
  ldscript = "str711fr2.lds"
else:
  print "Invalid STR7 CPU %s" % comp[ 'cpu' ]
  Exit( -1 )  
    
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files += " src/platform/arm_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append(CPPDEFINES = ["FOR" + comp[ 'cpu' ],'gcc'])

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

# Special Target Configuration
TARGET_FLAGS = ['-mcpu=arm7tdmi']
if cpumode == 'thumb':
  TARGET_FLAGS += ['-mthumb']
  comp.Append(CPPDEFINES = ['CPUMODE_THUMB'])
else:
  comp.Append(CPPDEFINES = ['CPUMODE_ARM'])

comp.Prepend(CCFLAGS = TARGET_FLAGS)
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,entry'])
comp.Prepend(ASFLAGS = [TARGET_FLAGS])

# Toolset data
tools[ 'str7' ] = {}

# Programming function for LPC2888
def progfunc_str7( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'str7' ][ 'progfunc' ] = progfunc_str7
