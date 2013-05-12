
/**************************************************************************//**
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
** PLATFORM : Infineon <Microcontroller name, step>                           **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** MODIFICATION DATE : September 14, 2012                                     **
**                                                                            **
*******************************************************************************/

/**
 * @file  MULTIPLEXER.h
 *
 * @brief This file contains all public data structures,enums and function
 *        prototypes.
 *
 */

#ifndef _MULTIPLEXER_H_
#define _MULTIPLEXER_H_

/* Defines	*/

/** Macro to write new value to the bits in Hibernate domain register */
#define WR_REGHIB(reg, mask, pos, val, pollmask) {  \
		 	 	 	 reg &= ~(mask);\
		 	 	 	 while(!(SCU_INTERRUPT->SRRAW & pollmask)){}; \
					 reg |= (val << pos) & mask;\
					 while(!(SCU_INTERRUPT->SRRAW & pollmask)){}; \
					 SCU_INTERRUPT->SRCLR |= pollmask;\
                    }
                    
/* VALUE : ON / OFF */
#define VALUE0 0
#define VALUE1 1
#define VALUE2 2
#define VALUE3 3
#define VALUE4 4
#define VALUE5 5
#define VALUE6 6
#define VALUE7 7

/* ERU : ERU_xA signals	*/ 
#define SIGNAL_ERU_xA0 0
#define SIGNAL_ERU_xA1 1
#define SIGNAL_ERU_xA2 2
#define SIGNAL_ERU_xA3 3

/* ERU : ERU_xB signals	*/ 
#define SIGNAL_ERU_xB0 0
#define SIGNAL_ERU_xB1 1
#define SIGNAL_ERU_xB2 2
#define SIGNAL_ERU_xB3 3

/* ERU : TR signals	*/ 
#define SIGNAL_TR0 0
#define SIGNAL_TR1 1

/* ERU : TRx signals	*/ 
#define SIGNAL_TRx0 0
#define SIGNAL_TRx1 1
#define SIGNAL_TRx2 2
#define SIGNAL_TRx3 3

/* ERU : ERS_ETL_FL signals	*/ 
#define SIGNAL_ERS_ETL_FL0 0
#define SIGNAL_ERS_ETL_FL1 1
#define SIGNAL_ERS_ETL_FL2 2
#define SIGNAL_ERS_ETL_FL3 3

/* ERU : ERU_OGU_y signals	*/ 
#define SIGNAL_ERU_OGU_y0 0
#define SIGNAL_ERU_OGU_y1 1
#define SIGNAL_ERU_OGU_y2 2
#define SIGNAL_ERU_OGU_y3 3

/****************************************************************************************
 *  PORTS : The IOCR Register bit field PC split into 3 bit fields namely
 *  PCx_PCR, PCx_PO, PCx_PCR to support the Mux/User configuration based on 
 *  the functionality of the bit fields. For more details please refer the 
 *  Port visio file prepared by Infineon Technologies.
 ****************************************************************************************/
/* PORT_IOCR  =  Port x Input/Output Control Register */
#define   PORT_IOCR_PC0_PCR_Pos  	(3U)
#define   PORT_IOCR_PC0_PCR_Msk  	(0x00000007U  << PORT_IOCR_PC0_PCR_Pos)

#define   PORT_IOCR_PC0_PO_Pos   	(6U)
#define   PORT_IOCR_PC0_PO_Msk   	(0x00000001U  << PORT_IOCR_PC0_PO_Pos)

#define   PORT_IOCR_PC0_OE_Pos   	(7U)
#define   PORT_IOCR_PC0_OE_Msk   	(0x00000001U  << PORT_IOCR_PC0_OE_Pos)

#define   PORT_IOCR_PC1_PCR_Pos  	(11U)
#define   PORT_IOCR_PC1_PCR_Msk  	(0x00000007U  << PORT_IOCR_PC1_PCR_Pos)

#define   PORT_IOCR_PC1_PO_Pos   	(14U)
#define   PORT_IOCR_PC1_PO_Msk   	(0x00000001U  << PORT_IOCR_PC1_PO_Pos)

#define   PORT_IOCR_PC1_OE_Pos   	(15U)
#define   PORT_IOCR_PC1_OE_Msk   	(0x00000001U  << PORT_IOCR_PC1_OE_Pos)

#define   PORT_IOCR_PC2_PCR_Pos  	(19U)
#define   PORT_IOCR_PC2_PCR_Msk  	(0x00000007U  << PORT_IOCR_PC2_PCR_Pos)

#define   PORT_IOCR_PC2_PO_Pos   	(22U)
#define   PORT_IOCR_PC2_PO_Msk   	(0x00000001U  << PORT_IOCR_PC2_PO_Pos)

#define   PORT_IOCR_PC2_OE_Pos   	(23U)
#define   PORT_IOCR_PC2_OE_Msk   	(0x00000001U  << PORT_IOCR_PC2_OE_Pos)

#define   PORT_IOCR_PC3_PCR_Pos  	(27U)
#define   PORT_IOCR_PC3_PCR_Msk  	(0x00000007U  << PORT_IOCR_PC3_PCR_Pos)

#define   PORT_IOCR_PC3_PO_Pos   	(30U)
#define   PORT_IOCR_PC3_PO_Msk   	(0x00000001U  << PORT_IOCR_PC3_PO_Pos)

#define   PORT_IOCR_PC3_OE_Pos   	(31U)
#define   PORT_IOCR_PC3_OE_Msk   	(0x00000001U  << PORT_IOCR_PC3_OE_Pos)


// IOCR_OE : INPUT / OUTPUT
#define PORT_IOCR_OE0 0
#define PORT_IOCR_OE1 1

// IOCR_PCR : PULLUP/PULLDOWN / ALT 
#define PORT_IOCR_PCR0 0
#define PORT_IOCR_PCR1 1
#define PORT_IOCR_PCR2 2
#define PORT_IOCR_PCR3 3
#define PORT_IOCR_PCR4 4
#define PORT_IOCR_PCR5 5
#define PORT_IOCR_PCR6 6
#define PORT_IOCR_PCR7 7

// HWSEL_HW : HW0 / HW1
#define PORT_HWSEL_SW 0
#define PORT_HWSEL_HW0 1
#define PORT_HWSEL_HW1 2

// PDISC_PDIS : PDIS0 / PDIS1
#define PORT_PDISC_PDIS0 0
#define PORT_PDISC_PDIS1 1

// HCU
#define   SCU_HIBERNATE_HDCR_HIBIO0SEL_PCR_Pos  	(16U)
#define   SCU_HIBERNATE_HDCR_HIBIO0SEL_PCR_Msk  	(0x00000007U  << SCU_HIBERNATE_HDCR_HIBIO0SEL_PCR_Pos)

#define   SCU_HIBERNATE_HDCR_HIBIO0SEL_OE_Pos  		(19U)
#define   SCU_HIBERNATE_HDCR_HIBIO0SEL_OE_Msk  		(0x00000001U  << SCU_HIBERNATE_HDCR_HIBIO0SEL_OE_Pos)

#define   SCU_HIBERNATE_HDCR_HIBIO1SEL_PCR_Pos  	(20U)
#define   SCU_HIBERNATE_HDCR_HIBIO1SEL_PCR_Msk  	(0x00000007U  << SCU_HIBERNATE_HDCR_HIBIO1SEL_PCR_Pos)

#define   SCU_HIBERNATE_HDCR_HIBIO1SEL_OE_Pos  		(23U)
#define   SCU_HIBERNATE_HDCR_HIBIO1SEL_OE_Msk  		(0x00000001U  << SCU_HIBERNATE_HDCR_HIBIO1SEL_OE_Pos)

//CAN
#define RXD_SIGNAL0 0 //RXDxA
#define RXD_SIGNAL1 1 //RXDxB
#define RXD_SIGNAL2 2 //RXDxC
#define RXD_SIGNAL3 3 //RXDxD
#define RXD_SIGNAL4 4 //RXDxE
#define RXD_SIGNAL5 5 //RXDxF
#define SRN0 0
#define SRN1 1
#define SRN2 2
#define SRN3 3
#define SRN4 4
#define SRN5 5
#define SRN6 6
#define SRN7 7

//POSIF
#define INSIGNAL0 0 //INxA
#define INSIGNAL1 1 //INxB
#define INSIGNAL2 2 //INxC
#define INSIGNAL3 3 //INxD
#define INSIGNAL4 4 //INxE
#define INSIGNAL5 5 //INxF
#define SET_1 1 //enable
#define SET_0 0 //disable
#define SR0 0
#define SR1 1

// CCU4 
// CC4yINS Input Selector Configuration
#define CCU4xINyA  0 // CCU4xINy[A]
#define CCU4xINyB  1 // CCU4xINy[B]
#define CCU4xINyC  2 // CCU4xINy[C]
#define CCU4xINyD  3 // CCU4xINy[D]
#define CCU4xINyE  4 // CCU4xINy[E]
#define CCU4xINyF  5 // CCU4xINy[F]
#define CCU4xINyG  6 // CCU4xINy[G]
#define CCU4xINyH  7 // CCU4xINy[H]
#define CCU4xINyI  8 // CCU4xINy[I]
#define CCU4xINyJ  9 // CCU4xINy[J]
#define CCU4xINyK  10 // CCU4xINy[K]
#define CCU4xINyL  11 // CCU4xINy[L]
#define CCU4xINyM  12 // CCU4xINy[M]
#define CCU4xINyN  13 // CCU4xINy[N]
#define CCU4xINyO  14 // CCU4xINy[O]
#define CCU4xINyP  15 // CCU4xINy[P]

// CCU8 
// CC8yINS Input Selector Configuration
#define CCU8xINyA  0 // CCU8xINy[A]
#define CCU8xINyB  1 // CCU8xINy[B]
#define CCU8xINyC  2 // CCU8xINy[C]
#define CCU8xINyD  3 // CCU8xINy[D]
#define CCU8xINyE  4 // CCU8xINy[E]
#define CCU8xINyF  5 // CCU8xINy[F]
#define CCU8xINyG  6 // CCU8xINy[G]
#define CCU8xINyH  7 // CCU8xINy[H]
#define CCU8xINyI  8 // CCU8xINy[I]
#define CCU8xINyJ  9 // CCU8xINy[J]
#define CCU8xINyK  10 // CCU8xINy[K]
#define CCU8xINyL  11 // CCU8xINy[L]
#define CCU8xINyM  12 // CCU8xST0
#define CCU8xINyN  13 // CCU8xST1
#define CCU8xINyO  14 // CCU8xST2
#define CCU8xINyP  15 // CCU8xST3

// CCUySRS Service Request Selector
#define CCU_SR0  0 // CCU_SR0
#define CCU_SR1  1 // CCU_SR1
#define CCU_SR2  2 // CCU_SR2
#define CCU_SR3  3 // CCU_SR3

// Ethernet MAC

#define ETH_RXDxA 0 	// ETH0_RXDxA
#define ETH_RXDxB 1 	// ETH0_RXDxB
#define ETH_RXDxC 2 	// ETH0_RXDxC
#define ETH_RXDxD 3 	// ETH0_RXDxD

#define ETH_RMIIA 0 	// ETH_RMIIA
#define ETH_RMIIB 1 	// ETH_RMIIB
#define ETH_RMIIC 2 	// ETH_RMIIC
#define ETH_RMIID 3 	// ETH_RMIID

#define ETH_CRS_DVA 0 	// ETH_CRS_DVA
#define ETH_CRS_DVB 1 	// ETH_CRS_DVB
#define ETH_CRS_DVC 2 	// ETH_CRS_DVC
#define ETH_CRS_DVD 3 	// ETH_CRS_DVD

#define ETH_CRSA 0 	// ETH_CRSA
#define ETH_CRSB 1 	// ETH_CRSB
#define ETH_CRSC 2 	// ETH_CRSC
#define ETH_CRSD 3 	// ETH_CRSD

#define ETH_RXERA 0 	// ETH_RXERA
#define ETH_RXERB 1 	// ETH_RXERB
#define ETH_RXERC 2 	// ETH_RXERC
#define ETH_RXERD 3 	// ETH_RXERD

#define ETH_COLA 0 	// ETH_COLA
#define ETH_COLB 1 	// ETH_COLB
#define ETH_COLC 2 	// ETH_COLC
#define ETH_COLD 3 	// ETH_COLD

#define ETH_CLK_TXA 0 	// ETH_CLK_TXA
#define ETH_CLK_TXB 1 	// ETH_CLK_TXB
#define ETH_CLK_TXC 2 	// ETH_CLK_TXC
#define ETH_CLK_TXD 3 	// ETH_CLK_TXD

#define ETH_MDIOA 0 	// ETH_MDIOA
#define ETH_MDIOB 1 	// ETH_MDIOB
#define ETH_MDIOC 2 	// ETH_MDIOC
#define ETH_MDIOD 3 	// ETH_MDIOD

// VADC
#define VADC_G_QCTRL0_XTWC_XTSEL_Msk        (0x000000EFU  << VADC_G_QCTRL0_XTSEL_Pos)
#define VADC_G_QCTRL0_GTWC_GTSEL_Msk        (0x0000008FU  << VADC_G_QCTRL0_GTSEL_Pos)
#define VADC_G_ASCTRL_XTWC_XTSEL_Msk        (0x000000EFU  << VADC_G_ASCTRL_XTSEL_Pos)
#define VADC_G_ASCTRL_GTWC_GTSEL_Msk        (0x0000008FU  << VADC_G_ASCTRL_GTSEL_Pos)
#define VADC_BRSCTRL_XTWC_XTSEL_Msk        (0x000000EFU  << VADC_BRSCTRL_XTSEL_Pos)
#define VADC_BRSCTRL_GTWC_GTSEL_Msk        (0x0000008FU  << VADC_BRSCTRL_GTSEL_Pos)

// Delta-Sigma Demodulator (DSD)

/* DSD_CH_DICFGx  =  Demodulator Input Configuration Register 0*/

#define   DSD_CH_DICFG_DSRCSEL_Pos  	(1U)
#define   DSD_CH_DICFG_DSRCSEL_Msk  	(0x00000007U  << DSD_CH_DICFG_DSRCSEL_Pos)

#define DSD_CH_DICFG_DSWC_DSRCSEL_Msk      (0x0000008FU  << DSD_CH_DICFG_DSRC_Pos)
#define DSD_CH_DICFG_SCWC_STROBE_CSRC_Msk  (0x000080FFU  << DSD_CH_DICFG_CSRC_Pos)
#define DSD_CH_DICFG_TRWC_TRSEL_Msk        (0x000000FFU  << DSD_CH_DICFG_ITRMODE_Pos)

// Demodulator Input Configuration Register x
#define DINxA	    1 // input A
#define DINxB	    2 // input B

#define MCLKxA 1  // MCLKxA
#define MCLKxB 2  // MCLKxB
#define MCLKxINTERNAL 15  // MCLK_INTERNAL
#define ITRxA  0 // Integration trigger, channel x, input A
#define ITRxB  1 // Integration trigger, channel x, input B
#define ITRxC  2 // Integration trigger, channel x, input C
#define ITRxD  3 // Integration trigger, channel x, input D

// Rectification Configuration Register x
#define SSRC0  0 // SSRC0
#define SSRC1  1 // SSRC1
#define SSRC2  2 // SSRC2
#define SSRC3  3 // SSRC3


/* USIC : FIFO DPTR & SIZE MASK and POS Values */ 
#define   USIC_CH_TBCTR_DPTRSIZE_Pos  	(0U)
#define   USIC_CH_TBCTR_DPTRSIZE_Msk  	(0x0700003FU << USIC_CH_TBCTR_DPTRSIZE_Pos)

#define   USIC_CH_RBCTR_DPTRSIZE_Pos  	(0U)
#define   USIC_CH_RBCTR_DPTRSIZE_Msk  	(0x0700003FU << USIC_CH_RBCTR_DPTRSIZE_Pos)
    
	

// Initializes Multiplexer configurations
void DAVE_MUX_Init(void);

#endif  // ifndef _MULTIPLEXER_H_
