-- Configuration file for the i386 backend

local utils = require "utils"

specific_files = "icommon.c descriptor_tables.c isr.c kb.c  monitor.c timer.c platform.c"
asm_files = "boot.s gdt.s interrupt.s"
local ldscript = "i386.ld"
  
-- Prepend with path
specific_files = utils.prepend_path( specific_files, sf( "src/platform/%s", platform ) )
asm_files = utils.prepend_path( asm_files, sf( "src/platform/%s", platform ) )
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

-- Standard GCC Flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addlib{ 'c','gcc','m' }

local target_flags = { '-march=i386','-mfpmath=387', '-m32' }

addcf{ target_flags, '-fno-builtin', '-fno-stack-protector' }
addlf{ target_flags, '-Wl,-e,start' }

-- Need to force the assembler rules as we're using nasm
asm_files:gsub( "%S+", function( name )
  local t = target( utils.concat_path{ odir, utils.linearize_fname( utils.replace_extension( name, "o" ) ) }, name, "nasm -felf -o $(TARGET) $(DEPENDS)" )
  be_targets[ #be_targets + 1 ] = t
end )

-- Toolset data
tools.i386 = {}

-- Add the programming function explicitly for this target
tools.i386.pre_build = function()
  target( 'prog', utils.concat_path{ outd, output .. ".elf" }, function() print "Visit http://www.eluaproject.net for instructions on how to use your eLua ELF file" end )
end

