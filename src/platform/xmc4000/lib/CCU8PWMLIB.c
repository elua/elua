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
** PLATFORM : Infineon XMC4000 Series                           			  **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR   : App Developer                                                   **
**                                                                            **
** MAY BE CHANGED BY USER [yes/no]: Yes                                       **
**                                                                            **
** MODIFICATION DATE : April 21, 2011                                         **
**                                                                            **
*******************************************************************************/
/*******************************************************************************
**                      Author(s) Identity                                    **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** KS           App Developer                                                 **
**                                                                            **
*******************************************************************************/

/**
 * @file  CCU8PWMLIB.c
 *
 * @brief This file contains implementations of all Public and Private functions
 *        of CCU8_PWMSinglePhaseDT_CCU8PWMLIB APP.
 *
 */

/*******************************************************************************
**                      Include Files                                         **
*******************************************************************************/
#include <DAVE3.h>
#include "../../inc/LIBS/CCU8PWMLIB.h"
/**
 * @cond INTERNAL_DOCS
 */
/**
  * @ingroup CCU8PWMLIB_privatefunc
  * @{
  */
#ifdef USE_PWM_LIB  
/*******************************************************************************
**                Private Function declarations                               **
*******************************************************************************/
/*******************************************************************************
**                Private Function declarations                               **
*******************************************************************************/
/**
 * @brief This function is used to initialize the APP.
 * It will configure second slice registers with the selected PWM parameters.
 * @param[in]   HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @return void
 */
void CCU8PWMLIB_lConfigureSecondSlice
(
  const CCU8PWMLIB_HandleType* HandlePtr
);

/**
 * @brief This function will set pwm frequency if timer concatenation is selected
 * @param[in]  HandlePtr Pointer to CCU8PWMLIB_HandleType structure
 * @param[in]  PwmTime This is PWM frequency in hertz.
 * @return status_t status
 */
status_t CCU8PWMLIB_lSetPwmFreqTimerConcat
(
    const CCU8PWMLIB_HandleType* HandlePtr,
    uint32_t PwmTime
);

/**
 * @}
 */



/*******************************************************************************
**                 Function definitions                                       **
*******************************************************************************/
/*<<<DD_CCU8PWMLIB_API_1>>>*/
/**
 * This function initializes the app and CCU8x_CC8y slice.
 */
status_t CCU8PWMLIB_Init(const CCU8PWMLIB_HandleType* HandlePtr)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;
    uint32_t DeadTimeConfVal[4] = {0x00UL, 0xDUL, 0x5UL, 0x9UL};

/*Clear the Timer value*/
    HandlePtr->CC8yRegsPtr->TCCLR = (uint32_t)CCU8PWMLIB_SLICE_CLEAR;
    HandlePtr->CC8yRegsPtr->TC = 0x00;

	/*<<<DD_CCU8PWMLIB_API_1_3>>>*/
	/* Configure external stop feature */
	if (HandlePtr->kExtStopTrig == (uint8_t)SET)
	{
	  HandlePtr->CC8yRegsPtr->INS &= ~((CCU8_CC8_INS_EV1EM_Msk)|
		  (CCU8_CC8_INS_LPF1M_Msk));
	  HandlePtr->CC8yRegsPtr->CMC &=  ~CCU8_CC8_CMC_ENDS_Msk;

	  HandlePtr->CC8yRegsPtr->INS |=
	  ((((uint32_t)HandlePtr->kStopEdge << \
		  CCU8_CC8_INS_EV1EM_Pos)& CCU8_CC8_INS_EV1EM_Msk) |
		  ((CCU8PWMLIB_LPF  << CCU8_CC8_INS_LPF1M_Pos)& CCU8_CC8_INS_LPF1M_Msk));
	  HandlePtr->CC8yRegsPtr->CMC |=
	  ((CCU8PWMLIB_EVENT_1  << CCU8_CC8_CMC_ENDS_Pos)& CCU8_CC8_CMC_ENDS_Msk);
	}

    /*<<<DD_CCU8PWMLIB_API_1_4>>>*/
    /* Configure trap feature */
    if ((HandlePtr->kTrapEnable == (uint8_t)SET) &&\
    		(HandlePtr->kTimerConcatenation != (uint8_t)SET))
    {
      HandlePtr->CC8yRegsPtr -> INS &= ~((CCU8_CC8_INS_EV2EM_Msk) |\
          (CCU8_CC8_INS_EV2LM_Msk) | (CCU8_CC8_INS_LPF2M_Msk));
      HandlePtr->CC8yRegsPtr -> INS |=
          (((0x00 << CCU8_CC8_INS_EV2EM_Pos)& CCU8_CC8_INS_EV2EM_Msk) |
          (((uint32_t)HandlePtr->kTrapLevel << CCU8_CC8_INS_EV2LM_Pos)& CCU8_CC8_INS_EV2LM_Msk) |
          ((0x00 << CCU8_CC8_INS_LPF2M_Pos)& CCU8_CC8_INS_LPF2M_Msk));
      HandlePtr->CC8yRegsPtr->CMC |= (0x01 << CCU8_CC8_CMC_TS_Pos);
    }/* End of  if (HandlePtr->kTrapEnable == (uint8_t)SET)*/

/* Configure timer counting mode, compare mode, external start, stop controls */
    HandlePtr->CC8yRegsPtr->TC &= ~((CCU8_CC8_TC_TCM_Msk) |
                           (CCU8_CC8_TC_TSSM_Msk) | (CCU8_CC8_TC_CMOD_Msk) |
                            (CCU8_CC8_TC_STRM_Msk) | (CCU8_CC8_TC_ENDM_Msk));
    HandlePtr->CC8yRegsPtr->TC =(((uint32_t)
      HandlePtr->CountingMode << CCU8_CC8_TC_TCM_Pos)& CCU8_CC8_TC_TCM_Msk)|
      ((HandlePtr->kTimerMode << CCU8_CC8_TC_TSSM_Pos)& CCU8_CC8_TC_TSSM_Msk)|
      ((CCU8PWMLIB_COMPARE_MODE  << CCU8_CC8_TC_CMOD_Pos)& CCU8_CC8_TC_CMOD_Msk)|
      (((uint32_t)
      HandlePtr->ExtStartConfig << CCU8_CC8_TC_STRM_Pos)& CCU8_CC8_TC_STRM_Msk)|
      (((uint32_t)
      HandlePtr->ExtStopConfig  << CCU8_CC8_TC_ENDM_Pos)& CCU8_CC8_TC_ENDM_Msk);

 /*Set the dither mode setting*/
    HandlePtr->CC8yRegsPtr->TC |=
        (HandlePtr->kDitherSetting  << CCU8_CC8_TC_DITHE_Pos) & CCU8_CC8_TC_DITHE_Msk;

    HandlePtr->CC8yRegsPtr->DITS =
    (HandlePtr->kDitherCompare  << CCU8_CC8_DITS_DCVS_Pos) & CCU8_CC8_DITS_DCVS_Msk;

/*Configure Trap mode*/
/*<<<DD_CCU8PWMLIB_API_1_6>>>*/
    if(HandlePtr->kTimerConcatenation != (uint8_t)SET)
    {
      WR_REG(HandlePtr->CC8yRegsPtr->TC, CCU8_CC8_TC_TRPSE_Msk,
             CCU8_CC8_TC_TRPSE_Pos, HandlePtr->kTrapSync);
      WR_REG(HandlePtr->CC8yRegsPtr->TC, CCU8_CC8_TC_TRPSW_Msk,
         CCU8_CC8_TC_TRPSW_Pos, HandlePtr->kTrapExitControl);
    }
    /*Set the prescalar divider and passive level of the o/p signal.*/
    WR_REG(HandlePtr->CC8yRegsPtr->PSC,CCU8_CC8_PSC_PSIV_Msk,
        CCU8_CC8_PSC_PSIV_Pos, HandlePtr->kCCUPrescalar);
    HandlePtr->CC8yRegsPtr->PSL =
            ((HandlePtr->kPassiveLevel0 << CCU8_CC8_PSL_PSL11_Pos) |
            (HandlePtr->kPassiveLevel1 << CCU8_CC8_PSL_PSL12_Pos));

/*Set the symmetric or asymmetric PWM setting*/
    WR_REG(HandlePtr->CC8yRegsPtr->CHC, CCU8_CC8_CHC_ASE_Msk,\
            CCU8_CC8_CHC_ASE_Pos,(uint32_t)HandlePtr->CompareMode);

/*Configure output connections as per passive state.*/
    HandlePtr->CC8yRegsPtr->CHC &= ~((CCU8_CC8_CHC_OCS2_Msk)|(CCU8_CC8_CHC_OCS2_Msk));
    HandlePtr->CC8yRegsPtr->CHC |=
        (((uint32_t)HandlePtr->kPassiveState0 << CCU8_CC8_CHC_OCS1_Pos) |
        (((~(uint32_t)HandlePtr->kPassiveState1 )<< CCU8_CC8_CHC_OCS2_Pos) & CCU8_CC8_CHC_OCS2_Msk));

    /* Configure the dead time mode and rising and falling dead time*/
    HandlePtr->CC8yRegsPtr->DTC &= ~((CCU8_CC8_DTC_DTCC_Msk) |
        (CCU8_CC8_DTC_DTE1_Msk)| (CCU8_CC8_DTC_DTE2_Msk) |
        (CCU8_CC8_DTC_DCEN1_Msk)|(CCU8_CC8_DTC_DCEN2_Msk));
    HandlePtr->CC8yRegsPtr->DTC = (uint32_t)
        (((HandlePtr->kDeadTimePrescalar  << CCU8_CC8_DTC_DTCC_Pos) & CCU8_CC8_DTC_DTCC_Msk)|
        DeadTimeConfVal[HandlePtr->DeadTimeConf]);

    HandlePtr->CC8yRegsPtr->DC1R &= ~((CCU8_CC8_DC1R_DT1R_Msk) |
        (CCU8_CC8_DC1R_DT1F_Msk));
    HandlePtr->CC8yRegsPtr->DC1R = (uint32_t)
        (( HandlePtr->kRisingDeadTime << CCU8_CC8_DC1R_DT1R_Pos)
            & CCU8_CC8_DC1R_DT1R_Msk)|
        (( HandlePtr->kFallingDeadTime << CCU8_CC8_DC1R_DT1F_Pos)
            & CCU8_CC8_DC1R_DT1F_Msk);

/*<<<DD_CCU8PWMLIB_API_1_7>>>*/
/*Set period register's value*/
    HandlePtr->CC8yRegsPtr->PRS = (uint32_t)(HandlePtr->kPeriodVal & 0xffff);

/*Set compare register's value*/
    HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)
        (HandlePtr->kCompareValue1 & 0xffff);
 /* Configure channel 2 register CR2 register in case of asymmetric mode*/
    if (HandlePtr->CompareMode == CCU8PWMLIB_ASYMMETRIC)
    {
      HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)
          (HandlePtr->kCompareValue2 & 0xffff);
    }

  /*Request SW shadow transfer for period, compare, dither and prescalar level*/
    HandlePtr->CC8yKernRegsPtr->GCSS |= ((0x01UL <<
                                  (4 * (uint32_t)HandlePtr->FirstSlice)) |
                                  (0x01UL <<
                                  ((4 * (uint32_t)HandlePtr->FirstSlice) + 1)) |
                                  (0x01UL <<
                                  ((4 * (uint32_t)HandlePtr->FirstSlice) + 2)));

/*Clear all CCU8 interrupts*/
    HandlePtr->CC8yRegsPtr->SWR |= CCU8PWMLIB_ALL_CCU8_INTR_CLEAR;
    
/*Configure the second slice if timer concatenation is set */
    if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
      CCU8PWMLIB_lConfigureSecondSlice(HandlePtr);
      HandlePtr->CC8yRegs1Ptr->INTE |= HandlePtr->InterruptControl;

    } /*End of if (HandlePtr->kTimerConcatenation == SET)*/
    else
    {
      HandlePtr->CC8yRegsPtr->INTE |= HandlePtr->InterruptControl;
    }
    return Status;
}

/*<<<DD_CCU8PWMLIB_nonAPI_1>>>*/
/**
 * This function initializes the second slice if timer concatenation is used
 */
void CCU8PWMLIB_lConfigureSecondSlice
(
  const CCU8PWMLIB_HandleType* HandlePtr
)
{
  uint32_t DeadTimeConfVal[4] = {0x00UL, 0xDUL, 0x5UL, 0x9UL};

/*Clear the timer */
  HandlePtr->CC8yRegs1Ptr->TCCLR = CCU8PWMLIB_SLICE_CLEAR;
  HandlePtr->CC8yRegs1Ptr->TC = 0x00;

/*Set period and compare values for second slice*/
  HandlePtr->CC8yRegs1Ptr->PRS = (uint32_t)
      ((HandlePtr->kPeriodVal & 0xFFFF0000) >> 16);
  HandlePtr->CC8yRegs1Ptr->CR1S = (uint32_t)
          ((HandlePtr->kCompareValue1 & 0xFFFF0000) >> 16);
  /*<<<DD_PWMSP002_API_non1_7>>>*/
/* Updae channel 2 compare register CR2 if asymmetric mode is selected */
  if (HandlePtr->CompareMode == CCU8PWMLIB_ASYMMETRIC)
  {
    HandlePtr->CC8yRegs1Ptr->CR2S = (uint32_t)
              ((HandlePtr->kCompareValue2 & 0xFFFF0000) >> 16);
  }

/*<<<DD_CCU8PWMLIB_nonAPI_1_1>>>*/
  /* Set timer concatenation bit */
  HandlePtr->CC8yRegs1Ptr->CMC |= (0x01 << CCU8_CC8_CMC_TCE_Pos);


/*<<<DD_CCU8PWMLIB_nonAPI_1_3>>>*/
  /* Configure external stop feature */
  if (HandlePtr->kExtStopTrig == (uint8_t)SET)
  {
    HandlePtr->CC8yRegs1Ptr->INS &= ~((CCU8_CC8_INS_EV1EM_Msk)|
        (CCU8_CC8_INS_LPF1M_Msk));
    HandlePtr->CC8yRegs1Ptr->CMC &= ~(CCU8_CC8_CMC_ENDS_Msk);
    HandlePtr->CC8yRegs1Ptr->INS |=
    ((((uint32_t)HandlePtr->kStopEdge << CCU8_CC8_INS_EV1EM_Pos) & CCU8_CC8_INS_EV1EM_Msk)|
        ((CCU8PWMLIB_LPF << CCU8_CC8_INS_LPF1M_Pos)& CCU8_CC8_INS_LPF1M_Msk));
    HandlePtr->CC8yRegs1Ptr->CMC |=
        ((CCU8PWMLIB_EVENT_1  << CCU8_CC8_CMC_ENDS_Pos)& CCU8_CC8_CMC_ENDS_Msk);
  }/*End of if (HandlePtr->kExtStopTrig == SET)*/

/*<<<DD_CCU8PWMLIB_nonAPI_1_4>>>*/
  /* Configure trap feature */
  if (HandlePtr->kTrapEnable == (uint8_t)SET)
  {
    HandlePtr->CC8yRegs1Ptr -> INS &= ~((CCU8_CC8_INS_EV2EM_Msk)|
       (CCU8_CC8_INS_EV2LM_Msk) | (CCU8_CC8_INS_LPF2M_Msk) );
    HandlePtr->CC8yRegs1Ptr -> INS |=
    (((0x00  << CCU8_CC8_INS_EV2EM_Pos)& CCU8_CC8_INS_EV2EM_Msk) |
     ((((uint32_t)HandlePtr->kTrapLevel  << CCU8_CC8_INS_EV2LM_Pos)& CCU8_CC8_INS_EV2LM_Msk) |
         ((0x00 << CCU8_CC8_INS_LPF2M_Pos)& CCU8_CC8_INS_LPF2M_Msk)));
    HandlePtr->CC8yRegs1Ptr->CMC |= (0x01 << CCU8_CC8_CMC_TS_Pos);
  }/*End of if (HandlePtr->kTrapEnable == SET)*/
/*Configure timer mode, external start, stop feature */
  HandlePtr->CC8yRegs1Ptr->TC &= ~((CCU8_CC8_TC_TCM_Msk) | \
      (CCU8_CC8_TC_TSSM_Msk) | (CCU8_CC8_TC_CMOD_Msk) |  \
      (CCU8_CC8_TC_STRM_Msk) | (CCU8_CC8_TC_ENDM_Msk)|( CCU8_CC8_TC_DITHE_Msk));
  HandlePtr->CC8yRegs1Ptr->TC =(((uint32_t)
      HandlePtr->CountingMode  << CCU8_CC8_TC_TCM_Pos)& CCU8_CC8_TC_TCM_Msk)|
      ((HandlePtr->kTimerMode  << CCU8_CC8_TC_TSSM_Pos)& CCU8_CC8_TC_TSSM_Msk)|
      ((CCU8PWMLIB_COMPARE_MODE  << CCU8_CC8_TC_CMOD_Pos)& CCU8_CC8_TC_CMOD_Msk)|
      (((uint32_t)
      HandlePtr->ExtStartConfig  << CCU8_CC8_TC_STRM_Pos)& CCU8_CC8_TC_STRM_Msk)|
      (((uint32_t)
      HandlePtr->ExtStopConfig << CCU8_CC8_TC_ENDM_Pos)& CCU8_CC8_TC_ENDM_Msk);

/*<<<DD_CCU8PWMLIB_nonAPI_1_5>>>*/
  /* Configure dither setting */
  HandlePtr->CC8yRegs1Ptr->TC |= (HandlePtr->kDitherSetting << CCU8_CC8_TC_DITHE_Pos) \
      & CCU8_CC8_TC_DITHE_Msk;
  HandlePtr->CC8yRegs1Ptr->DITS &= ~(CCU8_CC8_DITS_DCVS_Msk);
  HandlePtr->CC8yRegs1Ptr->DITS = (HandlePtr->kDitherCompare  << CCU8_CC8_DITS_DCVS_Pos)\
      & CCU8_CC8_DITS_DCVS_Msk;

/*<<<DD_CCU8PWMLIB_API_non1_6>>>*/
  WR_REG(HandlePtr->CC8yRegs1Ptr->TC, CCU8_CC8_TC_TRPSE_Msk,
               CCU8_CC8_TC_TRPSE_Pos, HandlePtr->kTrapSync);
  WR_REG(HandlePtr->CC8yRegs1Ptr->TC, CCU8_CC8_TC_TRPSW_Msk,
        CCU8_CC8_TC_TRPSW_Pos, HandlePtr->kTrapExitControl);

/*Set the prescalar divider and passive level of the o/p signal.*/
  WR_REG(HandlePtr->CC8yRegs1Ptr->PSC, CCU8_CC8_PSC_PSIV_Msk,
      CCU8_CC8_PSC_PSIV_Pos, HandlePtr->kCCUPrescalar);

/*Set passive level of the output signal*/
  HandlePtr->CC8yRegs1Ptr->PSL =
    ((HandlePtr->kPassiveLevel0 << CCU8_CC8_PSL_PSL11_Pos) |
    (HandlePtr->kPassiveLevel1 << CCU8_CC8_PSL_PSL12_Pos));

/*Set the symmetric or asymmetric PWM setting*/
  WR_REG(HandlePtr->CC8yRegs1Ptr->CHC, CCU8_CC8_CHC_ASE_Msk,\
        CCU8_CC8_CHC_ASE_Pos,(uint32_t)HandlePtr->CompareMode);

/*Configure output connections as per passive state.*/
  HandlePtr->CC8yRegs1Ptr->CHC &= ~((CCU8_CC8_CHC_OCS1_Msk)|
      CCU8_CC8_CHC_OCS2_Msk);
  HandlePtr->CC8yRegs1Ptr->CHC |=
    (((uint32_t)HandlePtr->kPassiveState0 << CCU8_CC8_CHC_OCS1_Pos) |
    ((~(uint32_t)HandlePtr->kPassiveState1) << CCU8_CC8_CHC_OCS2_Pos));
/*Configure dead time settings*/
  HandlePtr->CC8yRegs1Ptr->DTC &= ~((CCU8_CC8_DTC_DTCC_Msk) |
      (CCU8_CC8_DTC_DTE1_Msk)| (CCU8_CC8_DTC_DTE2_Msk) |
      (CCU8_CC8_DTC_DCEN1_Msk)|(CCU8_CC8_DTC_DCEN2_Msk));
  HandlePtr->CC8yRegs1Ptr->DTC |= (uint32_t)
  (((HandlePtr->kDeadTimePrescalar  << CCU8_CC8_DTC_DTCC_Pos)& CCU8_CC8_DTC_DTCC_Msk) |
      DeadTimeConfVal[HandlePtr->DeadTimeConf]);

  HandlePtr->CC8yRegs1Ptr->DC1R &= ~((CCU8_CC8_DC1R_DT1R_Msk)|
      (CCU8_CC8_DC1R_DT1F_Msk));
  HandlePtr->CC8yRegs1Ptr->DC1R |= (uint32_t)
  (( HandlePtr->kRisingDeadTime  << CCU8_CC8_DC1R_DT1R_Pos)& CCU8_CC8_DC1R_DT1R_Msk) |
  (( HandlePtr->kFallingDeadTime  << CCU8_CC8_DC1R_DT1F_Pos)& CCU8_CC8_DC1R_DT1F_Msk);
  
  /*Request SW shadow transfer for period, compare, dither and prescalar level*/
  HandlePtr->CC8yKernRegsPtr->GCSS |= ((0x01UL <<
                                  (4 * (uint32_t)HandlePtr->FirstSlice)) |
                                  (0x01UL <<
                                  ((4 * (uint32_t)HandlePtr->FirstSlice) + 1)) |
                                  (0x01UL <<
                                  ((4 * (uint32_t)HandlePtr->FirstSlice) + 2)));
  HandlePtr->CC8yKernRegsPtr->GCSS |= ((0x01 <<
                                      (4 * (uint32_t)HandlePtr->SecondSlice)) |
                                      (0x01UL <<
                                ((4 * (uint32_t)HandlePtr->SecondSlice) + 1)) |
                                     (0x01UL <<
                                ((4 * (uint32_t)HandlePtr->SecondSlice) + 2)));
}

/*<<<DD_CCU8PWMLIB_API_2>>>*/
/**
 * This function resets the app and the CCU8x_CC8y slice
 */
status_t CCU8PWMLIB_Deinit(const CCU8PWMLIB_HandleType* HandlePtr)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;

/*<<<DD_CCU8PWMLIB_API_2_1>>>*/
/*<<<DD_CCU8PWMLIB_API_2_3>>>*/
/*Clear the RUN bit of the slice.*/
    HandlePtr->CC8yRegsPtr->TCCLR = CCU8PWMLIB_SLICE_CLEAR;
/*Clear all interrupts and disable the interrupts.*/
    HandlePtr->CC8yRegsPtr->SWR = CCU8PWMLIB_ALL_CCU8_INTR_CLEAR;
    HandlePtr->CC8yRegsPtr->INTE = 0x00UL;

/*Clear all the registers.*/
    HandlePtr->CC8yRegsPtr->INS = 0x00UL;
    HandlePtr->CC8yRegsPtr->CMC = 0x00UL;
    HandlePtr->CC8yRegsPtr->TC = 0x00UL;
    HandlePtr->CC8yRegsPtr->PSC = 0x00UL;
    HandlePtr->CC8yRegsPtr->PSL = 0x00UL;
    HandlePtr->CC8yRegsPtr->DITS = 0x00UL;
    HandlePtr->CC8yRegsPtr->DTC = 0x00UL;
    HandlePtr->CC8yRegsPtr->DC1R = 0x00UL;
    HandlePtr->CC8yRegsPtr->CHC = 0x00UL;
    HandlePtr->CC8yRegsPtr->CR1S = 0x00UL;
    HandlePtr->CC8yRegsPtr->CR2S = 0x00UL;
    HandlePtr->CC8yRegsPtr->PRS = 0x00UL;

    HandlePtr->CC8yKernRegsPtr->GCSS |=
     (0x01 << (CCU8_GCSS_S0SE_Pos + 4* (uint32_t)HandlePtr->FirstSlice));

  /*Set IDLE mode.*/
    HandlePtr->CC8yKernRegsPtr->GIDLS = ((0x01UL<< (CCU8_GIDLS_SS0I_Pos +
                                          (uint32_t)HandlePtr->FirstSlice)) |
                                         (0x01UL << CCU8_GIDLS_CPRB_Pos) |
                                         (0x01UL << CCU8_GIDLS_PSIC_Pos));

/*<<<DD_CCU8PWMLIB_API_2_2>>>*/
    if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
/*Clear the RUN bit of the slice.*/
      HandlePtr->CC8yRegs1Ptr->TCCLR = CCU8PWMLIB_SLICE_CLEAR;
/*Clear all interrupts and disable the interrupts.*/
      HandlePtr->CC8yRegs1Ptr->SWR = CCU8PWMLIB_ALL_CCU8_INTR_CLEAR;
      HandlePtr->CC8yRegs1Ptr->INTE = 0x00UL;

/*Clear all the registers.*/
      HandlePtr->CC8yRegs1Ptr->INS = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->CMC = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->TC = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->PSC = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->PSL = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->DITS = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->DTC = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->DC1R = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->CHC = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->CR1S = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->CR2S = 0x00UL;
      HandlePtr->CC8yRegs1Ptr->PRS = 0x00UL;

      HandlePtr->CC8yKernRegsPtr->GCSS |=
          (0x01 << (CCU8_GCSS_S0SE_Pos + 4* (uint32_t)HandlePtr->SecondSlice));
   /*Set IDLE mode.*/
      HandlePtr->CC8yKernRegsPtr->GIDLS = ((0x01UL<< (CCU8_GIDLS_SS0I_Pos +
                                            (uint32_t)HandlePtr->SecondSlice)) |
                                           (0x01UL << CCU8_GIDLS_CPRB_Pos) |
                                           (0x01UL << CCU8_GIDLS_PSIC_Pos));
    }
    return Status;
}

/*<<<DD_CCU8PWMLIB_API_3>>>*/
/**
 * This function starts the app and sets the run bit of the timer
 */
status_t CCU8PWMLIB_Start(const CCU8PWMLIB_HandleType* HandlePtr)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;
/*<<<DD_CCU8PWMLIB_API_3_1>>>*/

/*<<<DD_CCU8PWMLIB_API_3_2>>>*/
    HandlePtr->CC8yRegsPtr->SWR = CCU8PWMLIB_ALL_CCU8_INTR_CLEAR;
    /**
     * App can be started from the top level app.
     * If Start is to be done by the App then clear the IDLE mode
     * If Start is 0 then clearing IDLE mode will be done by other App
     */
    if (HandlePtr->Start == (uint8_t)SET)
    {
      HandlePtr->CC8yKernRegsPtr->GIDLC |= HandlePtr->StartMask;
    }
    
    WR_REG(HandlePtr->CC8yRegsPtr->TC, CCU8_CC8_TC_TRAPE0_Msk,
             CCU8_CC8_TC_TRAPE0_Pos, HandlePtr->kTrapEnable);
    WR_REG(HandlePtr->CC8yRegsPtr->TC, CCU8_CC8_TC_TRAPE1_Msk,
             CCU8_CC8_TC_TRAPE1_Pos, HandlePtr->kTrapEnable);    
	if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
  	    WR_REG(HandlePtr->CC8yRegs1Ptr->TC, CCU8_CC8_TC_TRAPE0_Msk,
               CCU8_CC8_TC_TRAPE0_Pos, HandlePtr->kTrapEnable);
  	    WR_REG(HandlePtr->CC8yRegs1Ptr->TC, CCU8_CC8_TC_TRAPE1_Msk,
                 CCU8_CC8_TC_TRAPE1_Pos, HandlePtr->kTrapEnable);
    }        
    
/* Set RUN bit if external start is not configured */
    if (HandlePtr->kExtStartTrig == (uint8_t)RESET)
    {     
      if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
      {
        SET_BIT(HandlePtr->CC8yRegs1Ptr->TCSET, CCU8_CC8_TCSET_TRBS_Pos);
  	/* Configure trap settings */
      }
      SET_BIT(HandlePtr->CC8yRegsPtr->TCSET, CCU8_CC8_TCSET_TRBS_Pos);
       /* Configure trap settings */
    }
    else
    {
    	/*<<<DD_CCU8PWMLIB_API_1_2>>>*/
    	 /* Configure external start feature */
		if (HandlePtr->kExtStartTrig == (uint8_t)SET)
		{
		  HandlePtr->CC8yRegsPtr ->INS &= ~((CCU8_CC8_INS_EV0EM_Msk)|
				 (CCU8_CC8_INS_LPF0M_Msk));
		  HandlePtr->CC8yRegsPtr->CMC &= ~CCU8_CC8_CMC_STRTS_Msk;
		  HandlePtr->CC8yRegsPtr ->INS |=
				((((uint32_t)HandlePtr->kStartEdge <<
				 CCU8_CC8_INS_EV0EM_Pos)& CCU8_CC8_INS_EV0EM_Msk) |
				((CCU8PWMLIB_LPF  << CCU8_CC8_INS_LPF0M_Pos)& CCU8_CC8_INS_LPF0M_Msk));
		  HandlePtr->CC8yRegsPtr->CMC |=((CCU8PWMLIB_EVENT_0  <<
								  CCU8_CC8_CMC_STRTS_Pos)& CCU8_CC8_CMC_STRTS_Msk);
		}
		/*<<<DD_CCU8PWMLIB_nonAPI_1_2>>>*/
		/* Configure external start feature*/
		if ((HandlePtr->kExtStartTrig == (uint8_t)SET)&&\
			(HandlePtr->kTimerConcatenation == (uint8_t)SET)
		    )
		{
			HandlePtr->CC8yRegs1Ptr->INS &= ~((CCU8_CC8_INS_EV0EM_Msk) |
			   (CCU8_CC8_INS_LPF0M_Msk));
			HandlePtr->CC8yRegs1Ptr->CMC &= ~(CCU8_CC8_CMC_STRTS_Msk);
			HandlePtr->CC8yRegs1Ptr->INS |=
			((((uint32_t)HandlePtr->kStartEdge << CCU8_CC8_INS_EV0EM_Pos)& CCU8_CC8_INS_EV0EM_Msk) |
				((CCU8PWMLIB_LPF  << CCU8_CC8_INS_LPF0M_Pos)& CCU8_CC8_INS_LPF0M_Msk));
			HandlePtr->CC8yRegs1Ptr->CMC |=
				((CCU8PWMLIB_EVENT_0  << CCU8_CC8_CMC_STRTS_Pos)& CCU8_CC8_CMC_STRTS_Msk);
		}/*End of if (HandlePtr->kExtStartTrig == SET)*/

    }
    return Status;
}

/* This function enables the externat start feature.
 */
status_t CCU8PWMLIB_EnableExtStart(const CCU8PWMLIB_HandleType* HandlePtr)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  
  /*clear external start functionality */
  HandlePtr->CC8yRegsPtr->INS &= ~((CCU8_CC8_INS_EV0EM_Msk) |
     (CCU8_CC8_INS_LPF0M_Msk));
  HandlePtr->CC8yRegsPtr->INS |= ((uint32_t)HandlePtr->kStartEdge <<
             CCU8_CC8_INS_EV0EM_Pos) & CCU8_CC8_INS_EV0EM_Msk;
  if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    HandlePtr->CC8yRegs1Ptr->INS &= ~((CCU8_CC8_INS_EV0EM_Msk) |
        (CCU8_CC8_INS_LPF0M_Msk));
    HandlePtr->CC8yRegs1Ptr->INS |= ((uint32_t)HandlePtr->kStartEdge <<
             CCU8_CC8_INS_EV0EM_Pos) & CCU8_CC8_INS_EV0EM_Msk;
  }
  return Status;
}

/* This function disables the externat start feature.
 * This is useful for multiphase PWM app to disable this functionality to avoid spurious 
 * start of the slices due to other instances of the app. 
 */
status_t CCU8PWMLIB_DisableExtStart(const CCU8PWMLIB_HandleType* HandlePtr)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  
  /*clear external start functionality */
  HandlePtr->CC8yRegsPtr->INS &= ~((CCU8_CC8_INS_EV0EM_Msk) |
     (CCU8_CC8_INS_LPF0M_Msk));
  if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    HandlePtr->CC8yRegs1Ptr->INS &= ~((CCU8_CC8_INS_EV0EM_Msk) |
        (CCU8_CC8_INS_LPF0M_Msk));
  }
  return Status;
}

/*<<<DD_CCU8PWMLIB_API_4>>>*/
/**
 * This function stops the app and the timer
 */
status_t CCU8PWMLIB_Stop(const CCU8PWMLIB_HandleType* HandlePtr)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;

/*<<<DD_CCU8PWMLIB_API_4_1>>>*/
/*Clear RUN bit of the slice in use*/
    HandlePtr->CC8yRegsPtr->TCCLR = CCU8PWMLIB_SLICE_CLEAR;

/*<<<DD_CCU8PWMLIB_API_4_2>>>*/
    if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
/*Clear RUN bit of the slice in use*/
      HandlePtr->CC8yRegs1Ptr->TCCLR = CCU8PWMLIB_SLICE_CLEAR;

    }/*End of "if (HandlePtr->kTimerConcatenation == (uint8_t)SET)"*/
    /**
     * Set the IDLE mode only if Start parameter is set
     */
    if(HandlePtr->Start == (uint8_t)SET)
    {
      HandlePtr->CC8yKernRegsPtr->GIDLS |= HandlePtr->StartMask;
    }
    return Status;
 }

/*<<<DD_CCU8PWMLIB_API_5>>>*/
/**
 * This function changes the duty cycle of the PWM waveform by changing the
 * compare register value.
 * Sign = 0: Compare value is incremented by shift. This is not allowed in edge-aligned.
 * Sign = 1: Compare value is decremented by shift.
 */
status_t CCU8PWMLIB_SetCompare
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t Compare1,
  uint32_t Compare2
)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;
/*<<<DD_CCU8PWMLIB_API_5_3>>>*/
    /* Call the function as per configured mode */
    Status = HandlePtr->SetCompareFuncPtr((void*)HandlePtr, Compare1, Compare2);
    return Status;
}

status_t CCU8PWMLIB_SetCompareAsymmetric
(
    const void* HdlPtr,
    uint32_t Compare1,
    uint32_t Compare2
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  const CCU8PWMLIB_HandleType* HandlePtr = (CCU8PWMLIB_HandleType*) HdlPtr;  
  
  HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)(Compare1 & 0xffff);
  HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)(Compare2 & 0xffff);
  
  /*Request for shadow transfer*/
  Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
  return Status;
}

status_t CCU8PWMLIB_SetCompareSymmetric
(
    const void* HdlPtr,
    uint32_t Compare1,
    uint32_t Compare2
)
{
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
  const CCU8PWMLIB_HandleType* HandlePtr = (CCU8PWMLIB_HandleType*) HdlPtr;  
  Compare2 = 0;
  HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)(Compare1 & 0xffff);
  HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)(0);
  /*Request for shadow transfer*/
  Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
  return Status;
}

status_t CCU8PWMLIB_SetCompareEdgeAlignSymmetricTimerConcat
(
    const void* HdlPtr,
    uint32_t Compare1,
    uint32_t Compare2
)
{
  uint32_t FirstSliceCompareVal = 0;
  uint32_t SecondSliceCompareVal = 0;
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
  const CCU8PWMLIB_HandleType* HandlePtr = (CCU8PWMLIB_HandleType*) HdlPtr;  
  Compare2 = 0;
  FirstSliceCompareVal = (uint32_t)Compare1 % \
      (uint16_t)HandlePtr->CC8yRegsPtr->PRS;
  SecondSliceCompareVal = (uint32_t)Compare1 /
      (uint16_t)HandlePtr->CC8yRegsPtr->PRS;
  HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)(FirstSliceCompareVal & 0xffff);
  HandlePtr->CC8yRegs1Ptr->CR1S = (uint32_t)(SecondSliceCompareVal & 0xffff);
  HandlePtr->CC8yRegs1Ptr->CR2S = (uint32_t)(0);
  HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)(0);
  /*Request for shadow transfer*/
  Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
  return Status;
}


/**
 * This function changes the duty cycle of the app.
 * Sign = 0: Compare value is incremented by shift. This is not allowed in edge-aligned.
 * Sign = 1: Compare value is decremented by shift.
 */
status_t CCU8PWMLIB_SetDutyCycle
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;
    /*<<<DD_CCU8PWMLIB_API_15_2>>>*/
    /* duty cycle has to be in between 0 and 100 */
    /* Call the function as per configured mode */
    Status = HandlePtr->SetDutyFuncPtr((void*)HandlePtr, DutyCycle, Shift, Sign);

    return Status;
}
/**
 * This is the private function which sets the duty cycle for edge-aligned mode.
 * Period = PR + 1
 * Duty cycle (Asymmetric) = (CR2-CR1) / Period
 */
status_t CCU8PWMLIB_SetDutyEdgeAlignAsymmetric
(
  const void* HdlPtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
)
{
  uint32_t Comp = 0;
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
  const CCU8PWMLIB_HandleType* HandlePtr = (CCU8PWMLIB_HandleType*) HdlPtr;  
  /*<<<DD_CCU8PWMLIB_API_15_3>>>*/
  /*<<<DD_CCU8PWMLIB_API_15_4>>>*/ 
  Comp = (uint32_t)
      (((HandlePtr->CC8yRegsPtr->PRS + 1) * (100 - DutyCycle))/100);

  /* If shift value is greater than compare register value or if sign is 0, report error */
  if(((Sign == (uint8_t)RESET)&& Shift >0) || (Shift > Comp))
  {
    Status = (uint32_t)CCU8PWMLIB_INVALID_PARAM_ERROR;
  }
  else
  {
    /*  <<<DD_CCU8PWMLIB_nonAPI_3_3>>>*/
    HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)((Comp - Shift) & 0xffff);
    HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)
          ((HandlePtr->CC8yRegsPtr->PRS - Shift) & 0xffff);
    /*Request for shadow transfer*/
    Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
  }
  return (Status);
}

/**
 * This is the private function which sets the duty cycle for 
 * Edge-aligned mode with symmetric compare mode and without timer concatenation
 * Period = (PR + 1)
 * Duty Cycle(symmetric) = (PR-CR1)+1 / Period
 */
status_t CCU8PWMLIB_SetDutyEdgeAlignSymmetric
(
  const void* HdlPtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
)
{
  uint32_t Comp = 0;
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
  
  const CCU8PWMLIB_HandleType* HandlePtr = (CCU8PWMLIB_HandleType*) HdlPtr; 
  /*<<<DD_CCU8PWMLIB_API_15_3>>>*/
  /*<<<DD_CCU8PWMLIB_API_15_4>>>*/
  Comp = (uint32_t)
      (((HandlePtr->CC8yRegsPtr->PRS + 1) * (100 -DutyCycle))/100);

  /* If shift value is greater than compare register value or if sign is 0, report error */
  if(((Sign == (uint8_t)RESET)&& Shift >0) || (Shift > Comp))
  {
    Status = (uint32_t)CCU8PWMLIB_INVALID_PARAM_ERROR;
  }
  else {
  
    HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)(Comp & 0xffff);
    HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)(0);
    /*initiate the shadow transfer  */
    Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
  }
  return (Status);
}

/**
 * This is the private function which sets the duty cycle for 
 * edge-aligned mode with symmetric compare mode and with timer concatenation.
 * Period = PR + 1
 * Duty Cycle(symmetric) = (PR-CR1)+1 / Period
 */
status_t CCU8PWMLIB_SetDutyEdgeAlignSymmetricTimerConcat
(
  const void* HdlPtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
)
{
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
  uint32_t FirstSliceCompareVal1 = 0;
  uint32_t SecondSliceCompareVal1 = 0;
  uint32_t PeriodReg = 0;
  uint32_t Comp = 0;
  
  const CCU8PWMLIB_HandleType* HandlePtr = (CCU8PWMLIB_HandleType*) HdlPtr;  

  Comp = (HandlePtr->CC8yRegs1Ptr->PRS+ 1);
  Comp *=(HandlePtr->CC8yRegsPtr->PRS +1);
  Comp += 1;
  Comp = (uint32_t)((Comp * (100-DutyCycle))/100);
  PeriodReg = ((HandlePtr->CC8yRegs1Ptr->PRS)<< 16UL);
  PeriodReg |= (HandlePtr->CC8yRegsPtr->PRS);
   /* If shift value is greater than compare register value or if sign is 0, report error */
  if(((Sign == (uint8_t)RESET)&& Shift >0) || (Shift > Comp))
  {
    Status = (uint32_t)CCU8PWMLIB_INVALID_PARAM_ERROR;
  }

  else{
    /*  <<<DD_CCU8PWMLIB_nonAPI_2_1>>>*/
    /*  <<<DD_CCU8PWMLIB_nonAPI_2_2>>>*/
    FirstSliceCompareVal1 = (uint32_t)Comp % \
        (uint16_t)PeriodReg;
    SecondSliceCompareVal1 = (uint32_t)Comp /
        (uint16_t)PeriodReg;
    HandlePtr->CC8yRegsPtr->CR1S = (FirstSliceCompareVal1 & 0xFFFF);
    HandlePtr->CC8yRegs1Ptr->CR1S = (SecondSliceCompareVal1 & 0xFFFF);
    HandlePtr->CC8yRegs1Ptr->CR2S = (uint32_t)(0);
    HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)(0);
    /*  <<<DD_CCU8PWMLIB_nonAPI_2_3>>>*/
    /*Request for shadow transfer*/
    Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
  }
  return Status;
}

/**
 * This is the private function which sets the duty cycle for center-aligned mode.
 * Period = (PR + 1)* 2
 * Duty Cycle(symmetric) = 2(PR - CR1) + 1/ Period
 */
status_t CCU8PWMLIB_SetDutyCenterAlignSymmetric
(
  const void* HdlPtr,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
)
{
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
  uint32_t Comp = 0;
  const CCU8PWMLIB_HandleType* HandlePtr = (CCU8PWMLIB_HandleType*) HdlPtr;  
  Sign = 0;
  /* Find the compare register value for center-aligned mode. */
  Comp = (uint32_t)(((HandlePtr->CC8yRegsPtr->PRS ) * (100-DutyCycle))/100);
  if((HandlePtr->kTimerConcatenation == (uint8_t)SET) || (Shift>Comp))
  {
    Status = (uint32_t)CCU8PWMLIB_INVALID_PARAM_ERROR;
  }
  else{
    /*  <<<DD_CCU8PWMLIB_nonAPI_3_1>>>*/
    HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)(Comp & 0xffff);
    HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)(0);
      /* initiate the shadow transfer */
    Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
  }
  return (Status);

}

/**
 * This is the private function which sets the duty cycle for center-aligned mode.
 * Period = (PR + 1)* 2
 * Duty cycle (Asymmetric) = (PR-CR1) + (PR-CR2) +1 / Period
 */
status_t CCU8PWMLIB_SetDutyCenterAlignAsymmetric
(
    const void* HdlPtr,
    float DutyCycle,
    uint32_t Shift,
    uint8_t Sign
)
{
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;  
  uint32_t Comp = 0;
  const CCU8PWMLIB_HandleType* HandlePtr = (CCU8PWMLIB_HandleType*) HdlPtr;
  
  /* Find the compare register value for center-aligned mode. */
  Comp = (uint32_t)(((HandlePtr->CC8yRegsPtr->PRS) * (100-DutyCycle))/100);
  if((HandlePtr->kTimerConcatenation == (uint8_t)SET) || (Shift>Comp))
  {
    Status = (uint32_t)CCU8PWMLIB_INVALID_PARAM_ERROR;
  }
  
  else{
 /*  <<<DD_CCU8PWMLIB_nonAPI_3_3>>>*/  
    if (Sign == (uint8_t)RESET)
    {

      HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)((Comp + Shift) & 0xffff);
      HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)((Comp - Shift) & 0xffff);
      /*  <<<DD_CCU8PWMLIB_nonAPI_3_5>>>*/
    } /*end of if (Sign == 0)*/
    /*  <<<DD_CCU8PWMLIB_nonAPI_3_4>>>*/
    else if (Sign == (uint8_t)SET)
    {
      HandlePtr->CC8yRegsPtr->CR1S = (uint32_t)((Comp - Shift) & 0xffff);
      HandlePtr->CC8yRegsPtr->CR2S = (uint32_t)((Comp + Shift) & 0xffff);
    }
  /*  <<<DD_CCU8PWMLIB_nonAPI_3_6>>>*/
  /* initiate the shadow transfer */
    Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
  }
  return (Status);
}

/** This function changes the PWM frequency */
status_t CCU8PWMLIB_SetPeriodAndCompare
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t PwmFreq,
  uint32_t Compare1,
  uint32_t Compare2
)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;

    /* Update the period register after checking timer concatenation setting */
    HandlePtr->CC8yRegsPtr->PRS = PwmFreq & 0xffff;
    if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
      HandlePtr->CC8yRegs1Ptr->PRS = ((PwmFreq & 0xffff0000) >> 16UL);
      HandlePtr->CC8yRegsPtr->PRS = (PwmFreq & 0xffff);
    }
    /*<<<DD_CCU8PWMLIB_API_6_2>>>*/

    /*<<<DD_CCU8PWMLIB_API_6_3>>*/
/* Set compare register values and request shadow transfer */
    Status = HandlePtr->SetCompareFuncPtr((void*)HandlePtr, Compare1, Compare2);
    return Status;
}

/*<<<DD_CCU8PWMLIB_API_6>>>*/
/** This function changes the PWM frequency */
status_t CCU8PWMLIB_SetPeriod
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t PwmFreq
)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;

    /* Update the period register after checking timer concatenation setting */
    HandlePtr->CC8yRegsPtr->PRS = PwmFreq & 0xffff;
    if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
      HandlePtr->CC8yRegs1Ptr->PRS = ((PwmFreq & 0xffff0000) >> 16UL);
      HandlePtr->CC8yRegsPtr->PRS = (PwmFreq & 0xffff);
    }
    /*<<<DD_CCU8PWMLIB_API_6_2>>>*/

    /*<<<DD_CCU8PWMLIB_API_6_3>>*/

    /*Request for shadow transfer*/
    Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
   

    return Status;
}

/* This function changes the PWM frequency */
status_t CCU8PWMLIB_SetPwmFreqAndDutyCycle
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  float PwmFreq,
  float DutyCycle,
  uint32_t Shift,
  uint8_t Sign
)
{
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
  uint32_t PwmTime = 0UL;

  do
  {
/* Find the time reuired for given frequency and then the period register value
 * depending upon edge-aligned or center-aligned
 */

    PwmTime = (uint32_t)((1000000000) / (HandlePtr->kResolution * PwmFreq));

    if(HandlePtr->kTimerConcatenation == (uint8_t)RESET)
    {
      if(PwmTime > CCU8PWMLIB_MAX_VALUE)
      {
        Status = (uint32_t)CCU8PWMLIB_INVALID_PARAM_ERROR;
        break;
      } /* End of "if(PwmTime > CCU8PWMLIB_MAX_VALUE)" */
      if(HandlePtr->CountingMode == CCU8PWMLIB_EDGE_ALIGNED)
      {
        HandlePtr->CC8yRegsPtr->PRS = PwmTime - 1;
      } /* End of "if(HandlePtr->CountingMode == CCU8PWMLIB_EDGE_ALIGNED)" */
      else if(HandlePtr->CountingMode == CCU8PWMLIB_CENTER_ALIGNED)
      {
        HandlePtr->CC8yRegsPtr->PRS = (PwmTime - 1) >> 1;
      }/* End of "else if(HandlePtr->CountingMode == CCU8PWMLIB_CENTER_ALIGNED)" */
      break;
    } /* End of "if(HandlePtr->kTimerConcatenation == (uint8_t)RESET)" */

    if(HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
      Status = CCU8PWMLIB_lSetPwmFreqTimerConcat(HandlePtr, PwmTime);
      break;
    }
  } while (0);

  /* Call the function as per configured mode and request shadow transfer*/
  Status = HandlePtr->SetDutyFuncPtr((void*)HandlePtr, DutyCycle, Shift, Sign);
  return Status;
}


/*<<<DD_CCU8PWMLIB_API_16>>>*/
/* This function changes the PWM frequency */
status_t CCU8PWMLIB_SetPwmFreq
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  float PwmFreq
)
{
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
  uint32_t PwmTime = 0UL;

  do
  {
/* Find the time reuired for given frequency and then the period register value
 * depending upon edge-aligned or center-aligned
 */

    PwmTime = (uint32_t)((1000000000) / (HandlePtr->kResolution * PwmFreq));

    if(HandlePtr->kTimerConcatenation == (uint8_t)RESET)
    {
      if(PwmTime > CCU8PWMLIB_MAX_VALUE)
      {
        Status = (uint32_t)CCU8PWMLIB_INVALID_PARAM_ERROR;
        break;
      } /* End of "if(PwmTime > CCU8PWMLIB_MAX_VALUE)" */
      if(HandlePtr->CountingMode == CCU8PWMLIB_EDGE_ALIGNED)
      {
        HandlePtr->CC8yRegsPtr->PRS = PwmTime - 1;
      } /* End of "if(HandlePtr->CountingMode == CCU8PWMLIB_EDGE_ALIGNED)" */
      else if(HandlePtr->CountingMode == CCU8PWMLIB_CENTER_ALIGNED)
      {
        HandlePtr->CC8yRegsPtr->PRS = (PwmTime - 1) >> 1;
      }/* End of "else if(HandlePtr->CountingMode == CCU8PWMLIB_CENTER_ALIGNED)" */
       /*Request for shadow transfer*/
      Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
      break;
    } /* End of "if(HandlePtr->kTimerConcatenation == (uint8_t)RESET)" */

    if(HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
      Status = CCU8PWMLIB_lSetPwmFreqTimerConcat(HandlePtr, PwmTime);
      /*Request for shadow transfer*/
      Status = CCU8PWMLIB_SWRequestShadowTransfer(HandlePtr);
      break;
    }
  } while (0);


  return Status;
}

/**
 * This is the private function to set the period register if timer concatenation is enabled
 */
status_t CCU8PWMLIB_lSetPwmFreqTimerConcat
(
    const CCU8PWMLIB_HandleType* HandlePtr,
    uint32_t PwmTime
)
{
  uint32_t PeriodVal = PwmTime;
  uint32_t MsbPeriodVal, LsbPeriodVal;
  uint8_t Count = 0;
  status_t Status = (uint32_t)CCU8PWMLIB_OPER_NOT_ALLOWED_ERROR;
/*
 * In case of timer concatenation, period value of two slices needs to be found out.
 * Total Period = ((PR1 + 1) * PR2) + 1
 * PR1 is found out by dividing given period time by 2 till the value is less than
 * 65535. Quotient is the period register value of the second slice.
 */
  if(PwmTime > CCU8PWMLIB_MAX_VALUE)
  {
    do
    {
      PeriodVal = PeriodVal >> 1;
      Count++;
    }while(PeriodVal >= CCU8PWMLIB_MAX_VALUE);
    MsbPeriodVal = ((uint32_t)1 << Count) -1;
    LsbPeriodVal = PeriodVal;
  }
  else
  {
    LsbPeriodVal = PwmTime;
    MsbPeriodVal = 0;
  }
  /*<<<DD_CCU8PWMLIB_API_16_4>>>*/
  HandlePtr->CC8yRegsPtr->PRS = LsbPeriodVal & 0xffff;
  HandlePtr->CC8yRegs1Ptr->PRS = MsbPeriodVal & 0xffff;
  /*<<<DD_CCU8PWMLIB_API_16_5>>>*/
  return (Status);
}


/*<<<DD_CCU8PWMLIB_API_7>>*/
/**
 * This function sets the timer value when timer is not running
 */
status_t CCU8PWMLIB_SetTimerVal
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t TimerVal
)
{
    status_t Status = (uint32_t)DAVEApp_SUCCESS;
    HandlePtr->CC8yRegsPtr->TIMER = TimerVal & 0xffff;
    if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
      HandlePtr->CC8yRegs1Ptr->TIMER= ((TimerVal & 0xffff0000) >> 16UL);
    }
    return Status;
}
/*<<<DD_CCU8PWMLIB_API_8>>*/
/**
 * This function reads the timer status
 */
status_t CCU8PWMLIB_GetTimerStatus
(
 const CCU8PWMLIB_HandleType* HandlePtr,
 uint32_t* TimerStatusPtr
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  uint32_t SecondTimerStatus = 0;

/* In case of timer concatenation, both the timer status is read */
  if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    SecondTimerStatus = (uint32_t)(RD_REG(HandlePtr->CC8yRegs1Ptr->TCST,
                   CCU8_CC8_TCST_TRB_Msk, CCU8_CC8_TCST_TRB_Pos));
    *TimerStatusPtr = (uint32_t)((RD_REG(HandlePtr->CC8yRegsPtr->TCST,
                   CCU8_CC8_TCST_TRB_Msk, CCU8_CC8_TCST_TRB_Pos)) &
                 SecondTimerStatus
                 );
  }
  else
  {
    *TimerStatusPtr = (uint32_t) RD_REG(HandlePtr->CC8yRegsPtr->TCST,
                         CCU8_CC8_TCST_TRB_Msk, CCU8_CC8_TCST_TRB_Pos);
  }
  return Status;
}

/*<<<DD_CCU8PWMLIB_API_9>>*/
/**
 * This function reads the period, compare register values.
 */
status_t CCU8PWMLIB_GetTimerRegsVal
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  CCU8PWMLIB_TimerRegsType* TimerRegsPtr
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  uint32_t ScndSlTMRVal = 0UL;

/*<<<DD_CCU8PWMLIB_API_9_3>>>*/
  if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    TimerRegsPtr->CompReg1 = (uint32_t)(HandlePtr->CC8yRegs1Ptr->CR1S);
    TimerRegsPtr->CompReg1 *=(uint32_t)(HandlePtr->CC8yRegsPtr->PRS);
    TimerRegsPtr->CompReg1 += (uint32_t)(HandlePtr->CC8yRegsPtr->CR1S);                      
    TimerRegsPtr->CompReg2 = (uint32_t)(HandlePtr->CC8yRegs1Ptr->CR2S);
    TimerRegsPtr->CompReg2 *= (uint32_t)(HandlePtr->CC8yRegsPtr->PRS);
    TimerRegsPtr->CompReg2 += (uint32_t)(HandlePtr->CC8yRegsPtr->CR2S);
    TimerRegsPtr->PeriodReg = (uint32_t)((HandlePtr->CC8yRegs1Ptr->PRS)<<16UL);
    TimerRegsPtr->PeriodReg |= (HandlePtr->CC8yRegsPtr->PRS);
    ScndSlTMRVal = (RD_REG(HandlePtr->CC8yRegs1Ptr->TIMER,
        CCU8_CC8_TIMER_TVAL_Msk, CCU8_CC8_TIMER_TVAL_Pos) << 16UL);
    TimerRegsPtr->TimerReg = (uint32_t)((ScndSlTMRVal << 16UL) |
        (RD_REG(HandlePtr->CC8yRegsPtr->TIMER,
         CCU8_CC8_TIMER_TVAL_Msk, CCU8_CC8_TIMER_TVAL_Pos)));
  }
/*<<<DD_CCU8PWMLIB_API_9_2>>>*/
  else
  {
    TimerRegsPtr->CompReg1 = HandlePtr->CC8yRegsPtr->CR1S;
    TimerRegsPtr->CompReg2 = HandlePtr->CC8yRegsPtr->CR2S;
    TimerRegsPtr->PeriodReg = HandlePtr->CC8yRegsPtr->PRS;
    TimerRegsPtr->TimerReg = (uint32_t)RD_REG(HandlePtr->CC8yRegsPtr->TIMER,
                           CCU8_CC8_TIMER_TVAL_Msk, CCU8_CC8_TIMER_TVAL_Pos);
  }

  return Status;
}

/*<<<DD_CCU8PWMLIB_API_15>>>*/
/*
 * This function reads the period register value.
 * This function can be used by user to calculate the compare register value
 * as per required duty cycle.
 */
status_t CCU8PWMLIB_GetPeriodReg
(
  const CCU8PWMLIB_HandleType* HandlePtr,
  uint32_t* PeriodRegPtr
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    *PeriodRegPtr = ((HandlePtr->CC8yRegs1Ptr->PRS)<< 16UL);
    *PeriodRegPtr |= (HandlePtr->CC8yRegsPtr->PRS);
  }
  else
  {
    *PeriodRegPtr = HandlePtr->CC8yRegsPtr->PRS;
  }
  return Status;
}

/*<<<DD_CCU8PWMLIB_API_10>>>*/
/**
 * This function initiates the shadow transfer of period and compare registers.
 */
status_t CCU8PWMLIB_SWRequestShadowTransfer
(
 const CCU8PWMLIB_HandleType* HandlePtr
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
/*Request shadow transfer for the First Slice*/
  if(HandlePtr->ShadowTransfer == (uint32_t)SET)
  {
    HandlePtr->CC8yKernRegsPtr->GCSS |= HandlePtr->ShadowTransferMask;
  }

  return Status;
}

/*<<<DD_CCU8PWMLIB_API_13>>>*/
/**
 * This function resets the trap flag if SW exit from trap state is configured
 */
status_t CCU8PWMLIB_ResetTrapFlag(const CCU8PWMLIB_HandleType* HandlePtr)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  /* Clear the Trap interrupt is SW exit control is used */
  if (HandlePtr->kTrapExitControl == (uint8_t)SET)
  {
    SET_BIT(HandlePtr->CC8yRegsPtr->SWR, CCU8PWMLIB_EVENT2_INTERRUPT);
    if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
    {
      SET_BIT(HandlePtr->CC8yRegs1Ptr->SWR, CCU8PWMLIB_EVENT2_INTERRUPT);
    }
  }
  /* Clear the trap flag */
  SET_BIT(HandlePtr->CC8yRegsPtr->SWR, CCU8PWMLIB_TRAP_FLAG_CLEAR);
  if (HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    SET_BIT(HandlePtr->CC8yRegs1Ptr->SWR, CCU8PWMLIB_TRAP_FLAG_CLEAR);
  }
  return Status;
}
/**
 * This function sets the enable event bit for the event given in the argument.
 */
status_t CCU8PWMLIB_EnableEvent
(
     const CCU8PWMLIB_HandleType * HandlePtr,
     const CCU8PWMLIB_EventNameType Event
)
 {
   status_t Status = (uint32_t)DAVEApp_SUCCESS;
   if(HandlePtr->kTimerConcatenation == (uint8_t)SET)
   {
     SET_BIT(HandlePtr->CC8yRegs1Ptr->INTE, (uint8_t)Event);
   }
   else
   {
     SET_BIT(HandlePtr->CC8yRegsPtr->INTE,(uint8_t) Event);
   }
   return (Status);
 }

 /**
  * This function clears the enable event bit for the event given in the argument.
  */
status_t CCU8PWMLIB_DisableEvent
(
    const CCU8PWMLIB_HandleType * HandlePtr,
    const CCU8PWMLIB_EventNameType Event
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  if(HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    CLR_BIT(HandlePtr->CC8yRegs1Ptr->INTE,(uint8_t) Event);
  }
  else
  {
    CLR_BIT(HandlePtr->CC8yRegsPtr->INTE,(uint8_t) Event);
  }
  return (Status);
}

/**
 * This function clears the interrupt by software.
 */
status_t CCU8PWMLIB_ClearPendingEvent
(
    const CCU8PWMLIB_HandleType * HandlePtr,
    const CCU8PWMLIB_EventNameType Event
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  if(HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    SET_BIT(HandlePtr->CC8yRegs1Ptr->SWR,(uint8_t) Event);
  }
  else
  {
    SET_BIT(HandlePtr->CC8yRegsPtr->SWR,(uint8_t) Event);
  }
  return (Status);
}

/**
 * This function sets the interrupt by software Interrupt pulse is generated
 * if source is enabled.
 */
status_t CCU8PWMLIB_SetPendingEvent
(
    const CCU8PWMLIB_HandleType * HandlePtr,
    const CCU8PWMLIB_EventNameType Event
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;

  if(HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    SET_BIT(HandlePtr->CC8yRegs1Ptr->SWS, (uint8_t)Event);
  }
  else
  {
    SET_BIT(HandlePtr->CC8yRegsPtr->SWS, (uint8_t)Event);
  }
  return (Status);
}

/**
 * This function check whether given interrupt is set
 */
status_t CCU8PWMLIB_GetPendingEvent
(
    const CCU8PWMLIB_HandleType * HandlePtr,
    const CCU8PWMLIB_EventNameType Event,
    uint8_t* EvtStatus
)
{
  status_t Status = (uint32_t)DAVEApp_SUCCESS;
  if(HandlePtr->kTimerConcatenation == (uint8_t)SET)
  {
    *EvtStatus = RD_REG(HandlePtr->CC8yRegs1Ptr->INTS, (0x01 <<(uint8_t) Event),(uint8_t) Event) ? (uint8_t)SET : (uint8_t)RESET;
  }
  else
  {
    *EvtStatus = RD_REG(HandlePtr->CC8yRegsPtr->INTS, (0x01 <<(uint8_t) Event), (uint8_t)Event) ? (uint8_t)SET : (uint8_t)RESET;
  }
  return (Status);
}
#endif 

/**
 * @endcond
 */
/*CODE_BLOCK_END*/

