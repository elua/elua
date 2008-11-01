/********************************************************************
 * Project:    STR9-comStick GNU (UART)
 * File:       main.c
 *
 * System:     ARM9TDMI 32 Bit (STR912FW44X)
 * Compiler:   GCC 4.0.3
 *
 * Date:       2006-12-20
 * Author:     Applications@Hitex.de
 *
 * Rights:     Hitex Development Tools GmbH
 *             Greschbachstr. 12
 *             D-76229 Karlsruhe
 ********************************************************************
 * Description:
 *
 * This file is part of the GNU Example chain
 * The code is bassed on usage of the STmicro library functions
 * This is a small implementation of UART1 feature echoing external input
 * The application runs in ARM mode with high optimization level.
 *
 ********************************************************************
 * History:
 *
 *    Revision 1.0    2006/12/20      Gn
 *    Initial revision
 ********************************************************************
 * This is a preliminary version.
 *
 * WARRANTY:  HITEX warrants that the media on which the SOFTWARE is
 * furnished is free from defects in materials and workmanship under
 * normal use and service for a period of ninety (90) days. HITEX entire
 * liability and your exclusive remedy shall be the replacement of the
 * SOFTWARE if the media is defective. This Warranty is void if failure
 * of the media resulted from unauthorized modification, accident, abuse,
 * or misapplication.
 *
 * DISCLAIMER:  OTHER THAN THE ABOVE WARRANTY, THE SOFTWARE IS FURNISHED
 * "AS IS" WITHOUT WARRANTY OF ANY KIND. HITEX DISCLAIMS ALL OTHER WARRANTIES,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * NEITHER HITEX NOR ITS AFFILIATES SHALL BE LIABLE FOR ANY DAMAGES ARISING
 * OUT OF THE USE OF OR INABILITY TO USE THE SOFTWARE, INCLUDING DAMAGES FOR
 * LOSS OF PROFITS, BUSINESS INTERRUPTION, OR ANY SPECIAL, INCIDENTAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES EVEN IF HITEX HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGES.
 ********************************************************************/

#include "defines.h"

#define global extern   /* to declare external variables and functions      */
#include "91x_lib.h"

#include "main.h"
#define GPIO_Alt1 0x01

#define TxBufferSize   (countof(TxBuffer) - 1)
#define RxBufferSize   0xFF

/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Private variables ---------------------------------------------------------*/
   UART_InitTypeDef UART_InitStructure;
   u8 TxBuffer[] = "UART1 - 1,8,N,1@115.2k communication without flow control\n\r";
   u8 RxBuffer[RxBufferSize];
   u8 NbrOfDataToTransfer = TxBufferSize;
   u8 TxCounter = 0;
   u8 RxCounter = 0;

   GPIO_InitTypeDef  GPIO_InitStructure;
   TIM_InitTypeDef   TIM_InitStructure;

/* Private function prototypes -----------------------------------------------*/
   void SCU_Configuration(void);
   void GPIO_Configuration(void);
   void UART1_Configuration(void);
   static void Delay(u32 nCount);

int main (void)
{

   /* Configure the system clocks */
   SCU_Configuration();
   /* Configure the GPIOs */
   GPIO_Configuration();
   /* Configure and start the UART1 */
   UART1_Configuration();

   /* endless loop */
   while (1)
   {
      {
         if((UART_GetFlagStatus(UART1, UART_FLAG_RxFIFOEmpty) != SET)&&(RxCounter < RxBufferSize))
            {
            RxBuffer[0] = UART1->DR;
            UART_SendData(UART1, RxBuffer[0]);
            }
      }

      /* Turn OFF leds connected to P9.0, P9.1 pins */
      GPIO_WriteBit(GPIO8, GPIO_Pin_0, Bit_SET);

      /* Insert delay */
      Delay(0x1FFFF);

      /* Turn ON leds connected to P9.0, P9.1 pins */
      GPIO_WriteBit(GPIO8, GPIO_Pin_0, Bit_RESET);

      /* Insert delay */
      Delay(0x1FFFF);
   }
}

void SCU_Configuration(void)
{
   SCU_MCLKSourceConfig(SCU_MCLK_OSC);

   SCU_PLLFactorsConfig(192,25,2);            /* PLL = 96 MHz */
   SCU_PLLCmd(ENABLE);                        /* PLL Enabled  */

   SCU_MCLKSourceConfig(SCU_MCLK_PLL);        /* MCLK = PLL   */

   FMI_BankRemapConfig(4, 2, 0, 0x80000); /* Set Flash banks size & address */
   FMI_Config(FMI_READ_WAIT_STATE_2, FMI_WRITE_WAIT_STATE_0, FMI_PWD_ENABLE,\
             FMI_LVD_ENABLE, FMI_FREQ_HIGH); /* FMI Waite States */

   /* Enable VIC clock */
   SCU_AHBPeriphClockConfig(__VIC, ENABLE);
   SCU_AHBPeriphReset(__VIC, DISABLE);

   /* Set the PCLK Clock to MCLK/2 */
   SCU_PCLKDivisorConfig(SCU_PCLK_Div1);

    /* Enable the UART0 Clock */
   SCU_APBPeriphClockConfig(__UART1, ENABLE);

   /* Enable the clock for TIM0 and TIM1 */
   SCU_APBPeriphClockConfig(__TIM01, ENABLE);
   SCU_APBPeriphReset(__TIM01, DISABLE);

   SCU_APBPeriphClockConfig(__TIM23, ENABLE);
   SCU_APBPeriphReset(__TIM23, DISABLE);


/* Enable the GPIO3 Clock */
   SCU_APBPeriphClockConfig(__GPIO3, ENABLE);

   SCU_APBPeriphClockConfig(__GPIO4, ENABLE);  /* Enable the clock for the GPIO4 */

   /* Enable the __GPIO8 */
   SCU_APBPeriphClockConfig(__GPIO8 ,ENABLE);
   /* Enable the __GPIO9 */
   SCU_APBPeriphClockConfig(__GPIO9 ,ENABLE);
}
/* GPIO Configuration --------------------------------------------------------*/
void GPIO_Configuration(void)
{
   GPIO_DeInit(GPIO3);
   GPIO_DeInit(GPIO4);                         /* GPIO4 Deinitialization */
   GPIO_DeInit(GPIO9);
   /* IOs */
   GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
   GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;
   GPIO_Init (GPIO4, &GPIO_InitStructure);
   /* onboard LED */
   GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
   GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;
   GPIO_Init (GPIO9, &GPIO_InitStructure);
   GPIO_WriteBit(GPIO9, GPIO_Pin_0, Bit_RESET);

/* configure UART1_Rx pin GPIO3.2*/
   GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;
   GPIO_InitStructure.GPIO_IPConnected = GPIO_IPConnected_Enable;
   GPIO_InitStructure.GPIO_Alternate = GPIO_InputAlt1  ;
   GPIO_Init (GPIO3, &GPIO_InitStructure);

   /*Gonfigure UART1_Tx pin GPIO3.3*/
   GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
   GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;
   GPIO_InitStructure.GPIO_Alternate = GPIO_OutputAlt2  ;
   GPIO_Init (GPIO3, &GPIO_InitStructure);

}

/* UART1 configuration -------------------------------------------------------*/
void UART1_Configuration(void)
  {
/* UART1 configured as follow:
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - BaudRate = 115200 baud
        - no Hardware flow control enabled (RTS and CTS signals)
        - Receive and transmit enabled
        - Receive and transmit FIFOs are enabled
        - Transmit and Receive FIFOs levels have 8 bytes depth
  */
   UART_InitStructure.UART_WordLength = UART_WordLength_8D;
   UART_InitStructure.UART_StopBits = UART_StopBits_1;
   UART_InitStructure.UART_Parity = UART_Parity_No ;
   UART_InitStructure.UART_BaudRate = 115200;
   UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
   UART_InitStructure.UART_Mode = UART_Mode_Tx_Rx;
   UART_InitStructure.UART_FIFO = UART_FIFO_Enable;//UART_FIFO_Enable;
   UART_InitStructure.UART_TxFIFOLevel = UART_FIFOLevel_1_2; /* FIFO size 16 bytes, FIFO level 8 bytes */
   UART_InitStructure.UART_RxFIFOLevel = UART_FIFOLevel_1_2; /* FIFO size 16 bytes, FIFO level 8 bytes */

   UART_DeInit(UART1);
   UART_Init(UART1, &UART_InitStructure);

   /* Enable the UART0 */
   UART_Cmd(UART1, ENABLE);

   while(NbrOfDataToTransfer--)
   {
      UART_SendData(UART1, TxBuffer[TxCounter++]);
      while(UART_GetFlagStatus(UART1, UART_FLAG_TxFIFOFull) != RESET);
   }
}

/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
* Input          : nCount: specifies the delay time length.
*******************************************************************************/
static void Delay(u32 nCount)
{
   u32 j = 0;

   for(j = nCount; j != 0; j--);
}
/************************************** EOF *********************************/
