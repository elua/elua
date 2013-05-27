-- Configuration file for the linux (sim) backend

local utils = require "utils.utils"

specific_files = sf( "hostif_%s.c platform.c host.c", comp.cpu:lower() )
asm_files = "boot.s utils.s"
local ldscript = "i386.ld"
  
-- Override default optimize settings
delcf{ "-Os", "-fomit-frame-pointer" }
addcf{ "-O0", "-g" }

-- Prepend with path
specific_files = utils.prepend_path( specific_files, sf( "src/platform/%s", platform ) )
asm_files = utils.prepend_path( asm_files, sf( "src/platform/%s", platform ) )
local ldscript = sf( "src/platform/%s/%s", platform, ldscript ) 

-- Standard GCC flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addlib{ 'c','gcc','m' }

local target_flags = { '-march=i386','-mfpmath=387','-m32' }

addcf{ target_flags, '-fno-builtin', '-fno-stack-protector' }
addlf{ target_flags, '-Wl,-e,start', '-Wl,-static' }

-- Need to force the assembler rules as we're using nasm
asm_files:gsub( "%S+", function( name )
  local t = target( utils.concat_path{ odir, utils.linearize_fname( utils.replace_extension( name, "o" ) ) }, name, "nasm -felf -o $(TARGET) $(DEPENDS)" )
  be_targets[ #be_targets + 1 ] = t
end )

-- Toolset data
tools.sim = {}

-- Add the programming function explicitly for this target
tools.sim.pre_build = function()
  target( 'prog', utils.concat_path{ outd, output .. ".elf" }, function() print "Run the simulator (./run_elua_sim.sh) and enjoy :) Linux only." end )
end

