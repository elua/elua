# Configuration file for the linux backend

specific_files = "boot.s utils.s hostif_%s.c platform.c host.c" % comp[ 'cpu' ].lower()
ldscript = "i386.ld"
  
# override default optimize settings (-Os is broken right now)
comp.Replace(OPTFLAGS = ['-O0'])
comp.AppendUnique(CCFLAGS = '-g'])

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Standard GCC Flags
comp.Append(CCFLAGS = ['-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall'])
comp.Append(LINKFLAGS = ['-nostartfiles','-nostdlib','-T',ldscript,'-Wl,--gc-sections','-Wl,--allow-multiple-definition'])
#comp.Append(ASFLAGS = ['-x','assembler-with-cpp','-c','-Wall','$_CPPDEFFLAGS'])
comp.Append(LIBS = ['c','gcc','m'])

TARGET_FLAGS = ['-march=i386','-mfpmath=387','-m32']

comp.Prepend(CCFLAGS = [TARGET_FLAGS,'-fno-builtin','-fno-stack-protector'])
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,start','-Wl,-static'])
comp['AS'] = toolset[ 'asm' ]  # Need to force toolset
comp.Prepend(ASFLAGS = ['-felf'])

# Toolset data
tools[ 'sim' ] = {}

# Programming function for i386 (not needed, empty function)
def progfunc_dummy( target, source, env ):
  print "Run the simulator and enjoy :)"
  
tools[ 'sim' ][ 'progfunc' ] = progfunc_dummy

