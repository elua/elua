/**
 * @file	: lpc17xx_dac.c
 * @brief	: Contains all functions support for DAC firmware library on LPC17xx
 * @version	: 1.0
 * @date	: 3. April. 2009
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
/** @addtogroup DAC
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc17xx_dac.h"
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


#ifdef _DAC

/* Public Functions ----------------------------------------------------------- */
/** @addtogroup DAC_Public_Functions
 * @{
 */

/*********************************************************************//**
 * @brief 		Initial ADC configuration
 * 					- Maximum	current is 700 uA
 * 					- Value to AOUT is 0
 * @param[in] 	DACx pointer to LPC_DAC_TypeDef
 * @return 		None
 ***********************************************************************/
void 	DAC_Init(LPC_DAC_TypeDef *DACx)
{
	CHECK_PARAM(PARAM_DACx(DACx));
	/* Set default clock divider for DAC */
	// CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_DAC, CLKPWR_PCLKSEL_CCLK_DIV_4);
	//Set maximum current output
	DAC_SetBias(LPC_DAC,DAC_MAX_CURRENT_700uA);


}

/*********************************************************************//**
 * @brief 		Update value to DAC
 * @param[in] 	DACx pointer to LPC_DAC_TypeDef
 * @param[in] 	dac_value : value 10 bit to be converted to output
 * @return 		None
 ***********************************************************************/
void    DAC_UpdateValue (LPC_DAC_TypeDef *DACx,uint32_t dac_value)
{
	uint32_t tmp;
	CHECK_PARAM(PARAM_DACx(DACx));
	tmp = DACx->DACR & DAC_BIAS_EN;
	tmp |= DAC_VALUE(dac_value);
	//DACx->DACR	&= ~DAC_VALUE(0x3FF);
	//DACx->DACR 	|=  DAC_VALUE(dac_value);
	// Update value
	DACx->DACR = tmp;
}

/*********************************************************************//**
 * @brief 		Set Maximum current for DAC
 * @param[in] 	DACx pointer to LPC_DAC_TypeDef
 * @param[in] 	bias : 0 is 700 uA
 * 					   1    350 uA
 * @return 		None
 ***********************************************************************/
void    DAC_SetBias (LPC_DAC_TypeDef *DACx,uint32_t bias)
{
	CHECK_PARAM(PARAM_DAC_CURRENT_OPT(bias));
	DACx->DACR &=~DAC_BIAS_EN;
	if (bias  == DAC_MAX_CURRENT_350uA)
	{
		DACx->DACR |= DAC_BIAS_EN;
	}
}
/*********************************************************************//**
 * @brief 		To enable the DMA operation and control DMA timer
 * @param[in]	DACx pointer to LPC_DAC_TypeDef
 * @param[in] 	DAC_ConverterConfigStruct pointer to DAC_CONVERTER_CFG_Type
 * 					- DBLBUF_ENA :  enable/disable DACR double buffering feature
 * 					- CNT_ENA    :  enable/disable timer out counter
 * 					- DMA_ENA    :	enable/disable DMA access
 * @return 		None
 ***********************************************************************/
void    DAC_ConfigDAConverterControl (LPC_DAC_TypeDef *DACx,DAC_CONVERTER_CFG_Type *DAC_ConverterConfigStruct)
{
	CHECK_PARAM(PARAM_DACx(DACx));
	DACx->DACCTRL &= ~DAC_DACCTRL_MASK;
	if (DAC_ConverterConfigStruct->DBLBUF_ENA)
		DACx->DACCTRL	|= DAC_DBLBUF_ENA;
	if (DAC_ConverterConfigStruct->CNT_ENA)
		DACx->DACCTRL	|= DAC_CNT_ENA;
	if (DAC_ConverterConfigStruct->DMA_ENA)
		DACx->DACCTRL	|= DAC_DMA_ENA;
}
/*********************************************************************//**
 * @brief 		Set reload value for interrupt/DMA counter
 * @param[in] 	DACx pointer to LPC_DAC_TypeDef
 * @param[in] 	time_out time out to reload for interrupt/DMA counter
 * @return 		None
 ***********************************************************************/
void 	DAC_SetDMATimeOut(LPC_DAC_TypeDef *DACx, uint32_t time_out)
{
	CHECK_PARAM(PARAM_DACx(DACx));
	DACx->DACCNTVAL = DAC_CCNT_VALUE(time_out);
}

/**
 * @}
 */

#endif /* _DAC */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */
