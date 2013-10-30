// Generic platform configuration file

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "buf.h"
#include "sermux.h"
#include "legc.h"
#include "platform.h"
#include "auxmods.h"
#include "lualib.h"

// FIXME: ASF ill behaved - compiler.h does not check for previous definition of "UNDEFINED before defining it.  
//  It is also defined in lua source (but there it checks to be sure not defined).
//  Moving include of ELUA_CPU_HEADER before lua things solves that problem, but maybe there is another way?
#include ELUA_CPU_HEADER
#include ELUA_BOARD_HEADER
#include "platform_generic.h"     // generic platform header (include whatever else is missing here)

#endif

