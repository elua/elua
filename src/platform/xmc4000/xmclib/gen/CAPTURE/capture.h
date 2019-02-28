/**
 * @file capture.h
 * @date 2016-07-01
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

#ifndef CAPTURE_H
#define CAPTURE_H
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include "capture_conf.h"
#ifdef  CAPTURE_CCU4_USED
#include <GLOBAL_CCU4/global_ccu4.h>
#endif
#ifdef  CAPTURE_CCU8_USED
#include <GLOBAL_CCU8/global_ccu8.h>
#endif
#ifdef CAPTURE_GPIO_USED
#include "xmc_gpio.h"
#endif
#include <DAVE_Common.h>
/***********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * ENUMS
 **********************************************************************************************************************/
/**
 * @ingroup CAPTURE_enumerations
 * @{
 */

/**
 * @brief This type identifies the CCU4 or CCU8 capture selected.
 */
typedef enum CAPTURE_MODULE
{
  CAPTURE_MODULE_CCU4 = 0U, /**< CCU4 is selected */
  CAPTURE_MODULE_CCU8       /**< CCU8 is selected */
} CAPTURE_MODULE_t;

/**
 * @brief status of the CAPTURE APP
 */
typedef enum CAPTURE_STATUS
{
  CAPTURE_STATUS_SUCCESS = 0U,         /**< Status success */
  CAPTURE_STATUS_FAILURE,              /**< Status failure */
  CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED, /**< New values are not captured */
  CAPTURE_STATUS_INVALID_CONFIG        /**< Configuration is invalid */
} CAPTURE_STATUS_t;

/**
 * @brief status of the capture timer status
 */
typedef enum CAPTURE_TIMER_STATUS
{
  CAPTURE_TIMER_STATUS_NOT_INITIALISED = 0U, /**< Timer is not initialised */
  CAPTURE_TIMER_STATUS_IDLE,                 /**< Timer is in idle state */
  CAPTURE_TIMER_STATUS_RUNNING               /**< Timer is in running state */
} CAPTURE_TIMER_STATUS_t;

/**
 * @brief Signal analysis config
 */
typedef enum CAPTURE_EDGE
{
  CAPTURE_EDGE_RISE_TO_RISE = 0U, /**< Configured to capture the signal from rise to rise edge.
                                       In this configuration both period and duty can be captured.*/
  CAPTURE_EDGE_RISE_TO_FALL,      /**< Configured to capture the signal from rise to fall edge.
                                       In this configuration both period and duty cannot be captured.*/
  CAPTURE_EDGE_FALL_TO_RISE,      /**< Configured to capture the signal from fall to rise edge.
                                       In this configuration both period and duty cannot be captured.*/
  CAPTURE_EDGE_FALL_TO_FALL       /**< Configured to capture the signal from fall to fall edge.
                                       In this configuration both period and duty can be captured.*/
} CAPTURE_EDGE_t;

/**
 * @brief Define hardware capture registers.
 */
typedef enum CAPTURE_SLICE_CAP_REGISTER
{
  CAPTURE_SLICE_CAP_REGISTER_0  = 0U ,/**< C0V Register */
  CAPTURE_SLICE_CAP_REGISTER_1  = 1U ,/**< C1V Register */
  CAPTURE_SLICE_CAP_REGISTER_2  = 2U ,/**< C2V Register */
  CAPTURE_SLICE_CAP_REGISTER_3  = 3U  /**< C3V Register */
} CAPTURE_SLICE_CAP_REGISTER_t;

/**
 * @}
 */
/***********************************************************************************************************************
* DATA STRUCTURES
***********************************************************************************************************************/
/**
 * @ingroup CAPTURE_datastructures
 * @{
 */
#ifdef CAPTURE_GPIO_USED
/**
 * @brief Port pin selection for capture input
 */
typedef struct CAPTURE_GPIO
{
  XMC_GPIO_PORT_t* port;  /**< Reference to the port configuration */
  uint8_t pin;  /**< Selected pin */
} CAPTURE_GPIO_t;
#endif
/**
 * @brief Initialization parameters of the CAPTURE APP
 */
typedef struct CAPTURE
{
  uint32_t min_frequency; /**< minimum frequency that can be captured by the APP */
  uint32_t max_frequency; /**< maximum frequency that can be captured by the APP */
  const uint32_t shadow_mask;  /**< shadow transfer mask for the selected capture */
#ifdef  CAPTURE_CCU4_USED
  const XMC_CCU4_SLICE_EVENT_CONFIG_t *const ccu4_event0_ptr; /**< capture event 0 configuration */
  const XMC_CCU4_SLICE_EVENT_CONFIG_t *const ccu4_event1_ptr; /**< capture event 1 configuration */
  GLOBAL_CCU4_t *const global_ccu4_handler; /**< Reference to CCU4GLOBAL APP handler */
  XMC_CCU4_SLICE_t *const ccu4_slice_ptr;  /**< Reference to CCU4-CC4 slice identifier data handler */
  const XMC_CCU4_SLICE_CAPTURE_CONFIG_t *const ccu4_slice_config_ptr; /**< Reference to initialization data structure of
                                                                     the core capture functionality */
  const XMC_CCU4_SLICE_SR_ID_t ccu4_capture_event_node; /**< Service Request Id for period match event */
#endif
#ifdef  CAPTURE_CCU8_USED
  const XMC_CCU8_SLICE_EVENT_CONFIG_t *const ccu8_event0_ptr; /**< capture event 0 configuration */
  const XMC_CCU8_SLICE_EVENT_CONFIG_t *const ccu8_event1_ptr; /**< capture event 1 configuration */
  GLOBAL_CCU8_t *const global_ccu8_handler; /**< Reference to CCU8GLOBAL APP handler */
  XMC_CCU8_SLICE_t *const ccu8_slice_ptr; /**< Reference to CCU8-CC8 slice identifier data handler */
  const XMC_CCU8_SLICE_CAPTURE_CONFIG_t *const ccu8_slice_config_ptr; /**< Reference to initialization data structure of
                                                                     the core capture functionality */
  const XMC_CCU8_SLICE_SR_ID_t ccu8_capture_event_node; /**< Service Request Id for period match event */
#endif
#ifdef CAPTURE_GPIO_USED
  const CAPTURE_GPIO_t *const input; /**< capture input pin, if gpio is selected */
  const XMC_GPIO_CONFIG_t *const input_pin_config; /**< capture input pin configuration, if gpio is selected */
#endif
#ifdef  CAPTURE_INTERRUPT_ENABLED
  const XMC_SCU_INTERRUPT_EVENT_HANDLER_t irq_handler; /**< User defined capture interrupt handler */
  const IRQn_Type interrupt_node; /**< capture interrupt node */
  const uint32_t interrupt_priority; /**< capture interrupt preemption priority */
  const uint32_t interrupt_sub_priority; /**< capture interrupt sub-priority */
 #if (UC_SERIES == XMC14)
  const XMC_SCU_IRQCTRL_t interrupt_request_source; /**< capture interrupt request source */
 #endif
#endif
  const uint8_t ccu_slice_number;  /* Timer being used */
  const CAPTURE_MODULE_t capture_module; /**< Indicate which capture module is being used from CCU4 and CCU8 */
  CAPTURE_EDGE_t capture_edge_config; /**< capture edges, for which capturing is done */
  const bool start_control; /**< Indicate whether to start the APP during initialization itself */
  bool capture_event_enable; /**< Indicate the generation of period match event */
  bool initialized;  /* flag to indicate the initialization state of the APP instance */
} CAPTURE_t;

/**
 * @}
 */
/***********************************************************************************************************************
* API Prototypes
***********************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @ingroup CAPTURE_apidoc
 * @{
 */
/**
 * @brief Get CAPTURE APP version
 * @return DAVE_APP_VERSION_t APP version information (major, minor and patch number)
 * <BR>
 * \par<b>Description:</b><br>
 * The function can be used to check application software compatibility with a
 * specific version of the APP.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *   DAVE_STATUS_t status;
 *   DAVE_APP_VERSION_t app_version;
 *
 *   status = DAVE_Init();    // CAPTURE_Init() is called from DAVE_Init()
 *
 *   app_version = CAPTURE_GetAppVersion();
 *
 *   if (app_version.major != 4U)
 *   {
 *     // Probably, not the right version.
 *   }
 *
 *   while(1U)
 *   {
 *   }
 *   return 1;
 * }
 * @endcode<BR>
 */
DAVE_APP_VERSION_t CAPTURE_GetAppVersion(void);

/**
 * @brief Initializes a CAPTURE with generated configuration.
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @return CAPTURE_STATUS_t\n  CAPTURE_STATUS_SUCCESS : if initialization is successful\n
 *                           CAPTURE_STATUS_FAILURE : if initialization is failed\n
 * <BR>
 * \par<b>Description:</b><br>
 * <ul>
 * <li>Enable the clock for the slice and invoke the LLD API with generated configuration handle.</li>
 * <li>Load the Period, Compare and Prescaler shadow registers with the generated values and enable the shadow transfer
 * request. This loads the values into the actual registers and start the CAPTURE based on the configuration.</li>
 * <li>If "Start after initialization" is not enabled, CAPTURE_Start() can be invoked to start the capture.</li>
 * </ul>
 * <BR>
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *  while(1)
 *  {
 *  }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_Init(CAPTURE_t *const handler);

/**
 * @brief       Starts the capture if the initialization of the APP is successful.
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @return CAPTURE_STATUS_t\n CAPTURE_STATUS_SUCCESS : if capture start is successful\n
 *                          CAPTURE_STATUS_FAILURE : if capture start is failed\n
 * <BR>
 *
 * \par<b>Description:</b><br>
 * If "Start after initialization" is not enabled, CAPTURE_Start() has to be invoked to start the capture. CAPTURE_Stop()
 * can be used to stop the Timer. No need to reconfigure the capture to start again.
 *
 * <BR>
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *  CAPTURE_STATUS_t capture_status;
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *  if(init_status == DAVE_STATUS_SUCCESS)
 *  {
 *    capture_status = CAPTURE_Start(&CAPTURE_0);
 *  }
 *  while(1)
 *  {
 *  }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_Start(const CAPTURE_t *const handler);

/**
 * @brief Stops the CAPTURE, if it is running.
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @return CAPTURE_STATUS_t\n CAPTURE_STATUS_SUCCESS : if capture is running and stop is successful\n
 *                            CAPTURE_STATUS_FAILURE : if capture is in idle state, and stop is called\n
 *<BR>
 *
 * \par<b>Description:</b><br>
 * Clears the Timer run bit to stop. No further event is generated.
 *
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *  CAPTURE_STATUS_t capture_status;
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *  if(init_status == DAVE_STATUS_SUCCESS)
 *  {
 *    capture_status = CAPTURE_Start(&CAPTURE_0);
 *  }
 *
 *  if (capture_status == CAPTURE_STATUS_SUCCESS)
 *  {
 *    capture_status = CAPTURE_Stop(&CAPTURE_0);
 *  }
 *  while(1)
 *  {
 *  }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_Stop(const CAPTURE_t *const handler);

/**
 * @brief Returns the running state of the capture.
 *
 * @param handler pointer to the CAPTURE APP configuration.
 *
 * @return CAPTURE_TIMER_STATUS_t\n CAPTURE_TIMER_STATUS_NOT_INITIALISED : if timer is not initialised\n
 *                                  CAPTURE_TIMER_STATUS_IDLE : if the timer is in idle state\n
 *                                  CAPTURE_TIMER_STATUS_RUNNING : if the timer is running\n
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetTimerStatus() reads the run bit of the capture to indicate the actual state of the CAPTURE.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *  CAPTURE_STATUS_t capture_status;
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *  if(init_status == DAVE_STATUS_SUCCESS)
 *  {
 *    capture_status = CAPTURE_Start(&CAPTURE_0);
 *  }
 *
 *  if (CAPTURE_GetTimerStatus(&CAPTURE_0))
 *  {
 *    while(CAPTURE_GetTimerStatus(&CAPTURE_0));
 *
 *    capture_status = CAPTURE_Stop(&CAPTURE_0);
 *  }
 *  while(1)
 *  {
 *  }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_TIMER_STATUS_t CAPTURE_GetTimerStatus(const CAPTURE_t *const handler);

/**
 * @brief Returns the last capture timer value in timer ticks
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @param captured_time Out Parameter of the API. Stores the captured timer value into this out parameter.
 * @return <BR>
 *     ::CAPTURE_STATUS_t  Returns CAPTURE_STATUS_SUCCESS if there was new value present in the capture registers.
 *                         Returns CAPTURE_STATUS_FAILURE if there was failure to get capture handle or system failure event occurred.
 *                         Returns CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED if there is same timer value which is already captured and delivered.
 *                         Returns CAPTURE_STATUS_INVALID_CONFIG if desire output is not expect for current configuration.
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetCapturedTime() reads the current value of capture register of timer and returns to application.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *   if(init_status == DAVE_STATUS_FAILURE)
 *   {
 *     // Placeholder for error handler code. The while loop below can be replaced with an user error handler.
 *     XMC_DEBUG("DAVE APPs initialization failed\n");
 *
 *     while(1U)
 *     {
 *     }
 *   }
 *   while(1)
 *   {
 *     uint32_t captured_time=0, last_captured_time=0;
 *     if(CAPTURE_GetCapturedTime(&CAPTURE_0, &captured_time) == CAPTURE_STATUS_SUCCESS)
 *     {
 *       last_captured_time = captured_time;
 *     }
 *   }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_GetCapturedTime(const CAPTURE_t *const handler, uint32_t *const captured_time);

/**
 * @brief Returns the last capture timer value in Nanoseconds
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @param captured_time Out Parameter of the API. Stores the captured timer value into this out parameter.
 * @return <BR>
 *     ::CAPTURE_STATUS_t  Returns CAPTURE_STATUS_SUCCESS if there was new value present in the capture registers.
 *                         Returns CAPTURE_STATUS_FAILURE if there was failure to get capture handle or system failure event occurred.
 *                         Returns CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED if there is same timer value which is already captured and delivered.
 *                         Returns CAPTURE_STATUS_INVALID_CONFIG if desire output is not expect for current configuration.
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetCapturedTimeInNanoSec() reads the current value of capture register of timer and returns capture
 *                         time for the event in nanoseconds to application.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *   if(init_status == DAVE_STATUS_FAILURE)
 *   {
 *     // Placeholder for error handler code. The while loop below can be replaced with an user error handler.
 *     XMC_DEBUG("DAVE APPs initialization failed\n");
 *
 *     while(1U)
 *     {
 *     }
 *   }
 *   while(1)
 *   {
 *     uint32_t captured_time=0, last_captured_time=0;
 *     if(CAPTURE_GetCapturedTimeInNanoSec(&CAPTURE_0, &captured_time) == CAPTURE_STATUS_SUCCESS)
 *     {
 *       last_captured_time = captured_time;
 *     }
 *   }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_GetCapturedTimeInNanoSec(const CAPTURE_t *const handler, uint32_t *const captured_time);

/**
 * @brief Returns duty cycle of input signal in term of timer ticks
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @param signal_duty Out Parameter of the API. Stores duty cycle in timer tick into this out parameter.
 * @return <BR>
 *     ::CAPTURE_STATUS_t  Returns CAPTURE_STATUS_SUCCESS if there was duty cycle based on new capture data.
 *                         Returns CAPTURE_STATUS_FAILURE if there was failure to get capture handle or system failure event occurred.
 *                         Returns CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED if there is same capture value for which duty cycle value is already delivered.
 *                         Returns CAPTURE_STATUS_INVALID_CONFIG if desire output is not expect for current configuration.
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetDutyCycle() calculate Duty Cycle based on captured register and pre-scale values.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *   if(init_status == DAVE_STATUS_FAILURE)
 *   {
 *     // Placeholder for error handler code. The while loop below can be replaced with an user error handler.
 *     XMC_DEBUG("DAVE APPs initialization failed\n");
 *
 *     while(1U)
 *     {
 *     }
 *   }
 *   while(1)
 *   {
 *     uint32_t signal_duty=0, last_signal_duty=0;
 *     if(CAPTURE_GetDutyCycle (&CAPTURE_0, &signal_duty) == CAPTURE_STATUS_SUCCESS)
 *     {
 *       last_signal_duty = signal_duty;
 *     }
 *   }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_GetDutyCycle(const CAPTURE_t *const handler, uint32_t *const signal_duty);

/**
 * @brief Returns period of input signal in term of timer ticks
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @param signal_period Out Parameter of the API. Stores period in timer ticks into this out parameter.
 * @return <BR>
 *     ::CAPTURE_STATUS_t  Returns CAPTURE_STATUS_SUCCESS if there was period based on new capture data.
 *                         Returns CAPTURE_STATUS_FAILURE if there was failure to get capture handle or system failure event occurred.
 *                         Returns CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED if there is same capture value for which period value is already delivered..
 *                         Returns CAPTURE_STATUS_INVALID_CONFIG if desire output is not expect for current configuration.
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetPeriod() calculate period based on captured register and pre-scale values.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *   if(init_status == DAVE_STATUS_FAILURE)
 *   {
 *     // Placeholder for error handler code. The while loop below can be replaced with an user error handler.
 *     XMC_DEBUG("DAVE APPs initialization failed\n");
 *
 *     while(1U)
 *     {
 *     }
 *   }
 *   while(1)
 *   {
 *     uint32_t signal_period=0, last_signal_period=0;
 *     if(CAPTURE_GetPeriod (&CAPTURE_0, &signal_period) == CAPTURE_STATUS_SUCCESS)
 *     {
 *       last_signal_period = signal_period;
 *     }
 *   }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_GetPeriod(const CAPTURE_t *const handler, uint32_t *const signal_period);

/**
 * @brief Returns period and duty cycle of input signal in term of timer ticks
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @param signal_period Out Parameter of the API. Stores period in timer ticks into this out parameter.
 * @param signal_duty Out Parameter of the API. Stores duty cycle in timer tick into this out parameter.
 * @return <BR>
 *     ::CAPTURE_STATUS_t  Returns CAPTURE_STATUS_SUCCESS if there was outputs on new capture data.
 *                         Returns CAPTURE_STATUS_FAILURE if there was failure to get capture handle or system failure event occurred.
 *                         Returns CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED if there is same capture value for which outputs are already delivered..
 *                         Returns CAPTURE_STATUS_INVALID_CONFIG if desire output is not expect for current configuration.
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetPeriodDutyCycle() calculate period and duty cycle based on captured register and pre-scale values.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *   if(init_status == DAVE_STATUS_FAILURE)
 *   {
 *     // Placeholder for error handler code. The while loop below can be replaced with an user error handler.
 *     XMC_DEBUG("DAVE APPs initialization failed\n");
 *
 *     while(1U)
 *     {
 *     }
 *   }
 *   while(1)
 *   {
 *     uint32_t signal_period=0, last_signal_period=0;
 *     uint32_t signal_duty=0, last_signal_duty=0;
 *     if(CAPTURE_GetPeriodDutyCycle (&CAPTURE_0, &signal_period, &signal_duty) == CAPTURE_STATUS_SUCCESS)
 *     {
 *       last_signal_period = signal_period;
 *       last_signal_duty = signal_duty;
 *     }
 *   }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_GetPeriodDutyCycle(const CAPTURE_t *const handler,
                                            uint32_t *const signal_period,
                                            uint32_t *const signal_duty);

/**
 * @brief Returns duty cycle of input signal in term of percentage of period of input signal
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @param signal_duty Out Parameter of the API. Stores duty cycle in this out parameter.
 * @return <BR>
 *     ::CAPTURE_STATUS_t  Returns CAPTURE_STATUS_SUCCESS if there was duty cycle based on new capture data.
 *                         Returns CAPTURE_STATUS_FAILURE if there was failure to get capture handle or system failure event occurred.
 *                         Returns CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED if there is same capture value for which duty cycle value is already delivered.
 *                         Returns CAPTURE_STATUS_INVALID_CONFIG if desire output is not expect for current configuration.
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetDutyCycle() calculate Duty Cycle in percentage based on captured register, pre-scale values and period of input signal.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *   if(init_status == DAVE_STATUS_FAILURE)
 *   {
 *     // Placeholder for error handler code. The while loop below can be replaced with an user error handler.
 *     XMC_DEBUG("DAVE APPs initialization failed\n");
 *
 *     while(1U)
 *     {
 *     }
 *   }
 *   while(1)
 *   {
 *     uint32_t signal_duty=0, last_signal_duty=0;
 *     if(CAPTURE_GetDutyCycleInPercentage (&CAPTURE_0, &signal_duty) == CAPTURE_STATUS_SUCCESS)
 *     {
 *       last_signal_duty = signal_duty;
 *     }
 *   }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_GetDutyCycleInPercentage(const CAPTURE_t *const handler, uint32_t *const signal_duty);

/**
 * @brief Returns period of input signal in nanosecond granularity
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @param signal_period Out Parameter of the API. Stores period in this out parameter.
 * @return <BR>
 *     ::CAPTURE_STATUS_t  Returns CAPTURE_STATUS_SUCCESS if there was period based on new capture data.
 *                         Returns CAPTURE_STATUS_FAILURE if there was failure to get capture handle or system failure event occurred.
 *                         Returns CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED if there is same capture value for which period value is already delivered..
 *                         Returns CAPTURE_STATUS_INVALID_CONFIG if desire output is not expect for current configuration.
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetPeriod() calculate period based on captured register, pre-scale values and configured clock.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *   if(init_status == DAVE_STATUS_FAILURE)
 *   {
 *     // Placeholder for error handler code. The while loop below can be replaced with an user error handler.
 *     XMC_DEBUG("DAVE APPs initialization failed\n");
 *
 *     while(1U)
 *     {
 *     }
 *   }
 *   while(1)
 *   {
 *     uint32_t signal_period=0, last_signal_period=0;
 *     if(CAPTURE_GetPeriodInNanoSec (&CAPTURE_0, &signal_period) == CAPTURE_STATUS_SUCCESS)
 *     {
 *       last_signal_period = signal_period;
 *     }
 *   }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_GetPeriodInNanoSec(const CAPTURE_t *const handler, uint32_t *const signal_period);


/**
 * @brief Returns period in nanosecond granularity and duty cycle in percentage of input signal.
 *
 * @param handler pointer to the CAPTURE APP configuration.
 * @param signal_period Out Parameter of the API. Stores period in this out parameter.
 * @param signal_duty Out Parameter of the API. Stores duty cycle in this out parameter.
 * @return <BR>
 *     ::CAPTURE_STATUS_t  Returns CAPTURE_STATUS_SUCCESS if there was outputs on new capture data.
 *                         Returns CAPTURE_STATUS_FAILURE if there was failure to get capture handle or system failure event occurred.
 *                         Returns CAPTURE_STATUS_NEW_VAL_NOT_CAPTURED if there is same capture value for which outputs are already delivered..
 *                         Returns CAPTURE_STATUS_INVALID_CONFIG if desire output is not expect for current configuration.
 *<BR>
 *
 * \par<b>Description:</b><br>
 * CAPTURE_GetPeriodDutyCycle() calculate period and duty cycle based on captured register, pre-scale values and Max system Clock.
 *
 * Example Usage:
 * @code
 * #include <DAVE.h>
 * int main(void)
 * {
 *  DAVE_STATUS_t init_status;
 *
 *  init_status = DAVE_Init();    // CAPTURE_Init(&CAPTURE_0) will be called from DAVE_Init()
 *
 *   if(init_status == DAVE_STATUS_FAILURE)
 *   {
 *     // Placeholder for error handler code. The while loop below can be replaced with an user error handler.
 *     XMC_DEBUG("DAVE APPs initialization failed\n");
 *
 *     while(1U)
 *     {
 *     }
 *   }
 *   while(1)
 *   {
 *     uint32_t signal_period=0, last_signal_period=0;
 *     uint32_t signal_duty=0, last_signal_duty=0;
 *     if(CAPTURE_GetPeriodInNanoSecDutyCycleInPercentage (&CAPTURE_0, &signal_period, &signal_duty) ==
 *        CAPTURE_STATUS_SUCCESS)
 *     {
 *       last_signal_period = signal_period;
 *       last_signal_duty = signal_duty;
 *     }
 *   }
 *  return 1;
 * }
 * @endcode<BR>
 */
CAPTURE_STATUS_t CAPTURE_GetPeriodInNanoSecDutyCycleInPercentage(const CAPTURE_t *const handler,
                                                                 uint32_t *const signal_period,
                                                                 uint32_t *const signal_duty);

/**
 * @}
 */
#include "capture_extern.h"   /* Included to access the APP Handles at Main.c */

#ifdef __cplusplus
}
#endif

#endif /* CAPTURE_H */
