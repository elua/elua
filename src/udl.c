// eLua micro dynamic loader (udl)

#include "platform_conf.h"
#ifdef BUILD_UDL

#include "udl.h"
#include "type.h"

// ***************************************************************************
// Internal data structures and variables

typedef struct
{
  const char* pname;
  u32 loadaddr;
} udl_module_data;

static udl_module_data udl_modules[ UDL_MAX_MODULES ];
extern u32 uld_endflash[];

void udl_init()
{
}

// Loookup a symbol in the eLua symbol table
u32 udl_get_elua_symbol( u32 idx )
{
  return uld_endflash[ idx ];
}

#else // #ifdef BUILD_UDL

void udl_init()
{
}

#endif // #ifdef BUILD_UDL
