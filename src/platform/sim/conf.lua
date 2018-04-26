-- Configuration file for the linux (sim) backend

specific_files = sf( "boot.s utils.s hostif_%s.c platform.c host.c", comp.cpu:lower() )
local ldscript = "i386.ld"
  
-- Override default optimize settings
delcf{ "-Os", "-fomit-frame-pointer" }
addcf{ "-O0", "-g" }

-- Prepend with path
specific_files = utils.prepend_path( specific_files, sf( "src/platform/%s", platform ) )
local ldscript = sf( "src/platform/%s/%s", platform, ldscript ) 

-- Standard GCC flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addlib{ 'c','gcc','m' }
addm({"_READ_WRITE_RETURN_TYPE=_ssize_t"})

local target_flags = { '-march=i386','-mfpmath=387','-m32' }

addcf{ target_flags, '-fno-builtin', '-fno-stack-protector' }
addlf{ target_flags, '-Wl,-e,start', '-Wl,-static' }
addaf{ '-felf' }

-- Need to force the assembler command as we're using nasm
ascmd = "nasm -felf -o $(TARGET) $(FIRST)"
-- Also tell the builder that we don't need dependency checks for assembler files
builder:set_asm_dep_cmd( false )

-- Toolset data
tools.sim = {}

-- Programming function for i386 (not needed, empty function)
tools.sim.progfunc = function( target, deps )
  print "Run the simulator (./run_elua_sim.sh) and enjoy :) Linux only."
  return 0
end

-- Add the programming function explicitly for this target
tools.sim.pre_build = function()
  local t = builder:target( "#phony:prog", { exetarget }, tools.sim.progfunc )
  builder:add_target( t, "build eLua firmware image", { "prog" } )
end

