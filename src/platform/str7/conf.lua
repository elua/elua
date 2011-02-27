-- Configuration file for the STR7 backend

local cpumode = ( builder:get_option( 'cpumode' ) or 'thumb' ):lower()
specific_files = "platform.c crt0.s 71x_rccu.c 71x_uart.c 71x_apb.c 71x_gpio.c 71x_tim.c"
local ldscript = 'str711fr2.lds'
    
-- Prepend with path
specific_files = utils.prepend_path( specific_files, sf( "src/platform/%s", platform ) )
specific_files = specific_files .. " src/platform/arm_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm{ "FOR" .. comp.cpu:upper(), 'gcc' }

-- Standard GCC flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing','-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addaf{ '-x', 'assembler-with-cpp', '-Wall' }
addlib{ 'c','gcc','m' }

-- Special target configuration
local target_flasg = '-mcpu=arm7tdmi'
if cpumode == 'thumb' then
  target_flags = { target_flags, '-mthumb' }
  addm( 'CPUMODE_THUMB' )
else
  addm( 'CPUMODE_ARM' )
end

addcf( target_flags )
addlf{ target_flags, '-Wl,-e,entry' }
addaf( target_flags )

-- Toolset data
tools.str7 = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.str7.prog_flist = { output .. ".bin" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

