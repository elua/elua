# Configuration file for the XMC4500 microcontroller

specific_files = "platform.c startup_XMC4500.s lib/system_XMC4500.c lib/UART001.c lib/UART001_Conf.c lib/RESET001.c lib/CLK001.c lib/DAVE3.c lib/MULTIPLEXER.c"

ldscript = "xmc4500f144k1024.ld"

comp.Append( CPPPATH = ['src/platform/' + platform + '/lib'] )
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
specific_files = specific_files + " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

comp.Append( CPPDEFINES = [ "FOR" + cnorm( comp[ 'cpu' ] ), "FOR" + cnorm( comp[ 'board' ] ), 'gcc', 'CORTEX_M4' ] )
comp.Append( CPPDEFINES = [ "UC_ID=4502", "DAVE_CE" ] )

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

TARGET_FLAGS = ['-mcpu=cortex-m4','-mthumb', '-mfloat-abi=soft']

# Configure general flags for target
comp.Prepend(CCFLAGS = [TARGET_FLAGS,'-mlittle-endian'])
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,__Xmc4500_reset_cortex_m','-Wl,-static'])
comp.Prepend(ASFLAGS = TARGET_FLAGS)

# Toolset data
tools[ 'xmc4000' ] = {}

# Programming function
def progfunc_xmc4000( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O binary %s %s.bin" % ( toolset[ 'bin' ], outname, output ) )
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'xmc4000' ][ 'progfunc' ] = progfunc_xmc4000

