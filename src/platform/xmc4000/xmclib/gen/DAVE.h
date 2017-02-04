
/**
 * @cond
 ***********************************************************************************************************************
 *
 * Copyright (c) 2015, Infineon Technologies AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
 * following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this list of conditions and the  following
 *   disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *   following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *   Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
 *   products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * To improve the quality of the software, users are encouraged to share modifications, enhancements or bug fixes
 * with Infineon Technologies AG (dave@infineon.com).
 ***********************************************************************************************************************
 *
 * Change History
 * --------------
 *
 * 2014-06-16:
 *     - Initial version<br>
 * @endcond
 *
 */

#ifndef _DAVE_H_
#define _DAVE_H_

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/

/** #include DAVE APP Header files. */
#include "xmc_common.h"  

#include "CLOCK_XMC4/clock_xmc4.h"
  

#include "UART/uart.h"
  

#include "RTC/rtc.h"
  

#include "CPU_CTRL_XMC4/cpu_ctrl_xmc4.h"
  

#include "SYSTIMER/systimer.h"

#include "SDMMC_BLOCK/sdmmc_block.h"

#include "FATFS/fatfs.h"

  
#include "GUI_SEGGERLIBRARY/gui_seggerlibrary.h"


#include "SPI_MASTER/spi_master.h"

/**********************************************************************************************************************
 * ENUMS
 **********************************************************************************************************************/
typedef enum DAVE_STATUS
{
  DAVE_STATUS_SUCCESS = 0,
  DAVE_STATUS_FAILURE,
  DAVE_STATUS_ALREADY_INITIALIZED
} DAVE_STATUS_t;

/***********************************************************************************************************************
* API PROTOTYPES
***********************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

DAVE_STATUS_t DAVE_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /** ifndef _DAVE_H_ */

