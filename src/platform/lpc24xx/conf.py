# Configuration file for the LPC24xx backend

cpumode = ARGUMENTS.get( 'cpumode', 'arm' ).lower()

specific_files = "startup.s irq.c target.c platform.c"
if comp[ 'cpu' ] == 'LPC2468':
  ldscript = "lpc2468.lds"
else:
  print "Invalid CPU %s" % comp[ 'cpu' ]
  Exit( -1 )  
    
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
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

comp.Prepend(CCFLAGS = TARGET_FLAGS)
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,entry'])
comp.Prepend(ASFLAGS = [TARGET_FLAGS,'-D__ASSEMBLY__'])


# Toolset data
tools[ 'lpc24xx' ] = {}

# Programming function for LPC24xx
def progfunc_lpx24xx( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Generating binary image..."
  os.system( "%s -O ihex %s %s.hex" % ( toolset[ 'bin' ], outname, output ) )
  
tools[ 'lpc24xx' ][ 'progfunc' ] = progfunc_lpx24xx

