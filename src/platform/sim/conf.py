# Configuration file for the linux backend

specific_files = "boot.s utils.s hostif_%s.c platform.c host.c" % cputype.lower()
ldscript = "i386.ld"
  
# override default optimize settings (-Os is broken right now)
opt = " -g -O0 "

# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'sim' ] = {}
tools[ 'sim' ][ 'cccom' ] = "%s %s $_CPPINCFLAGS -march=i386 -mfpmath=387 -m32 -ffunction-sections -fdata-sections -fno-builtin -fno-stack-protector %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], opt, cdefs )
tools[ 'sim' ][ 'linkcom' ] = "%s -nostartfiles -nostdlib -march=i386 -mfpmath=387 -m32 -T %s -Wl,--gc-sections -Wl,-e,start -Wl,--allow-multiple-definition -Wl,-static -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( toolset[ 'compile' ], ldscript, local_libs )
tools[ 'sim' ][ 'ascom' ] = "%s -felf $SOURCE" % toolset[ 'asm' ]

# Programming function for i386 (not needed, empty function)
def progfunc_dummy( target, source, env ):
  print "Run the simulator and enjoy :)"
  
tools[ 'sim' ][ 'progfunc' ] = progfunc_dummy

