-- Configuration file for the STR9 backend

local cpumode = ( builder:get_option( 'cpumode' ) or 'arm' ):lower()

specific_files = "startup912.s startup_generic.s platform.c 91x_scu.c 91x_fmi.c 91x_gpio.c 91x_uart.c 91x_tim.c 91x_vic.c interrupt.c str9_pio.c 91x_i2c.c 91x_wiu.c 91x_adc.c platform_int.c 91x_ssp.c 91x_can.c"

local ldscript = "str912fw44.lds"

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

-- Special target configuration
local target_flags = '-mcpu=arm966e-s'
if cpumode == 'thumb' then
  target_flags = { target_flags, '-mthumb' }
  addm( 'CPUMODE_THUMB' )
else
  addm( 'CPUMODE_ARM' )
end

-- Toolchain 'arm-gcc' requires '-mfpu=fpa' for some reason
if comp.toolchain:lower() == 'arm-gcc' then
  target_flags = { target_flags, '-mfpu=fpa' }
end  

addcf( target_flags )
addlf{ target_flags, '-Wl,-e,_startup' }
addaf( target_flags )

-- Toolset data
tools.str9 = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.str9.prog_flist = { output .. ".bin" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

