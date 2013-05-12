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
** PLATFORM : Infineon XMC4000 Series                                         **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR   : App Developer                                                   **
**                                                                            **
** MAY BE CHANGED BY USER [Yes/No]: Yes                                       **
**                                                                            **
** MODIFICATION DATE : May 10, 2012                                           **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                       Author(s) Identity                                   **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** PAE          App Developer                                                 **
*******************************************************************************/

/**
 * @file CLK001.h
 *
 * @brief  Header file for Clock App
 *
 */

#ifndef CLK001_H_
#define CLK001_H_

/*******************************************************************************
**                           Include Files                                    **
*******************************************************************************/
#include <DAVE3.h>


/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/

/**
 * @ingroup CLK001_publicparam
 * @{
 */


/**
 * @}
 */

/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/
/*******************************************************************************
**                      Global Type Definitions                               **
*******************************************************************************/
/*******************************************************************************
** FUNCTION PROTOTYPES                                                        **
*******************************************************************************/
/**
 * @ingroup CLK001_apidoc
 * @{
 */

 /**
 * @brief     Initializes SCU Clock registers based on user configuration
 *
 * @param[in]  None
 *
 * @return     None
 * <b>Reentrant: NO </b><BR>
 *
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * @code
 * #include <DAVE3.h>
 * int main(void)
 * {
 *   // Initializes all App configurations ...
 *   DAVE_Init(); // CLK001_Init() will be called from DAVE_Init()
 * }
 *
 * @endcode <BR> </p>
 */ 
void CLK001_Init(void);

 /**
 * @brief  Clock App APIs are invoked before the BSS and DATA sections are 
 *         initialized.Global/static variables must not be accessed since
 *         they are unreliable at this point in time.Instead, stack must be 
 *         used. CStart calls SetInitStatus first with 0 before invoking
 *         CMSIS startup which in turn calls AllowPLLInitByStartup().After
 *         the clock tree is setup, Cstart performs program loading and 
 *         thereafter calls SetInitStatus with 1. This is needed for clock 
 *         app to prevent itself from initializing all over again when
 *         invoked in a different context (DAVE_Init)
 */ 
void SetInitStatus(uint32_t);

 /**
 * @brief  This routine is called by CMSIS startup to find out if clock tree 
 *         setup should be done by it.This routine is WEAKLY defined in CStart.
 *         In the absence of clock app, the weak definition takes precedence
 *         which always permits clock tree setup by CStart. When clock app is
 *         defined, this function overrides the CStart definition. Clock tree 
 *         setup is launched and upon completion, control is ceded back to 
 *         CStart.CStart abstains from setting up clock tree and instead 
 *         proceeds with program  loading. Return 0 to disallow CStart from
 *         performing clock tree setup.
 */
uint32_t AllowPLLInitByStartup(void);

/**
 * @}
 */

#endif /* CLK001_H_ */

/*CODE_BLOCK_END*/

