
/**************************************************************************//**
 *
 * Copyright (C) 2011 Infineon Technologies AG. All rights reserved.
 *
 * Infineon Technologies AG (Infineon) is supplying this software for use with
 * Infineon's microcontrollers.
 * This file can be freely distributed within development tools that are
 * supporting such microcontrollers.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
********************************************************************************
**                                                                            **
**                                                                            **
** PLATFORM : Infineon <Microcontroller name, step>                           **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** MODIFICATION DATE : June 20, 2012                                       **
**                                                                            **
*******************************************************************************/

/**
 * @file  DAVE3.h
 *
 * @brief This file contains all public data structures,enums and function
 *        prototypes
 *
 */

#ifndef _DAVE3_H_
#define _DAVE3_H_

//****************************************************************************
// @Prototypes Of Global Functions
//****************************************************************************

void DAVE_Init(void);

void SystemInit_DAVE3(void);
      
//****************************************************************************
// @Project Includes
//****************************************************************************

#include <XMC4500.h>

#include "types.h"

#include "MULTIPLEXER.h"


// #include APP HEADER FILES.

#include "LIBS.h" 

#include "DBG001.h" 

#include "CLK001.h" 

#include "RESET001.h" 

#include "UART001.h" 
      
#endif  // ifndef _DAVE3_H_
