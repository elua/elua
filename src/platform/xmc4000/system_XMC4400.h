/**************************************************************************//**
 * @file     system_XMC4400.h
 * @brief    Header file for the XMC4400-Series systeminit
 *           
 * @version  V1.7
 * @date     10. February 2015
 *
 * @note
 * Copyright (C) 2011 Infineon Technologies AG. All rights reserved.

 *
 * @par
 * Infineon Technologies AG (Infineon) is supplying this software for use with Infineon’s microcontrollers.  
 * This file can be freely distributed within development tools that are supporting such microcontrollers. 

 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 ******************************************************************************/

#ifndef SYSTEM_XMC4400_H
#define SYSTEM_XMC4400_H

/*******************************************************************************
 * HEADER FILES
 *******************************************************************************/

#include <stdint.h>

/*******************************************************************************
 * MACROS
 *******************************************************************************/

#define	OFI_FREQUENCY        (24000000UL)  /**< 24MHz Backup Clock (fOFI) frequency. */
#define OSI_FREQUENCY        (32768UL)    /**< 32KHz Internal Slow Clock source (fOSI) frequency. */  

/*******************************************************************************
 * GLOBAL VARIABLES
 *******************************************************************************/

extern uint32_t SystemCoreClock;        /*!< System Clock Frequency (Core Clock)  */
extern uint8_t g_chipid[16];            /*!< Unique chip ID */
extern uint32_t g_hrpwm_char_data[3];   /*!< HRPWM characterization data */

/*******************************************************************************
 * API PROTOTYPES
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the system
 *
 */
void SystemInit(void);

/**
 * @brief Initialize CPU settings
 *
 */
void SystemCoreSetup(void);

/**
 * @brief Initialize clock
 *
 */
void SystemCoreClockSetup(void);

/**
 * @brief Update SystemCoreClock variable
 *
 */
void SystemCoreClockUpdate(void);

/**
 * @brief Returns frequency of the high performace oscillator
 * User needs to overload this function to return the correct oscillator frequency
 */
uint32_t OSCHP_GetFrequency(void);

#ifdef __cplusplus
}
#endif


#endif
