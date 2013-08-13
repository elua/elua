local fname = "inc/pin_names.h"
local nports, npins = 16, 32
local sf = string.format

local per_names = { "UART", "SPI" }
local pin_names = { { "RX", "TX", "RTS", "CTS" }, { "MISO", "MOSI", "SCK", "SS" } }
local max_per = 16

f = assert( io.open( fname, "wt" ) )
f:write( [[// Automatically generated

#ifndef __PIN_NAMES_H__
#define __PIN_NAMES_H__

#include "platform.h"

]])
for port = 0, nports - 1 do
  for pin = 0, npins - 1 do
    f:write( sf( "#define P%d_%d      PLATFORM_IO_ENCODE( %d, %d, PLATFORM_IO_ENC_PIN )\n", port, pin, port, pin ) )
  end
end
f:write( "\n" )
for port = 0, nports - 1 do
  for pin = 0, npins - 1 do
    f:write( sf( "#define P%s_%d      PLATFORM_IO_ENCODE( %d, %d, PLATFORM_IO_ENC_PIN )\n", string.char( 65 + port ), pin, port, pin ) )
  end
end
f:write( "\n" )
-- Generate _PERIPHERAL%d_PIN%s macros for pinmap
for idx, p in pairs( per_names ) do
  for i = 0, max_per - 1 do
    for j = 1, #pin_names[ idx ] do
      f:write( sf( "#define _%s%d_%s    PINMAP_%s, %d, PINMAP_%s_%s\n", p, i, pin_names[ idx ][ j ], p, i, p, pin_names[ idx ][ j ] ) )
    end
  end
  f:write( "\n" )
end
f:write( "\n#endif\n" )
f:close()

