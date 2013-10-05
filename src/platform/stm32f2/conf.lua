-- Configuration file for the STM32F2 microcontroller

addi( sf( 'src/platform/%s/FWLib/library/inc', platform ) )
addi( sf( 'src/platform/%s/FWLib/USB/STM32_USB_OTG_Driver/inc', platform ) )
addi( sf( 'src/platform/%s/FWLib/USB/STM32_USB_Device_Library/Class/cdc/inc', platform ) )
addi( sf( 'src/platform/%s/FWLib/USB/STM32_USB_Device_Library/Core/inc', platform ) )
addi( sf( 'src/platform/%s/FWLib/USB/VCP/inc', platform ) )

local fwlib_files = utils.get_files( "src/platform/" .. platform .. "/FWLib/library/src", ".*%.c$" )
fwlib_files = fwlib_files .. " " .. utils.get_files( "src/platform/" .. platform .. "/FWLib/USB/", "%.c$" )
specific_files = "system_stm32f2xx.c startup_stm32f2xx.s platform.c stm32f2xx_it.c platform_int.c"

local ldscript = "stm32.ld"
  
-- Prepend with path
specific_files = fwlib_files .. " " .. utils.prepend_path( specific_files, "src/platform/" .. platform )
specific_files = specific_files .. " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm( { "FOR" .. cnorm( comp.cpu ), "FOR" .. cnorm( comp.board ), 'gcc', 'USE_STDPERIPH_DRIVER', 'STM32F2XX', 'CORTEX_M3' } )

-- Standard GCC Flags
addcf( { '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' } )
addlf( { '-nostartfiles','-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' } )
addaf( { '-x', 'assembler-with-cpp', '-c', '-Wall' } )
addlib( { 'c','gcc','m' } )

local target_flags = { '-mcpu=cortex-m3', '-mthumb' }

-- Configure general flags for target
addcf( { target_flags, '-mlittle-endian' } )
addlf( { target_flags, '-Wl,-e,Reset_Handler', '-Wl,-static' } )
addaf( target_flags )

-- USB macros
-- Currently, CDC works only over the USB FS interface, HS seems to have some issues
-- This would be handled better by the backend configurator interface
-- addm( { 'USE_USB_OTG_HS', 'USE_EMBEDDED_PHY' } )
addm( 'USE_USB_OTG_FS' )

-- Toolset data
tools.stm32f2 = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.stm32f2.prog_flist = { output .. ".bin", output .. ".hex" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

