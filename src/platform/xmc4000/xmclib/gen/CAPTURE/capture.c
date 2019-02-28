/**
 * @file capture.h
 * @date 2016-07-01
 *
 *
 * @cond
 ***********************************************************************************************************************
 * CAPTURE v4.0.2 - Configures the properties of CCU4 or CCU8 peripheral as a capture.
 *
 * Copyright (c) 2016, Infineon Technologies AG
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
 * 2016-07-01:
 *     - Initial version<br>
 *
 * @endcond
 *
 */

/**********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include "capture.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define CAPTURE_PERIOD_16BIT_MAX         (0xFFFFU)  /* Maximum period value */

/**********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * LOCAL ROUTINES
 **********************************************************************************************************************/
#ifdef CAPTURE_CCU4_USED
static void CAPTURE_CCU4_lInit(const CAPTURE_t *const handler);
static void CAPTURE_CCU4_lShadowTransfer(const CAPTURE_t *const handler);
/* @brief Retrieves the latest captured timer value */
static CAPTURE_STATUS_t CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(const XMC_CCU4_SLICE_t *const slice,
                                                                         const XMC_CCU4_SLICE_CAP_REG_SET_t set,
                                                                         uint32_t *const val_ptr);
#endif

#ifdef CAPTURE_CCU8_USED
static void CAPTURE_CCU8_lInit(const CAPTURE_t *const handler);
static void CAPTURE_CCU8_lShadowTransfer(const CAPTURE_t *const handler);
/* @brief Retrieves the latest captured timer value */
static CAPTURE_STATUS_t CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(const XMC_CCU8_SLICE_t *const slice,
                                                                         const XMC_CCU8_SLICE_CAP_REG_SET_t set,
                                                                         uint32_t *const val_ptr);
#endif

static void CAPTURE_lCalculateTimeTickFromTimerValue(const CAPTURE_t * const handler,
                                                     uint32_t * const timer_val_ptr,
                                                     bool is_increment);

static uint32_t CAPTURE_lCalculateTimeInNanoSecFromTimerTick(const CAPTURE_t * const handler,
                                                             uint32_t timer_val);

 /*********************************************************************************************************************
 * API IMPLEMENTATION
 **********************************************************************************************************************/
 /* Returns the version of the capture APP. */
DAVE_APP_VERSION_t CAPTURE_GetAppVersion(void)
{
  DAVE_APP_VERSION_t version;

  version.major = CAPTURE_MAJOR_VERSION;
  version.minor = CAPTURE_MINOR_VERSION;
  version.patch = CAPTURE_PATCH_VERSION;

  return version;
}

/* This function initializes a capture APP based on user configuration. */
CAPTURE_STATUS_t CAPTURE_Init(CAPTURE_t *const handler)
{
  CAPTURE_STATUS_t status;

  XMC_ASSERT ("CAPTURE_Init:handler NULL", (handler != NULL))

  status = CAPTURE_STATUS_SUCCESS;
  /* Check for APP instance is initialized or not */
  if (false == handler->initialized)
  {
#ifdef CAPTURE_CCU4_USED
    if (CAPTURE_MODULE_CCU4 == handler->capture_module)
    {
      status = (CAPTURE_STATUS_t) GLOBAL_CCU4_Init (handler->global_ccu4_handler);
      if (status == CAPTURE_STATUS_SUCCESS)
      {
        /* Configure CCU4 capture for the required time tick settings */
        CAPTURE_CCU4_lInit (handler);
      }
    }
#endif

#ifdef CAPTURE_CCU8_USED
    if (CAPTURE_MODULE_CCU8 == handler->capture_module)
    {
      status = (CAPTURE_STATUS_t) GLOBAL_CCU8_Init (handler->global_ccu8_handler);
      if (status == CAPTURE_STATUS_SUCCESS)
      {
        /* Configure CCU8 capture for the required time tick settings */
        CAPTURE_CCU8_lInit (handler);
      }
    }
#endif

#ifdef CAPTURE_GPIO_USED
    if (handler->input != NULL)
    {
      XMC_GPIO_Init (handler->input->port, handler->input->pin, handler->input_pin_config);
    }
#endif

#ifdef CAPTURE_INTERRUPT_ENABLED
    if (handler->capture_event_enable == true)
    {
      /* Set priority of the Transmit interrupt */
#if UC_FAMILY == XMC4
      NVIC_SetPriority (handler->interrupt_node, NVIC_EncodePriority (NVIC_GetPriorityGrouping (),
                                                                      handler->interrupt_priority,
                                                                      handler->interrupt_sub_priority));
#else
      NVIC_SetPriority(handler->interrupt_node, handler->interrupt_priority);
#endif

#if UC_SERIES == XMC14
      XMC_SCU_SetInterruptControl(handler->interrupt_node, handler->interrupt_request_source);
#endif

    }
#endif

    /* update the initialization flag as true for particular instance. */
    handler->initialized = true;

    /* Check whether the start of the timer is enabled during initialization or not */
    if (handler->start_control == true)
    {
      status = CAPTURE_Start(handler);
    }
  }

  return (status);
}

/* This function starts the capture timer. */
CAPTURE_STATUS_t CAPTURE_Start(const CAPTURE_t *const handler)
{
  CAPTURE_STATUS_t status;

  XMC_ASSERT ("CAPTURE_Start:handler NULL", (handler != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
#ifdef CAPTURE_CCU4_USED
    if (CAPTURE_MODULE_CCU4 == handler->capture_module)
    {
      (void)handler->ccu4_slice_ptr->CV[0];
      (void)handler->ccu4_slice_ptr->CV[1];
      (void)handler->ccu4_slice_ptr->CV[2];
      (void)handler->ccu4_slice_ptr->CV[3];

      /* Start the capture manually */
      XMC_CCU4_SLICE_ClearTimer (handler->ccu4_slice_ptr);
      /* Start the capture manually */
      XMC_CCU4_SLICE_StartTimer (handler->ccu4_slice_ptr);
    }
#endif

#ifdef CAPTURE_CCU8_USED
    if (CAPTURE_MODULE_CCU8 == handler->capture_module)
    {
      (void)handler->ccu8_slice_ptr->CV[0];
      (void)handler->ccu8_slice_ptr->CV[1];
      (void)handler->ccu8_slice_ptr->CV[2];
      (void)handler->ccu8_slice_ptr->CV[3];
      /* Start the capture manually */
      XMC_CCU8_SLICE_ClearTimer (handler->ccu8_slice_ptr);
      /* Start the capture manually */
      XMC_CCU8_SLICE_StartTimer (handler->ccu8_slice_ptr);
    }
#endif
    status = CAPTURE_STATUS_SUCCESS;
#ifdef CAPTURE_INTERRUPT_ENABLED
    if (handler->capture_event_enable == true)
    {
      NVIC_EnableIRQ (handler->interrupt_node);
    }
#endif
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}

/* This function stops the capture timer. */
CAPTURE_STATUS_t CAPTURE_Stop(const CAPTURE_t *const handler)
{
  CAPTURE_STATUS_t status;

  XMC_ASSERT ("CAPTURE_Stop:handler NULL", (handler != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
#ifdef CAPTURE_CCU4_USED
    if (CAPTURE_MODULE_CCU4 == handler->capture_module)
    {
      XMC_CCU4_SLICE_StopTimer (handler->ccu4_slice_ptr);
    }
#endif

#ifdef CAPTURE_CCU8_USED
    if (CAPTURE_MODULE_CCU8 == handler->capture_module)
    {
      XMC_CCU8_SLICE_StopTimer (handler->ccu8_slice_ptr);
    }
#endif

#ifdef CAPTURE_INTERRUPT_ENABLED
    if (handler->capture_event_enable == true)
    {
      NVIC_DisableIRQ (handler->interrupt_node);
    }
#endif

    status = CAPTURE_STATUS_SUCCESS;
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}

CAPTURE_TIMER_STATUS_t CAPTURE_GetTimerStatus(const CAPTURE_t *const handler)
{
  CAPTURE_TIMER_STATUS_t status = CAPTURE_TIMER_STATUS_IDLE;

  XMC_ASSERT ("CAPTURE_GetTimerStatus:handler NULL", (handler != NULL))

  if (true == handler->initialized)
  {
#ifdef CAPTURE_CCU4_USED
    if (CAPTURE_MODULE_CCU4 == handler->capture_module)
    {
      if (XMC_CCU4_SLICE_IsTimerRunning (handler->ccu4_slice_ptr))
      {
        status = CAPTURE_TIMER_STATUS_RUNNING;
      }
    }
#endif

#ifdef CAPTURE_CCU8_USED
    if (CAPTURE_MODULE_CCU8 == handler->capture_module)
    {
      if (XMC_CCU8_SLICE_IsTimerRunning (handler->ccu8_slice_ptr))
      {
        status = CAPTURE_TIMER_STATUS_RUNNING;
      }
    }
#endif
  }
  else
  {
    status = CAPTURE_TIMER_STATUS_NOT_INITIALISED;
  }

  return (status);
}

CAPTURE_STATUS_t CAPTURE_GetCapturedTime(const CAPTURE_t *const handler, uint32_t *const captured_time)
{
  CAPTURE_STATUS_t status = CAPTURE_STATUS_SUCCESS;
  CAPTURE_STATUS_t status1 = CAPTURE_STATUS_SUCCESS;

  XMC_ASSERT ("CAPTURE_GetCapturedTime:handler NULL", (handler != NULL))
  XMC_ASSERT ("CAPTURE_GetCapturedTime:NULL data pointer", (captured_time != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
#ifdef CAPTURE_CCU4_USED
    if (CAPTURE_MODULE_CCU4 == handler->capture_module)
    {
      if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
          (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
      {
        uint32_t captured_time_low_reg = 0U;
        uint32_t captured_time_high_reg = 0U;

        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_LOW,
                                                                  &captured_time_low_reg);

        status1 = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                   XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                   &captured_time_high_reg);

        /*calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &captured_time_low_reg, true);
        /* calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &captured_time_high_reg, false);
        /* add both high and lower register value*/
        *captured_time = captured_time_low_reg + captured_time_high_reg;

        if (status1 == CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED)
        {
          status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
        }
      }
      else
      {
        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                  captured_time);

        /* calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, captured_time, false);
      }
    }
#endif

#ifdef CAPTURE_CCU8_USED
    if (CAPTURE_MODULE_CCU8 == handler->capture_module)
    {
      if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
          (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
      {
        uint32_t captured_time_low_reg = 0U;
        uint32_t captured_time_high_reg = 0U;

        status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_LOW,
                                                                  &captured_time_low_reg);

        status1 = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                   XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                   &captured_time_high_reg);

        /*calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &captured_time_low_reg, true);
        /* calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &captured_time_high_reg, false);
        /* add both high and lower register value*/
        *captured_time = captured_time_low_reg + captured_time_high_reg;

        if (status1 == CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED)
        {
          status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
        }
      }
      else
      {
        status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                  captured_time);
        /* calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, captured_time, false);
      }
    }
#endif
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}


CAPTURE_STATUS_t CAPTURE_GetCapturedTimeInNanoSec(const CAPTURE_t *const handler, uint32_t *const captured_time)
{
  CAPTURE_STATUS_t status = CAPTURE_STATUS_SUCCESS;
  CAPTURE_STATUS_t status1 = CAPTURE_STATUS_SUCCESS;

  XMC_ASSERT ("CAPTURE_GetCapturedTimeInNanoSec:handler NULL", (handler != NULL))
  XMC_ASSERT ("CAPTURE_GetCapturedTimeInNanoSec:NULL data pointer", (captured_time != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
#ifdef CAPTURE_CCU4_USED
    if (CAPTURE_MODULE_CCU4 == handler->capture_module)
    {
      if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
          (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
      {
        uint32_t captured_time_low_reg = 0U;
        uint32_t captured_time_high_reg = 0U;

        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_LOW,
                                                                  &captured_time_low_reg);

        status1 = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                   XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                   &captured_time_high_reg);

        /*calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &captured_time_low_reg, true);
        /* calculate time in Nanosecond from timer tick and pre-scale value of captured register**/
        captured_time_low_reg = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, captured_time_low_reg);

        /* calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &captured_time_high_reg, false);
        /* calculate time in Nanosecond from timer tick and pre-scale value of captured_time_high_reg**/
        captured_time_high_reg = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, captured_time_high_reg);

        /* add both high and lower register value*/
        *captured_time = captured_time_low_reg + captured_time_high_reg;

        if (status1 == CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED)
        {
          status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
        }
      }
      else
      {
        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                  captured_time);

        /* calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, captured_time, true);
        /* calculate time in Nanosecond from timer tick and pre-scale value of captured register**/
        *captured_time = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, *captured_time);
      }
    }
#endif

#ifdef CAPTURE_CCU8_USED
    if (CAPTURE_MODULE_CCU8 == handler->capture_module)
    {
      if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
          (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
      {
        uint32_t captured_time_low_reg = 0U;
        uint32_t captured_time_high_reg = 0U;

        status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_LOW,
                                                                  &captured_time_low_reg);

        status1 = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                   XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                   &captured_time_high_reg);

        /*calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &captured_time_low_reg, true);
        /* calculate time in Nanosecond from timer tick and pre-scale value of captured register**/
        captured_time_low_reg = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, captured_time_low_reg);

        /* calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &captured_time_high_reg, false);
        /* calculate time in Nanosecond from timer tick and pre-scale value of captured_time_high_reg**/
        captured_time_high_reg = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, captured_time_high_reg);

        /* add both high and lower register value*/
        *captured_time = captured_time_low_reg + captured_time_high_reg;

        if (status1 == CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED)
        {
          status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
        }
      }
      else
      {
        status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                  captured_time);
        /* calculate time tick from capture value and pre-scale value of captured register**/
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, captured_time, true);
        /* calculate time in Nanosecond from timer tick and pre-scale value of captured register**/
        *captured_time = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, *captured_time);
      }
    }
#endif
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}

CAPTURE_STATUS_t CAPTURE_GetDutyCycle(const CAPTURE_t *const handler, uint32_t *const signal_duty)
{
  CAPTURE_STATUS_t status = CAPTURE_STATUS_SUCCESS;
  uint32_t duty_val = 0U;

  XMC_ASSERT ("CAPTURE_GetDutyCycle:handler NULL", (handler != NULL))
  XMC_ASSERT ("CAPTURE_GetDutyCycle:NULL data pointer", (signal_duty != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
    if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
        (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
    {
#ifdef CAPTURE_CCU4_USED
      if (CAPTURE_MODULE_CCU4 == handler->capture_module)
      {
        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                  &duty_val);
      }
#endif

#ifdef CAPTURE_CCU8_USED
      if (CAPTURE_MODULE_CCU8 == handler->capture_module)
      {
        status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                  &duty_val);
      }
#endif
      if (status == CAPTURE_STATUS_SUCCESS)
      {
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);
        *signal_duty = duty_val;
      }
      else
      {
        *signal_duty = 0U;
      }
    }
    else
    {
      status = CAPTURE_STATUS_INVALID_CONFIG;
    }
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}

CAPTURE_STATUS_t CAPTURE_GetDutyCycleInPercentage(const CAPTURE_t *const handler, uint32_t *const signal_duty)
{
  CAPTURE_STATUS_t status = CAPTURE_STATUS_SUCCESS;
  CAPTURE_STATUS_t status1 = CAPTURE_STATUS_SUCCESS;

  uint32_t period_val = 0U;
  uint32_t duty_val = 0U;
  uint32_t norm_period_val = 0U;

  XMC_ASSERT ("CAPTURE_GetDutyCycleInPercentage:handler NULL", (handler != NULL))
  XMC_ASSERT ("CAPTURE_GetDutyCycleInPercentage:NULL data pointer", (signal_duty != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
    if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
        (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
    {
#ifdef CAPTURE_CCU4_USED
      if (CAPTURE_MODULE_CCU4 == handler->capture_module)
      {
        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_LOW,
                                                                  &period_val);

        status1 = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                   XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);


          CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);

          CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);

      }
#endif

#ifdef CAPTURE_CCU8_USED
      if (CAPTURE_MODULE_CCU8 == handler->capture_module)
      {
        status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_LOW,
                                                                  &period_val);

        status1 = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                   XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);

        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);

        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);
      }
#endif
      period_val += duty_val;
      norm_period_val = (uint32_t)__CLZ (period_val);
      period_val = period_val << norm_period_val;
      duty_val = duty_val << norm_period_val;

      period_val = period_val >> 7U;
      duty_val = duty_val >> 7U;
      if (period_val > 0U)
      {
        /* Rounding Positive for Duty*/
        *signal_duty = ((duty_val * 100U) + (period_val >> 1)) / period_val;
      }
      else
      {
        *signal_duty = 0U;
        status = CAPTURE_STATUS_FAILURE;
      }

      if (status1 == CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED)
      {
        status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
      }

    }
    else
    {
      status = CAPTURE_STATUS_INVALID_CONFIG;
    }
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}

CAPTURE_STATUS_t CAPTURE_GetPeriod(const CAPTURE_t *const handler, uint32_t *const signal_period)
{
  CAPTURE_STATUS_t status = CAPTURE_STATUS_SUCCESS;
  CAPTURE_STATUS_t status1 = CAPTURE_STATUS_SUCCESS;
  uint32_t period_val = 0U;
  uint32_t duty_val = 0U;

  XMC_ASSERT ("CAPTURE_GetPeriod:handler NULL", (handler != NULL))
  XMC_ASSERT ("CAPTURE_GetPeriod:NULL data pointer", (signal_period != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
    if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
        (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
    {
#ifdef CAPTURE_CCU4_USED
      if (CAPTURE_MODULE_CCU4 == handler->capture_module)
      {
        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_LOW,
                                                                  &period_val);
        status1 = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                   XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);

        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);
      }
#endif

#ifdef CAPTURE_CCU8_USED
      if (CAPTURE_MODULE_CCU8 == handler->capture_module)
      {
        status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_LOW,
                                                                  &period_val);
        status1 = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                   XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);

        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);
      }
#endif
      *signal_period = period_val + duty_val;

      if (status1 == CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED)
      {
        status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
      }
    }
    else
    {
      status = CAPTURE_STATUS_INVALID_CONFIG;
    }
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}

CAPTURE_STATUS_t CAPTURE_GetPeriodInNanoSec(const CAPTURE_t *const handler,
                                            uint32_t *const signal_period)
{
  CAPTURE_STATUS_t status = CAPTURE_STATUS_SUCCESS;
  CAPTURE_STATUS_t status1 = CAPTURE_STATUS_SUCCESS;
  uint32_t period_val = 0U;
  uint32_t duty_val = 0U;

  XMC_ASSERT ("CAPTURE_GetPeriodInNanoSec:handler NULL", (handler != NULL))
  XMC_ASSERT ("CAPTURE_GetPeriodInNanoSec:NULL data pointer", (signal_period != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
    if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
        (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
    {
#ifdef CAPTURE_CCU4_USED
      if (CAPTURE_MODULE_CCU4 == handler->capture_module)
      {
        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_LOW,
                                                                  &period_val);
        CAPTURE_lCalculateTimeTickFromTimerValue(handler, &period_val, true);
        period_val = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, period_val);

        status1 = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                   XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);
        CAPTURE_lCalculateTimeTickFromTimerValue(handler, &duty_val, false);
        duty_val = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, duty_val);
      }
#endif

#ifdef CAPTURE_CCU8_USED
      if (CAPTURE_MODULE_CCU8 == handler->capture_module)
      {
        status =  CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                   XMC_CCU8_SLICE_CAP_REG_SET_LOW,
                                                                   &period_val);
        CAPTURE_lCalculateTimeTickFromTimerValue(handler, &period_val, true);
        period_val = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, period_val);

        status1 = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                   XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);
        CAPTURE_lCalculateTimeTickFromTimerValue(handler, &duty_val, false);
        duty_val = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, duty_val);
      }
#endif
      *signal_period = period_val + duty_val;

      if (status1 == CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED)
      {
        status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
      }
    }
    else
    {
      status = CAPTURE_STATUS_INVALID_CONFIG;
    }
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}

CAPTURE_STATUS_t CAPTURE_GetPeriodDutyCycle(const CAPTURE_t *const handler,
                                            uint32_t *const signal_period,
                                            uint32_t *const signal_duty)
{
  CAPTURE_STATUS_t status = CAPTURE_STATUS_SUCCESS;
  CAPTURE_STATUS_t status1 = CAPTURE_STATUS_SUCCESS;
  uint32_t period_val = 0U;
  uint32_t duty_val = 0U;

  XMC_ASSERT ("CAPTURE_GetPeriodDutyCycle:handler NULL", (handler != NULL))
  XMC_ASSERT ("CAPTURE_GetPeriodDutyCycle:NULL data pointer", (signal_period != NULL))
  XMC_ASSERT ("CAPTURE_GetPeriodDutyCycle:NULL data pointer", (signal_duty != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
    if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
        (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
    {
#ifdef CAPTURE_CCU4_USED
      if (CAPTURE_MODULE_CCU4 == handler->capture_module)
      {
        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_LOW,
                                                                  &period_val);
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);

        status1 = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                   XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);
      }
#endif

#ifdef CAPTURE_CCU8_USED
      if (CAPTURE_MODULE_CCU8 == handler->capture_module)
      {
        status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_LOW,
                                                                  &period_val);
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);

        status1 = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                   XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);
        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);
      }
#endif

      period_val = period_val + duty_val;
      *signal_duty = duty_val;
      *signal_period = period_val;

      if (status1 == CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED)
      {
        status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
      }
    }
    else
    {
      status = CAPTURE_STATUS_INVALID_CONFIG;
    }
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}

CAPTURE_STATUS_t CAPTURE_GetPeriodInNanoSecDutyCycleInPercentage(const CAPTURE_t *const handler,
                                                                 uint32_t *const signal_period,
                                                                 uint32_t *const signal_duty)
{
  CAPTURE_STATUS_t status = CAPTURE_STATUS_SUCCESS;
  CAPTURE_STATUS_t status1 = CAPTURE_STATUS_SUCCESS;
  uint32_t period_val = 0U;
  uint32_t duty_val = 0U;
  uint32_t period_val_nanosec = 0U;
  uint32_t duty_val_nanosec = 0U;
  uint32_t norm_period_val = 0U;

  XMC_ASSERT ("CAPTURE_GetPeriodInNanoSecDutyCycleInPercentage:handler NULL", (handler != NULL))
  XMC_ASSERT ("CAPTURE_GetPeriodInNanoSecDutyCycleInPercentage:NULL data pointer", (signal_period != NULL))
  XMC_ASSERT ("CAPTURE_GetPeriodInNanoSecDutyCycleInPercentage:NULL data pointer", (signal_duty != NULL))

  /* Check for APP instance is initialized or not */
  if (true == handler->initialized)
  {
    if ((handler->capture_edge_config == CAPTURE_EDGE_RISE_TO_RISE) ||
        (handler->capture_edge_config == CAPTURE_EDGE_FALL_TO_FALL))
    {
#ifdef CAPTURE_CCU4_USED
      if (CAPTURE_MODULE_CCU4 == handler->capture_module)
      {
        status = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                  XMC_CCU4_SLICE_CAP_REG_SET_LOW,
                                                                  &period_val);

        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);
        period_val_nanosec = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, period_val);


        status1 = CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(handler->ccu4_slice_ptr,
                                                                   XMC_CCU4_SLICE_CAP_REG_SET_HIGH,
                                                                   &duty_val);

        CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);
        duty_val_nanosec = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, duty_val);

      }
#endif

#ifdef CAPTURE_CCU8_USED
     if (CAPTURE_MODULE_CCU8 == handler->capture_module)
     {
       status = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                 XMC_CCU8_SLICE_CAP_REG_SET_LOW,
                                                                 &period_val);

       CAPTURE_lCalculateTimeTickFromTimerValue (handler, &period_val, true);
       period_val_nanosec = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, period_val);


       status1 = CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(handler->ccu8_slice_ptr,
                                                                  XMC_CCU8_SLICE_CAP_REG_SET_HIGH,
                                                                  &duty_val);
       CAPTURE_lCalculateTimeTickFromTimerValue (handler, &duty_val, false);
       duty_val_nanosec = CAPTURE_lCalculateTimeInNanoSecFromTimerTick(handler, duty_val);
     }
#endif
     period_val_nanosec = period_val_nanosec + duty_val_nanosec;
     *signal_period = period_val_nanosec;

     period_val += duty_val;

     norm_period_val = (uint32_t)__CLZ (period_val);

     period_val = period_val << norm_period_val;
     duty_val = duty_val << norm_period_val;

     period_val = period_val >> 7U;
     duty_val = duty_val >> 7U;

     if (period_val > 0U)
     {
       /* Rounding Positive*/
       *signal_duty = ((duty_val * 100U) + (period_val >> 1)) / period_val;
       if (status1 != CAPTURE_STATUS_SUCCESS)
       {
         status = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;
       }
     }
     else
     {
       *signal_duty = 0U;
       status = CAPTURE_STATUS_FAILURE;
     }
    }
    else
    {
      status = CAPTURE_STATUS_INVALID_CONFIG;
    }
  }
  else
  {
    status = CAPTURE_STATUS_FAILURE;
  }

  return (status);
}
/*********************************************************************************************************************
* PRIVATE API IMPLEMENTATION
**********************************************************************************************************************/
#ifdef CAPTURE_CCU4_USED
static void CAPTURE_CCU4_lInit(const CAPTURE_t *const handler)
{
  /* Configure the timer with required settings */
  XMC_CCU4_SLICE_CaptureInit(handler->ccu4_slice_ptr, handler->ccu4_slice_config_ptr);
  /* programs the timer period and compare register according to time interval value and do the shadow transfer */
  CAPTURE_CCU4_lShadowTransfer(handler);

  /************Configure External Events***************/
  /* Configure slice to a external event 0 */
  XMC_CCU4_SLICE_ConfigureEvent(handler->ccu4_slice_ptr, XMC_CCU4_SLICE_EVENT_0, handler->ccu4_event0_ptr);
  /* Configure slice to a external event 1 */
  XMC_CCU4_SLICE_ConfigureEvent(handler->ccu4_slice_ptr, XMC_CCU4_SLICE_EVENT_1, handler->ccu4_event1_ptr);
  XMC_CCU4_SLICE_Capture0Config(handler->ccu4_slice_ptr, XMC_CCU4_SLICE_EVENT_0);
  XMC_CCU4_SLICE_Capture1Config(handler->ccu4_slice_ptr, XMC_CCU4_SLICE_EVENT_1);

  #ifdef  CAPTURE_INTERRUPT_ENABLED
  if (true == handler->capture_event_enable)
  {
    /* Binds a period match event to an NVIC node  */
    XMC_CCU4_SLICE_SetInterruptNode(handler->ccu4_slice_ptr,
                                   XMC_CCU4_SLICE_IRQ_ID_EVENT0,
                                   handler->ccu4_capture_event_node);
    /* Enables a timer(period match) event  */
    XMC_CCU4_SLICE_EnableEvent(handler->ccu4_slice_ptr, XMC_CCU4_SLICE_IRQ_ID_EVENT0);
  }
  #endif

  /* Enable the clock for selected timer */
  XMC_CCU4_EnableClock(handler->global_ccu4_handler->module_ptr, handler->ccu_slice_number);
}

static void CAPTURE_CCU4_lShadowTransfer(const CAPTURE_t *const handler)
{
  /* programs the timer period register according to time interval value */
  XMC_CCU4_SLICE_SetTimerPeriodMatch(handler->ccu4_slice_ptr, CAPTURE_PERIOD_16BIT_MAX);
  /* programs the timer compare register for 50% duty cycle */
  XMC_CCU4_SLICE_SetTimerCompareMatch(handler->ccu4_slice_ptr, 0x0U);
  /* Transfers value from shadow timer registers to actual timer registers */
  XMC_CCU4_EnableShadowTransfer(handler->global_ccu4_handler->module_ptr, handler->shadow_mask);
}
#endif

#ifdef CAPTURE_CCU8_USED
static void CAPTURE_CCU8_lInit(const CAPTURE_t *const handler)
{
  /* Configure the timer with required settings */
  XMC_CCU8_SLICE_CaptureInit(handler->ccu8_slice_ptr, handler->ccu8_slice_config_ptr);
  /* programs the timer period and compare register according to time interval value and do the shadow transfer */
  CAPTURE_CCU8_lShadowTransfer(handler);

  /************Configure External Events***************/
  /* Configure slice to a external event 0 */
  XMC_CCU8_SLICE_ConfigureEvent(handler->ccu8_slice_ptr, XMC_CCU8_SLICE_EVENT_0, handler->ccu8_event0_ptr);
  /* Configure slice to a external event 1 */
  XMC_CCU8_SLICE_ConfigureEvent(handler->ccu8_slice_ptr, XMC_CCU8_SLICE_EVENT_1, handler->ccu8_event1_ptr);
  XMC_CCU8_SLICE_Capture0Config(handler->ccu8_slice_ptr, XMC_CCU8_SLICE_EVENT_0);
  XMC_CCU8_SLICE_Capture1Config(handler->ccu8_slice_ptr, XMC_CCU8_SLICE_EVENT_1);

  #ifdef  CAPTURE_INTERRUPT_ENABLED
  if (true == handler->capture_event_enable)
  {
    /* Binds a period match event to an NVIC node  */
    XMC_CCU8_SLICE_SetInterruptNode(handler->ccu8_slice_ptr,
                                    XMC_CCU8_SLICE_IRQ_ID_EVENT0,
                                    handler->ccu8_capture_event_node);
    /* Enables a timer(period match) event  */
    XMC_CCU8_SLICE_EnableEvent(handler->ccu8_slice_ptr, XMC_CCU8_SLICE_IRQ_ID_EVENT0);
  }
  #endif


  /* Enable the clock for selected timer */
  XMC_CCU8_EnableClock(handler->global_ccu8_handler->module_ptr, handler->ccu_slice_number);
}

static void CAPTURE_CCU8_lShadowTransfer(const CAPTURE_t *const handler)
{
  /* programs the timer period register according to time interval value */
  XMC_CCU8_SLICE_SetTimerPeriodMatch(handler->ccu8_slice_ptr, CAPTURE_PERIOD_16BIT_MAX);
  /* programs the timer compare register for 50% duty cycle */
  XMC_CCU8_SLICE_SetTimerCompareMatch(handler->ccu8_slice_ptr, XMC_CCU8_SLICE_COMPARE_CHANNEL_1, 0x0U);
  /* Transfers value from shadow timer registers to actual timer registers */
  XMC_CCU8_EnableShadowTransfer(handler->global_ccu8_handler->module_ptr, handler->shadow_mask);
}
#endif

static void CAPTURE_lCalculateTimeTickFromTimerValue(const CAPTURE_t *const handler,
                                                     uint32_t *const timer_val_ptr,
                                                     bool is_increment)
{
  /* Check for APP instance is initialized or not */
  uint32_t psc_psiv_val = 0U;
  bool is_float_prescaler = false;
  uint32_t cxv_captv_val = (uint32_t)((uint32_t)*timer_val_ptr & 0xFFFFU);
  uint32_t cxv_fpcv_val = (uint32_t)((uint32_t)*timer_val_ptr >> 16U) & 0xFU;

  /* Period measured should be added 1 to get actual value*/
  if (is_increment == true)
  {
    cxv_captv_val = cxv_captv_val + 1U;
  }
#ifdef CAPTURE_CCU4_USED
  if (CAPTURE_MODULE_CCU4 == handler->capture_module)
  {
    psc_psiv_val = handler->ccu4_slice_config_ptr->prescaler_initval;
    if (handler->ccu4_slice_config_ptr->prescaler_mode == (uint32_t)XMC_CCU4_SLICE_PRESCALER_MODE_FLOAT)
    {
      is_float_prescaler =  true;
    }
  }
#endif
#ifdef CAPTURE_CCU8_USED
  if (CAPTURE_MODULE_CCU8 == handler->capture_module)
  {
    psc_psiv_val = handler->ccu8_slice_config_ptr->prescaler_initval;
    if (handler->ccu8_slice_config_ptr->prescaler_mode == (uint32_t)XMC_CCU8_SLICE_PRESCALER_MODE_FLOAT)
    {
      is_float_prescaler =  true;
    }
  }
#endif

  if (is_float_prescaler == true)
  {
    int32_t loop = 0;
    uint32_t prescaler_value;
    uint32_t timer_val = 0U;

    prescaler_value = cxv_fpcv_val - psc_psiv_val;

    for (loop = (int32_t)prescaler_value; loop > 0; loop--)
    {
      timer_val = (uint32_t)timer_val << 1U;
      timer_val += 65535U;
    }

    timer_val += (uint32_t)(cxv_captv_val * (1UL << prescaler_value));
    *timer_val_ptr = timer_val;
  }
  else
  {
    *timer_val_ptr = cxv_captv_val;
  }
}

static uint32_t CAPTURE_lCalculateTimeInNanoSecFromTimerTick(const CAPTURE_t *const handler, uint32_t timer_val)
{
  uint64_t singal_period_long_long;
  uint32_t inv_sig_resolution_in_sec = 1U;
  uint32_t norm_timer_val;
  uint32_t norm_inv_sig_resolution_in_sec;
  uint32_t min_norm;
  uint32_t psc_psiv_val = 0U;

#ifdef CAPTURE_CCU4_USED
  if (CAPTURE_MODULE_CCU4 == handler->capture_module)
  {
    psc_psiv_val = handler->ccu4_slice_config_ptr->prescaler_initval;
    inv_sig_resolution_in_sec = handler->global_ccu4_handler->module_frequency >> psc_psiv_val;
  }
#endif
#ifdef CAPTURE_CCU8_USED
  if (CAPTURE_MODULE_CCU8 == handler->capture_module)
  {
    psc_psiv_val = handler->ccu8_slice_config_ptr->prescaler_initval;
    inv_sig_resolution_in_sec = handler->global_ccu8_handler->module_frequency >> psc_psiv_val;
  }
#endif

  norm_timer_val = (uint32_t)__CLZ(timer_val);
  norm_inv_sig_resolution_in_sec = (uint32_t)__CLZ(inv_sig_resolution_in_sec);

  if (norm_timer_val > norm_inv_sig_resolution_in_sec)
  {
    min_norm = norm_inv_sig_resolution_in_sec;
  }
  else
  {
    min_norm = norm_timer_val;
  }

  /* Normalized the value to get better precision*/
  timer_val = timer_val << min_norm;
  inv_sig_resolution_in_sec = inv_sig_resolution_in_sec << min_norm;

  singal_period_long_long = (((uint64_t) timer_val) * 1000000000U) / inv_sig_resolution_in_sec;

  return ((uint32_t)singal_period_long_long);
}

#ifdef  CAPTURE_CCU4_USED
/* @brief Retrieves the latest captured timer value from CCU4*/
static CAPTURE_STATUS_t CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue(const XMC_CCU4_SLICE_t * const slice,
                                                                         const XMC_CCU4_SLICE_CAP_REG_SET_t set,
                                                                         uint32_t * const val_ptr)
{
  CAPTURE_STATUS_t retval;

  XMC_ASSERT ("CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue:Invalid Register Set",
              ((set == XMC_CCU4_SLICE_CAP_REG_SET_LOW) || (set == XMC_CCU4_SLICE_CAP_REG_SET_HIGH)))

  XMC_ASSERT ("CAPTURE_CCU4_lSLICE_GetLastestCapturedTimerValue:Invalid Extended Capture Mode ",
              ((slice->TC) & CCU4_CC4_TC_ECM_Msk) == 0)

  retval = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;

  if (set == XMC_CCU4_SLICE_CAP_REG_SET_HIGH)
  {
    /* read capture values*/
    *val_ptr = slice->CV[CAPTURE_SLICE_CAP_REGISTER_3];
    /* check for new data or full Flag*/
    if ((*val_ptr) & CCU4_CC4_CV_FFL_Msk)
    {
      retval = CAPTURE_STATUS_SUCCESS;
    }
    else
    {
      volatile uint32_t capture_reg_value_low;
      capture_reg_value_low = slice->CV[CAPTURE_SLICE_CAP_REGISTER_2];
      /* check for new data or full Flag*/
      if (capture_reg_value_low & CCU4_CC4_CV_FFL_Msk)
      {
        retval = CAPTURE_STATUS_SUCCESS;
        *val_ptr = capture_reg_value_low;
      }
    }
  }
  else
  {
    /* read capture values*/
    *val_ptr = slice->CV[CAPTURE_SLICE_CAP_REGISTER_1];
    /* check for new data or full Flag*/
    if ((*val_ptr) & CCU4_CC4_CV_FFL_Msk)
    {
      retval = CAPTURE_STATUS_SUCCESS;
    }
    else
    {
      volatile uint32_t capture_reg_value_low;
      capture_reg_value_low = slice->CV[CAPTURE_SLICE_CAP_REGISTER_0];
      /* check for new data or full Flag*/
      if (capture_reg_value_low & CCU4_CC4_CV_FFL_Msk)
      {
        retval = CAPTURE_STATUS_SUCCESS;
        *val_ptr = capture_reg_value_low;
      }
    }
  }

  return retval;
}
#endif

#ifdef  CAPTURE_CCU8_USED
/* @brief Retrieves the latest captured timer value from CCU8*/
static CAPTURE_STATUS_t CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue(const XMC_CCU8_SLICE_t * const slice,
                                                                         const XMC_CCU8_SLICE_CAP_REG_SET_t set,
                                                                         uint32_t * const val_ptr)
{
  CAPTURE_STATUS_t retval;

  XMC_ASSERT ("CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue:Invalid Register Set",
              ((set == XMC_CCU8_SLICE_CAP_REG_SET_LOW) || (set == XMC_CCU8_SLICE_CAP_REG_SET_HIGH)))

  XMC_ASSERT ("CAPTURE_CCU8_lSLICE_GetLastestCapturedTimerValue:Invalid Extended Capture Mode ",
              ((slice->TC) & CCU8_CC8_TC_ECM_Msk) == 0)

  retval = CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED;

  if (set == XMC_CCU8_SLICE_CAP_REG_SET_HIGH)
  {
    /* read capture values*/
    *val_ptr = slice->CV[CAPTURE_SLICE_CAP_REGISTER_3];
    /* check for new data or full Flag*/
    if ((*val_ptr) & CCU8_CC8_CV_FFL_Msk)
    {
      retval = CAPTURE_STATUS_SUCCESS;
    }
    else
    {
      volatile uint32_t capture_reg_value_low;
      capture_reg_value_low = slice->CV[CAPTURE_SLICE_CAP_REGISTER_2];
      /* check for new data or full Flag*/
      if (capture_reg_value_low & CCU8_CC8_CV_FFL_Msk)
      {
        retval = CAPTURE_STATUS_SUCCESS;
        *val_ptr = capture_reg_value_low;
      }
    }
  }
  else
  {
    /* read capture values*/
    *val_ptr = slice->CV[CAPTURE_SLICE_CAP_REGISTER_1];
    /* check for new data or full Flag*/
    if ((*val_ptr) & CCU8_CC8_CV_FFL_Msk)
    {
      retval = CAPTURE_STATUS_SUCCESS;
    }
    else
    {
      volatile uint32_t capture_reg_value_low;
      capture_reg_value_low = slice->CV[CAPTURE_SLICE_CAP_REGISTER_0];
      /* check for new data or full Flag*/
      if (capture_reg_value_low & CCU8_CC8_CV_FFL_Msk)
      {
        retval = CAPTURE_STATUS_SUCCESS;
        *val_ptr = capture_reg_value_low;
      }
    }
  }

  return retval;
}
#endif
