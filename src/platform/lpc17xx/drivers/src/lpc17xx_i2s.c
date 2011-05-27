/**
 * @file	: lpc17xx_i2s.c
 * @brief	: Contains all functions support for I2S firmware library on LPC17xx
 * @version	: 1.0
 * @date	: 13. May. 2009
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
/** @addtogroup I2S
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc17xx_i2s.h"
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


#ifdef _I2S

/* Private Variables ---------------------------------------------------------- */
static fnI2SCbs_Type *_apfnI2SCbs[2] = {
		NULL, 	// I2S transmit Call-back function pointer
		NULL, 	// I2S receive Call-back function pointer
};


/* Private Functions ---------------------------------------------------------- */
/** @defgroup I2S_Private_Functions
 * @{
 */

/********************************************************************//**
 * @brief		Get I2S wordwidth value
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode is the I2S mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		The wordwidth value, should be: 8,16 or 32
 *********************************************************************/
uint8_t I2S_GetWordWidth(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode) {
	uint8_t value;

	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if (TRMode == I2S_TX_MODE) {
		value = (I2Sx->I2SDAO) & 0x03; /* get wordwidth bit */
	} else {
		value = (I2Sx->I2SDAI) & 0x03; /* get wordwidth bit */
	}
	switch (value) {
	case I2S_WORDWIDTH_8:
		return 8;
		break;
	case I2S_WORDWIDTH_16:
		return 16;
		break;
	default:
		return 32;
	}
}
/********************************************************************//**
 * @brief		Get I2S channel value
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode is the I2S mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		The channel value, should be: 1(mono) or 2(stereo)
 *********************************************************************/
uint8_t I2S_GetChannel(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode) {
	uint8_t value;

	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if (TRMode == I2S_TX_MODE) {
		value = (I2Sx->I2SDAO) & 0x04; /* get bit[2] */
	} else {
		value = (I2Sx->I2SDAI) & 0x04; /* get bit[2] */
	}
	switch (value) {
	case I2S_MONO:
		return 1;
		break;
	default:
		return 2;
	}
}

/**
 * @}
 */


/* Public Functions ----------------------------------------------------------- */
/** @addtogroup I2S_Public_Functions
 * @{
 */

/********************************************************************//**
 * @brief		Initialize I2S
 * 					- Turn on power and clock
 * 					- Setup I2S pin select
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @return 		none
 *********************************************************************/
void I2S_Init(LPC_I2S_TypeDef *I2Sx) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));

	// Turn on power and clock
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCI2S, ENABLE);
	LPC_I2S->I2SDAI = LPC_I2S->I2SDAO = 0x00;
}

/********************************************************************//**
 * @brief		Configuration I2S, setting:
 * 					- master/slave mode
 * 					- wordwidth value
 * 					- channel mode
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode: transmit/receive mode, should be:
 * 					- I2S_TX_MODE: transmit mode
 * 					- I2S_RX_MODE: receive mode
 * @param[in]	ConfigStruct pointer to I2S_CFG_Type structure
 *              which will be initialized.
 * @return 		none
 *********************************************************************/
void I2S_Config(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode, I2S_CFG_Type* ConfigStruct)
{
	uint32_t bps, config;

	CHECK_PARAM(PARAM_I2Sx(I2Sx));

	CHECK_PARAM(PARAM_I2S_WORDWIDTH(ConfigStruct->wordwidth));
	CHECK_PARAM(PARAM_I2S_CHANNEL(ConfigStruct->mono));
	CHECK_PARAM(PARAM_I2S_STOP(ConfigStruct->stop));
	CHECK_PARAM(PARAM_I2S_RESET(ConfigStruct->reset));
	CHECK_PARAM(PARAM_I2S_WS_SEL(ConfigStruct->ws_sel));
	CHECK_PARAM(PARAM_I2S_MUTE(ConfigStruct->mute));

	/* Setup clock */
	bps = (ConfigStruct->wordwidth +1)*8;

	/* Calculate audio config */
	config = (bps - 1)<<6 | (ConfigStruct->ws_sel)<<5 | (ConfigStruct->reset)<<4 |
		(ConfigStruct->stop)<<3 | (ConfigStruct->mono)<<2 | (ConfigStruct->wordwidth);

	if(TRMode == I2S_RX_MODE){
		LPC_I2S->I2SDAI = config;
	}else{
		LPC_I2S->I2SDAO = config;
	}
}

/********************************************************************//**
 * @brief		DeInitial both I2S transmit or receive
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @return 		none
 *********************************************************************/
void I2S_DeInit(LPC_I2S_TypeDef *I2Sx) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));

	// Turn off power and clock
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCI2S, DISABLE);
}

/********************************************************************//**
 * @brief		Get I2S Buffer Level
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode: Transmit/receive mode, should be:
 * 					- I2S_TX_MODE: transmit mode
 * 					- I2S_RX_MODE: receive mode
 * @return 		current level of Transmit/Receive Buffer
 *********************************************************************/
uint8_t I2S_GetLevel(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode)
{
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if(TRMode == I2S_TX_MODE)
	{
		return ((I2Sx->I2SSTATE >> 16) & 0xFF);
	}
	else
	{
		return ((I2Sx->I2SSTATE >> 8) & 0xFF);
	}
}
/********************************************************************//**
 * @brief		I2S Start: clear all STOP,RESET and MUTE bit, ready to operate
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @return 		none
 *********************************************************************/
void I2S_Start(LPC_I2S_TypeDef *I2Sx)
{
	//Clear STOP,RESET and MUTE bit
	I2Sx->I2SDAO &= ~I2S_DAI_RESET;
	I2Sx->I2SDAI &= ~I2S_DAI_RESET;
	I2Sx->I2SDAO &= ~I2S_DAI_STOP;
	I2Sx->I2SDAI &= ~I2S_DAI_STOP;
	I2Sx->I2SDAO &= ~I2S_DAI_MUTE;
}
/********************************************************************//**
 * @brief		I2S Send data
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	BufferData pointer to uint32_t is the data will be send
 * @return 		none
 *********************************************************************/
void I2S_Send(LPC_I2S_TypeDef *I2Sx, uint32_t BufferData) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_DATA(BufferData));

	I2Sx->I2STXFIFO = BufferData;
}

/********************************************************************//**
 * @brief		I2S Receive Data
 * @param[in]	I2Sx pointer to LPC_I2S_TypeDef
 * @return 		received value
 *********************************************************************/
uint32_t I2S_Receive(LPC_I2S_TypeDef* I2Sx) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));

	return (I2Sx->I2SRXFIFO);

}
/********************************************************************//**
 * @brief		I2S Pause
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		none
 *********************************************************************/
void I2S_Pause(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if (TRMode == I2S_TX_MODE) //Transmit mode
	{
		I2Sx->I2SDAO |= I2S_DAO_STOP;
	} else //Receive mode
	{
		I2Sx->I2SDAI |= I2S_DAI_STOP;
	}
}
/********************************************************************//**
 * @brief		I2S Mute
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		none
 *********************************************************************/
void I2S_Mute(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if (TRMode == I2S_TX_MODE) //Transmit mode
	{
		I2Sx->I2SDAO |= I2S_DAO_MUTE;
	} else //Receive mode
	{
		I2Sx->I2SDAI |= I2S_DAI_MUTE;
	}
}

/********************************************************************//**
 * @brief		I2S Stop
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		none
 *********************************************************************/
void I2S_Stop(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if (TRMode == I2S_TX_MODE) //Transmit mode
	{
		I2Sx->I2SDAO &= ~I2S_DAO_MUTE;
		I2Sx->I2SDAO |= I2S_DAO_STOP;
		I2Sx->I2SDAO |= I2S_DAO_RESET;
	} else //Receive mode
	{
		I2Sx->I2SDAI |= I2S_DAI_STOP;
		I2Sx->I2SDAI |= I2S_DAI_RESET;
	}
}

/********************************************************************//**
 * @brief		Set frequency for I2S
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	Freq is the frequency for I2S will be set. It can range
 * 				from 16-96 kHz(16, 22.05, 32, 44.1, 48, 96kHz)
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		Status: ERROR or SUCCESS
 *********************************************************************/
Status I2S_FreqConfig(LPC_I2S_TypeDef *I2Sx, uint32_t Freq, uint8_t TRMode) {

	/* Calculate bit rate
	 * The formula is:
	 *      bit_rate = channel*wordwidth - 1
	 * 48kHz sample rate for 16 bit stereo date requires
	 * a bit rate of 48000*16*2=1536MHz (MCLK)
	 */
	uint32_t i2sPclk;
	uint64_t divider;
	uint8_t bitrate, channel, wordwidth;
	uint32_t x, y;
	uint16_t dif;
	uint16_t error;
	uint8_t x_divide, y_divide;
	uint16_t ErrorOptimal = 0xFFFF;

	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PRAM_I2S_FREQ(Freq));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	i2sPclk = CLKPWR_GetPCLK(CLKPWR_PCLKSEL_I2S);
	if(TRMode == I2S_TX_MODE)
	{
		channel = I2S_GetChannel(I2Sx,I2S_TX_MODE);
		wordwidth = I2S_GetWordWidth(I2Sx,I2S_TX_MODE);
	}
	else
	{
		channel = I2S_GetChannel(I2Sx,I2S_RX_MODE);
		wordwidth = I2S_GetWordWidth(I2Sx,I2S_RX_MODE);
	}
	bitrate = channel * wordwidth - 1;
	if (TRMode == I2S_TX_MODE)// Transmitter
	{
		I2Sx->I2STXBITRATE = bitrate;
	} else //Receiver
	{
		I2Sx->I2SRXBITRATE = bitrate;
	}
	/* Calculate X and Y divider
	 * The MCLK rate for the I2S transmitter is determined by the value
	 * in the I2STXRATE/I2SRXRATE register. The required I2STXRATE/I2SRXRATE
	 * setting depends on the desired audio sample rate desired, the format
	 * (stereo/mono) used, and the data size.
	 * The formula is:
	 * 		I2S_MCLK = PCLK * (X/Y) / 2
	 * We have:
	 * 		I2S_MCLK = Freq * bit_rate;
	 * So: (X/Y) = (Freq * bit_rate)/PCLK*2
	 * We use a loop function to chose the most suitable X,Y value
	 */

	divider = ((uint64_t)(Freq *( bitrate+1)* 2)<<16) / i2sPclk;
	for (y = 255; y > 0; y--) {
		x = y * divider;
		dif = x & 0xFFFF;
		if(dif>0x8000) error = 0x10000-dif;
		else error = dif;
		if (error == 0)
		{
			y_divide = y;
			break;
		}
		else if (error < ErrorOptimal)
		{
			ErrorOptimal = error;
			y_divide = y;
		}
	}
	x_divide = (y_divide * Freq *( bitrate+1)* 2)/i2sPclk;
	if (TRMode == I2S_TX_MODE)// Transmitter
	{
		I2Sx->I2STXRATE = y_divide | (x_divide << 8);
	} else //Receiver
	{
		I2Sx->I2SRXRATE = y_divide | (x_divide << 8);
	}
	return SUCCESS;
}
/********************************************************************//**
 * @brief		I2S set bitrate
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	bitrate value will be set
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		none
 *********************************************************************/
void I2S_SetBitRate(LPC_I2S_TypeDef *I2Sx, uint8_t bitrate, uint8_t TRMode)
{
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_BITRATE(bitrate));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if(TRMode == I2S_TX_MODE)
	{
		I2Sx->I2STXBITRATE = (bitrate -1);
	}
	else
	{
		I2Sx->I2SRXBITRATE = (bitrate -1);
	}
}
/********************************************************************//**
 * @brief		Configuration operating mode for I2S
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	ModeConfig pointer to I2S_MODEConf_Type will be used to
 * 				configure, should be:
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		none
 *********************************************************************/
void I2S_ModeConfig(LPC_I2S_TypeDef *I2Sx, I2S_MODEConf_Type* ModeConfig,
		uint8_t TRMode)
{
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_CLKSEL(ModeConfig->clksel));
	CHECK_PARAM(PARAM_I2S_4PIN(ModeConfig->fpin));
	CHECK_PARAM(PARAM_I2S_MCLK(ModeConfig->mcena));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if (TRMode == I2S_TX_MODE) {
		I2Sx->I2STXMODE &= ~0x0F; //clear bit 3:0 in I2STXMODE register
		if (ModeConfig->clksel == I2S_CLKSEL_1) {
			I2Sx->I2STXMODE |= 0x02;
		}
		if (ModeConfig->fpin == I2S_4PIN_ENABLE) {
			I2Sx->I2STXMODE |= (1 << 2);
		}
		if (ModeConfig->mcena == I2S_MCLK_ENABLE) {
			I2Sx->I2STXMODE |= (1 << 3);
		}
	} else {
		I2Sx->I2SRXMODE &= ~0x0F; //clear bit 3:0 in I2STXMODE register
		if (ModeConfig->clksel == I2S_CLKSEL_1) {
			I2Sx->I2SRXMODE |= 0x02;
		}
		if (ModeConfig->fpin == I2S_4PIN_ENABLE) {
			I2Sx->I2SRXMODE |= (1 << 2);
		}
		if (ModeConfig->mcena == I2S_MCLK_ENABLE) {
			I2Sx->I2SRXMODE |= (1 << 3);
		}
	}
}

/********************************************************************//**
 * @brief		Configure DMA operation for I2S
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	DMAConfig pointer to I2S_DMAConf_Type will be used to configure
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @return 		none
 *********************************************************************/
void I2S_DMAConfig(LPC_I2S_TypeDef *I2Sx, I2S_DMAConf_Type* DMAConfig,
		uint8_t TRMode)
{
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_DMA(DMAConfig->DMAIndex));
	CHECK_PARAM(PARAM_I2S_DMA_DEPTH(DMAConfig->depth));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if (TRMode == I2S_RX_MODE) {
		if (DMAConfig->DMAIndex == I2S_DMA_1) {
			LPC_I2S->I2SDMA1 = (DMAConfig->depth) << 8;
		} else {
			LPC_I2S->I2SDMA2 = (DMAConfig->depth) << 8;
		}
	} else {
		if (DMAConfig->DMAIndex == I2S_DMA_1) {
			LPC_I2S->I2SDMA1 = (DMAConfig->depth) << 16;
		} else {
			LPC_I2S->I2SDMA2 = (DMAConfig->depth) << 16;
		}
	}
}

/********************************************************************//**
 * @brief		Enable/Disable DMA operation for I2S
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	DMAIndex chose what DMA is used, should be:
 * 				- I2S_DMA_1: DMA1
 * 				- I2S_DMA_2: DMA2
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @param[in]	NewState is new state of DMA operation, should be:
 * 				- ENABLE
 * 				- DISABLE
 * @return 		none
 *********************************************************************/
void I2S_DMACmd(LPC_I2S_TypeDef *I2Sx, uint8_t DMAIndex, uint8_t TRMode,
		FunctionalState NewState)
{
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));
	CHECK_PARAM(PARAM_I2S_DMA(DMAIndex));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));

	if (TRMode == I2S_RX_MODE) {
		if (DMAIndex == I2S_DMA_1) {
			if (NewState == ENABLE)
				I2Sx->I2SDMA1 |= 0x01;
			else
				I2Sx->I2SDMA1 &= ~0x01;
		} else {
			if (NewState == ENABLE)
				I2Sx->I2SDMA2 |= 0x01;
			else
				I2Sx->I2SDMA2 &= ~0x01;
		}
	} else {
		if (DMAIndex == I2S_DMA_1) {
			if (NewState == ENABLE)
				I2Sx->I2SDMA1 |= 0x02;
			else
				I2Sx->I2SDMA1 &= ~0x02;
		} else {
			if (NewState == ENABLE)
				I2Sx->I2SDMA2 |= 0x02;
			else
				I2Sx->I2SDMA2 &= ~0x02;
		}
	}
}

/********************************************************************//**
 * @brief		Configure IRQ for I2S
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @param[in]	level: is the FIFO level that triggers IRQ request
 * @param[in]	pfnI2SCbs: the pointer to call-back function handle this interrupt
 * @return 		none
 *********************************************************************/
void I2S_IRQConfig(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode, uint8_t level,  fnI2SCbs_Type *pfnI2SCbs) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_I2S_TRX(TRMode));
	CHECK_PARAM(PARAM_I2S_IRQ_LEVEL(level));

	if (TRMode == I2S_RX_MODE) {
		I2Sx->I2SIRQ |= (level << 8);
	} else {
		I2Sx->I2SIRQ |= (level << 16);
	}
	//setup Call-Back funtion for receive interrupt
	_apfnI2SCbs[TRMode] = pfnI2SCbs;
}

/********************************************************************//**
 * @brief		Enable/Disable IRQ for I2S
 * @param[in]	I2Sx: I2S peripheral selected, should be: I2S
 * @param[in]	TRMode is transmit/receive mode, should be:
 * 				- I2S_TX_MODE: transmit mode
 * 				- I2S_RX_MODE: receive mode
 * @param[in]	NewState is new state of DMA operation, should be:
 * 				- ENABLE
 * 				- DISABLE
 * @return 		none
 *********************************************************************/
void I2S_IRQCmd(LPC_I2S_TypeDef *I2Sx, uint8_t TRMode, FunctionalState NewState) {
	CHECK_PARAM(PARAM_I2Sx(I2Sx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	if (TRMode == I2S_RX_MODE) {
		if (NewState == ENABLE)
			I2Sx->I2SIRQ |= 0x01;
		else
			I2Sx->I2SIRQ &= ~0x01;
		//Enable DMA

	} else {
		if (NewState == ENABLE)
			I2Sx->I2SIRQ |= 0x02;
		else
			I2Sx->I2SIRQ &= ~0x02;
	}
}
/*********************************************************************//**
 * @brief		Standard I2S interrupt handler, this function will check
 * 				all interrupt status of I2S channels, then execute the call
 * 				back function if they're already installed
 * @param[in]	None
 * @return		None
 **********************************************************************/
void I2S_IntHandler(void)
{
	uint8_t rx_level,
			tx_level,
			tx_depth_irq,
			rx_depth_irq;


	if((LPC_I2S->I2SIRQ)& 0x01){ //receive interrupt
		rx_level = ((LPC_I2S->I2SSTATE)>>8)&0xFF;
		rx_depth_irq = ((LPC_I2S->I2SIRQ)>>8)&0xFF;

		if (rx_level >= rx_depth_irq)//receive interrupt
		{
			_apfnI2SCbs[1]();
		}
	}
	else if(((LPC_I2S->I2SIRQ)>>1)& 0x01)
	{
		tx_level = ((LPC_I2S->I2SSTATE)>>16)&0xFF;
		tx_depth_irq = ((LPC_I2S->I2SIRQ)>>16)&0xFF;
		if(tx_level <= tx_depth_irq)//transmit interrupt
		{
			_apfnI2SCbs[0]();
		}
	}
}

/**
 * @}
 */

#endif /* _I2S */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */

