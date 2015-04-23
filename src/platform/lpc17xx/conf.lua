-- Configuration file for the LPC17xx backend

addi( sf( 'src/platform/%s/drivers/inc', platform ) )
addi( sf( 'src/platform/%s/usbstack/inc', platform ) )

local fwlib_files = utils.get_files( sf( "src/platform/%s/drivers/src", platform ), ".*%.c$" )
fwlib_files = fwlib_files .. " " .. utils.get_files( sf( "src/platform/%s/usbstack/src", platform ), ".*%.c$" )
specific_files = "startup_LPC17xx.c system_LPC17xx.c core_cm3.c platform.c"

local board = comp.board:upper()

if board == "MBED" then
  specific_files = specific_files .. " mbed_pio.c"
else
  specific_files = specific_files .. " lpc17xx_pio.c"
end

local ldscript = "LPC17xx.ld"

-- Prepend with path
specific_files = fwlib_files .. " " .. utils.prepend_path( specific_files, sf( "src/platform/%s", platform ) )
specific_files = specific_files .. " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm{ "FOR" .. comp.cpu:upper(), 'gcc', 'CORTEX_M3' }

-- Standard GCC flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addaf{ '-x', 'assembler-with-cpp', '-Wall' }
addlib{ 'c','gcc','m' }

local target_flags = { '-mcpu=cortex-m3','-mthumb' }

-- Configure general flags for target
addcf{ target_flags, '-mlittle-endian' }
addlf{ target_flags, '-Wl,-e,Reset_Handler', '-Wl,-static' }
addaf{ target_flags }

-- Toolset data
tools.lpc17xx = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.lpc17xx.prog_flist = { output .. ".bin" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

