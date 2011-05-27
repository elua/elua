/**
 * @file	: lpc17xx_timer.c
 * @brief	: Contains all functions support for Timer firmware library on LPC17xx
 * @version	: 1.0
 * @date	: 14. April. 2009
 * @author	: HieuNguyen
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
/** @addtogroup TIM
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"

/* If this source file built with example, the LPC17xx FW library configuration
 * file in each example directory ("lpc17xx_libcfg.h") must be included,
 * otherwise the default FW library configuration file must be included instead
 */
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc17xx_libcfg.h"
#else
#include "lpc17xx_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */

#ifdef _TIM

/* Private Functions ---------------------------------------------------------- */
/** @defgroup TIM_Private_Functions
 * @{
 */

uint32_t TIM_GetPClock (uint32_t timernum);
uint32_t TIM_ConverUSecToVal (uint32_t timernum, uint32_t usec);
uint32_t TIM_ConverPtrToTimeNum (LPC_TIM_TypeDef *TIMx);


/*********************************************************************//**
 * @brief 		Get peripheral clock of each timer controller
 * @param[in]	timernum Timer number
 * @return 		Peripheral clock of timer
 **********************************************************************/

uint32_t TIM_GetPClock (uint32_t timernum)
{
	uint32_t clkdlycnt;
	switch (timernum)
	{
	case 0:
		clkdlycnt = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER0);
		break;

	case 1:
		clkdlycnt = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER1);
		break;

	case 2:
		clkdlycnt = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER2);
		break;

	case 3:
		clkdlycnt = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER3);
		break;
	}
	return clkdlycnt;
}


/*********************************************************************//**
 * @brief 		Convert a time to a timer count value
 * @param[in]	timernum Timer number
 * @param[in]	usec Time in microseconds
 * @return 		The number of required clock ticks to give the time delay
 **********************************************************************/
uint32_t TIM_ConverUSecToVal (uint32_t timernum, uint32_t usec)
{
	uint64_t clkdlycnt;

	// Get Pclock of timer
	clkdlycnt = (uint64_t) TIM_GetPClock (timernum);

	clkdlycnt = (clkdlycnt * usec) / 1000000;
	return (uint32_t) clkdlycnt;
}


/*********************************************************************//**
 * @brief 		Convert a timer register pointer to a timer number
 * @param[in]	TIMx Pointer to a timer register set
 * @return 		The timer number (0 to 3) or -1 if register pointer is bad
 **********************************************************************/
uint32_t TIM_ConverPtrToTimeNum (LPC_TIM_TypeDef *TIMx)
{
	uint32_t tnum = -1;

	if (TIMx == LPC_TIM0)
	{
		tnum = 0;
	}
	else if (TIMx == LPC_TIM1)
	{
		tnum = 1;
	}
	else if (TIMx == LPC_TIM2)
	{
		tnum = 2;
	}
	else if (TIMx == LPC_TIM3)
	{
		tnum = 3;
	}

	return tnum;
}

/**
 * @}
 */


/* Public Functions ----------------------------------------------------------- */
/** @addtogroup TIM_Public_Functions
 * @{
 */

/*********************************************************************//**
 * @brief 		Get Interrupt Status
 * @param[in]	TIMx Timer selection, should be TIM0, TIM1, TIM2, TIM3
 * @param[in]	IntFlag
 * @return 		FlagStatus
 * 				- SET : interrupt
 * 				- RESET : no interrupt
 **********************************************************************/
FlagStatus TIM_GetIntStatus(LPC_TIM_TypeDef *TIMx, uint8_t IntFlag)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_INT_TYPE(IntFlag));
	uint8_t temp = (TIMx->IR)& TIM_IR_CLR(IntFlag);
	if (temp)
		return SET;

	return RESET;

}
/*********************************************************************//**
 * @brief 		Get Capture Interrupt Status
 * @param[in]	TIMx Timer selection, should be TIM0, TIM1, TIM2, TIM3
 * @param[in]	IntFlag
 * @return 		FlagStatus
 * 				- SET : interrupt
 * 				- RESET : no interrupt
 **********************************************************************/
FlagStatus TIM_GetIntCaptureStatus(LPC_TIM_TypeDef *TIMx, uint8_t IntFlag)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_INT_TYPE(IntFlag));
	uint8_t temp = (TIMx->IR) & (1<<(4+IntFlag));
	if(temp)
		return SET;
	return RESET;
}
/*********************************************************************//**
 * @brief 		Clear Interrupt pending
 * @param[in]	TIMx Timer selection, should be TIM0, TIM1, TIM2, TIM3
 * @param[in]	IntFlag should be in TIM_INT_TYPE enum
 * @return 		None
 **********************************************************************/
void TIM_ClearIntPending(LPC_TIM_TypeDef *TIMx, uint8_t IntFlag)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_INT_TYPE(IntFlag));
	TIMx->IR |= TIM_IR_CLR(IntFlag);
}

/*********************************************************************//**
 * @brief 		Clear Capture Interrupt pending
 * @param[in]	TIMx Timer selection, should be TIM0, TIM1, TIM2, TIM3
 * @param[in]	IntFlag should be in TIM_INT_TYPE enum
 * @return 		None
 **********************************************************************/
void TIM_ClearIntCapturePending(LPC_TIM_TypeDef *TIMx, uint8_t IntFlag)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_INT_TYPE(IntFlag));
	TIMx->IR |= (1<<(4+IntFlag));
}

/*********************************************************************//**
* @brief 		Configuration for Timer at initial time
* @param[in] 	TimerCounterMode Should be :
* 					- PrescaleOption = TC_PRESCALE_USVAL,
* 					- PrescaleValue = 1
* 				Counter mode with
* 					- Caption channel 0
* @param[in] 	TIM_ConfigStruct pointer to TIM_TIMERCFG_Type or
* 				TIM_COUNTERCFG_Type
* @return 		None
 **********************************************************************/
void TIM_ConfigStructInit(uint8_t TimerCounterMode, void *TIM_ConfigStruct)
{
	if (TimerCounterMode == TIM_TIMER_MODE )
	{
		TIM_TIMERCFG_Type * pTimeCfg = (TIM_TIMERCFG_Type *)TIM_ConfigStruct;
		pTimeCfg->PrescaleOption = TIM_PRESCALE_USVAL;
		pTimeCfg->PrescaleValue = 1;
	}
	else
	{
		TIM_COUNTERCFG_Type * pCounterCfg = (TIM_COUNTERCFG_Type *)TIM_ConfigStruct;
		pCounterCfg->CountInputSelect = TIM_COUNTER_INCAP0;

	}
}

/*********************************************************************//**
 * @brief 		Initial Timer/Counter device
 * 				 	Set Clock frequency for Timer
 * 					Set initial configuration for Timer
 * @param[in]	TIMx  Timer selection, should be TIM0, TIM1, TIM2, TIM3
 * @param[in]	TimerCounterMode TIM_MODE_OPT
 * @param[in]	TIM_ConfigStruct pointer to TIM_TIMERCFG_Type
 * 				that contains the configuration information for the
 *                    specified Timer peripheral.
 * @return 		None
 **********************************************************************/
void TIM_Init(LPC_TIM_TypeDef *TIMx, uint8_t TimerCounterMode, void *TIM_ConfigStruct)
{
	TIM_TIMERCFG_Type *pTimeCfg;
	TIM_COUNTERCFG_Type *pCounterCfg;
	uint32_t timer;

	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_MODE_OPT(TimerCounterMode));

	timer = TIM_ConverPtrToTimeNum(TIMx) ;
	//set power
	if (TIMx== LPC_TIM0)
	{
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM0, ENABLE);
		CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER0, CLKPWR_PCLKSEL_CCLK_DIV_4);
	}
	else if (TIMx== LPC_TIM1)
	{
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM1, ENABLE);
		CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER1, CLKPWR_PCLKSEL_CCLK_DIV_4);

	}

	else if (TIMx== LPC_TIM2)
	{
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM2, ENABLE);
		CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER2, CLKPWR_PCLKSEL_CCLK_DIV_4);
	}
	else if (TIMx== LPC_TIM3)
	{
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM3, ENABLE);
		CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER3, CLKPWR_PCLKSEL_CCLK_DIV_4);

	}

	TIMx->CCR &= ~TIM_CTCR_MODE_MASK;
	TIMx->CCR |= TIM_TIMER_MODE;

	TIMx->TC =0;
	TIMx->PC =0;
	TIMx->PR =0;
	if (TimerCounterMode == TIM_TIMER_MODE )
	{
		pTimeCfg = (TIM_TIMERCFG_Type *)TIM_ConfigStruct;
		if (pTimeCfg->PrescaleOption  == TIM_PRESCALE_TICKVAL)
		{
			TIMx->PR   = pTimeCfg->PrescaleValue -1  ;
		}
		else
		{
			TIMx->PR   = TIM_ConverUSecToVal (TIM_ConverPtrToTimeNum(TIMx),pTimeCfg->PrescaleValue)-1;
		}
	}
	else
	{

		pCounterCfg = (TIM_COUNTERCFG_Type *)TIM_ConfigStruct;
		TIMx->CCR  &= ~TIM_CTCR_INPUT_MASK;
		if (pCounterCfg->CountInputSelect == TIM_COUNTER_INCAP1)
			TIMx->CCR |= _BIT(2);
	}

	// Clear interrupt pending
	TIMx->IR = 0xFFFFFFFF;

}

/*********************************************************************//**
 * @brief 		Close Timer/Counter device
 * @param[in]	TIMx  Pointer to timer device
 * @return 		None
 **********************************************************************/
void TIM_DeInit (LPC_TIM_TypeDef *TIMx)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	// Disable timer/counter
	TIMx->TCR = 0x00;

	// Disable power
	if (TIMx== LPC_TIM0)
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM0, DISABLE);

	else if (TIMx== LPC_TIM1)
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM1, DISABLE);

	else if (TIMx== LPC_TIM2)
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM2, DISABLE);

	else if (TIMx== LPC_TIM3)
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM2, DISABLE);

}

/*********************************************************************//**
 * @brief	 	Start/Stop Timer/Counter device
 * @param[in]	TIMx Pointer to timer device
 * @param[in]	NewState
 * 				-	ENABLE  : set timer enable
 * 				-	DISABLE : disable timer
 * @return 		None
 **********************************************************************/
void TIM_Cmd(LPC_TIM_TypeDef *TIMx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	if (NewState == ENABLE)
	{
		TIMx->TCR	|=  TIM_ENABLE;
	}
	else
	{
		TIMx->TCR &= ~TIM_ENABLE;
	}
}

/*********************************************************************//**
 * @brief 		Reset Timer/Counter device,
 * 					Make TC and PC are synchronously reset on the next
 * 					positive edge of PCLK
 * @param[in]	TIMx Pointer to timer device
 * @return 		None
 **********************************************************************/
void TIM_ResetCounter(LPC_TIM_TypeDef *TIMx)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	TIMx->TCR |= TIM_RESET;
	TIMx->TCR &= ~TIM_RESET;
}

/*********************************************************************//**
 * @brief 		Configuration for Match register
 * @param[in]	TIMx Pointer to timer device
 * @param[in]   TIM_MatchConfigStruct Pointer to TIM_MATCHCFG_Type
 * 					- MatchChannel : choose channel 0 or 1
 * 					- IntOnMatch	 : if SET, interrupt will be generated when MRxx match
 * 									the value in TC
 * 					- StopOnMatch	 : if SET, TC and PC will be stopped whenM Rxx match
 * 									the value in TC
 * 					- ResetOnMatch : if SET, Reset on MR0 when MRxx match
 * 									the value in TC
 * 					-ExtMatchOutputType: Select output for external match
 * 						 +	 0:	Do nothing for external output pin if match
 *						 +   1:	Force external output pin to low if match
 *						 + 	 2: Force external output pin to high if match
 *						 + 	 3: Toggle external output pin if match
 *					MatchValue: Set the value to be compared with TC value
 * @return 		None
 **********************************************************************/
void TIM_ConfigMatch(LPC_TIM_TypeDef *TIMx, TIM_MATCHCFG_Type *TIM_MatchConfigStruct)
{
	uint32_t timer;

	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_EXTMATCH_OPT(TIM_MatchConfigStruct->ExtMatchOutputType));

	timer = TIM_ConverPtrToTimeNum(TIMx) ;
	switch(TIM_MatchConfigStruct->MatchChannel)
	{
	case 0:
		TIMx->MR0 = TIM_MatchConfigStruct->MatchValue;
		break;
	case 1:
		TIMx->MR1 = TIM_MatchConfigStruct->MatchValue;
		break;
	}
	//interrupt on MRn
	TIMx->MCR &=~TIM_MCR_CHANNEL_MASKBIT(TIM_MatchConfigStruct->MatchChannel);

	if (TIM_MatchConfigStruct->IntOnMatch)
		TIMx->MCR |= TIM_INT_ON_MATCH(TIM_MatchConfigStruct->MatchChannel);

	//reset on MRn
	if (TIM_MatchConfigStruct->ResetOnMatch)
		TIMx->MCR |= TIM_RESET_ON_MATCH(TIM_MatchConfigStruct->MatchChannel);

	//stop on MRn
	if (TIM_MatchConfigStruct->StopOnMatch)
		TIMx->MCR |= TIM_STOP_ON_MATCH(TIM_MatchConfigStruct->MatchChannel);
//	TIMx->MCR = 0x02;

	// match output type

	TIMx->EMR 	&= ~TIM_EM_MASK(TIM_MatchConfigStruct->MatchChannel);
	TIMx->EMR    = TIM_EM_SET(TIM_MatchConfigStruct->MatchChannel,TIM_MatchConfigStruct->ExtMatchOutputType);
}
/*********************************************************************//**
 * @brief 		Configuration for Capture register
 * @param[in]	TIMx Pointer to timer device
 * 					- CaptureChannel: set the channel to capture data
 * 					- RisingEdge    : if SET, Capture at rising edge
 * 					- FallingEdge	: if SET, Capture at falling edge
 * 					- IntOnCaption  : if SET, Capture generate interrupt
 * @param[in]   TIM_CaptureConfigStruct	Pointer to TIM_CAPTURECFG_Type
 * @return 		None
 **********************************************************************/
void TIM_ConfigCapture(LPC_TIM_TypeDef *TIMx, TIM_CAPTURECFG_Type *TIM_CaptureConfigStruct)
{
	uint32_t timer;

	CHECK_PARAM(PARAM_TIMx(TIMx));
	timer = TIM_ConverPtrToTimeNum(TIMx) ;
	TIMx->CCR &= ~TIM_CCR_CHANNEL_MASKBIT(TIM_CaptureConfigStruct->CaptureChannel);

	if (TIM_CaptureConfigStruct->RisingEdge)
		TIMx->CCR |= TIM_CAP_RISING(TIM_CaptureConfigStruct->CaptureChannel);

	if (TIM_CaptureConfigStruct->FallingEdge)
		TIMx->CCR |= TIM_CAP_FALLING(TIM_CaptureConfigStruct->CaptureChannel);

	if (TIM_CaptureConfigStruct->IntOnCaption)
		TIMx->CCR |= TIM_INT_ON_CAP(TIM_CaptureConfigStruct->CaptureChannel);
}

/*********************************************************************//**
 * @brief 		Read value of capture register in timer/counter device
 * @param[in]	TIMx Pointer to timer/counter device
 * @param[in]	CaptureChannel: capture channel number
 * @return 		Value of capture register
 **********************************************************************/
uint32_t TIM_GetCaptureValue(LPC_TIM_TypeDef *TIMx, uint8_t CaptureChannel)
{
	CHECK_PARAM(PARAM_TIMx(TIMx));
	CHECK_PARAM(PARAM_TIM_COUNTER_INPUT_OPT(CaptureChannel));

	if(CaptureChannel==0)
		return TIMx->CR0;
	else
		return TIMx->CR1;
}

/**
 * @}
 */

#endif /* _TIMER */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */
