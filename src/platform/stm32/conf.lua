-- Configuration file for the STM32 microcontroller

addi( sf( 'src/platform/%s/FWLib/library/inc', platform ) )

local fwlib_files = utils.get_files( "src/platform/" .. platform .. "/FWLib/library/src", ".*%.c$" )

specific_files = "core_cm3.c system_stm32f10x.c startup_stm32f10x_hd.s platform.c stm32f10x_it.c lcd.c lua_lcd.c platform_int.c enc.c buzzer.c tft.c"

local ldscript = "stm32.ld"
  
-- Prepend with path
specific_files = fwlib_files .. " " .. utils.prepend_path( specific_files, "src/platform/" .. platform )
specific_files = specific_files .. " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm( { "FOR" .. cnorm( comp.cpu ), "FOR" .. cnorm( comp.board ), 'gcc', 'CORTEX_M3' } )

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

-- Toolset data
tools.stm32 = {}

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
tools.stm32.pre_build = function()
  local burntarget = builder:target( "#phony:burn", { progtarget }, burnfunc )
  burntarget:force_rebuild( true )
  burntarget:set_target_args( builder:get_target_args() )
  builder:add_target( burntarget, "burn image to board using stm32ld (use COM port name as argument)", { "burn" } )
end

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.stm32.prog_flist = { output .. ".bin", output .. ".hex" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

