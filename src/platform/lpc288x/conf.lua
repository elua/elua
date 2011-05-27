-- Configuration file for the LPC288x backend

local cpumode = ( builder:get_option( 'cpumode' ) or 'arm' ):lower()

specific_files = "lpc28xx.s platform.c target.c uart.c"

local ldscript = "lpc2888.lds"

if cpumode == 'thumb' then
  print "ERROR: due to a hardware limitation, it is not possible to run Thumb code from the LPC2888 internal flash."  
  print "Compile again, this time with cpumode=arm (or without specifying 'cpumode')"
  os.exit( -1 )
end  
  
-- Prepend with path
specific_files = utils.prepend_path( specific_files, sf( "src/platform/%s", platform ) )
specific_files = specific_files .. " src/platform/arm_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm{ "FOR" .. comp.cpu:upper(), 'gcc' }

-- Standard GCC flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addaf{ '-x', 'assembler-with-cpp', '-Wall' }
addlib{ 'c','gcc','m' }

-- Special target configuration
local target_flags = { '-mcpu=arm7tdmi' }
addm( 'CPUMODE_ARM' )
 
addcf( target_flags )
addlf{ target_flags, '-Wl,-e,HardReset' }
addaf{ target_flags,'-D__ASSEMBLY__' }

-- Toolset data
tools.lpc288x = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.lpc288x.prog_flist = { output .. ".bin" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

