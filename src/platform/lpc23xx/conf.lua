-- Configuration file for the LPC23xx backend

local cpumode = ( builder:get_option( 'cpumode' ) or 'arm' ):lower()

specific_files = "startup.s irq.c target.c platform.c platform_int.c"

local ldscript = "lpc2368.lds"

-- Prepend with path
specific_files = utils.prepend_path( specific_files, sf( "src/platform/%s", platform ) )
specific_files = specific_files .. " src/platform/arm_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm{ "FOR" .. comp.cpu:upper(), 'gcc' }

-- Standard GCC Flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addaf{ '-x', 'assembler-with-cpp', '-Wall' }
addlib{ 'c','gcc','m' }

-- Specific target configuration
local target_flags = '-mcpu=arm7tdmi'
if cpumode == 'thumb' then
  target_flags = { target_flags, '-mthumb' }
  addm( 'CPUMODE_THUMB' )
else
  addm( 'CPUMODE_ARM' )
end

addcf( target_flags )
addlf{ target_flags, '-Wl,-e,entry' }
addaf{ target_flags, '-D___ASSEMBLY__' }

-- Toolset data
tools.lpc23xx = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.lpc23xx.prog_flist = { output .. ".hex" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

