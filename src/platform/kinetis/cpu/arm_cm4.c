/*
 * File:		arm_cm4.c
 * Purpose:		Generic high-level routines for ARM Cortex M4 processors
 *
 * Notes:
 */

#include "common_kinetis.h"

/***********************************************************************/
/*
 * Configures the ARM system control register for STOP (deep sleep) mode
 * and then executes the WFI instruction to enter the mode.
 *
 * Parameters:
 * none
 *
 * Note: Might want to change this later to allow for passing in a parameter
 *       to optionally set the sleep on exit bit.
 */

void stop (void)
{
	/* Set the SLEEPDEEP bit to enable deep sleep mode (STOP) */
	SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK;	

	/* WFI instruction will start entry into STOP mode */
	asm("WFI");
}
/***********************************************************************/
/*
 * Configures the ARM system control register for WAIT (sleep) mode
 * and then executes the WFI instruction to enter the mode.
 *
 * Parameters:
 * none
 *
 * Note: Might want to change this later to allow for passing in a parameter
 *       to optionally set the sleep on exit bit.
 */

void wait (void)
{
	/* Clear the SLEEPDEEP bit to make sure we go into WAIT (sleep) mode instead
	 * of deep sleep.
	 */
	SCB_SCR &= ~SCB_SCR_SLEEPDEEP_MASK;	

	/* WFI instruction will start entry into WAIT mode */
	asm("WFI");
}
/***********************************************************************/
/*
 * Change the value of the vector table offset register to the specified value.
 *
 * Parameters:
 * vtor     new value to write to the VTOR
 */

void write_vtor (int vtor)
{
        /* Write the VTOR with the new value */
        SCB_VTOR = vtor;	
}
/***********************************************************************/


