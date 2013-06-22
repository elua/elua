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
** PLATFORM : Infineon XMC4000 Series   			                          **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR   : App Developer                                                   **
**                                                                            **
** MAY BE CHANGED BY USER [yes/Yes]: Ys                                       **
**                                                                            **
** MODIFICATION DATE : Dec 29, 2011                                           **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                       Author(s) Identity                                   **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** BT         App Developer                                                   **
*******************************************************************************/

/**
 * @file RESET001.h
 *
 * @brief  Reset App implementation header file.
 *
 */

 #ifndef RESET001_H_
 #define RESET001_H_
 /*******************************************************************************
**                      Include Files                                         **
*******************************************************************************/


#include <DAVE3.h>


/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/

/**
 * @ingroup RESET001_publicparam
 * @{
 */


/*******************************************************************************
**                      Global Type Definitions                               **
*******************************************************************************/


/**
 * @brief This enumeration contains Reset Info Type
 */
typedef enum RESET001_InfoType{
  /** Power on reset */
   POWER_ON_RESET = 0x00000001U,
  /** SWD reset  */
   SWD_RESET = 0x00000002U,
  /** PVC reset */
   PVC_RESET = 0x00000004U,
  /** CPU system reset */
   CPU_SYS_RESET = 0x00000008U,
  /** CPU lockup reset */
   CPU_LOCKUP_RESET = 0x00000010U,
  /** WDT reset */
   WDT_RESET = 0x00000020U,
  /** TCU reset */
   TCU_RESET = 0x00000040U,
  /** Parity error reset */
   PARITY_ERROR_RESET = 0x00000080U
}RESET001_InfoType;

/**
 * @brief This enumeration contains Peripheral ID Type
 */
typedef enum RESET001_ResetnType{
  /** ADC peripheral module */
   PER0_VADC = 0x00000001U,
  /** DSD peripheral module */
   PER0_DSD = 0x00000002U,
  /** CCU4x peripheral module */
   PER0_CCU40 = 0x00000004U,
  /** CCU4x peripheral module */
   PER0_CCU41 = 0x00000008U,
  /** CCU4x peripheral module */
   PER0_CCU42 = 0x00000010U,
  /** CCU8x peripheral module  */
   PER0_CCU80 = 0x00000080U,
  /** CCU8x peripheral module */
   PER0_CCU81 = 0x00000100U,
  /** POSIF peripheral module */
   PER0_POSIF0 = 0x00000200U,
  /** POSIF peripheral module */
   PER0_POSIF1 = 0x00000400U,
  /** USIC0 peripheral module */
   PER0_USIC0 = 0x00000800U,
  /** ERU peripheral module */
   PER0_ERU1 = 0x00010000U,
  /** CCU4x peripheral module */
   PER1_CCU43 = 0x10000001U ,
  /** LED TS peripheral module */
   PER1_LEDTSCU = 0x10000008U,
  /** MCAN peripheral module */
   PER1_MCAN0 = 0x10000010U,
  /** DAC peripheral module */
   PER1_DAC = 0x10000020U,
  /** MMC peripheral module */
   PER1_MMC = 0x10000040U,
  /** USICx peripheral module */
   PER1_USIC1 = 0x10000080U,
  /** USICx peripheral module */
   PER1_USIC2 = 0x10000100U,
  /** Port &amp; Pins peripheral module */
   PER1_PPORTS = 0x10000200U,
  /** Watch Dog peripheral module */
   PER2_WDT = 0x20000002U,
  /** Ethernet peripheral module */
   PER2_ETH0 = 0x20000004U,
  /** DMA Unit 0 peripheral module */
   PER2_DMA0 = 0x20000010U,
  /** DMA Unit 1 peripheral module */
   PER2_DMA1 = 0x20000020U,
  /** FCE peripheral module */
   PER2_FCE = 0x20000040U,
  /** USB peripheral module */
   PER2_USB = 0x20000080U,
  /** External Bus Unit peripheral module */
   PER3_EBU = 0x30000004U
}RESET001_ResetnType;

/**
 * @}
 */

/*******************************************************************************
**                      Global Constant Declarations                          **
*******************************************************************************/


/*******************************************************************************
**                      Extern Variables                                      **
*******************************************************************************/


/*******************************************************************************
**                     FUNCTION PROTOTYPES                                    **
*******************************************************************************/


/**
 * @ingroup RESET001_apidoc
 * @{
 */

/**
 * @brief       This function is to enable reset of peripheral by software
 *
 * @param [in]  Resetn Peripheral ID
 *
 * @return     void
 *
 * <b>Reentrant: NO </b><BR>
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * @code
 * #include <DAVE3.h>
 * int main(void)
 * {
 *    uint32_t status = 0;
 *    DAVE_Init();
 *    // Assert the module
 *    RESET001_AssertReset(PER0_ERU1);
 *    // Get the status  of the module
 *    status = RESET001_GetStatus(PER0_ERU1);
 *    if(status == 1)
 *    {
 *      // De-assert the module
 *      RESET001_DeassertReset(PER0_ERU1);
 *    }
 *  }
 * @endcode<BR> </p>
 *
 */
void RESET001_AssertReset(RESET001_ResetnType Resetn);

/**
 * @brief      This function is used to clear the  Reset of the desired peripheral ( de-assert reset).
 *
 * @param[in]  Resetn Peripheral ID
 *
 * @return     void
 *
 * <b>Reentrant: NO </b><BR>
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * @code
 * #include <DAVE3.h>
 * int main(void)
 * {
 *    uint32_t status = 0;
 *    DAVE_Init();
 *    // Assert the module
 *    RESET001_AssertReset(PER0_ERU1);
 *    // Get the status  of the module
 *    status = RESET001_GetStatus(PER0_ERU1);
 *    if(status == 1)
 *    {
 *      // De-assert the module
 *      RESET001_DeassertReset(PER0_ERU1);
 *    }
 *  }
 * @endcode<BR> </p>
 *
 */
void RESET001_DeassertReset(RESET001_ResetnType Resetn);


/**
 * @brief      API to get the  reason of last reset.
 *
 * @return     RESET001_InfoType enum  value
 *
 * <b>Reentrant: NO </b><BR>
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * @code
 * #include <DAVE3.h>
 * int main(void)
 * {
 *    RESET001_InfoType ResetInfo;
 *    DAVE_Init();
 *    // Get the Information about last reset
 *    ResetInfo = RESET001_GetResetInfo();
 *  }
 * @endcode<BR> </p>
 *
 */
RESET001_InfoType RESET001_GetResetInfo(void);


/**
 * @brief      Get the reset status of the peripheral
 *
 * @param[in]   Resetn Peripheral ID
 *
 * @return     status_t
 *
 * <b>Reentrant: NO </b><BR>
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * @code
 * #include <DAVE3.h>
 * int main(void)
 * {
 *    uint32_t status = 0;
 *    DAVE_Init();
 *    // Assert the module
 *    RESET001_AssertReset(PER0_ERU1);
 *    // Get the status  of the module
 *    status = RESET001_GetStatus(PER0_ERU1);
 *    if(status == 1)
 *    {
 *      // De-assert the module
 *      RESET001_DeassertReset(PER0_ERU1);
 *    }
 *  }
 * @endcode<BR> </p>
 *
 */
status_t RESET001_GetStatus(RESET001_ResetnType Resetn);


/**
 *@}
 */

#endif /* RESET001_H_ */
