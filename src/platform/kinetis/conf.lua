-- Configuration file for the kinetis microcontroller
specific_files = ""

-- Prepend with path
specific_files = utils.prepend_path( specific_files, sf( "src/platform/%s", platform) )
specific_files = specific_files .. " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm{ "FOR" .. comp.cpu:upper(), 'gcc', 'CORTEX_M4' }

-- Standard GCC flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addaf{ '-x', 'assembler-with-cpp', '-Wall' }
addlib{ 'c','gcc','m' }

local target_flags =  {'-mcpu=cortex-m4','-mthumb' }

-- Configure general flags for target
addcf{ target_flags, '-mlittle-endian' }
addlf{ target_flags, '-Wl,-e,ResetISR', '-Wl,-static' }
addaf( target_flags )

-- Toolset data
tools.kinetis = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.kinetis.prog_flist = { output .. ".bin" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

