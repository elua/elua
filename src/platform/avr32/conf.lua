-- Configuration file for the AVR32 microcontrollers

specific_files = "crt0.s trampoline.s platform.c exception.s intc.c pm.c flashc.c pm_conf_clocks.c usart.c gpio.c tc.c spi.c platform_int.c adc.c pwm.c i2c.c ethernet.c lcd.c rtc.c usb-cdc.c"
addm( "FORAVR32" )

-- See board.h for possible BOARD values.
if comp.board:upper()  == "ATEVK1100" then
  specific_files = specific_files .. " sdramc.c"
  addm( 'BOARD=1' )
elseif comp.board:upper()  == "ATEVK1101" then
  addm( 'BOARD=2' )
elseif comp.board:upper():sub( 1,7 )  == "MIZAR32" then
  specific_files = specific_files .. " sdramc.c"
  addm( 'BOARD=98' )
else
  print( sf( "Invalid board for %s platform (%s)", platform, comp.board ) )
  os.exit( -1 )
end

-- Prepend with path
specific_files = utils.prepend_path( specific_files, "src/platform/" .. platform )
-- Choose ldscript according to choice of bootloader
if comp.bootloader == 'none' then
  print "Compiling for FLASH execution"
  ldscript = sf( "src/platform/%s/%s.ld", platform, comp.cpu:lower() )
else
  print "Compiling for SDRAM execution"
  ldscript = sf( "src/platform/%s/%s_%s.ld", platform, comp.cpu:lower(), comp.bootloader )
end
addm( 'BOOTLOADER_' .. comp.bootloader:upper() )

-- Standard GCC Flags
addcf( { '-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-Wall' } )
addlf( { '-nostartfiles','-nostdlib','-Wl,--gc-sections','-Wl,--allow-multiple-definition','-Wl,--relax','-Wl,--direct-data','-T', ldscript } )
addaf( { '-x','assembler-with-cpp' } )
addlib( { 'c','gcc','m' } )

-- Target-specific flags
addcf( sf( '-mpart=%s', comp.cpu:sub( 5 ):lower() ) )
addaf( sf( '-mpart=%s', comp.cpu:sub( 5 ):lower() ) )
addlf( '-Wl,-e,crt0' )
if comp.bootloader == "emblod" then addm( "ELUA_FIRMWARE_SIZE=0x8000" ) else addm( "ELUA_FIRMWARE_SIZE=0" ) end

-- Toolset data
tools.avr32 = {}

-- Image burn function (DFU)
local function burnfunc( target, deps )
  print "Burning image to target ..."
  local fname = output .. ".hex"
  local cmd = ''
  local res
  if utils.is_windows() then -- use batchisp in Windows
    res = os.execute( sf( "batchisp -hardware usb -device %s -operation erase f memory flash blankcheck loadbuffer %s program verify start reset 0", comp.cpu:lower(), fname ) )
  else -- use "dfu-programmer" if not running in Windows
    res = utils.check_command( "dfu-programmer at32uc3a0512 version" )
    if res ~= 0 then
      print "Cannot find 'dfu-programmer', install it first"
      return -1
    end
    local basecmd = "dfu-programmer " .. comp.cpu:lower()
    res = os.execute( basecmd .. " erase" )
    if res == 0 then
      res = os.execute( basecmd .. " flash " .. fname )
      if res == 0 then
        os.execute( basecmd .. " start" )
      end
    end
  end
  return res == 0 and 0 or -1
end

-- Add a 'burn' target before the build starts
tools.avr32.pre_build = function()
  local burntarget = builder:target( "#phony:burn", { progtarget }, burnfunc )
  burntarget:force_rebuild( true )
  builder:add_target( burntarget, "burn image to AVR32 board using the DFU bootloader", { "burn" } )
end

-- Array of file names that will be checked against the 'prog' target; their absence will force a rebuild
tools.avr32.prog_flist = { output .. ".hex" }

-- We use 'gcc' as the assembler
toolset.asm = toolset.compile

