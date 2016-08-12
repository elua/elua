
-- Configuration file for the XMC4500 microcontroller

specific_files = "platform.c startup_XMC4500.S system_XMC4500.c xmclib/src/xmc4_eru.c xmclib/src/xmc4_flash.c xmclib/src/xmc4_gpio.c xmclib/src/xmc4_rtc.c xmclib/src/xmc4_scu.c xmclib/src/xmc_can.c xmclib/src/xmc_ccu4.c xmclib/src/xmc_ccu8.c xmclib/src/xmc_common.c xmclib/src/xmc_dac.c xmclib/src/xmc_dma.c xmclib/src/xmc_dsd.c xmclib/src/xmc_ebu.c xmclib/src/xmc_ecat.c xmclib/src/xmc_eru.c xmclib/src/xmc_eth_mac.c xmclib/src/xmc_fce.c xmclib/src/xmc_gpio.c xmclib/src/xmc_hrpwm.c xmclib/src/xmc_i2c.c xmclib/src/xmc_i2s.c xmclib/src/xmc_ledts.c xmclib/src/xmc_posif.c xmclib/src/xmc_rtc.c xmclib/src/xmc_sdmmc.c xmclib/src/xmc_spi.c xmclib/src/xmc_uart.c xmclib/src/xmc_usbd.c xmclib/src/xmc_usic.c xmclib/src/xmc_vadc.c xmclib/src/xmc_wdt.c xmclib/gen/CLOCK_XMC4/clock_xmc4.c xmclib/gen/CLOCK_XMC4/clock_xmc4_conf.c xmclib/gen/UART/uart.c xmclib/gen/UART/uart_conf.c xmclib/gen/CPU_CTRL_XMC4/cpu_ctrl_xmc4.c xmclib/gen/CPU_CTRL_XMC4/cpu_ctrl_xmc4_conf.c xmclib/gen/FATFS/fatfs.c xmclib/gen/FATFS/fatfs_conf.c xmclib/gen/FATFS/sltha.c xmclib/gen/RTC/rtc.c xmclib/gen/RTC/rtc_conf.c xmclib/gen/SDMMC_BLOCK/sdmmc_block_private_sd.c xmclib/gen/SDMMC_BLOCK/sdmmc_block.c xmclib/gen/SDMMC_BLOCK/sdmmc_block_conf.c xmclib/gen/SYSTIMER/systimer.c xmclib/gen/SYSTIMER/systimer_conf.c xmclib/gen/DAVE.c pot.c dts.c rtc.c"

local ldscript = ""
local cpu = comp.cpu:upper()

if cpu == 'XMC4500F144K1024' then
  ldscript = "xmc4500_linker_script.ld"
end

addi( "src/platform/" .. platform .. "/xmclib/inc" )
addi( "src/platform/" .. platform .. "/xmclib/gen" )
addi( "src/platform/" .. platform .. "/xmclib/gen/UART" )
addi( "src/platform/" .. platform .. "/xmclib/gen/CLOCK_XMC4" )
addi( "src/platform/" .. platform .. "/xmclib/gen/CPU_CTRL_XMC4" )
addi( "src/platform/" .. platform .. "/xmclib/gen/FATFS" )
addi( "src/platform/" .. platform .. "/xmclib/gen/RTC" )
addi( "src/platform/" .. platform .. "/xmclib/gen/SDMMC_BLOCK" )
addi( "src/platform/" .. platform .. "/xmclib/gen/SYSTIMER" )
  
-- Prepend with path
specific_files = utils.prepend_path( specific_files, "src/platform/" .. platform )
specific_files = specific_files .. " src/platform/cortex_utils.s src/platform/arm_cortex_interrupts.c"
ldscript = sf( "src/platform/%s/%s", platform, ldscript )

addm( { "FOR" .. cnorm( comp.cpu ), "FOR" .. cnorm( comp.board ), 'gcc', 'CORTEX_M4' } )

if cpu == 'XMC4500F144K1024' then
  addm( { "XMC4500_F144x1024" } )
end

-- Standard GCC Flags
addcf( { '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-Wall' } )
addlf( { '-nostartfiles','-nostdlib', '-T', ldscript, '-Wl,--gc-sections', '-Wl,--allow-multiple-definition' } )
addaf( { '-x', 'assembler-with-cpp', '-c', '-Wall' } )
addlib( { 'c','gcc','m' } )

local target_flags = { '-mcpu=cortex-m4', '-mthumb', '-mfloat-abi=soft' }

-- Configure general flags for target
addcf( { target_flags, '-mlittle-endian' } )
addlf( { target_flags, '-Wl,-e,Reset_Handler', '-Wl,-static' } )
addaf( target_flags )

-- Toolset data
tools.xmc4000 = {}

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.xmc4000.prog_flist = { output .. ".hex" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile
