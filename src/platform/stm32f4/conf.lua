local utils = require "utils.utils"
local sf = string.format

addi( sf( 'src/platform/%s/FWLib/library/inc', platform ) )
addi( sf( 'src/platform/%s/FWLib/USB/STM32_USB_OTG_Driver/inc', platform ) )
addi( sf( 'src/platform/%s/FWLib/USB/STM32_USB_Device_Library/Class/cdc/inc', platform ) )
addi( sf( 'src/platform/%s/FWLib/USB/STM32_USB_Device_Library/Core/inc', platform ) )
addi( sf( 'src/platform/%s/FWLib/USB/VCP/inc', platform ) )

local fwlib_files = utils.get_files( "src/platform/" .. platform .. "/FWLib/library/src", ".*%.c$" )
if comp.board ~= "stm32f4-nucleo" and comp.board ~= "nucleo-f411re" then
  fwlib_files = fwlib_files .. " " .. utils.get_files( "src/platform/" .. platform .. "/FWLib/USB/", "%.c$" )
end
specific_files = "system_stm32f4xx.c startup_stm32f4xx.s stm32f4xx_it.c platform.c platform_int.c cpu.c stm32_pio.c enc.c"
local ldscript = comp.cpu == "STM32F401RE" and "stm32f401re.ld" or "stm32.ld"

-- Prepend with path
specific_files = fwlib_files .. " " .. utils.prepend_path( specific_files, "src/platform/" .. platform )
specific_files = specific_files .. " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm( { "FOR" .. cnorm( comp.cpu ), "FOR" .. cnorm( comp.board ), 'gcc', 'USE_STDPERIPH_DRIVER', 'STM32F4XX', 'CORTEX_M4' } )

-- Standard GCC Flags
--delcf( { '-Os' } )
--addcf( { '-O0' } )
addcf( { '-g' } )
addcf( { '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' } )
addlf( { '-nostartfiles','-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition', sf( "-Wl,-Map=%s.map", output ) } )
addaf( { '-x', 'assembler-with-cpp', '-c', '-Wall' } )
addlib( { 'c','gcc','m' } )

local target_flags = { '-mcpu=cortex-m4', '-mthumb','-mfloat-abi=hard', '-mfpu=fpv4-sp-d16' }

-- Configure general flags for target
addcf( { target_flags, '-mlittle-endian' } )
addlf( { target_flags, '-Wl,-e,Reset_Handler', '-Wl,-static' } )
addaf( target_flags )

-- USB macros
-- Currently, CDC works only over the USB FS interface, HS seems to have some issues
-- This would be handled better by the backend configurator interface
-- addm( { 'USE_USB_OTG_HS', 'USE_EMBEDDED_PHY' } )
addm( 'USE_USB_OTG_FS' )

tools.stm32f4 = {}

-- Image burn function (stm32ld)
-- args[1]: COM port name
local function burnfunc( target, deps, args )
  if type( args ) ~= "table" or #args == 0 then
    print "Error: stm32ld needs the port name, specify it as a target argument"
    return -1
  end
  print "Burning image to target ..."
  local cmd = sf( 'stm32ld %s 115200 %s', args[ 1 ], output .. ".bin" )
  local res = os.execute( cmd )
  return res == 0 and 0 or -1
end

-- Add a 'burn' target before the build starts
tools.stm32f4.pre_build = function()
  local burntarget = builder:target( "#phony:burn", { progtarget }, burnfunc )
  burntarget:force_rebuild( true )
  burntarget:set_target_args( builder:get_target_args() )
  builder:add_target( burntarget, "burn image to board using stm32ld (use COM port name as argument)", { "burn" } )
end

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.stm32f4.prog_flist = { output .. ".bin", output .. ".hex" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

