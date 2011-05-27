/**
 * @file	: lpc17xx_rit.c
 * @brief	: Contains all functions support for RIT firmware library on LPC17xx
 * @version	: 1.0
 * @date	: 7. May. 2009
 * @author	: NguyenCao
 **************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup RIT
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc17xx_rit.h"
#include "lpc17xx_clkpwr.h"

/* If this source file built with example, the LPC17xx FW library configuration
 * file in each example directory ("lpc17xx_libcfg.h") must be included,
 * otherwise the default FW library configuration file must be included instead
 */
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc17xx_libcfg.h"
#else
#include "lpc17xx_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */

#ifdef _RIT

/* Public Functions ----------------------------------------------------------- */
/** @addtogroup RIT_Public_Functions
 * @{
 */

/******************************************************************************//*
 * @brief 		Initial for RIT
 * 					- Turn on power and clock
 * 					- Setup default register values
 * @param[in]	RITx is RIT peripheral selected, should be: RIT
 * @return 		None
 *******************************************************************************/
void RIT_Init(LPC_RIT_TypeDef *RITx)
{
	CHECK_PARAM(PARAM_RITx(RITx));
	CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCRIT, ENABLE);
	//Set up default register values
	RITx->RICOMPVAL = 0xFFFFFFFF;
	RITx->RIMASK	= 0x00000000;
	RITx->RICTRL	= 0x0C;
	RITx->RICOUNTER	= 0x00000000;
	// Turn on power and clock

}
/******************************************************************************//*
 * @brief 		DeInitial for RIT
 * 					- Turn off power and clock
 * 					- ReSetup default register values
 * @param[in]	RITx is RIT peripheral selected, should be: RIT
 * @return 		None
 *******************************************************************************/
void RIT_DeInit(LPC_RIT_TypeDef *RITx)
{
	CHECK_PARAM(PARAM_RITx(RITx));

	// Turn off power and clock
	CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCRIT, DISABLE);
	//ReSetup default register values
	RITx->RICOMPVAL = 0xFFFFFFFF;
	RITx->RIMASK	= 0x00000000;
	RITx->RICTRL	= 0x0C;
	RITx->RICOUNTER	= 0x00000000;
}
/******************************************************************************//*
 * @brief 		Set compare value, mask value and time counter value
 * @param[in]	RITx is RIT peripheral selected, should be: RIT
 * @param[in]	value: pointer to RIT_CMP_VAL Structure
 * @return 		None
 *******************************************************************************/
void RIT_TimerConfig(LPC_RIT_TypeDef *RITx, RIT_CMP_VAL *value)
{
	CHECK_PARAM(PARAM_RITx(RITx));

	RITx->RICOMPVAL	= value->CMPVAL;
	RITx->RIMASK	= value->MASKVAL;
	RITx->RICOUNTER	= value->COUNTVAL;
}
/******************************************************************************//*
 * @brief 		Enable/Disable Timer
 * @param[in]	RITx is RIT peripheral selected, should be: RIT
 * @param[in]	NewState 	New State of this function
 * 					-ENABLE: Enable Timer
 * 					-DISABLE: Disable Timer
 * @return 		None
 *******************************************************************************/
void RIT_Cmd(LPC_RIT_TypeDef *RITx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_RITx(RITx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	//Enable or Disable Timer
	if(NewState==ENABLE)
	{
		RITx->RICTRL |= RIT_CTRL_TEN;
	}
	else
	{
		RITx->RICTRL &= ~RIT_CTRL_TEN;
	}
}
/******************************************************************************//*
 * @brief 		Timer Enable/Disable Clear
 * @param[in]	RITx is RIT peripheral selected, should be: RIT
 * @param[in]	NewState 	New State of this function
 * 						-ENABLE: The timer will be cleared to 0 whenever
 * 				the counter value equals the masked compare value specified
 * 				by the contents of RICOMPVAL and RIMASK register
 * 						-DISABLE: The timer will not be clear to 0
 * @return 		None
 *******************************************************************************/
void RIT_TimerClearCmd(LPC_RIT_TypeDef *RITx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_RITx(RITx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	//Timer Enable/Disable Clear
	if(NewState==ENABLE)
	{
		RITx->RICTRL |= RIT_CTRL_ENCLR;
	}
	else
	{
		RITx->RICTRL &= ~RIT_CTRL_ENCLR;
	}
}
/******************************************************************************//*
 * @brief 		Timer Enable/Disable on break
 * @param[in]	RITx is RIT peripheral selected, should be: RIT
 * @param[in]	NewState 	New State of this function
 * 					-ENABLE: The timer is halted whenever a hardware break condition occurs
 * 					-DISABLE: Hardware break has no effect on the timer operation
 * @return 		None
 *******************************************************************************/
void RIT_TimerEnableOnBreakCmd(LPC_RIT_TypeDef *RITx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_RITx(RITx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	//Timer Enable/Disable on break
	if(NewState==ENABLE)
	{
		RITx->RICTRL |= RIT_CTRL_ENBR;
	}
	else
	{
		RITx->RICTRL &= ~RIT_CTRL_ENBR;
	}
}
/******************************************************************************//*
 * @brief 		Check whether interrupt flag is set or not
 * @param[in]	RITx is RIT peripheral selected, should be: RIT
 * @return 		Current interrupt status, could be: SET/RESET
 *******************************************************************************/
IntStatus RIT_GetIntStatus(LPC_RIT_TypeDef *RITx)
{
	uint8_t result;
	CHECK_PARAM(PARAM_RITx(RITx));
	if((RITx->RICTRL&RIT_CTRL_INTEN)==1)	result= SET;
	else return RESET;
	//clear interrupt flag
	RITx->RICTRL |= RIT_CTRL_INTEN;
	return result;
}

/**
 * @}
 */

#endif /* _RIT */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */
