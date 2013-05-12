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
********************************************************************************
**                                                                            **
**                                                                            **
** PLATFORM : Infineon XMC4000 Series                           			  **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR   : App Developer                                                   **
**                                                                            **
** MAY BE CHANGED BY USER [yes/no]: Yes                                       **
**                                                                            **
** MODIFICATION DATE : April 20, 2011                                         **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                       Author(s) Identity                                   **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** KS           App Developer                                                 **
*******************************************************************************/

/**
 * @file  CCU8PWMLIB.h
 *
 * @brief This file contains all public data structures,enums and function
 *        prototypes for SinglePhPWM with dead time APP.
 *
 */

#ifndef CCU8PWMLIB_H_
#define CCU8PWMLIB_H_
/*******************************************************************************
**                      Include Files                                         **
*******************************************************************************/
#include <DAVE3.h>


/**
  * @ingroup CCU8PWMLIB_publicparam
  * @{
  */
/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/

/*<<<DD_CCU8PWMLIB_MACRO_1>>>*/
/**This is mask for Event 0*/
#define CCU8PWMLIB_EVENT_0  0x01

/*<<<DD_CCU8PWMLIB_MACRO_2>>>*/
/**This is mask for Event 1*/
#define CCU8PWMLIB_EVENT_1  0x02

/*<<<DD_CCU8PWMLIB_MACRO_3>>>*/
/**This is mask for event 2*/
#define CCU8PWMLIB_EVENT_2  0x03

/*<<<DD_CCU8PWMLIB_MACRO_4>>>*/
/**Active low level*/
#define CCU8PWMLIB_ACTIVE_LOW  0x00

/*<<<DD_CCU8PWMLIB_MACRO_5>>>*/
/**Active high level*/
#define CCU8PWMLIB_ACTIVE_HIGH  0x01

/*<<<DD_CCU8PWMLIB_MACRO_6>>>*/
/**To clear all the interrupts of CCU*/
#define CCU8PWMLIB_ALL_CCU8_INTR_CLEAR  0x00000F3F

/*<<<DD_CCU8PWMLIB_MACRO_7>>>*/
/**To clear the CCU8 slice.*/
#define CCU8PWMLIB_SLICE_CLEAR  0x07

/*<<<DD_CCU8PWMLIB_MACRO_8>>>*/
/**low pass filter value*/
#define CCU8PWMLIB_LPF  0x0

/*<<<DD_CCU8PWMLIBD_MACRO_9>>>*/
/**To count the period register value from the PWM frequency and the CCU8 clock.
 */
#define CCU8PWMLIB_COUNT(PwmTime, Res)  (uint32_t)(PwmTime/Res)

/*<<<DD_CCU8PWMLIB_MACRO_10>>>*/
/**This is the period register value in edge-aligned mode*/
#define CCU8PWMLIB_EDGE_PERIOD_VAL(count)  (uint32_t)(count -1)

/*<<<DD_CCU8PWMLIB_MACRO_11>>>*/
/**This is the period register value in center-aligned mode*/
#define  CCU8PWMLIB_CENTER_PERIOD_VAL(count)  (uint32_t)((count <<1) -1 )

/**This is the value to set compare mode*/
#define CCU8PWMLIB_COMPARE_MODE 0x00

#define CCU8PWMLIB_EVENT2_INTERRUPT 0xA

#define CCU8PWMLIB_TRAP_FLAG_CLEAR 0xB

#define CCU8PWMLIB_MAX_VALUE 65535UL

#define CCU8PWMLIB_TC_MAX_VALUE 0xFFFFFFFFUL

typedef status_t (*CCU8PWMLIB_SetCompareFuncionPtr)(const void*, uint32_t, uint32_t);
typedef status_t (*CCU8PWMLIB_SetDutyFunctionPtr)(const void*, float, uint32_t, uint8_t);


/*******************************************************************************
 *                                ENUMERATIONS                                **
 ******************************************************************************/
/**
 * This structure enumerates the states of the Single Phase PWM with dead time
 * APP.
 */
typedef enum CCU8PWMLIB_StateType
{
  /**
   * This is the default state after power on reset
   */
  CCU8PWMLIB_UNINITIALIZED,
  /**
   * This is the INITIALIZED state in which APP is initialized as per selected
   * parameters
   */
  CCU8PWMLIB_INITIALIZED,
  /**
   * This state indicates that CCU8_CCy slice is running in PWM mode.
   */
  CCU8PWMLIB_RUNNING,
  /**
   * This state indicates that CCU4_CCy slice timer is stopped.
   */
  CCU8PWMLIB_STOPPED,

} CCU8PWMLIB_StateType;

/**
 * This enumerates the CCU8x_CCy slice used by Single Phase PWM with Dead time
 * APP.
 */
typedef enum CCU8PWMLIB_CCUInUseType
{
  /**
   * This is slice 0 of CCU8
   */
  CCU8PWMLIB_SLICE0,
  /**
   * This is slice 1 of CCU8
   */
  CCU8PWMLIB_SLICE1,
  /**
   * This is slice 2 of CCU8
   */
  CCU8PWMLIB_SLICE2,
  /**
   * This is slice 3 of CCU8
   */
  CCU8PWMLIB_SLICE3
} CCU8PWMLIB_CCUInUseType;

/**
 * This enumerates the error codes of this APP which will be returned from a
 * function as a return parameter.
 */
typedef enum CCU8PWMLIB_ErrorCodesType
{
  /**
   * This error code would be returned if the current API operation is not possible
   * because the App is in certain state.
   */
  CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR = 1,
  /**
   * This error code means that the parameter passed to an API are invalid
   */
  CCU8PWMLIB_INVALID_PARAM_ERROR,
  /**
   * This error code means that the trap condition is set.
   */
  CCU8PWMLIB_IN_TRAP_ERROR,
  /* Debug Log Codes starts here*/
  /**
   * DebugLog Message Id for Function Entry
   */
  CCU8PWMLIB_FUNCTION_ENTRY,
  /**
   * DebugLog Message Id for Function Exit
   */
  CCU8PWMLIB_FUNCTION_EXIT

} CCU8PWMLIB_ErrorCodesType;

/**
 * This enumerates dead time configuration for the CCU8_CCy slice channels
 */
typedef enum CCU8PWMLIB_DeadTimeConfType
{
  /**
   * Disable dead time generation
   */
  CCU8PWMLIB_DISABLE,
  /**
   * Enable dead time generation
   */
  CCU8PWMLIB_ENABLE,
  /**
   * Enable dead time generation for direct output
   */
  CCU8PWMLIB_ENABLEDIRECTOUTPUT,
  /**
   * Enable dead time generation for inverted output.
   */
  CCU8PWMLIB_ENABLEINVERTEDOUTPUT
} CCU8PWMLIB_DeadTimeConfType;

/**
 * This enumerates the compare mode of the CCU8_CCy slice timer - symmetric
 * comparison or asymmetric comparison
 */
typedef enum CCU8PWMLIB_CompareModeType
{
  /**
   * In this mode channel 1 uses compare register 1 for rising and falling
   * compare match and channel 2 uses compare register 2 for rising and falling
   * compare match. Waveform is symmetric over a PWM period.
   */
  CCU8PWMLIB_SYMMETRIC,
  /**
   * In this mode channel 1 uses compare register 1 for rising compare match
   * and channel 2 uses compare register 2 for falling compare match. Waveform
   * is not symmetric over a period.
   */
  CCU8PWMLIB_ASYMMETRIC
} CCU8PWMLIB_CompareModeType;

typedef enum CCU8PWMLIB_EdgeTriggerType{
    /**
     * Trigger is not required.
     */
  CCU8PWMLIB_NOTRIGGER,
    /**
     * Trigger on rising edge
     */
  CCU8PWMLIB_RISINGEDGE,
    /**
     * Trigger on falling edge
     */
  CCU8PWMLIB_FALLINGEDGE,
    /**
     * Trigger on both edges
     */
  CCU8PWMLIB_BOTHEDGES
}CCU8PWMLIB_EdgeTriggerType;

/**
 * This enumerates the CCU8_CCy slice timer operating mode - edge-aligned or
 * center-aligned.
 */
typedef enum CCU8PWMLIB_CountingModeType
{
  /**
   * In this mode with up counting direction, timer counts up to the period
   * value and then clears to zero to start counting again. And in down
   * counting, timer decrements from period value to zero and starts again with
   * period value.
   */
  CCU8PWMLIB_EDGE_ALIGNED,
  /**
   * In this mode timer counts up till the period value and then counts down to
   * zero.With down counting selected first, timer will decrement from period
   * value to zero and then will increment again to period value to complete
   * one cycle.
   */
  CCU8PWMLIB_CENTER_ALIGNED
}CCU8PWMLIB_CountingModeType;

/**
 * This enumerates the external start configuration of the timer.
 */
typedef enum CCU8PWMLIB_ExtStartConfigType
{
  /**
   * This will set the run bit of the timer.
   */
  CCU8PWMLIB_START_TIMER,
  /**
   * This will first clear the timer and then set the run bit of the timer.
   */
  CCU8PWMLIB_CLEAR_START_TIMER
} CCU8PWMLIB_ExtStartConfigType;

/**
 * This enumerates the external stop configuration of the timer.
 */
typedef enum CCU8PWMLIB_ExtStopConfigType
{
  /**
   * This will reset the RUN bit of the timer
   */
  CCU8PWMLIB_STOP_TIMER,
  /**
   * This will clear the timer register value but keeps the timer running.
   */
  CCU8PWMLIB_CLEAR_TIMER,
  /**
   * This will clear the timer register value and also clears the RUN bit of the
   * timer.
   */
  CCU8PWMLIB_CLEAR_STOP_TIMER
} CCU8PWMLIB_ExtStopConfigType;

/**
 * This enumerates the events of the CCU8PWMLIB app.
 */
typedef enum CCU8PWMLIB_EventNameType
{
  /**
   * This is the period match interrupt
   */
  CCU8PWMLIB_PERIODMATCHEVENT = 0,
  /**
   * This is the compare match while counting UP interrupt
   */
  CCU8PWMLIB_COMPAREMATCHEVENT = 2,
  /**
   * This is the external start interrupt i.e. Event 0 interrupt
   */
  CCU8PWMLIB_EXTSTARTEVENT = 8,
  /**
   * This is the external stop interrupt i.e. Event 1 interrupt
   */
  CCU8PWMLIB_EXTSTOPEVENT = 9,
  /**
   * This is the trap interrupt i.e. Event 2 interrupt
   */
  CCU8PWMLIB_TRAPEVENT = 10,
}CCU8PWMLIB_EventNameType;
/*******************************************************************************
 *                             STRUCTURES                                     **
 ******************************************************************************/
/**
 * This structure holds the values of the compare register, period register.
 */
typedef struct CCU8PWMLIB_TimerRegsType
{
  /**
   * This structure holds the values of the compare register, period register.
   */
  uint32_t CompReg1;
  /**
   * This is the compare register value of channel 2
   */
  uint32_t CompReg2;
  /**
   * This is the period register value
   */
  uint32_t PeriodReg;
  /**
   * This is the current timer register value
   */
  uint32_t TimerReg;
} CCU8PWMLIB_TimerRegsType;



/**
 * This structure will hold the configuration parameters which change at run
 * time.
 */
typedef struct CCU8PWMLIB_DynamicHandleType
{

/**
 * This is the state of the app.
 */
  CCU8PWMLIB_StateType State;
} CCU8PWMLIB_DynamicHandleType;

/**
 * This structure holds all the static configuration parameters for the Single
 * Phase PWM with Dead Time APP.
 */

typedef struct CCU8PWMLIB_HandleType
{
    /**
     * This parameter tells whether to start the CCU8_CCy slice with external
     * trigger or by SW trigger
     */
    const uint8_t kExtStartTrig;
    /**
     * This parameter tells whether to stop the CCU8_CCy slice with external
     * trigger or by SW trigger
     */
    const uint8_t kExtStopTrig;
    /**
     * This parameter tells whether direct output CCST will be connected to output
     * pin 0 or inverted status output is connected to Out 0.
     */
    const uint8_t kPassiveState0;
    /**
     * This parameter tells whether direct output CCST will be connected to output
     * pin 1 or inverted status output is connected to Out 1.
     */
    const uint8_t kPassiveState1;
    /**
     * This parameter selects the passive level for output 0 - Active High or
     * Active Low
     */
    const uint8_t kPassiveLevel0;
    /**
     * This parameter selects the passive level for output 1 - Active High or
     * Active Low
     */
    const uint8_t kPassiveLevel1;
    /**
     * This parameter is used to store whether CCU8_CCy slice is in single shot
     * mode or in periodic mode
     */
    const uint8_t kTimerMode;
     /**
      * This parameter tells whether timer concatenation is required or not.
      */
    const uint8_t kTimerConcatenation;
    /**
      * This indicates whether TRAP signal will have an effect on output signal.
      */
    const uint8_t kTrapEnable;
    /**
      * This configures the synchronization with the PWM signal is required or
      * not.
      */
    const uint8_t kTrapSync;
    /**
     * This parameter gives the active level of the Trap signal
     */
    const uint8_t kTrapLevel;  
    /**
     * This indicates whether to exit from TRAP state automatically when TRAP
     * condition is removed or SW trigger is required to exit from the TRAP state.
     */
    const uint8_t kTrapExitControl;
    /**
     * This parameter saves the dither compare value
     */
    const uint8_t kDitherCompare; 
    /**
     * This is the dead time prescalar divider value.
     */
    const uint8_t kDeadTimePrescalar;
    /**
     * This parameter configures dither mode.
     */
    const uint8_t kDitherSetting;
    /**
     * This tells whether to enable shadow transfer in this App
     * or it will be taken care by some other app
     */
    const uint8_t ShadowTransfer;
    /**
     * This tells whether to clear IDLE mode in this App
     * or it will be taken care by some other app
     */
    const uint8_t Start;    
    /**
     * This decides whether to start the app after initialization
     */  
    const uint8_t StartControl;       
    /**
    * This is the  the falling dead time value for channel 1
    */
   const uint16_t kFallingDeadTime;
   /**
    * This is the the rising dead time value for channel 1
    */
   const uint16_t kRisingDeadTime;    
    
    /**
   * This is the resolution of the CCU8
   */
  const float kResolution;

  /**
   * This is the compare register value of channel 1 which corresponds to
   * initial duty cycle
   */
  const uint32_t kCompareValue1;
  /**
   * This is the compare register value of channel 2 which corresponds to
   * initial duty cycle. This will be used in case of asymmetric PWM mode.
   */
  const uint32_t kCompareValue2;
  /**
   * This is the prescalar divider value.
   */
  const uint32_t kCCUPrescalar;
  /**
   * This parameter gives the active edge of the external start signal
   */
  const CCU8PWMLIB_EdgeTriggerType kStartEdge;
  /**
   * This parameter gives the active edge of the external stop signal
   */
  const CCU8PWMLIB_EdgeTriggerType kStopEdge;
  /**
    * Period value computed as per counting mode
    */
  const uint32_t kPeriodVal;
 /**
   * This is pointer to the CMSIS CCU8 kernel register structure.
   */
  CCU8_GLOBAL_TypeDef* const CC8yKernRegsPtr;
  /**
   * This is pointer to the CMSIS CCU8 slice register structure.
   */
  CCU8_CC8_TypeDef* const CC8yRegsPtr;
  /**
   * This is pointer to the CMSIS CCU8 slice register structure.
   */
  CCU8_CC8_TypeDef* const CC8yRegs1Ptr;
/**
 * This is the ointer to the dynamic handle
 */
  CCU8PWMLIB_DynamicHandleType* const DynamicHandleType;
  /**
   * This is the counting mode of the timer
   */
  const CCU8PWMLIB_CountingModeType CountingMode;
  /**
   * This is the functionality of the external stop
   */
  const CCU8PWMLIB_ExtStopConfigType ExtStopConfig;
  /**
   * This is the dead time configuration
   */
  const CCU8PWMLIB_DeadTimeConfType DeadTimeConf;
  /** This is the compare mode of the timer - symmetric/asymmetric
   *
   */
  const CCU8PWMLIB_CompareModeType CompareMode;
  /**
   * This is the functionality of the external stop
   */
  const CCU8PWMLIB_ExtStartConfigType ExtStartConfig;
  /**
   * This saves the slice number of the first slice.
   */
  const CCU8PWMLIB_CCUInUseType FirstSlice;
  /**
   * This saves the slice number od the second slice in case of timer concatenation
   */
  const CCU8PWMLIB_CCUInUseType SecondSlice;
  /**
   * Bitmask for shadow transfer as per slice number
   */
  const uint32_t ShadowTransferMask;
  /**
   * Bitmask for clearing IDLE mask as per slice used
    */
  const uint32_t StartMask;
  /**
   * This decides whether to enable interrupts at initialization time. This stores the information in a format of 32-bit register (INTE)
   * with a bit-field for each interrupt.
   */
  const uint32_t InterruptControl;
 /**
   * This is the function pointer for the SetCompare function
   */
  CCU8PWMLIB_SetCompareFuncionPtr SetCompareFuncPtr;
  /**
   * This is the function pointer for the SetDutyCycle function
   */
  CCU8PWMLIB_SetDutyFunctionPtr SetDutyFuncPtr;
} CCU8PWMLIB_HandleType;

/**
 * @}
 */

/**
 * @ingroup CCU8PWMLIB_apidoc
 * @{
 */
/*******************************************************************************
 **FUNCTION PROTOTYPES                                                        **
*******************************************************************************/
/**
 * @brief This function is used to initialize the APP.
 * It will configure CCU8_CCy slice registers with the selected PWM parameters.
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return void\n
* <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 * 
 */
status_t CCU8PWMLIB_Init(const CCU8PWMLIB_HandleType* HandlePtr);

/**
 * @brief This will reset the CCU8_CCy slice registers to default state.
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * else
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_Deinit(const CCU8PWMLIB_HandleType* HandlePtr);

/**
 * @brief This function enables the externat start feature.
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * else
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_EnableExtStart(const CCU8PWMLIB_HandleType* HandlePtr);

/**
 * @brief This function disables the externat start feature.
 * This is useful for multiphase PWM app to disable this functionality to avoid spurious 
 * start of the slices due to other instances of the app. 
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * else
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_DisableExtStart(const CCU8PWMLIB_HandleType* HandlePtr);

/**
 * @brief This will start the Single Phase PWM with Dead Time APP which will in
 * turn start the CCU8_CCy slice.\n
 * <b>This function needs to be called to start the App
 * even if External Start feature of CCU8 is configured. </b>
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called in the state other
 * than CCU8PWMLIB_INITIALIZED <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
*/
status_t CCU8PWMLIB_Start(const CCU8PWMLIB_HandleType* HandlePtr);

/**
 * @brief This will stop the Single Phase PWM with Dead Time APP which
 * will in turn stop the CCU8_CCy slice.\n
 * <b> This function needs to be called even if external stop feature is configured </b>
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when not in the
 * state CCU8PWMLIB_RUNNING or   <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 *
*/
status_t CCU8PWMLIB_Stop(const CCU8PWMLIB_HandleType* HandlePtr);

/**
 * @brief This function will modify the duty cycle of the output waveform
 * which will change the pulse width.\n
 * <b>Duty cycle is given in terms of the compare register value and not in terms
 * of the percentage.</b>
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   Compare1 It corresponds to compare register 1 value.
 * @param[in]   Compare2 It corresponds to compare register 2 value. \n
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_UNINITIALIZED   <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
*/
status_t CCU8PWMLIB_SetCompare
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t Compare1,
  uint32_t Compare2
  
);

/**
 * @brief This function will modify the duty cycle of the output waveform
 * which will change the pulse width.\n
 * <b>Duty cycle is given in terms of percentage.</b>
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   DutyCycle It corresponds to duty cycle in percentage.
 * @param[in]   Shift It controls the duty cycle value.
 * @param[in]   Sign It controls addition and subtraction of shift
 * Sign = 0: Compare value is incremented by shift. This is not allowed in edge-aligned.
 * Sign = 1: Compare value is decremented by shift.
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_UNINITIALIZED   <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
*/
status_t CCU8PWMLIB_SetDutyCycle
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
);

/**
 * @brief This function will modify the pwm frequency of the output waveform
 * which will change the pulse width.\n
 * <b>In case of timer concatenation,given value is split into two 16-bit values
 * and they are programmed in Second and First slice.\n
 * e.g. 0x80000010 value is written as 0x8000 as Period register of Second slice
 * and 0x0010 as period register of first slice.
 * Total PWM period is ((0x8000 +1) * 0x0010) + 1.
 * </b>
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   PwmFreq It corresponds to period register's value.
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_RUNNING   <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_SetPeriod
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t PwmFreq
);

/**
 * @brief This function will modify the pwm frequency and duty cycle of the output waveform
 * which will change the pulse width.\n
 * Frequency is given as period register value and duty cycle is given as compare register value. 
 * <b>In case of timer concatenation,given value is split into two 16-bit values
 * and they are programmed in Second and First slice.\n
 * e.g. 0x80000010 value is written as 0x8000 as Period register of Second slice
 * and 0x0010 as period register of first slice.
 * Total PWM period is ((0x8000 +1) * 0x0010) + 1.
 * </b>
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   PwmFreq It corresponds to period register's value.
 * @param[in]   Compare1 It corresponds to compare register 1 value.
 * @param[in]   Compare2 It corresponds to compare register 2 value. \n
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
<BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_SetPeriodAndCompare
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t PwmFreq,
  uint32_t Compare1,
  uint32_t Compare2
);

/**
 * @brief This function will modify the pwm frequency of the output waveform
 * \n
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   PwmFreq It corresponds to frequency in hertz
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_RUNNING   <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_SetPwmFreq
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  float PwmFreq
);
/**
 * @brief This function will modify the pwm frequency and duty cycle of the output waveform
 * \n
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   PwmFreq It corresponds to frequency in hertz
  * @param[in]   DutyCycle It corresponds to duty cycle in percentage.
 * @param[in]   Shift It controls the duty cycle value.
 * @param[in]   Sign It controls addition and subtraction of shift
 * Sign = 0: Compare value is incremented by shift. This is not allowed in edge-aligned.
 * Sign = 1: Compare value is decremented by shift.
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
<BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_SetPwmFreqAndDutyCycle
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  float PwmFreq,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
);

/**
 * @brief This will update the timer register value when timer is stopped.
 * This is used to set the starting value of the timer. It is useful API in the
 * single shot mode.
 *
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   TimerVal
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
<BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_SetTimerVal
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t TimerVal
);

/**
 * @brief This will request the update of the period register, compare register
 * values from the respective shadow transfers.
 * This function needs to be called when shadow compare and period registers are
 * updated explicitely by top level app.\n
 * <b>NOTE: </b> SetDutyCycle and SetPwmFreq functions handle shadow
 * transfer request. No need to call this function after changing the duty cycle
 * and PWM frequency.
 *  @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_UNINITIALIZED   <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 * 
 */
status_t CCU8PWMLIB_SWRequestShadowTransfer
(
  const CCU8PWMLIB_HandleType* HandlePtr
);

/**
 * @brief This function will read the period register value.This will be useful
 * to calculate the compare register values.
 * @param[out]  PeriodRegPtr
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_UNINITIALIZED   state <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_GetPeriodReg
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t* PeriodRegPtr
);

/**
 * @brief This function will reset the trap flag by checking whether trap
 * condition is reset.
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_INITIALIZED or CCU8PWMLIB_RUNNING state <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_ResetTrapFlag(const CCU8PWMLIB_HandleType* HandlePtr);

/**
 * @brief This function will read the timer values of compare register and
 * period register.
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   TimerRegsPtr Pointer to CCU8PWMLIB_TimerRegsType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_UNINITIALIZED  \n
 * CCU8PWMLIB_INVALID_PARAM_ERROR: If TimerRegs* is invalid. <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_GetTimerRegsVal
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  CCU8PWMLIB_TimerRegsType* TimerRegsPtr
);

/**
 * @brief This function will read the timer status whether timer is running or
 * idle.
 * @param[out] TimerStatusPtr
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return status_t\n
 * DAVEApp_SUCCESS: if function is successful\n
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If this function is called when in the state
 * CCU8PWMLIB_UNINITIALIZED  . <BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_GetTimerStatus
(
 const CCU8PWMLIB_HandleType* HandlePtr,
 uint32_t* TimerStatusPtr
);

/**
 * @brief This function will enable the event. This should be called by the user first
 * for interrupt handling.
 *
 * @param [in] HandlePtr pointer to the Instance variable<BR>
 * @param [in] Event Event to be enabled <BR>
 * @return status_t<BR>
 * DAVEApp_SUCCESS: if the function is successful<BR>
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If the function is called in
 * CCU8PWMLIB_UNINITIALIZED State.<BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_EnableEvent
(
     const CCU8PWMLIB_HandleType * HandlePtr,
     const CCU8PWMLIB_EventNameType Event
);

/**
 * @brief This function will disable the event.
 *
 * @param [in] HandlePtr pointer to the Instance variable<BR>
 * @param [in] Event Event to be disabled <BR>
 * @return status_t<BR>
 * DAVEApp_SUCCESS: if the function is successful<BR>
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If the function is called in
 * CCU8PWMLIB_UNINITIALIZED State.<BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_DisableEvent
(
    const CCU8PWMLIB_HandleType * HandlePtr,
    const CCU8PWMLIB_EventNameType Event
);

/**
 * @brief This function will clear the pending event.
 *
 * @param [in] HandlePtr pointer to the Instance variable<BR>
 * @param [in] Event Event to be cleared <BR>
 * @return status_t<BR>
 * DAVEApp_SUCCESS: if the function is successful<BR>
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If the function is called in
 * CCU8PWMLIB_UNINITIALIZED State.<BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_ClearPendingEvent
(
    const CCU8PWMLIB_HandleType * HandlePtr,
    const CCU8PWMLIB_EventNameType Event
);

/**
 * @brief This function will forcefully set the event.This function should not be called within
 * interrut handler to avoid infinite loop.
 *
 * @param [in] HandlePtr pointer to the Instance variable<BR>
 * @param [in] Event Event to be set <BR>
 * @return status_t<BR>
 * DAVEApp_SUCCESS: if the function is successful<BR>
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If the function is called in
 * CCU8PWMLIB_UNINITIALIZED State.<BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_SetPendingEvent
(
    const CCU8PWMLIB_HandleType * HandlePtr,
    const CCU8PWMLIB_EventNameType Event
);

/**
 * @brief This function will check whether given event is set.
 *
 * @param [in] HandlePtr pointer to the Instance variable<BR>
 * @param [in] Event Event to be set <BR>
 * @param [in] EvtStatus Status of the eventwhether it is set <BR>
 * @return status_t<BR>
 * DAVEApp_SUCCESS: if the function is successful<BR>
 * CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR: If the function is called in
 * CCU8PWMLIB_UNINITIALIZED State.<BR>
 *
 * <b>Reentrancy:  Yes</b><BR>
 * <b>Sync/Async:  Synchronous</b><BR>
 */
status_t CCU8PWMLIB_GetPendingEvent
(
    const CCU8PWMLIB_HandleType * HandlePtr,
    const CCU8PWMLIB_EventNameType Event,
    uint8_t* EvtStatus
);

/**
 * @brief This function will be internally called by CCU8PWMLIB_SetDutyCycle
 *        when edge aligned asymmetric mode is selected.
 * @param[in]   HdlPtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   DutyCycle It corresponds to compare register's value.
 * @param[in]   Shift It controls the duty cycle value.
 * @param[in]   Sign It controls addition and subtraction of shift
 * @return status_t
 */
status_t CCU8PWMLIB_SetDutyEdgeAlignAsymmetric
(
  const void* HdlPtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
);

/**
 * @brief This function will be internally called by CCU8PWMLIB_SetDutyCycle
 *        when edge aligned symmetric mode is selected.
 * @param[in]   HdlPtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   DutyCycle It corresponds to compare register's value.
 * @param[in]   Shift It controls the duty cycle value.
 * @param[in]   Sign It controls addition and subtraction of shift
 * @return status_t
 */
status_t CCU8PWMLIB_SetDutyEdgeAlignSymmetric
(
  const void* HdlPtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
);

/**
 * @brief This function will be internally called by CCU8PWMLIB_SetDutyCycle
 *        when edge aligned symmetric with timer concatenation mode is selected.
 * @param[in]   HdlPtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   DutyCycle It corresponds to compare register's value.
 * @param[in]   Shift It controls the duty cycle value.
 * @param[in]   Sign It controls addition and subtraction of shift
 * @return status_t
 */
status_t CCU8PWMLIB_SetDutyEdgeAlignSymmetricTimerConcat
(
  const void* HdlPtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
);

/**
 * @brief This function will be internally called by CCU8PWMLIB_SetDutyCycle
 *        when center aligned symmetric mode is selected.
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   DutyCycle It corresponds to compare register's value.
 * @param[in]   Shift It controls the duty cycle value.
 * @param[in]   Sign It controls addition and substraction of shift
 * @return status_t
 */
status_t CCU8PWMLIB_SetDutyCenterAlignSymmetric
(
  const void*  HandlePtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
);

/**
 * @brief This function will be internally called by CCU8PWMLIB_SetDutyCycle
 *        when center aligned symmetric mode is selected.
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   DutyCycle It corresponds to compare register's value.
 * @param[in]   Shift It controls the duty cycle value.
 * @param[in]   Sign It controls addition and substraction of shift
 * @return status_t
 */
status_t CCU8PWMLIB_SetDutyCenterAlignAsymmetric
(
  const void*  HandlePtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
);

/**
 * @brief This function will be internally called by CCU8PWMLIB_SetCompare
 *        when asymmetric mode is selected.
 * @param[in]   HdlPtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   Compare1 Compare register1 value
 * @param[in]   Compare2 Compare register 2 value
 * @return status_t
 */
status_t CCU8PWMLIB_SetCompareAsymmetric
(
    const void* HdlPtr,
    uint32_t Compare1,
    uint32_t Compare2
);

/**
 * @brief This function will be internally called by CCU8PWMLIB_SetCompare
 *        when symmetric mode is selected.
 * @param[in]   HdlPtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   Compare1 Compare register1 value
 * @param[in]   Compare2 Compare register 2 value
 * @return status_t
 */
status_t CCU8PWMLIB_SetCompareSymmetric
(
    const void* HdlPtr,
    uint32_t Compare1,
    uint32_t Compare2
);

/**
 * @brief This function will be internally called by CCU8PWMLIB_SetCompare
 *        when edge-aligned symmetric mode with timer concatenation is selected.
 * @param[in]   HdlPtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]   Compare1 Compare register1 value
 * @param[in]   Compare2 Compare register 2 value
 * @return status_t
 */
status_t CCU8PWMLIB_SetCompareEdgeAlignSymmetricTimerConcat
(
    const void* HdlPtr,
    uint32_t Compare1,
    uint32_t Compare2
);

/**
 * @}
 */


#endif /* CCU8PWMLIB_H_ */

