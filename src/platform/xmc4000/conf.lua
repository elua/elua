-- Configuration file for the XMC4500 microcontroller

specific_files = "platform.c startup_XMC4500.s lib/system_XMC4500.c lib/UART001.c lib/UART001_Conf.c lib/RESET001.c lib/CLK001.c lib/DAVE3.c lib/MULTIPLEXER.c"

local ldscript = "xmc4500f144k1024.ld"

addi( "src/platform/" .. platform .. "/lib" )
  
-- Prepend with path
specific_files = utils.prepend_path( specific_files, "src/platform/" .. platform )
specific_files = specific_files .. " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm( { "FOR" .. cnorm( comp.cpu ), "FOR" .. cnorm( comp.board ), 'gcc', 'CORTEX_M4' } )
addm( { "UC_ID=4502", "DAVE_CE" } )

-- Standard GCC Flags
addcf( { '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' } )
addlf( { '-nostartfiles','-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' } )
addaf( { '-x', 'assembler-with-cpp', '-c', '-Wall' } )
addlib( { 'c','gcc','m' } )

local target_flags = { '-mcpu=cortex-m4', '-mthumb', '-mfloat-abi=soft' }

-- Configure general flags for target
addcf( { target_flags, '-mlittle-endian' } )
addlf( { target_flags, '-Wl,-e,__Xmc4500_reset_cortex_m', '-Wl,-static' } )
addaf( target_flags )

-- Toolset data
tools.xmc4000 = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.xmc4000.prog_flist = { output .. ".hex" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

