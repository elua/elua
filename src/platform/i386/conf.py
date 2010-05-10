# Configuration file for the i386 backend

specific_files = "boot.s common.c descriptor_tables.c gdt.s interrupt.s isr.c kb.c  monitor.c timer.c platform.c"
ldscript = "i386.ld"
  
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
comp.Prepend(LINKFLAGS = [TARGET_FLAGS,'-Wl,-e,start'])
comp['AS'] = toolset[ 'asm' ]  # Need to force toolset
comp.Prepend(ASFLAGS = ['-felf'])

# Toolset data
tools[ 'i386' ] = {}

# Programming function for i386 (not needed, empty function)
def progfunc_i386( comp[ 'target' ], source, env ):
  outname = output + ".elf"
  os.system( "%s %s" % ( toolset[ 'size' ], outname ) )
  print "Visit http://www.eluaproject.net for instructions on how to use your eLua ELF file"
  
tools[ 'i386' ][ 'progfunc' ] = progfunc_i386
