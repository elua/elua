/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_uart.c
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file provides all the UART firmware functions
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "71x_uart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : UART_Init
* Description    : This function initializes the selected UART registers to
*                  their reset values.
* Input          : - UARTx: selects the UART to be configured UART
*                    (x can be 0,1, 2 or 3).
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_Init(UART_TypeDef *UARTx)
{
  UARTx->CR = 0x0000;
  UARTx->IER = 0x0000;
  (void)UARTx->RxBUFR;
  UARTx->RxRSTR = 0xFFFF;
  UARTx->TxRSTR = 0xFFFF;
  UARTx->TOR = 0x0000;
  UARTx->GTR = 0x0000;
}

/*******************************************************************************
* Function Name  : UART_ModeConfig
* Description    : This function configures the mode of the selected UART.
* Input          : - UARTx: selects the UART to be configured
*                    (x can be 0,1, 2 or 3).
*                  - UART_Mode: selects the UART modeThe UART mode,
*                    it can be one of the following parameters:
*                    - UARTM_8D   for 8-bit data format
*                    - UARTM_7D_P for 7-bit data + parity format
*                    - UART_9D    for 9-bit data format
*                    - UART_8D_W  for 8-bit data + wake-up bit format
*                    - UART_8D_P  for 8-bit data + parity bit format
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_ModeConfig(UART_TypeDef *UARTx, UARTMode_TypeDef UART_Mode)
{
  UARTx->CR = (UARTx->CR & 0xFFF8) | (u16)UART_Mode;
}

/*******************************************************************************
* Function Name  : UART_BaudRateConfig
* Description    : This function configures the baud rate of the selected UART.
* Input          : - UARTx: selects the UART to be configured
*                    (x can be 0,1, 2 or 3).
*                  - BaudRate: The baudrate value in bps.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_BaudRateConfig(UART_TypeDef *UARTx, u32 BaudRate)
{
  u32 tmpBaudRate = 0;

  /* Configure BaudRate */
  tmpBaudRate = (u32)((RCCU_FrequencyValue(RCCU_PCLK1) * 10) / (16 * BaudRate));

  /*Search the reload value (Integer)*/
  if (tmpBaudRate - ((tmpBaudRate / 10) * 10) < 5)
  {
    UARTx->BR = tmpBaudRate / 10;
  }
  else
  {
    UARTx->BR = (tmpBaudRate / 10) + 1;
  }
}


/*******************************************************************************
* Function Name  : UART_ParityConfig
* Description    : This function configures the data parity of the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - Parity: the parity type, it can be:
*                    - UART_EVEN_PARITY  for even parity configuration.
*                    - UART_ODD_PARITY   for odd parity configuration.
*                    - UART_NO_PARITY    for no parity configuration.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_ParityConfig(UART_TypeDef *UARTx, UARTParity_TypeDef Parity)
{
  UARTx->CR = (UARTx->CR & 0xFFDF) | (u16)Parity;
}

/*******************************************************************************
* Function Name  : UART_StopBitsConfig
* Description    : This function configures the number of stop bits of the
*                  selected UART.
* Input          : - UARTx: selects the UART to be configured.
*                    (x can be 0,1, 2 or 3).
*                  - StopBits: the number of stop bits, it can be:
*                    - UART_0_5_StopBits for 0.5 stop bit.
*                    - UART_1_StopBits for 1 stop bit.
*                    - UART_1_5_StopBits  for 1.5 stop bits.
*                    - UART_2_StopBits for 2 stop bits.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_StopBitsConfig(UART_TypeDef *UARTx, UARTStopBits_TypeDef StopBits)
{
  UARTx->CR = (UARTx->CR & 0xFFE7) | (u16)StopBits;
}
/*******************************************************************************
* Function Name  : UART_Config
* Description    : This function configures the Baud rate, parity mode, 
*                  the number of stop bits and the UART mode of the selected 
*                  UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - BaudRate: the baudrate value in bps.
*                  - Parity: selects the parity type, it can be:
                     - UART_EVEN_PARITY  for even parity configuration.
*                    - UART_ODD_PARITY   for odd parity configuration.
*                    - UART_NO_PARITY    for no parity configuration.
*                  - StopBits: selects the number of the stop bits, it can be:
*                    - UART_0_5_StopBits for 0.5 stop bit.
*                    - UART_1_StopBits for 1 stop bit.
*                    - UART_1_5_StopBits  for 1.5 stop bits.
*                    - UART_2_StopBits for 2 stop bits.
*                  - Mode: selects the UART mode, it can be one of the following 
*                    parameters:
*                    - UARTM_8D   for 8-bit data format.
*                    - UARTM_7D_P for 7-bit data + parity format.
*                    - UART_9D    for 9-bit data format.
*                    - UART_8D_W  for 8-bit data + wake-up bit format.
*                    - UART_8D_P  for 8-bit data + parity bit format.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_Config(UART_TypeDef *UARTx, u32 BaudRate, UARTParity_TypeDef Parity,
                 UARTStopBits_TypeDef StopBits, UARTMode_TypeDef Mode)
{
  UART_ModeConfig(UARTx, Mode);
  UART_BaudRateConfig(UARTx, BaudRate);
  UART_ParityConfig(UARTx, Parity);
  UART_StopBitsConfig(UARTx, StopBits);
}

/*******************************************************************************
* Function Name  : UART_ItConfig
* Description    : This function enables or disables one or several interrupt
*                  sources of the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - UART_Flag: selects one or several UART interrupt source.
*                  - NewState: specifies whether the interrupt source is 
*                    enabled or disabled (ENABLE or DISABLE).
* Output         : None.
* Return         : None.
* Note           : The UART interrupt flags are listed in the file uart.h
*                  except UART_TxFull flag will have no effect when using 
*                  this function.
*******************************************************************************/
void UART_ItConfig(UART_TypeDef *UARTx, u16 UART_Flag, FunctionalState NewState)
{
  if (NewState == ENABLE)
  {
    UARTx->IER |= UART_Flag;
  }
  else
  {
    UARTx->IER &= ~UART_Flag;
  }
}

/*******************************************************************************
* Function Name  : UART_FifoConfig
* Description    : This function enables or disables the Rx and Tx FIFOs of
*                  the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - NewState: specifies whether the FIFOs are enabled or
*                    disabled (ENABLE or DISABLE).
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_FifoConfig(UART_TypeDef *UARTx, FunctionalState NewState)
{
  if (NewState == ENABLE)
  {
    UARTx->CR |= 0x0400;
  }
  else
  {
    UARTx->CR &= ~0x0400;
  }
}

/*******************************************************************************
* Function Name  : UART_FifoReset
* Description    : This function resets the Rx and the Tx FIFOs of the
*                  selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - FIFO: Selects the FIFO to reset, it can be:
*                    - UART_RxFIFO
*                    - UART_TxFIFO
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_FifoReset(UART_TypeDef *UARTx, UARTFIFO_TypeDef FIFO)
{
  if (FIFO == UART_RxFIFO)
  {
    UARTx->RxRSTR = 0xFFFF;
  }
  else
  {
    UARTx->TxRSTR = 0xFFFF;
  }
}

/*******************************************************************************
* Function Name  : UART_LoopBackConfig
* Description    : This function enables or disables the loop back mode of
*                  the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - NewState: specifies whether the loop back is enabled
*                    or disabled (ENABLE or DISABLE).
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_LoopBackConfig(UART_TypeDef *UARTx, FunctionalState NewState)
{
  if (NewState == ENABLE)
  {
    UARTx->CR |= 0x0040;
  }
  else
  {
    UARTx->CR &= ~0x0040;
  }
}

/*******************************************************************************
* Function Name  : UART_TimeOutPeriodConfig
* Description    : This function configures the UART Time Out Period of the
*                  selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - TimeOutPeriod: the time-out period value.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_TimeOutPeriodConfig(UART_TypeDef *UARTx, u16 TimeOutPeriod)
{
  UARTx->TOR = TimeOutPeriod;
}

/*******************************************************************************
* Function Name  : UART_GuardTimeConfig
* Description    : This function configures the UART Guard Time.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - GuardTime: the guard time value.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_GuardTimeConfig(UART_TypeDef *UARTx, u16 GuardTime)
{
  UARTx->GTR = GuardTime;
}
/*******************************************************************************
* Function Name  : UART_RxConfig
* Description    : This function enables or disables the selected UART data
*                  reception.
* Input          : - UARTx: selects the UART to be configured
*                    (x can be 0,1, 2 or 3).
*                  - NewState: specifies whether the reception is enabled or
*                    disabled (ENABLE or DISABLE).
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_RxConfig(UART_TypeDef *UARTx, FunctionalState NewState)
{
  if (NewState == ENABLE)
  {
    UARTx->CR |= 0x0100;
  }
  else
  {
    UARTx->CR &= ~0x0100;
  }
}

/*******************************************************************************
* Function Name  : UART_OnOffConfig
* Description    : This function sets On/Off the selected UART.
* Input          : - UARTx: selects the UART to be configured
*                    (x can be 0,1, 2 or 3).
*                  - NewState: ENABLE or DISABLE.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_OnOffConfig(UART_TypeDef *UARTx, FunctionalState NewState)
{
  if (NewState == ENABLE)
  {
    UARTx->CR |= 0x0080;
  }
  else
  {
    UARTx->CR &= ~0x0080;
  }
}

/*******************************************************************************
* Function Name  : UART_ByteSend
* Description    : This function is used to send a 7-bit byte or an 8-bit byte
*                  using the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - Data: a pointer on the data byte to send.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_ByteSend(UART_TypeDef *UARTx, u8 *Data)
{
  /* if FIFO ENABLED */
  if (UARTx->CR & 0x0400)
  {
    
    while ((UARTx->SR & UART_TxFull))
    {
      /* Wait until the TxFIFO contains at least 1 free place */
    }
  }
  /* if FIFO DISABLED */
  else
  {
    
    while (!(UARTx->SR & UART_TxEmpty))
    {
      /* Wait until the transmit shift register is empty */
    }
  }
  UARTx->TxBUFR = *Data;
}

/*******************************************************************************
* Function Name  : UART_9BitByteSend
* Description    : This function sends a 9-bit data using the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - Data: a pointer on the 9-bit data to send.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_9BitByteSend(UART_TypeDef *UARTx, u16 *Data)
{
  /* if FIFO ENABLED */
  if (UARTx->CR & 0x0400)
  {
    
    while ((UARTx->SR & UART_TxFull))
    {
    /* Wait until the TxFIFO contains at least 1 free place */
    }
  }
  /* if FIFO DISABLED */
  else
  {
    
    while (!(UARTx->SR & UART_TxEmpty))
    {
      /* Wait until the transmit shift register is empty */
    }
  }

  UARTx->TxBUFR = ((*Data) & 0x01FF);
}

/*******************************************************************************
* Function Name  : UART_DataSend
* Description    : This routine is used to send several 7-bit bytes or 8-bit
*                  bytes using the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - Data: the bytes start address.
*                  - DataLength: the data length in bytes.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_DataSend(UART_TypeDef *UARTx, u8 *Data, u8 DataLength)
{

  while (DataLength--)
  {
    UART_ByteSend(UARTx, Data);
    Data++;
  }
}

/*******************************************************************************
* Function Name  : UART_9BitDataSend
* Description    : This function sends several 9-bit data using the selected
*                  UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - Data: the bytes start address.
*                  - DataLength: the data bytes number.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_9BitDataSend(UART_TypeDef *UARTx, u16 *Data, u8 DataLength)
{
  while (DataLength--)
  {
    UART_9BitByteSend(UARTx, Data);
    Data++;
  }
}

/*******************************************************************************
* Function Name  : UART_StringSend
* Description    : This function sends a string to the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - String: a pointer on the string to send.
* Output         : None.
* Return         : None.
*******************************************************************************/
void UART_StringSend(UART_TypeDef *UARTx, u8 *String)
{
  u8 *Data = String;
  
  while (*Data != '\0')
  {
    UART_ByteSend(UARTx, Data++);
  }
  
  /* Send the character end of string  */
  *Data = '\0';
  UART_ByteSend(UARTx, Data);
}

/*******************************************************************************
* Function Name  : UART_ByteReceive
* Description    : This routine is used to get a 7 or an 8-bit byte from 
*                  the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
* Return         : The received 8-bit data.
*******************************************************************************/
u8 UART_ByteReceive(UART_TypeDef *UARTx)
{
  /* Wait while the UART_RxFIFO is empty and no Timeoutidle */
  while (!(UARTx->SR & UART_RxBufNotEmpty))
  {
    /* Wait */
  }

  /* then read the Receive Buffer Register */
  return (u8)UARTx->RxBUFR;
}

/*******************************************************************************
* Function Name  : UART_9BitByteReceive
* Description    : This function gets a 9-bit data from the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - Data: a pointer on the data where the data will be stored.
*                  - TimeOut: The time-out period value.
* Output         : The received 9-bit data.
* Return         : The UARTx_SR register content before reading the received
*                  data.
*******************************************************************************/
u16 UART_9BitByteReceive(UART_TypeDef *UARTx, u16 *Data, u8 TimeOut)
{
  u16 wStatus = 0;

  /* Reload the Timeout counter */
  UARTx->TOR = TimeOut;

  /* while the UART_RxFIFO is empty and no Timeoutidle */
  while (!((wStatus = UARTx->SR) & (UART_TimeOutIdle | UART_RxBufNotEmpty)))
  {
    /* Wait */
  }

  /* then read the RxBUFR*/
  *Data = (u16)UARTx->RxBUFR;
  return wStatus;
}

/*******************************************************************************
* Function Name  : UART_9BitDataReceive
* Description    : This function gets several 9-bits data from the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - Data: a pointer on the buffer where the data will be stored.
*                  - DataLength: the number of bytes to receive.
*                  - TimeOut: the time-out period value.
* Output         : The received 9-bit data buffer.
* Return         : The UARTx_SR register contents.
*******************************************************************************/
u16 UART_9BitDataReceive(UART_TypeDef *UARTx, u16 *Data, u8 DataLength,
                         u8 TimeOut)
{
  u16 wStatus = 0;

  while (DataLength--)
  {
    wStatus = UART_9BitByteReceive(UARTx, Data++, TimeOut);
  }
  return wStatus;
}

/*******************************************************************************
* Function Name  : UART_StringReceive
* Description    : This function gets a string which ends with: End of string
*                  or Carriage return characters from the selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
*                  - Data: a pointer on the buffer where the data will be stored.
* Output         : The received string.
* Return         : The UARTx_SR register content before reading the received
*                  data.
*******************************************************************************/
u16 UART_StringReceive(UART_TypeDef *UARTx, u8 *Data)
{
  u8 *pSTRING = Data;
  u16 wStatus = 0;

  do
  {
    /* while the UART_RxFIFO is empty  */
    while (!((wStatus = UARTx->SR) & (UART_RxHalfFull | UART_RxBufNotEmpty)))
    {
      /* Wait */
    }

    /* then read the RxBUFR */
    *(pSTRING++) = (u8)UARTx->RxBUFR;

  } while ((*(pSTRING - 1) != 0x0D) && (*(pSTRING - 1) != '\0'));

  *(pSTRING - 1) = '\0';

  return wStatus;
}

/*******************************************************************************
* Function Name  : UART_FlagStatus
* Description    : This routine returns the UARTx_SR register content of the
*                  selected UART.
* Input          : - UARTx: the selected UART (x can be 0,1, 2 or 3).
* Output         : None.
* Return         : None.
*******************************************************************************/
u16 UART_FlagStatus(UART_TypeDef *UARTx)
{
  return UARTx->SR;
}

#ifdef USE_SERIAL_PORT

/*******************************************************************************
* Function Name  : SendChar
* Description    : This function sends a character using  the defined UART.
* Input          : - ch: a pointer on the character to send.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SendChar( u8 *ch )
{
#ifdef USE_UART0
#define  UARTx  UART0
#endif /* Use_UART0 */

#ifdef USE_UART1
#define  UARTx  UART1
#endif /* Use_UART1 */

#ifdef USE_UART2
#define  UARTx  UART2
#endif /* Use_UART2 */

#ifdef USE_UART3
#define  UARTx  UART3
#endif /* Use_UART3 */

  UART_ByteSend(UARTx, (u8 *)ch);
}

#endif /* USE_SERIAL_PORT */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
