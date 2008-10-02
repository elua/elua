/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_uart.h
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file contains all the functions prototypes for the
*                      UART firmware library.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __71x_UART_H
#define __71x_UART_H

/* Includes ------------------------------------------------------------------*/
#include "71x_map.h"
#include "71x_rccu.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  UART_RxFIFO,
  UART_TxFIFO
} UARTFIFO_TypeDef;

typedef enum
{
  UART_EVEN_PARITY = 0x0000,
  UART_ODD_PARITY  = 0x0020,
  UART_NO_PARITY   = 0x0000
} UARTParity_TypeDef;

typedef enum
{
  UART_0_5_StopBits  = 0x0000,
  UART_1_StopBits    = 0x0008,
  UART_1_5_StopBits  = 0x0010,
  UART_2_StopBits    = 0x0018
} UARTStopBits_TypeDef;

typedef enum
{
  UARTM_8D   = 0x0001,
  UARTM_7D_P = 0x0003,
  UARTM_9D   = 0x0004,
  UARTM_8D_W = 0x0005,
  UARTM_8D_P = 0x0007
} UARTMode_TypeDef;

/* Exported constants --------------------------------------------------------*/
/* UART flags definition */
#define UART_TxFull          0x0200
#define UART_RxHalfFull      0x0100
#define UART_TimeOutIdle     0x0080
#define UART_TimeOutNotEmpty 0x0040
#define UART_OverrunError    0x0020
#define UART_FrameError      0x0010
#define UART_ParityError     0x0008
#define UART_TxHalfEmpty     0x0004
#define UART_TxEmpty         0x0002
#define UART_RxBufNotEmpty   0x0001

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void UART_Init(UART_TypeDef *UARTx);
void UART_ModeConfig(UART_TypeDef *UARTx, UARTMode_TypeDef UART_Mode);
void UART_BaudRateConfig(UART_TypeDef *UARTx, u32 BaudRate);
void UART_ParityConfig(UART_TypeDef *UARTx, UARTParity_TypeDef Parity);
void UART_StopBitsConfig(UART_TypeDef *UARTx, UARTStopBits_TypeDef StopBits);
void UART_Config(UART_TypeDef *UARTx, u32 BaudRate, UARTParity_TypeDef Parity,
                 UARTStopBits_TypeDef StopBits, UARTMode_TypeDef Mode);
void UART_ItConfig(UART_TypeDef *UARTx, u16 UART_Flag, FunctionalState NewState);
void UART_FifoConfig(UART_TypeDef *UARTx, FunctionalState NewState);
void UART_FifoReset(UART_TypeDef *UARTx, UARTFIFO_TypeDef FIFO);
void UART_LoopBackConfig(UART_TypeDef *UARTx, FunctionalState NewState);
void UART_TimeOutPeriodConfig(UART_TypeDef *UARTx, u16 TimeOutPeriod);
void UART_GuardTimeConfig(UART_TypeDef *UARTx, u16 GuardTime);
void UART_RxConfig(UART_TypeDef *UARTx, FunctionalState NewState);
void UART_OnOffConfig(UART_TypeDef *UARTx, FunctionalState NewState);
void UART_ByteSend(UART_TypeDef *UARTx, u8 *Data);
void UART_9BitByteSend(UART_TypeDef *UARTx, u16 *Data);
void UART_DataSend(UART_TypeDef *UARTx, u8 *Data, u8 DataLength);
void UART_9BitDataSend(UART_TypeDef *UARTx, u16 *Data, u8 DataLength);
void UART_StringSend(UART_TypeDef *UARTx, u8 *String);
u8 UART_ByteReceive(UART_TypeDef *UARTx);
u16 UART_9BitByteReceive(UART_TypeDef *UARTx, u16 *Data, u8 TimeOut);
u16 UART_9BitDataReceive(UART_TypeDef *UARTx, u16 *Data, u8 DataLength,
                         u8 TimeOut);
u16 UART_StringReceive(UART_TypeDef *UARTx, u8 *Data);
u16 UART_FlagStatus(UART_TypeDef *UARTx);

#ifdef USE_SERIAL_PORT
void SendChar(u8 *ch);
#endif /* USE_SERIAL_PORT */

#endif /*__71x_UART_H*/

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
