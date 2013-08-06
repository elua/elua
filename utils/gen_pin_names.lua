local fname = "inc/pin_names.h"
local nports, npins = 16, 32
local sf = string.format

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
f:write( "\n#endif\n" )
f:close()

