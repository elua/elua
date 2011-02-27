-- Configuration file for the i386 backend

specific_files = "boot.s common.c descriptor_tables.c gdt.s interrupt.s isr.c kb.c  monitor.c timer.c platform.c"
local ldscript = "i386.ld"
  
-- Prepend with path
specific_files = utils.prepend_path( specific_files, sf( "src/platform/%s", platform ) )
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

-- Standard GCC Flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addlib{ 'c','gcc','m' }

local target_flags = { '-march=i386','-mfpmath=387', '-m32' }

addcf{ target_flags, '-fno-builtin', '-fno-stack-protector' }
addlf{ target_flags, '-Wl,-e,start' }
addaf{ '-felf' }

-- Need to force the assembler command as we're using nasm
ascmd = "nasm -felf -o $(TARGET) $(FIRST)"
-- Also tell the builder that we don't need dependency checks for assembler files
builder:set_asm_dep_cmd( false )

-- Toolset data
tools.i386 = {}

-- Programming function for i386 (not needed, empty function)
tools.i386.progfunc = function( target, deps )
  local outname = deps[ 1 ]:target_name()
  os.execute( sf( "%s %s", toolset.size, outname ) )
  print "Visit http://www.eluaproject.net for instructions on how to use your eLua ELF file"
  return 0
end

-- Add the programming function explicitly for this target
tools.i386.pre_build = function()
  local t = builder:target( "#phony:prog", { exetarget }, tools.i386.progfunc )
  builder:add_target( t, "build eLua firmware image", { "prog" } )
end

