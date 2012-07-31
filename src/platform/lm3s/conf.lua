-- Configuration file for the LM3S microcontroller

addi( sf( 'src/platform/%s/inc', platform ) )
addi( sf( 'src/platform/%s/driverlib', platform ) )
local cpu = comp.board:upper()

-- Only include USB headers/paths for boards which support it
if cpu == 'LM3S9B92' or board == 'LM3S9D92' then
  addi( sf( 'src/platform/%s/usblib', platform ) )
  addi( sf( 'src/platform/%s/usblib/device', platform ) )
end

specific_files = "startup_gcc.c platform.c platform_int.c lm3s_pio.c"
local fwlib_files = utils.get_files( "src/platform/" .. platform .. "/driverlib", ".*%.c$" )


local board = comp.board:upper()
if board == 'EK-LM3S1968' or board == 'EK-LM3S6965' or board == 'EK-LM3S8962' then
  specific_files = specific_files .. " rit128x96x4.c disp.c"
  addm 'ENABLE_DISP' 
end

-- The default for the Eagle 100 board is to start the image at 0x2000,
-- so that the built in Ethernet boot loader can be used to upload it
if board == 'EAGLE-100' then
  addlf '-Wl,-Ttext,0x2000'
end

if cpu == 'LM3S9B92' or cpu == 'LM3S9D92' then
   fwlib_files = fwlib_files .. " " .. utils.get_files( "src/platform/" .. platform .. "/usblib", ".*%.c$" ) 
   fwlib_files = fwlib_files .. " " .. utils.get_files( "src/platform/" .. platform .. "/usblib/device", ".*%.c$" )
   specific_files = specific_files .. "  usb_serial_structs.c"
end

if board == 'EK-LM3S9B92'  then
   ldscript = "lm3s-9b92.ld"
elseif board == 'SOLDERCORE' or board == "EK-LM3S9D92" then
   ldscript = "lm3s-9d92.ld"
else
   ldscript = "lm3s.ld"
end

-- Prepend with path
specific_files = fwlib_files .. " " .. utils.prepend_path( specific_files, "src/platform/" .. platform )
specific_files = specific_files .. " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm{ "FOR" .. comp.cpu:upper(), 'gcc', 'CORTEX_M3' }

-- Standard GCC flags
addcf{ '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' }
addlf{ '-nostartfiles', '-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' }
addaf{ '-x', 'assembler-with-cpp', '-Wall' }
addlib{ 'c','gcc','m' }

local target_flags =  {'-mcpu=cortex-m3','-mthumb' }

-- Configure general flags for target
addcf{ target_flags, '-mlittle-endian' }
addlf{ target_flags, '-Wl,-e,ResetISR', '-Wl,-static' }
addaf( target_flags )

-- Toolset data
tools.lm3s = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.lm3s.prog_flist = { output .. ".bin" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

