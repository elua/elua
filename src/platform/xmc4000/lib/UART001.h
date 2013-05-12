/******************************************************************************
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
*******************************************************************************
**                                                                           **
**                                                                           **
** PLATFORM : Infineon XMC4000/XMC1000 Series                                **
**                                                                           **
** COMPILER : Compiler Independent                                           **
**                                                                           **
** AUTHOR   : App Developer                                                  **
**                                                                           **
** MAY BE CHANGED BY USER [yes/No]: Yes                                      **
**                                                                           **
** MODIFICATION DATE : Mar 26, 2013                                          **
**                                                                           **
******************************************************************************/

/******************************************************************************
**                       Author(s) Identity                                  **
*******************************************************************************
**                                                                           **
** Initials     Name                                                         **
** --------------------------------------------------------------------------**
** PAE        App Developer                                                  **
******************************************************************************/

/**
 * @file UART001.h
 *
 * @brief  Header file for UART001 App
 *
 */
/*
 * Revision History
 * 04 Dec 2012 v1.0.4  Changes from 1.0.2 are
 *                     1. Modified as per coding guidelines and MISRA checks
 * 26 Mar 2013 v1.0.10 1. Updated for the example code for
 *                     UART001_GetFlagStatus() API.
 *                     2. Added file revision history.
 *                     3. Updated to support XMC1xxx devices.
 *                     4. Updated examples for UART001_WriteData() and
 *                     UART001_ReadData() macros.
 *                     5. Modified the structure UART001_HandleType to add baud
 *                     parameters for the baud rate.
 */
#ifndef UART001_H_
#define UART001_H_

/******************************************************************************
**                      Include Files                                        **
******************************************************************************/


#include <DAVE3.h>
#include "Usic.h"


/******************************************************************************
**                      Global Macro Definitions                             **
******************************************************************************/

/**
 * @ingroup UART001_apidoc
 * @{
 */
/** 
 * @brief  Macro which writes a data word into the transmit FIFO buffer register. 
 * @param[in]  Handle  USIC device handle.
 * @param[in]  Data  Data to be transmitted.
 *
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 * #include <DAVE3.h>
 * int main(void)
 * {
 *  uint16_t data = 0xAAAA;
 *  DAVE_Init();
 *  // Send data 0xAAAA
 *  UART001_WriteData(UART001_Handle0,data);
 *  while(1);
 *  return 0;
 * }
 * @endcode<BR> </p>
 *
 */
#define UART001_WriteData(Handle,Data) (Handle.UartRegs->IN[0] = Data)

/** 
 * @brief Macro which reads a data word from the receive FIFO buffer register. 
 * @param[in]  Handle  USIC device handle.
 *
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 * #include <DAVE3.h>
 *
 * uint16_t Readdata;
 * int main(void)
 * {
 *
 *  DAVE_Init();
 *  while(1);
 *  return 0;
 * }
 *
 *  void RecvFifoHandler()
 * {
 * 	 Readdata = UART001_ReadData(UART001_Handle0);
 * }
 * @endcode<BR> </p>
 *
 */
#define UART001_ReadData(Handle)  ((uint16_t)Handle.UartRegs->OUTR)

/**
 * @}
 */
/******************************************************************************
**                      Global Type Definitions                              **
******************************************************************************/

/**
 * @ingroup UART001_publicparam
 * @{
 */

/**
 * @brief This data type describes the possible status values returned
 *  by functions.
 */
typedef enum UART001_StatusType
{
  /**
   * Indicates Flag is not set
   */
  /**
	* @cond INTERNAL_DOCS
	* @param MODULENAME UART001
	* @endcond
  */
  /**
  * @cond INTERNAL_DOCS
  *	@param ERRCODESTRING1 UART001_RESET
  * @param STRCODESTRING1 Flag is not set
  * @endcond
  */
  UART001_RESET = 1,
  /**
   * Indicates Flag is set
   */
  /**
   * @cond INTERNAL_DOCS
   * @param ERRCODESTRING2 UART001_SET
   * @param STRCODESTRING2 Flag is set
   * @endcond
   */
  UART001_SET,
  /**
   * UART Busy
   */
  /**
  * @cond INTERNAL_DOCS
  *	@param ERRCODESTRING3 UART001_BUSY
  *	@param STRCODESTRING3 Uart channel busy
  * @endcond
  */
  UART001_BUSY,
  /**
   * Invalid parameter
   */
  /**
   * @cond INTERNAL_DOCS
   * @param ERRCODESTRING4 UART001_INVALID_PARAM
   * @param STRCODESTRING4 Invalid parameter
   * @endcond
  */
  UART001_INVALID_PARAM,
  /**
   * Unknown error
   */
  /**
   * @cond INTERNAL_DOCS
   * @param ERRCODESTRING5 UART001_ERROR
   * @param STRCODESTRING5 Uart Error
   * @endcond
  */
  UART001_ERROR,
  /**
   * Function Entry
   */
  /**
   * @cond INTERNAL_DOCS
   * @param ERRCODESTRING6 UART001_FUN_ENTRY
   * @param STRCODESTRING6 Entered function \%s
   * @endcond
  */
  UART001_FUN_ENTRY,
  /**
   *  Function Exit
   */
  /**
   * @cond INTERNAL_DOCS
   * @param ERRCODESTRING7 UART001_FUN_EXIT
   * @param STRCODESTRING7 Exited function \%s
   * @endcond
  */
  UART001_FUN_EXIT
}UART001_StatusType;



/**
 * @brief This data type describes the available UART Status flags
 */
typedef enum UART001_FlagStatusType
{
	/** Transmission Idle */
  UART001_TXIDLE_FLAG = 0,
  /** Reception Idle */
  UART001_RXIDLE_FLAG = 1,
  /** Receiver Noise Detected */
  UART001_RECV_NOISE_FLAG = 4,
  /** Format Error in Stop Bit 0 */
  UART001_FORMAT_ERR0_FLAG = 5,
  /** Format Error in Stop Bit 1 */
  UART001_FORMAT_ERR1_FLAG = 6,
  /** Receive Frame Finished */
  UART001_REC_FRAME_FIN_FLAG = 7,
  /** Transmitter Frame Finished*/
  UART001_TRANSMIT_FRAME_FIN_FLAG = 8,
  /** Transfer Status BUSY */
  UART001_TRANSFER_STATUS_BUSY_FLAG = 9,
  /** Receiver Start Indication Flag */
  UART001_RECV_START_IND_FLAG = 10,
  /** Data Lost Indication Flag */
  UART001_DATA_LOST_IND_FLAG = 11,
  /** Transmit Shift Indication Flag */
  UART001_TRANS_SHIFT_IND_FLAG = 12,
  /** Transmit Buffer Indication Flag */
  UART001_TRANS_BUFFER_IND_FLAG = 13,
  /** Receive Indication Flag */
  UART001_RECV_IND_FLAG = 14,
  /** Alternative receive indication flag */
  UART001_ALT_REC_IND_FLAG = 15,
  /** FIFO Standard Receive Buffer Event */
  UART001_FIFO_STD_RECV_BUF_FLAG = 16,                           
  /** FIFO Receive Buffer Error Event */						  
  UART001_FIFO_RECV_BUF_ERR_FLAG = 17,                
  /** FIFO Alternative Receive Buffer Event */
  UART001_FIFO_ALTRECV_BUF_FLAG = 18,                 
  /** FIFO Standard Transmit Buffer Event */
  UART001_FIFO_STD_TRANSMIT_BUF_FLAG = 19,            
  /** FIFO Transmit Buffer Error Event */
  UART001_FIFO_TRANSMIT_BUF_ERR_FLAG = 20              
}UART001_FlagStatusType;


/**
 * @brief This data type describes UART App instance handle details
 */
 typedef struct UART001_HandleType
{
  /** CMSIS defined structure for USIC module */
  USIC_CH_TypeDef* UartRegs;
  /** USIC Operation mode */
  UART_ModeType Mode;
  /** No of stopbit selected */
  UART_StopBitType StopBit;
  /** Parity  selected */
  UART_ParityType Parity;
  /** Desired baud rate */
  uint32_t BaudRate;
  /** Word Length */
  uint16_t DataBits;
  /** TX FIFO Trigger value */
  uint8_t TxLimit;
  /** RX FIFO Triger value */
  uint8_t RxLimit;
  /** Tx FIFO Size value */
  uint8_t TxFifoSize;
  /** Rx FIFO Size value */
  uint8_t RxFifoSize;
  /** Enable Receiver Noise Interrupt */
  bool RecvNoiseEn;
  /** Enable Format Error Interrupt */
  bool FormatErrEn;
  /** Enable Frame Finished Interrupt */
  bool FrameFinEn;
  /** Calculated step value */
  uint16_t BGR_STEP;
  /** Calculated pdiv value */
  uint16_t BGR_PDIV;
  /** Calculated  dctq value */
  uint16_t BGR_DCTQ;
  /** Calculated  sampling point value */
  /** SP = (((1+dctq)/2)+1) */
  uint16_t BGR_SP;
  /** Calculated  pctq value */
  uint8_t BGR_PCTQ;
}UART001_HandleType;


/**
 * @}
 */
/******************************************************************************
**                      Global Constant Declarations                         **
******************************************************************************/


/******************************************************************************
**                      Extern Variables                                     **
******************************************************************************/
/**
 * @ingroup UART001_apidoc
 * @{
 */

/******************************************************************************
** FUNCTION PROTOTYPES                                                       **
******************************************************************************/

/**
 * @brief Initializes all the UART channel registers consumed by UART001 App. 
 * All the UART001
 * App instances are configured as per the UI configuration.
 *
 * @return     None
 *
 * <b>Reentrant: NO </b><BR>
 * <b>Sync/Async:  Synchronous</b>
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 *  #include <DAVE3.h>
 *
 *  int main(void)
 *  {
 *	  // ... Initializes Apps configurations ...
 *    DAVE_Init(); // UART001_Init() will be called inside DAVE_Init()
 *    while(1)
 *    {
 *
 *    }
 * }
 * @endcode<BR> </p>
 *
 */
void  UART001_Init (void);

/**
 * @brief Function provided to reset the App to default values.
 *
 *
 * @param[in]  Handle USIC device handle
 *
 * @return     None
 *
 * <b>Reentrant: NO </b><BR>
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 *  #include <DAVE3.h>
 *
 *  int main(void)
 *  {
 *    UART001_DeInit(&UART001_Handle0);
 *    while(1)
 *    {
 *
 *    }
 * }
 * @endcode<BR> </p>
 *
 */
void  UART001_DeInit (const UART001_HandleType* Handle);

/**
 * @brief Function which allows changing of baudrate,parity & stop bit  
 * configuration at run time.
 *
 *
 * @param[in]  Handle USIC device handle
 * @param[in]  BaudRate Desired Baud Rate in Bauds
 * @param[in]  Parity Desired Parity
 * @param[in]  Stopbit Desired no of Stop bit(s)
 *
 * @return     status <BR>
 *             DAVEApp_SUCCESS	: for success.<BR>
 *             UART001_ERROR  	: for failure case.<BR>
 *             UART001_BUSY 	: UART channel is busy.<BR>
 *
 * <b>Reentrant: NO </b><BR>
 *
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 *  #include <DAVE3.h>
 *
 *  int main(void)
 *  {
 *    uint32_t Status = UART001_ERROR;
 *    // ... Initializes Apps configurations ...
 *    DAVE_Init();
 *    Status = UART001_Configure\
                    (&UART001_Handle0,19200,UART_PARITY_EVEN,UART_ONE_STOPBIT);
 *    if(Status == DAVEApp_SUCCESS)
 *    {
 *        // Uart channel reconfiguration successful 
 *    }
 *    // ... infinite loop ...
 *    while(1)
 *    {
 *
 *    }
 *  }
 * @endcode<BR> </p>
 *
 */
status_t  UART001_Configure
(
  const UART001_HandleType* Handle,
  uint32_t BaudRate,
  UART_ParityType Parity,
  UART_StopBitType Stopbit
);


/**
 * @brief This function reads out  the content of the USIC receive
 * FIFO Buffer until there is data in the buffer or reached the
 * input count value. 
 * Note: Fifo trigger limit should be adjusted manually to read
 * required amount of data.
 *
 *
 * @param[in]  Handle USIC device handle
 * @param[out] DataPtr  Pointer to data
 * @param[in]  Count   Total no of words to be read.
 *
 * @return     uint32_t Number of data read from FIFO register <BR>
 *
 * <b>Reentrant: NO </b><BR>
 *
 * <b>Note</b>: Fifo trigger limit should be adjusted manually in the UI based
 * on the number of data's to be buffered.
 *
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 * #include <DAVE3.h>
 *
 * uint16_t Readdata[11];
 * int main(void)
 * {
 *
 *  DAVE_Init();
 *  while(1);
 *  return 0;
 * }
 *
 *  void RecvFifoHandler()
 * {
 * 	 UART001_ReadDataMultiple(&UART001_Handle0,Readdata,11);
 * }
 * @endcode<BR> </p>
 *
 */
uint32_t UART001_ReadDataMultiple(
const UART001_HandleType* Handle,
uint16_t* DataPtr,
uint32_t Count);

int UART001_ReadChar( const UART001_HandleType *handle, int blocking );

/**
 * @brief This function writes out a word to the USIC FIFO transmit
 *			 buffer register until the FIFO buffer is full or
 *			 reached the Count value.
 * Note: Fifo trigger limit should be adjusted manually to transmit 
 * to transmit required amount of data.
 *
 * Reentrant : No
 *
 * @param[in]  Handle  USIC device handle
 * @param[in]  DataPtr Pointer to data
 * @param[in]  Count   Total no of words to be transmitted.
 *
 * @return     uint32_t Number of data written to buffer <BR>
 *
 * <b>Reentrant: NO </b><BR>
 *
 * <b>Note</b>: Fifo trigger limit should be adjusted manually in the UI based
 * on the number of data's to be buffered.
 *
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 * #include <DAVE3.h>
 * int main(void)
 * {
 *  uint32_t status = 0;
 *  uint8_t data[] = "H E L L O   W O R L D";
 *  DAVE_Init();
 *  // Send hello world to PC hyper terminal
 *  status = UART001_WriteDataMultiple(&UART001_Handle0, (uint16_t*)&data, \
 *                                      (uint32_t)sizeof(data)/2);
 *  if(status == 11)
 *  {
 * 	 //data transmitted
 *  }
 *  while(1);
 *  return 0;
 * }
 * @endcode<BR> </p>
 *
 */

uint32_t UART001_WriteDataMultiple(
const UART001_HandleType* Handle,
uint16_t* DataPtr,
uint32_t Count);

void UART001_WriteString(
const UART001_HandleType* Handle,
const char *DataPtr);

/**
 * @brief The function is used to get the event flag status specified.
 * 			   It returns UART_SET if the corresponding flag
 *			   is set else otherwise.
 *
 *
 * @param[in]  Handle USIC device handle
 * @param[in]  Flag   UART Flag which needs to be checked
 *
 * @return     status_t <BR>
 *             UART_SET    : indicates Flag is set.<BR>
 *             UART_RESET  : indicates Flag is reset.<BR>
 *
 * <b>Reentrant: NO </b><BR>
 *
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 *  #include <DAVE3.h>
 * 
 *  int main(void)
 *  {
 *    uint32_t Status = UART001_ERROR;
 *    uint16_t Data;
 *    // Initializes Apps configurations ...
 *    DAVE_Init();
 *    Status = UART001_Configure\
                    (&UART001_Handle0,19200,UART_PARITY_EVEN,UART_ONE_STOPBIT);
 *    if(Status == DAVEApp_SUCCESS)
 *    {
 *      // Uart channel reconfiguration successful
 *      //wait till standard receive buffer event is occurred
 *      while((UART001_GetFlagStatus\
 *               (&UART001_Handle0,UART001_FIFO_STD_RECV_BUF_FLAG))!=UART001_SET)
 *      {}
 *      //Read Receive FIFO buffer till the buffer is empty
 *      Data = UART001_ReadData(UART001_Handle0);
 *      UART001_ClearFlag(&UART001_Handle0,UART001_FIFO_STD_RECV_BUF_FLAG);
 *      if(Data == 0x43)
 *      {
 *     	 //Checking the data received
 *      }
 *    }
 *    // ... infinite loop ...
 *    while(1)
 *    {
 * 
 *    }
 *  }
 * 
 * @endcode<BR> </p>
 *
 */
status_t UART001_GetFlagStatus\
                (const UART001_HandleType* Handle,UART001_FlagStatusType Flag);

/**
 * @brief This function clears the specified event flag status.
 *
 *
 * @param[in]  Handle USIC device handle
 * @param[in]  Flag   UART Flag which needs to be cleared
 *
 * @return    None
 *
 * <b>Reentrant: NO </b><BR>
 *
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * <BR><I><b>UART001_Handle0</b></I> is a global structure of type
 * UART001_HandleType which will be configured in UART001_Conf.c file using
 * UI parameters.
 * @code
 *  #include <DAVE3.h>
 * 
 *  int main(void)
 *  {
 *    uint32_t Status = UART001_ERROR;
 *    uint16_t Data;
 *    // Initializes Apps configurations ...
 *    DAVE_Init();
 *    Status = UART001_Configure\
                   (&UART001_Handle0,19200,UART_PARITY_EVEN,UART_ONE_STOPBIT);
 *    if(Status == DAVEApp_SUCCESS)
 *    {
 *      // Uart channel reconfiguration successful
 *      //wait till standard receive buffer event is occurred
 *      while((UART001_GetFlagStatus\
 *               (&UART001_Handle0,UART001_FIFO_STD_RECV_BUF_FLAG))!=UART001_SET)
 *      {}
 *      //Read Receive FIFO buffer till the buffer is empty
 *      Data = UART001_ReadData(UART001_Handle0);
 *      UART001_ClearFlag(&UART001_Handle0,UART001_FIFO_STD_RECV_BUF_FLAG);
 *      if(Data == 0x43)
 *      {
 *     	 //Checking the data received
 *      }
 *    }
 *    // ... infinite loop ...
 *    while(1)
 *    {
 * 
 *    }
 *  }
 * 
 * @endcode<BR> </p>
 *
 */
void UART001_ClearFlag\
               (const UART001_HandleType* Handle,UART001_FlagStatusType Flag);

/**
 *@}
 */
 
/* Include App config file */
#include "UART001_Conf.h"

#endif /* UART001_H_ */

