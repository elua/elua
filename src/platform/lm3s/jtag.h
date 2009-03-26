/*
  jtag.h - JTAG declarations for LM3S hosts
*/

#ifndef _JTAG_H
#define _JTAG_H

#include "hw_gpio.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "gpio.h"
#include "interrupt.h"
#include "sysctl.h"

/* API function declarations */

/* -------------------- JTAG functions -------------------- */
void JtagEnable(void);
void JtagDisable(void);

#endif /* _JTAG_H */
