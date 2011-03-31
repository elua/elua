// Serial multiplexer definitions 

#ifndef __SERMUX_H__
#define __SERMUX_H__

#define SERMUX_SERVICE_ID_FIRST  0xD0
#define SERMUX_SERVICE_ID_LAST   0xD7
#define SERMUX_SERVICE_MAX       ( SERMUX_SERVICE_ID_LAST - SERMUX_SERVICE_ID_FIRST + 1 )

#define SERMUX_ESCAPE_CHAR       0xC0
#define SERMUX_FORCE_SID_CHAR    0xFF

#define SERMUX_ESCAPE_XOR_MASK   0x20
#define SERMUX_ESC_MASK          0x100

#endif
