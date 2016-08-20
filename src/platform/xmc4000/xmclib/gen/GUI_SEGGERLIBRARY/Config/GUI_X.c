/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.28 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software  belongs to SEGGER.
emWin is protected by international copyright laws.

This  file  has been  licensed to  Infineon Technologies AG,  a german
company at the address Am Campeon 1-12 in 85579 Neubiberg, Germany and
is  sublicensed  and  distributed  by   Infineon  Technologies  AG  in
accordance with the DAVE (TM) 3 Software License Agreement to be used 
for and with Infineon's Cortex-M0, M0+ and M4 based 32-bit microcon-
troller products only.

Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------*/
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
 * 2015-02-16:
 *     - Initial version<br>
 *
 * 2015-05-18:
 *     - GUI_X_Delay is modified
 * @endcond
 *
 */
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include <xmc_common.h>
#include "GUI.h"
#include "../gui_seggerlibrary_conf.h"
#if (GUI_SEGGERLIBRARY_RTOS_PRESENT == 0)
#include "SYSTIMER/systimer.h"
#endif

#if (CMSIS_RTOS_RTX_PRESENT == 1)
/* CMSIS_RTOS_RTX include files */
#include "CMSIS_RTOS_RTX/cmsis_rtos_rtx.h"
#include "CMSIS_RTOS_RTX/SRC/rt_Time.h"

#define SEMAPHORE_BLK_TIME   1000
/** Lock Semaphore */
osSemaphoreDef(LockSemaphore);
osSemaphoreId LockSemaphoreId;
int LockSemaphoreIndex;
#endif

#if (FREERTOS_PRESENT == 1)
/* FreeRTOS include files */
#include "FREERTOS/FreeRTOS.h"
#include "FREERTOS/task.h"
#include "FREERTOS/timers.h"
#include "FREERTOS/semphr.h"
#endif
/***********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
 /***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
#if ((GUI_SEGGERLIBRARY_RTOS_PRESENT == 0) || (CMSIS_RTOS_RTX_PRESENT == 1))
volatile uint32_t OS_TimeMS;
#endif

#if (FREERTOS_PRESENT == 1)
static xSemaphoreHandle xQueueMutex;
static xSemaphoreHandle xSemaTxDone;
#endif

/***********************************************************************************************************************
 * LOCAL ROUTINES
 **********************************************************************************************************************/

/*********************************************************************
*
*      Timing:
*                 GUI_X_GetTime()
*                 GUI_X_Delay(int)

  Some timing dependent routines require a GetTime
  and delay function. Default time unit (tick), normally is
  1 ms.
*/
#if (GUI_SEGGERLIBRARY_RTOS_PRESENT == 0)
/* Non-RTOS */
int GUI_X_GetTime(void) {
  OS_TimeMS = SYSTIMER_GetTime();
  return OS_TimeMS;
}

void GUI_X_Delay(int ms) {
  uint32_t tStart = OS_TimeMS;
  ms = ms * 1000;
  do
  {
    OS_TimeMS = SYSTIMER_GetTime();
  }
  while ((OS_TimeMS-tStart) < ms );
}
#endif

#if (CMSIS_RTOS_RTX_PRESENT == 1)
/* CMSIS RTOS support */
int GUI_X_GetTime(void)
{
	return OS_TimeMS++;
}

void GUI_X_Delay(int ms)
{
	osDelay( ms );
	OS_TimeMS += ms;
}
#endif

#if (FREERTOS_PRESENT == 1)
/* FreeRTOS support */
int GUI_X_GetTime(void)
{
  return ((int) xTaskGetTickCount());
}

void GUI_X_Delay(int ms)
{
	vTaskDelay( ms );
}
#endif
/*********************************************************************
*
*       GUI_X_Init()
*
* Note:
*     GUI_X_Init() is called from GUI_Init is a possibility to init
*     some hardware which needs to be up and running before the GUI.
*     If not required, leave this routine blank.
*/

void GUI_X_Init(void) {}


/*********************************************************************
*
*       GUI_X_ExecIdle
*
* Note:
*  Called if WM is in idle state
*/

void GUI_X_ExecIdle(void) {}


/*********************************************************************
*
* Multitasking:
*
* GUI_X_InitOS()
* GUI_X_GetTaskId()
* GUI_X_Lock()
* GUI_X_Unlock()
*
* Note:
* The following routines are required only if emWin is used in a
* true multi task environment, which means you have more than one
* thread using the emWin API.
* In this case the
* #define GUI_OS 1
* needs to be in GUIConf.h
*/
#if (CMSIS_RTOS_RTX_PRESENT == 1)
/* CMSIS RTOS Init OS */
void GUI_X_InitOS(void)
{
  /* Create Lock Semaphore */
  LockSemaphoreId = osSemaphoreCreate (osSemaphore(LockSemaphore), 1);
}

void GUI_X_Unlock(void)
{
  /* Release the Lock Semaphore */
  osSemaphoreRelease(LockSemaphoreId);
}

void GUI_X_Lock(void)
{
  /* Take the Lock semaphore  */
  LockSemaphoreIndex = osSemaphoreWait(LockSemaphoreId, SEMAPHORE_BLK_TIME);
}

/* Get Task handle */
U32 GUI_X_GetTaskId(void)
{
	return 1;
}
#endif

#if (FREERTOS_PRESENT == 1)
/* FreeRTOS Init OS */
void GUI_X_InitOS(void)
{
  /* Create Mutex lock */
  xQueueMutex = xSemaphoreCreateMutex();
  configASSERT (xQueueMutex != NULL);

  /* Queue Semaphore */
  vSemaphoreCreateBinary( xSemaTxDone );
  configASSERT ( xSemaTxDone != NULL );
}

void GUI_X_Unlock(void)
{
  xSemaphoreGive( xQueueMutex );
}

void GUI_X_Lock(void)
{
  if(xQueueMutex == NULL)
  {
    GUI_X_InitOS();
  }

  xSemaphoreTake( xQueueMutex, portMAX_DELAY );
}

/* Get Task handle */
U32 GUI_X_GetTaskId(void)
{
  return ((U32) xTaskGetCurrentTaskHandle());
}

void GUI_X_WaitEvent (void)
{
  while( xSemaphoreTake(xSemaTxDone, portMAX_DELAY ) != pdTRUE );
}


void GUI_X_SignalEvent (void)
{
  xSemaphoreGive( xSemaTxDone );
}
#endif

/*********************************************************************
*
*      Logging: OS dependent

Note:
  Logging is used in higher debug levels only. The typical target
  build does not use logging and does therefor not require any of
  the logging routines below. For a release build without logging
  the routines below may be eliminated to save some space.
  (If the linker is not function aware and eliminates unreferenced
  functions automatically)

*/

void GUI_X_Log     (const char *s) { GUI_USE_PARA(s); }
void GUI_X_Warn    (const char *s) { GUI_USE_PARA(s); }
void GUI_X_ErrorOut(const char *s) { GUI_USE_PARA(s); }


/*************************** End of file ****************************/
