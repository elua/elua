/**
 * @file	: lpc17xx_adc.c
 * @brief	: Contains all functions support for ADC firmware library on LPC17xx
 * @version	: 1.0
 * @date	: 3. April. 2009
 * @author	: NgaDinh
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
/** @addtogroup ADC
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc17xx_adc.h"
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


#ifdef _ADC

/* Public Functions ----------------------------------------------------------- */
/** @addtogroup ADC_Public_Functions
 * @{
 */

/*********************************************************************//**
 * @brief 		Initial for ADC
 * 					- Set bit PCADC
 * 					- Set clock for ADC
 * 					- Set Clock Frequency
 *
 * @param[in]	ADCx pointer to LPC_ADC_TypeDef
 * @param[in]	ConvFreq Clock frequency
 * @return 		None
 **********************************************************************/
void ADC_Init(LPC_ADC_TypeDef *ADCx, uint32_t ConvFreq)

{
	uint32_t temp, tmp;

	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_FREQUENCY(ConvFreq));

	// Turn on power and clock
	CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCAD, ENABLE);
	// Set clock divider for ADC to 4 from CCLK as default
	// CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_ADC,CLKPWR_PCLKSEL_CCLK_DIV_4);

	ADCx->ADCR = 0;

	//Enable PDN bit
	tmp = ADC_CR_PDN;
	// Set clock frequency
	temp = CLKPWR_GetPCLK(CLKPWR_PCLKSEL_ADC) ;
	temp = (temp /ConvFreq) - 1;
	tmp |=  ADC_CR_CLKDIV(temp);

	ADCx->ADCR = tmp;
}


/*********************************************************************//**
* @brief 		Close ADC
* @param[in]	ADCx pointer to ADC
* @return 		None
**********************************************************************/
void 		ADC_DeInit(LPC_ADC_TypeDef *ADCx)
{
	CHECK_PARAM(PARAM_ADCx(ADCx));

	// Clear PDN bit
	ADCx->ADCR &= ~ADC_CR_PDN;
	// Turn on power and clock
	CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCAD, DISABLE);
}


/*********************************************************************//**
* @brief 		Get Result conversion from A/D data register
* @param[in]	channel number which want to read back the result
* @return 		Result of conversion
*********************************************************************/
uint32_t ADC_GetData(uint32_t channel)
{
	uint32_t adc_value;

	CHECK_PARAM(PARAM_ADC_CHANNEL_SELECTION(channel));

	adc_value = *(uint32_t *)((&LPC_ADC->ADDR0) + channel);
	return ADC_GDR_RESULT(adc_value);
}

/*********************************************************************//**
* @brief 		Set start mode for ADC
* @param[in]	ADCx pointer to LPC_ADC_TypeDef
* @param[in]	start_mode Start mode choose one of modes in
* 							'ADC_START_OPT' enumeration type definition
* @return 		None
*********************************************************************/
void ADC_StartCmd(LPC_ADC_TypeDef *ADCx, uint8_t start_mode)
{
	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_START_OPT(start_mode));

	ADCx->ADCR &= ~ADC_CR_START_MASK;
	ADCx->ADCR |=ADC_CR_START_MODE_SEL((uint32_t)start_mode);
}


/*********************************************************************//**
* @brief 		ADC Burst mode setting
*
* @param[in]	ADCx pointer to ADC
* @param[in]	NewState
* 				-	1: Set Burst mode
* 				-	0: reset Burst mode
* @return 		None
**********************************************************************/
void ADC_BurstCmd(LPC_ADC_TypeDef *ADCx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_ADCx(ADCx));

	ADCx->ADCR &= ~ADC_CR_BURST;
	if (NewState){
		ADCx->ADCR |= ADC_CR_BURST;
	}

}

/*********************************************************************//**
* @brief 		Set AD conversion in power mode
*
* @param[in]	ADCx pointer to ADC
* @param[in]	NewState
* 				-	1: AD converter is optional
* 				-	0: AD Converter is in power down mode
* @return 		None
**********************************************************************/
void ADC_PowerdownCmd(LPC_ADC_TypeDef *ADCx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_ADCx(ADCx));

	ADCx->ADCR &= ~ADC_CR_PDN;
	if (NewState){
		ADCx->ADCR |= ADC_CR_PDN;
	}
}

/*********************************************************************//**
* @brief 		Set Edge start configuration
*
* @param[in]	ADCx pointer to ADC
* @param[in]	EdgeOption is ADC_START_ON_RISING and ADC_START_ON_FALLING
* 					0:ADC_START_ON_RISING
* 					1:ADC_START_ON_FALLING
*
* @return 		None
**********************************************************************/
void ADC_EdgeStartConfig(LPC_ADC_TypeDef *ADCx, uint8_t EdgeOption)
{
	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_START_ON_EDGE_OPT(EdgeOption));

	ADCx->ADCR &= ~ADC_CR_EDGE;
	if (EdgeOption){
		ADCx->ADCR |= ADC_CR_EDGE;
	}
}

/*********************************************************************//**
* @brief 		ADC interrupt configuration
* @param[in]	ADCx pointer to ADC
* @param[in]	IntType
* @param[in]	NewState:
* 					- SET : enable ADC interrupt
* 					- RESET: disable ADC interrupt
*
* @return 		None
**********************************************************************/
void ADC_IntConfig (LPC_ADC_TypeDef *ADCx, ADC_TYPE_INT_OPT IntType, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_TYPE_INT_OPT(IntType));

	ADCx->ADINTEN &= ~ADC_INTEN_CH(IntType);
	if (NewState){
		ADCx->ADINTEN |= ADC_INTEN_CH(IntType);
	}
}

/*********************************************************************//**
* @brief 		Enable/Disable ADC channel number
* @param[in]	ADCx pointer to ADC
* @param[in]	Channel channel number
* @param[in]	NewState Enable or Disable
*
* @return 		None
**********************************************************************/
void ADC_ChannelCmd (LPC_ADC_TypeDef *ADCx, uint8_t Channel, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_CHANNEL_SELECTION(Channel));

	if (NewState == ENABLE) {
		ADCx->ADCR |= ADC_CR_CH_SEL(Channel);
	} else {
		ADCx->ADCR &= ~ADC_CR_CH_SEL(Channel);
	}
}

/*********************************************************************//**
* @brief 		Get ADC result
* @param[in]	ADCx pointer to ADC
* @param[in]	channel channel number
* @return 		Data conversion
**********************************************************************/
uint16_t ADC_ChannelGetData(LPC_ADC_TypeDef *ADCx, uint8_t channel)
{
	uint32_t adc_value;

	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_CHANNEL_SELECTION(channel));

	adc_value = *(uint32_t *) ((&ADCx->ADDR0) + channel);
	return ADC_DR_RESULT(adc_value);
}

/*********************************************************************//**
* @brief 		Get ADC Chanel status from ADC data register
* @param[in]	ADCx pointer to ADC
* @param[in]	channel channel number
* @param[in]  	StatusType
*              		 	0:Burst status
*               		1:Done 	status
* @return 		SET / RESET
**********************************************************************/
FlagStatus ADC_ChannelGetStatus(LPC_ADC_TypeDef *ADCx, uint8_t channel, uint32_t StatusType)
{
	uint32_t temp;

	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_CHANNEL_SELECTION(channel));
	CHECK_PARAM(PARAM_ADC_DATA_STATUS(StatusType));

	temp =  *(uint32_t *) ((&ADCx->ADDR0) + channel);
	if (StatusType) {
		temp &= ADC_DR_DONE_FLAG;
	}else{
		temp &= ADC_DR_OVERRUN_FLAG;
	}
	if (temp) {
		return SET;
	} else {
		return RESET;
	}

}

/*********************************************************************//**
* @brief 		Get ADC Data from AD Global register
* @param[in]	ADCx pointer to ADC
* @param[in]	channel channel number
* @return 		Result of conversion
**********************************************************************/
uint16_t ADC_GlobalGetData(LPC_ADC_TypeDef *ADCx, uint8_t channel)
{
	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_CHANNEL_SELECTION(channel));

	//ADCx->ADGDR &= ~ADC_GDR_CH_MASK;
	//ADCx->ADGDR |= ADC_GDR_CH(channel);
	return (uint16_t)(ADC_GDR_RESULT(ADCx->ADGDR));
}

/*********************************************************************//**
* @brief 		Get ADC Chanel status from AD global data register
* @param[in]	ADCx pointer to ADC
* @param[in]  	StatusType
*              		 	0:Burst status
*               		1:Done 	status
* @return 		SET / RESET
**********************************************************************/
FlagStatus	ADC_GlobalGetStatus(LPC_ADC_TypeDef *ADCx, uint32_t StatusType)
{
	uint32_t temp;

	CHECK_PARAM(PARAM_ADCx(ADCx));
	CHECK_PARAM(PARAM_ADC_DATA_STATUS(StatusType));

	temp =  ADCx->ADGDR;
	if (StatusType){
		temp &= ADC_DR_DONE_FLAG;
	}else{
		temp &= ADC_DR_OVERRUN_FLAG;
	}
	if (temp){
		return SET;
	}else{
		return RESET;
	}
}

/**
 * @}
 */

#endif /* _ADC */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */

