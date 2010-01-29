/*****************************************************************************
 *   target.h:  Header file for NXP LPC288x Family Microprocessors
 *
 *   Copyright(C) 2007, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2007.02.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __TARGET_H 
#define __TARGET_H

#ifdef __cplusplus
   extern "C" {
#endif

//This segment should not be modified
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAIN_CLOCK            12000000
#define Fcclk	                60000000

/* Below Fpclk is primarily used for PCLK going to the
UART, since UART_FDR is set such Fractional Divider has
NO impact on the baudrate generator, thus, Fpclk is set
to 60Mhz, if the UART_FDR is set such Fractional Divider
has impact on the baud rate generation, then, Fpclk should
be set to 30000000. */
#define Fpclk	60000000

#define FLASH_ADDR		0x10400000
#define RAM_ADDR		0x400000

/* divided by 2 */
#define SYSFDCR0_MADD	0x48
#define SYSFDCR0_MSUB	0xB8

/* divided by 4 */
#define SYSFDCR1_MADD	0x48
#define SYSFDCR1_MSUB	0xE8

/* divided by 10 */
#define SYSFDCR3_MADD	0x48
#define SYSFDCR3_MSUB	0xF8

/* divided by 2 */
#define SYSFDCR4_MADD	0x48
#define SYSFDCR4_MSUB	0xB8

/* divied by 2 */
#define SYSFDCR5_MADD	0x48
#define SYSFDCR5_MSUB	0xB8

/* divied by 4 */
#define DAIOFDCR0_MADD	0x48
#define DAIOFDCR0_MSUB	0xE8

/* divided by 4 */
#define DAIOFDCR1_MADD	0x48
#define DAIOFDCR1_MSUB	0xE8

/* divied by 16 */
#define DAIOFDCR2_MADD	0x78
#define DAIOFDCR2_MSUB	0xF8

/* divied by 2 */
#define DAIOFDCR3_MADD	0x48
#define DAIOFDCR3_MSUB	0xB8

/* divied by 512 */
#define DAIOFDCR4_MADD	0x1FF
#define DAIOFDCR4_MSUB	0x3FF

/* divied by 16 */
#define DAIOFDCR5_MADD	0x78
#define DAIOFDCR5_MSUB	0xF8

// UART data
#define UART_DMA_ENABLE		0

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

// SDRAM data
#define SDRAM_BASE_ADDR   0x30000000
#define SDRAM_SIZE        0x02000000

void lpc288x_init();

#ifdef __cplusplus
   }
#endif
 
#endif /* end __TARGET_H */
/******************************************************************************
**                            End Of File
******************************************************************************/
