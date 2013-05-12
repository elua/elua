/******************************************************************************
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
** PLATFORM : Infineon XMC4000 Series                           			  **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR   : App Developer                                                   **
**                                                                            **
** MAY BE CHANGED BY USER [yes/no]: Yes                                       **
**                                                                            **
** MODIFICATION DATE : April 20, 2011                                         **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                       Author(s) Identity                                   **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** MV           App Developer                                                 **
*******************************************************************************/
/**
 * @file types.h
 *
 * @brief  This file contains types defs.
 *
 */
#ifndef TYPES_CONFIG_H_
#define TYPES_CONFIG_H_

#include "type.h"

#define DAVEApp_SUCCESS  0
#define NULL_PTR NULL

typedef uint32_t dma_addr_t;

typedef uint32_t status_t;
typedef uint32_t handle_t;


#if defined(__TASKING__) || defined (__GNUC__)
#else
typedef uint32_t time_t;
#endif

typedef enum SetResetType{
	RESET,
	SET
}SetResetType;

#ifndef __STDBOOL_H__
typedef unsigned char bool;
#endif

#ifndef EOF
#define EOF     (-1)
#endif      // EOF

#ifndef MIN
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#endif /* TYPES_CONFIG_H_ */

