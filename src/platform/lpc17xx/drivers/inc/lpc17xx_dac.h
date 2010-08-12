/***********************************************************************//**
 * @file	: lpc17xx_dac.h
 * @brief	: Contains all macro definitions and function prototypes
 * 				support for DAC firmware library on LPC17xx
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
 **************************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @defgroup DAC
 * @ingroup LPC1700CMSIS_FwLib_Drivers
 * @{
 */

#ifndef LPC17XX_DAC_H_
#define LPC17XX_DAC_H_

/* Includes ------------------------------------------------------------------- */
#include "LPC17xx.h"
#include "lpc_types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/* Public Types --------------------------------------------------------------- */
/** @defgroup DAC_Public_Types
 * @{
 */

/**
 * @brief Current option in ADC confuration option */
typedef enum
{
	DAC_MAX_CURRENT_700uA = 0, 	/*!< The settling time of the DAC is 1 us max,
								and the maximum	current is 700 uA */
	DAC_MAX_CURRENT_350uA		/*!< The settling time of the DAC is 2.5 us
								and the maximum current is 350 uA */
} DAC_CURRENT_OPT;
#define	PARAM_DAC_CURRENT_OPT(OPTION) ((OPTION == DAC_MAX_CURRENT_700uA)\
||(OPTION == DAC_MAX_CURRENT_350uA))

/**
 * @brief Configuration for DA converter control register */
typedef struct
{

	uint8_t  DBLBUF_ENA; 	/**<
	                         -0: Disable DACR double buffering
	                         -1: when bit CNT_ENA, enable DACR double buffering feature
							 */
	uint8_t  CNT_ENA;		/*!<
	                         -0: Time out counter is disable
	                         -1: Time out conter is enable
							 */
	uint8_t  DMA_ENA;		/*!<
		                         -0: DMA access is disable
		                         -1: DMA burst request
							*/
	uint8_t RESERVED;

} DAC_CONVERTER_CFG_Type;

/**
 * @}
 */


/* Public Macros -------------------------------------------------------------- */
/** @defgroup DAC_Public_Macros
 * @{
 */

/** After the selected settling time after this field is written with a
new VALUE, the voltage on the AOUT pin (with respect to VSSA)
is VALUE/1024 × VREF */
#define DAC_VALUE(n) 		((uint32_t)((n&0x3FF)<<6))
/** If this bit = 0: The settling time of the DAC is 1 microsecond max,
 * and the maximum current is 700 microAmpere
 * If this bit = 1: The settling time of the DAC is 2.5 microsecond
 * and the maximum current is 350 microAmpere */
#define DAC_BIAS_EN			((uint32_t)(1<<16))
/** Value to reload interrupt DMA counter */
#define DAC_CCNT_VALUE(n)  ((uint32_t)(n&0xff))

/** DCAR double buffering */
#define DAC_DBLBUF_ENA		((uint32_t)(1<<1))
/** DCAR Time out count enable */
#define DAC_CNT_ENA			((uint32_t)(1<<2))
/** DCAR DMA access */
#define DAC_DMA_ENA			((uint32_t)(1<<3))
/** DCAR DACCTRL mask bit */
#define DAC_DACCTRL_MASK	((uint32_t)(0x0F))

/** Macro to determine if it is valid DAC peripheral */
#define PARAM_DACx(n)	(((uint32_t *)n)==((uint32_t *)LPC_DAC))

/**
 * @}
 */

/* Public Functions ----------------------------------------------------------- */
/** @defgroup DAC_Public_Functions
 * @{
 */

void 	DAC_Init(LPC_DAC_TypeDef *DACx);
void    DAC_UpdateValue (LPC_DAC_TypeDef *DACx, uint32_t dac_value);
void    DAC_SetBias (LPC_DAC_TypeDef *DACx,uint32_t bias);
void    DAC_ConfigDAConverterControl (LPC_DAC_TypeDef *DACx,DAC_CONVERTER_CFG_Type *DAC_ConverterConfigStruct);
void 	DAC_SetDMATimeOut(LPC_DAC_TypeDef *DACx,uint32_t time_out);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* LPC17XX_DAC_H_ */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */

