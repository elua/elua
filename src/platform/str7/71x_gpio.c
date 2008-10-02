/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : 71x_gpio.c
* Author             : MCD Application Team
* Version            : V4.0
* Date               : 10/09/2007
* Description        : This file provides all the GPIO firmware functions.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "71x_gpio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : GPIO_Config
* Description    : Configures the selected GPIO I/O pins according to the input
*                  passed in parameter.
* Input          : - GPIOx: selects the port to be configured (x can be 0,1 or 2).
*                  - Port_Pins: this parameter specifies the port pin placement.
*                  You can select more than one pin.
*                  - Pins Mode: specifies the pin mode.
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Config (GPIO_TypeDef *GPIOx, u16 Port_Pins,
                  GpioPinMode_TypeDef GPIO_Mode)
{
  switch (GPIO_Mode)
  {
    case GPIO_HI_AIN_TRI:
      GPIOx->PC0 &= ~Port_Pins;
      GPIOx->PC1 &= ~Port_Pins;
      GPIOx->PC2 &= ~Port_Pins;
      break;

    case GPIO_IN_TRI_TTL:
      GPIOx->PC0 |= Port_Pins;
      GPIOx->PC1 &= ~Port_Pins;
      GPIOx->PC2 &= ~Port_Pins;
      break;

    case GPIO_IN_TRI_CMOS:
      GPIOx->PC0 &= ~Port_Pins;
      GPIOx->PC1 |= Port_Pins;
      GPIOx->PC2 &= ~Port_Pins;
      break;

    case GPIO_IPUPD_WP:
      GPIOx->PC0 |= Port_Pins;
      GPIOx->PC1 |= Port_Pins;
      GPIOx->PC2 &= ~Port_Pins;
      break;

    case GPIO_OUT_OD:
      GPIOx->PC0 &= ~Port_Pins;
      GPIOx->PC1 &= ~Port_Pins;
      GPIOx->PC2 |= Port_Pins;
      break;

    case GPIO_OUT_PP:
      GPIOx->PC0 |= Port_Pins;
      GPIOx->PC1 &= ~Port_Pins;
      GPIOx->PC2 |= Port_Pins;
      break;

    case GPIO_AF_OD:
      GPIOx->PC0 &= ~Port_Pins;
      GPIOx->PC1 |= Port_Pins;
      GPIOx->PC2 |= Port_Pins;
      break;

    case GPIO_AF_PP:
      GPIOx->PC0 |= Port_Pins;
      GPIOx->PC1 |= Port_Pins;
      GPIOx->PC2 |= Port_Pins;
      break;
  }
}

/*******************************************************************************
* Function Name  : GPIO_BitRead
* Description    : Reads the specified data port bit and returns its value.
* Input          : - GPIOx: selects the port to be read. x can be 0, 1 or 2.
*                  - Port_Pin: Specifies the pin to see the value.
* Output         : None.
* Return         : The selected port pin value.
*******************************************************************************/
u8 GPIO_BitRead(GPIO_TypeDef *GPIOx, u8 Port_Pin)
{
  return ((GPIOx->PD >> Port_Pin) & 0x0001);
}

/*******************************************************************************
* Function Name  : GPIO_ByteRead
* Description    : Reads the specified data port byte and returns its value.
* Input          : - GPIOx: selects the port to be read. x can be 0, 1 or 2.
*                  - Port_Byte: specifies which byte to be read (upper or lower
*                  eight bits)
*                     - GPIO_MSB corresponds to the upper byte.
*                     - GPIO_LSB corresponds to the lower byte.
* Output         : None.
* Return         : The GPIO_MSB or GPIO_LSB of the selected PD register.
*******************************************************************************/
u8 GPIO_ByteRead(GPIO_TypeDef *GPIOx, u8 Port_Byte)
{
  return (u8)(GPIOx->PD >> Port_Byte);
}

/*******************************************************************************
* Function Name  : GPIO_WordRead
* Description    : Reads the value of the specified GPIOx PD data port register
*                  and returns its value.
* Input          : - GPIOx: selects the port to be read. x can be 0, 1 or 2.
* Output         : None.
* Return         : The specified port data value.
*******************************************************************************/
u16 GPIO_WordRead(GPIO_TypeDef *GPIOx)
{
  return GPIOx->PD;
}

/*******************************************************************************
* Function Name  : GPIO_BitWrite
* Description    : Sets or clears the selected data port bit.
* Input          : - GPIOx: selects the port. x can be 0, 1 or 2.
*                  - Port_Pin: Pin number.
*                  - Bit_Val: bit value (0 or 1).
* Output         : None.
* Return         : None.
*******************************************************************************/
void GPIO_BitWrite(GPIO_TypeDef *GPIOx, u8 Port_Pin, u8 Bit_Val)
{
  if (Bit_Val & 0x01)
  {
    GPIOx->PD |= 1 << Port_Pin;
  }
  else
  {
    GPIOx->PD &= ~(1 << Port_Pin);
  }
}

/*******************************************************************************
* Function Name  : GPIO_ByteWrite
* Description    : Write byte value to the selected GPIOx PD register.
* Input          : - GPIOx: selects the port. x can be 0, 1 or 2.
*                  - Port_Byte: specifies which byte to be written (upper or 
                   lower eight bits)
*                     - GPIO_MSB corresponds to the upper byte.
*                     - GPIO_LSB corresponds to the lower byte.
*                  - Port_Val: the value of the byte to be written.
* Output         : None.
* Return         : None.
*******************************************************************************/
void GPIO_ByteWrite(GPIO_TypeDef *GPIOx, u8 Port_Byte, u8 Port_Val)
{
     
  if(Port_Byte)
  {            
   GPIOx->PD &= 0x00FF;
   GPIOx->PD |= (u16)Port_Val << 8;
  }
  /* If LSB selected */
 else
  {
   GPIOx->PD &= 0xFF00;
   GPIOx->PD |= Port_Val;
  }
}

/*******************************************************************************
* Function Name  : GPIO_WordWrite
* Description    : Writes a value in to the selected data port register.
* Input          : - GPIOx: selects the port. x can be 0, 1 or 2.
*                  - Port_Val: the value of the word to be written.
* Output         : None.
* Return         : None.
*******************************************************************************/
void GPIO_WordWrite(GPIO_TypeDef *GPIOx, u16 Port_Val)
{
  GPIOx->PD = Port_Val;
}

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
