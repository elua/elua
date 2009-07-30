# Configuration file for the i386 backend

specific_files = "boot.s common.c descriptor_tables.c gdt.s interrupt.s isr.c kb.c  monitor.c timer.c platform.c"
ldscript = "i386.ld"
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'i386' ] = {}
tools[ 'i386' ][ 'cccom' ] = "%s %s $_CPPINCFLAGS -march=i386 -mfpmath=387 -m32 -ffunction-sections -fdata-sections -fno-builtin -fno-stack-protector %s -Wall -c $SOURCE -o $TARGET" % ( toolset[ 'compile' ], opt, cdefs )
tools[ 'i386' ][ 'linkcom' ] = "%s -nostartfiles -nostdlib -march=i386 -mfpmath=387 -m32 -T %s -Wl,--gc-sections -Wl,-e,start -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( toolset[ 'compile' ], ldscript, local_libs )
tools[ 'i386' ][ 'ascom' ] = "%s -felf $SOURCE" % toolset[ 'asm' ]

# Programming function for i386 (not needed, empty function)
def progfunc_i386( target, source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Visit http://www.eluaproject.net for instructions on how to use your eLua ELF file"
  
tools[ 'i386' ][ 'progfunc' ] = progfunc_i386
