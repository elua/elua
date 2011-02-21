-- Configuration file for the STM32 microcontroller

addi( sf( 'src/platform/%s/FWLib/library/inc', platform ) )

local fwlib_files = utils.get_files( "src/platform/" .. platform .. "/FWLib/library/src", ".*%.c$" )

specific_files = "core_cm3.c system_stm32f10x.c startup_stm32f10x_hd.s platform.c stm32f10x_it.c lcd.c lua_lcd.c platform_int.c enc.c"

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

-- Programming function
tools.stm32.progfunc = function( target, deps )
  local outname = deps[ 1 ]:target_name()
  os.execute( sf( "%s %s", toolset.size, outname ) )
  print "Generating binary image..."
  os.execute( sf( "%s -O binary %s %s.bin", toolset.bin, outname, output ) )
  os.execute( sf( "%s -O ihex %s %s.hex", toolset.bin, outname, output ) )
  return 0
end

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

