//*****************************************************************************
//
// adc.h - ADC headers for using the ADC driver functions.
//
// Copyright (c) 2005-2009 Luminary Micro, Inc.  All rights reserved.
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 4781 of the Stellaris Peripheral Driver Library.
//
//*****************************************************************************

#ifndef __ADC_H__
#define __ADC_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Values that can be passed to ADCSequenceConfigure as the ulTrigger
// parameter.
//
//*****************************************************************************
#define ADC_TRIGGER_PROCESSOR   0x00000000  // Processor event
#define ADC_TRIGGER_COMP0       0x00000001  // Analog comparator 0 event
#define ADC_TRIGGER_COMP1       0x00000002  // Analog comparator 1 event
#define ADC_TRIGGER_COMP2       0x00000003  // Analog comparator 2 event
#define ADC_TRIGGER_EXTERNAL    0x00000004  // External event
#define ADC_TRIGGER_TIMER       0x00000005  // Timer event
#define ADC_TRIGGER_PWM0        0x00000006  // PWM0 event
#define ADC_TRIGGER_PWM1        0x00000007  // PWM1 event
#define ADC_TRIGGER_PWM2        0x00000008  // PWM2 event
#define ADC_TRIGGER_ALWAYS      0x0000000F  // Always event

//*****************************************************************************
//
// Values that can be passed to ADCSequenceStepConfigure as the ulConfig
// parameter.
//
//*****************************************************************************
#define ADC_CTL_TS              0x00000080  // Temperature sensor select
#define ADC_CTL_IE              0x00000040  // Interrupt enable
#define ADC_CTL_END             0x00000020  // Sequence end select
#define ADC_CTL_D               0x00000010  // Differential select
#define ADC_CTL_CH0             0x00000000  // Input channel 0
#define ADC_CTL_CH1             0x00000001  // Input channel 1
#define ADC_CTL_CH2             0x00000002  // Input channel 2
#define ADC_CTL_CH3             0x00000003  // Input channel 3
#define ADC_CTL_CH4             0x00000004  // Input channel 4
#define ADC_CTL_CH5             0x00000005  // Input channel 5
#define ADC_CTL_CH6             0x00000006  // Input channel 6
#define ADC_CTL_CH7             0x00000007  // Input channel 7
#define ADC_CTL_CH8             0x00000008  // Input channel 8
#define ADC_CTL_CH9             0x00000009  // Input channel 9
#define ADC_CTL_CH10            0x0000000A  // Input channel 10
#define ADC_CTL_CH11            0x0000000B  // Input channel 11
#define ADC_CTL_CH12            0x0000000C  // Input channel 12
#define ADC_CTL_CH13            0x0000000D  // Input channel 13
#define ADC_CTL_CH14            0x0000000E  // Input channel 14
#define ADC_CTL_CH15            0x0000000F  // Input channel 15
#define ADC_CTL_CMP0            0x00080000  // Select Comparator 0
#define ADC_CTL_CMP1            0x00090000  // Select Comparator 1
#define ADC_CTL_CMP2            0x000A0000  // Select Comparator 2
#define ADC_CTL_CMP3            0x000B0000  // Select Comparator 3
#define ADC_CTL_CMP4            0x000C0000  // Select Comparator 4
#define ADC_CTL_CMP5            0x000D0000  // Select Comparator 5
#define ADC_CTL_CMP6            0x000E0000  // Select Comparator 6
#define ADC_CTL_CMP7            0x000F0000  // Select Comparator 7

//*****************************************************************************
//
// Values that can be passed to ADCComparatorConfigure as part of the
// ulConfig parameter.
//
//*****************************************************************************
#define ADC_COMP_TRIG_NONE      0x00000000  // Trigger Disabled
#define ADC_COMP_TRIG_LOW_ALWAYS \
                                0x00001000  // Trigger Low Always
#define ADC_COMP_TRIG_LOW_ONCE  0x00001100  // Trigger Low Once
#define ADC_COMP_TRIG_LOW_HALWAYS \
                                0x00001200  // Trigger Low Always (Hysteresis)
#define ADC_COMP_TRIG_LOW_HONCE 0x00001300  // Trigger Low Once (Hysteresis)
#define ADC_COMP_TRIG_MID_ALWAYS \
                                0x00001400  // Trigger Mid Always
#define ADC_COMP_TRIG_MID_ONCE  0x00001500  // Trigger Mid Once
#define ADC_COMP_TRIG_HIGH_ALWAYS \
                                0x00001C00  // Trigger High Always
#define ADC_COMP_TRIG_HIGH_ONCE 0x00001D00  // Trigger High Once
#define ADC_COMP_TRIG_HIGH_HALWAYS \
                                0x00001E00  // Trigger High Always (Hysteresis)
#define ADC_COMP_TRIG_HIGH_HONCE \
                                0x00001F00  // Trigger High Once (Hysteresis)

#define ADC_COMP_INT_NONE       0x00000000  // Interrupt Disabled
#define ADC_COMP_INT_LOW_ALWAYS \
                                0x00000010  // Interrupt Low Always
#define ADC_COMP_INT_LOW_ONCE   0x00000011  // Interrupt Low Once
#define ADC_COMP_INT_LOW_HALWAYS \
                                0x00000012  // Interrupt Low Always
                                            // (Hysteresis)
#define ADC_COMP_INT_LOW_HONCE  0x00000013  // Interrupt Low Once (Hysteresis)
#define ADC_COMP_INT_MID_ALWAYS \
                                0x00000014  // Interrupt Mid Always
#define ADC_COMP_INT_MID_ONCE   0x00000015  // Interrupt Mid Once
#define ADC_COMP_INT_HIGH_ALWAYS \
                                0x0000001C  // Interrupt High Always
#define ADC_COMP_INT_HIGH_ONCE  0x0000001D  // Interrupt High Once
#define ADC_COMP_INT_HIGH_HALWAYS \
                                0x0000001E  // Interrupt High Always
                                            // (Hysteresis)
#define ADC_COMP_INT_HIGH_HONCE \
                                0x0000001F  // Interrupt High Once (Hysteresis)

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void ADCIntRegister(unsigned long ulBase, unsigned long ulSequenceNum,
                           void (*pfnHandler)(void));
extern void ADCIntUnregister(unsigned long ulBase,
                             unsigned long ulSequenceNum);
extern void ADCIntDisable(unsigned long ulBase, unsigned long ulSequenceNum);
extern void ADCIntEnable(unsigned long ulBase, unsigned long ulSequenceNum);
extern unsigned long ADCIntStatus(unsigned long ulBase,
                                  unsigned long ulSequenceNum,
                                  tBoolean bMasked);
extern void ADCIntClear(unsigned long ulBase, unsigned long ulSequenceNum);
extern void ADCSequenceEnable(unsigned long ulBase,
                              unsigned long ulSequenceNum);
extern void ADCSequenceDisable(unsigned long ulBase,
                               unsigned long ulSequenceNum);
extern void ADCSequenceConfigure(unsigned long ulBase,
                                 unsigned long ulSequenceNum,
                                 unsigned long ulTrigger,
                                 unsigned long ulPriority);
extern void ADCSequenceStepConfigure(unsigned long ulBase,
                                     unsigned long ulSequenceNum,
                                     unsigned long ulStep,
                                     unsigned long ulConfig);
extern long ADCSequenceOverflow(unsigned long ulBase,
                                unsigned long ulSequenceNum);
extern void ADCSequenceOverflowClear(unsigned long ulBase,
                                     unsigned long ulSequenceNum);
extern long ADCSequenceUnderflow(unsigned long ulBase,
                                 unsigned long ulSequenceNum);
extern void ADCSequenceUnderflowClear(unsigned long ulBase,
                                      unsigned long ulSequenceNum);
extern long ADCSequenceDataGet(unsigned long ulBase,
                               unsigned long ulSequenceNum,
                               unsigned long *pulBuffer);
extern void ADCProcessorTrigger(unsigned long ulBase,
                                unsigned long ulSequenceNum);
extern void ADCSoftwareOversampleConfigure(unsigned long ulBase,
                                           unsigned long ulSequenceNum,
                                           unsigned long ulFactor);
extern void ADCSoftwareOversampleStepConfigure(unsigned long ulBase,
                                               unsigned long ulSequenceNum,
                                               unsigned long ulStep,
                                               unsigned long ulConfig);
extern void ADCSoftwareOversampleDataGet(unsigned long ulBase,
                                         unsigned long ulSequenceNum,
                                         unsigned long *pulBuffer,
                                         unsigned long ulCount);
extern void ADCHardwareOversampleConfigure(unsigned long ulBase,
                                           unsigned long ulFactor);
extern void ADCComparatorConfigure(unsigned long ulBase, unsigned long ulComp,
                                   unsigned long ulConfig);
extern void ADCComparatorRegionSet(unsigned long ulBase, unsigned long ulComp,
                                   unsigned long ulLowRef,
                                   unsigned long ulHighRef);
extern void ADCComparatorReset(unsigned long ulBase, unsigned long ulComp,
                               tBoolean bTrigger, tBoolean bInterrupt);
extern void ADCComparatorIntDisable(unsigned long ulBase,
                                    unsigned long ulSequenceNum);
extern void ADCComparatorIntEnable(unsigned long ulBase,
                                   unsigned long ulSequenceNum);
extern unsigned long ADCComparatorIntStatus(unsigned long ulBase);
extern void ADCComparatorIntClear(unsigned long ulBase,
                                  unsigned long ulStatus);


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __ADC_H__
