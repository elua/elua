
/*CODE_BLOCK_BEGIN[UART001_Conf.c]*/
/*******************************************************************************
 Copyright (c) 2011, Infineon Technologies AG                                 **
 All rights reserved.                                                         **
                                                                              **
 Redistribution and use in source and binary forms, with or without           **
 modification,are permitted provided that the following conditions are met:   **
                                                                              **
 *Redistributions of source code must retain the above copyright notice,      **
 this list of conditions and the following disclaimer.                        **
 *Redistributions in binary form must reproduce the above copyright notice,   **
 this list of conditions and the following disclaimer in the documentation    **
 and/or other materials provided with the distribution.                       **
 *Neither the name of the copyright holders nor the names of its contributors **
 may be used to endorse or promote products derived from this software without** 
 specific prior written permission.                                           **
                                                                              **
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  **
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    **
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   **
 ARE  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   **
 LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         **
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         **
 SUBSTITUTE GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    **
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      **
 CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)       **
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   **
 POSSIBILITY OF SUCH DAMAGE.                                                  **
                                                                              **
 To improve the quality of the software, users are encouraged to share        **
 modifications, enhancements or bug fixes with Infineon Technologies AG       **
 dave@infineon.com).                                                          **
                                                                              **
********************************************************************************
**                                                                            **
**                                                                            **
** PLATFORM : Infineon XMC4000 Series                                         **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR   : App Developer                                                   **
**                                                                            **
** MAY BE CHANGED BY USER [yes/no]: Yes                                       **
**                                                                            **
**                                                                            **
*******************************************************************************/
/**
 * @file   UART001_Conf.c
 * 
 * @App Version UART001 <1.0.0>
 *
 * @brief  Configuration file generated based on UI settings 
 *         of UART001 App
 *
 */

/*******************************************************************************
 ** INCLUDE FILES                                                             **
 ******************************************************************************/
#include <DAVE3.h>




const UART001_HandleType UART001_Handle0  = 
{
  // Temp Code for testing Eval functions
   .UartRegs = USIC0_CH0, /* Usic Channel offset value */
   .Mode = 0, /* Mode */
   .StopBit = 0,/* StopBit */
   .Parity  = 0,/* Parity */
   .DataBits = 7,/* Word Length */
   .BaudRate  = 115200,/*Baud Rate */
   .TxLimit = 1,/* FIFO Tigger Level */
   .RxLimit = 1,/* FIFO Tigger Level */
   .TxFifoSize = 1,/* Tx FIFO Size */
   .RxFifoSize = 1,/* Rx FIFO Size */
   .RecvNoiseEn = 0, /* Protocol specific interrupt enable */
   .FormatErrEn = 0, /* Protocol specific interrupt enable */
   .FrameFinEn = 0, /* Protocol specific interrupt enable */   
   .BGR_STEP = (uint16_t)698,  /* Baud Rate Generation step value */
   .BGR_PDIV = (uint16_t)70,   /* Baud Rate Generation  pdiv value */
   .BGR_DCTQ = (uint16_t)9, /* Baud Rate Generation dctq value */   
   .BGR_SP = (uint16_t)6, /* Baud Rate Generation sampling point */
   .BGR_PCTQ = (uint8_t)0  /* Baud Rate Generation pctq value */
};


/*CODE_BLOCK_END*/
