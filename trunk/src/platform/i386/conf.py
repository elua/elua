# Configuration file for the i386 backend

specific_files = "boot.s common.c descriptor_tables.c gdt.s interrupt.s isr.c kb.c  monitor.c timer.c platform.c"
ldscript = "i386.ld"
  
# Prepend with path
specific_files = " ".join( [ "src/platform/%s/%s" % ( platform, f ) for f in specific_files.split() ] )
ldscript = "src/platform/%s/%s" % ( platform, ldscript )

# Toolset data
tools[ 'i386' ] = {}
tools[ 'i386' ][ 'cccom' ] = "i686-elf-gcc %s %s -march=i386 -mfpmath=387 -m32 -ffunction-sections -fdata-sections -fno-builtin -fno-stack-protector %s -Wall -c $SOURCE -o $TARGET" % ( opt, local_include, cdefs )
tools[ 'i386' ][ 'linkcom' ] = "i686-elf-gcc -nostartfiles -nostdlib -march=i386 -mfpmath=387 -m32 -T %s -Wl,--gc-sections -Wl,-e,start -Wl,--allow-multiple-definition -o $TARGET $SOURCES -lc -lgcc -lm %s" % ( ldscript, local_libs )
tools[ 'i386' ][ 'ascom' ] = "nasm -felf $SOURCE"

# Programming function for i386 (not needed, empty function)
def progfunc_i386( target, source, env ):
  outname = output + ".elf"
  os.system( "i686-elf-size %s" % outname )
  print "Visit http://elua.berlios.de for instructions on how to use your eLua ELF file"
  
tools[ 'i386' ][ 'progfunc' ] = progfunc_i386
