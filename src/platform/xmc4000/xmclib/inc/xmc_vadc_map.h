/**
 * @file xmc_vadc_map.h
 * @date 2015-12-01
 *
 * @cond
*********************************************************************************************************************
 * XMClib v2.1.8 - XMC Peripheral Driver Library 
 *
 * Copyright (c) 2015-2016, Infineon Technologies AG
 * All rights reserved.                        
 *                                             
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the 
 * following conditions are met:   
 *                                                                              
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer.                        
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
 * disclaimer in the documentation and/or other materials provided with the distribution.                       
 * 
 * Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote 
 * products derived from this software without specific prior written permission.                                           
 *                                                                              
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE  
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR  
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                  
 *                                                                              
 * To improve the quality of the software, users are encouraged to share modifications, enhancements or bug fixes with 
 * Infineon Technologies AG dave@infineon.com).                                                          
 *********************************************************************************************************************
 *
 * Change History
 * --------------
 *
 * 2015-02-15:
 *     - Initial version
 *
 * 2015-12-01:
 *     - Added:
 *     - XMC4300 device supported
 *
 *     - Fixed: 
 *     - Wrong MACRO name corrected for XMC4200/4100 devices.
 *       XMC_VADC_G3_SAMPLE renamed to XMC_VADC_G1_SAMPLE
 * @endcond 
 *
 */

#ifndef XMC_ADC_MAP_H
#define XMC_ADC_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#if (UC_SERIES == XMC11)

/**********************************************************************************************************************
 * MACROS
 *********************************************************************************************************************/
/*  Group request source Gating input connection mappings */
#define XMC_CCU_40_ST3         XMC_VADC_REQ_GT_A
#define XMC_CCU_40_ST2         XMC_VADC_REQ_GT_B
#define XMC_CCU_40_ST1         XMC_VADC_REQ_GT_C
#define XMC_CCU_40_ST0         XMC_VADC_REQ_GT_D
#define XMC_ERU_0_PDOUT2       XMC_VADC_REQ_GT_K
#define XMC_ERU_0_PDOUT3       XMC_VADC_REQ_GT_L
#define XMC_ERU_0_PDOUT0       XMC_VADC_REQ_GT_O
#define XMC_ERU_0_PDOUT1       XMC_VADC_REQ_GT_P

/* Group request source Trigger input connection mappings */
#define XMC_CCU_40_SR2         XMC_VADC_REQ_TR_A
#define XMC_CCU_40_SR3         XMC_VADC_REQ_TR_B
#define XMC_ERU_0_IOUT2        XMC_VADC_REQ_TR_G
#define XMC_ERU_0_IOUT3        XMC_VADC_REQ_TR_H
#define XMC_ERU_0_IOUT0        XMC_VADC_REQ_TR_M
#define XMC_ERU_0_IOUT1        XMC_VADC_REQ_TR_N
#define XMC_REQ_GT_SEL         XMC_VADC_REQ_TR_P

#endif


#if (UC_SERIES == XMC12)

/*  Group request source Gating input connection mappings */
#define XMC_CCU_40_ST3          XMC_VADC_REQ_GT_A
#define XMC_CCU_40_ST2          XMC_VADC_REQ_GT_B
#define XMC_CCU_40_ST1          XMC_VADC_REQ_GT_C
#define XMC_CCU_40_ST0          XMC_VADC_REQ_GT_D
#define XMC_LEDTS_0_FN          XMC_VADC_REQ_GT_I
#define XMC_LEDTS_1_FN          XMC_VADC_REQ_GT_J
#define XMC_ERU_0_PDOUT2        XMC_VADC_REQ_GT_K
#define XMC_ERU_0_PDOUT3        XMC_VADC_REQ_GT_L
#define XMC_ERU_0_PDOUT0        XMC_VADC_REQ_GT_O
#define XMC_ERU_0_PDOUT1        XMC_VADC_REQ_GT_P

/* Group request source Trigger input connection mappings */
#define XMC_CCU_40_SR2          XMC_VADC_REQ_TR_A
#define XMC_CCU_40_SR3          XMC_VADC_REQ_TR_B
#define XMC_BCCU0_TRIGOUT       XMC_VADC_REQ_TR_F
#define XMC_ERU_0_IOUT2         XMC_VADC_REQ_TR_G
#define XMC_ERU_0_IOUT3         XMC_VADC_REQ_TR_H
#define XMC_ERU_0_IOUT0         XMC_VADC_REQ_TR_M
#define XMC_ERU_0_IOUT1         XMC_VADC_REQ_TR_N
#define XMC_REQ_GT_SEL          XMC_VADC_REQ_TR_P

#endif

#if (UC_SERIES == XMC13)

/*  Group request source Gating input connection mappings */
#define XMC_CCU_40_ST3          XMC_VADC_REQ_GT_A
#define XMC_CCU_40_ST2          XMC_VADC_REQ_GT_B
#define XMC_CCU_40_ST1          XMC_VADC_REQ_GT_C
#define XMC_CCU_40_ST0          XMC_VADC_REQ_GT_D
#define XMC_CCU_80_ST3_A        XMC_VADC_REQ_GT_E
#define XMC_CCU_81_ST3          XMC_VADC_REQ_GT_F
#define XMC_ERU_0_PDOUT2        XMC_VADC_REQ_GT_K
#define XMC_ERU_0_PDOUT3        XMC_VADC_REQ_GT_L
#define XMC_CCU_80_ST0          XMC_VADC_REQ_GT_M
#define XMC_CCU_80_ST1          XMC_VADC_REQ_GT_N
#define XMC_ERU_0_PDOUT0        XMC_VADC_REQ_GT_O
#define XMC_ERU_0_PDOUT1        XMC_VADC_REQ_GT_P

/* Group request source Trigger input connection mappings */
#define XMC_CCU_40_SR2          XMC_VADC_REQ_TR_A
#define XMC_CCU_40_SR3          XMC_VADC_REQ_TR_B
#define XMC_BCCU0_TRIGOUT       XMC_VADC_REQ_TR_F
#define XMC_ERU_0_IOUT2         XMC_VADC_REQ_TR_G
#define XMC_ERU_0_IOUT3         XMC_VADC_REQ_TR_H
#define XMC_CCU_80_SR2          XMC_VADC_REQ_TR_I
#define XMC_CCU_80_SR3          XMC_VADC_REQ_TR_J
#define XMC_ERU_0_IOUT0         XMC_VADC_REQ_TR_M
#define XMC_ERU_0_IOUT1         XMC_VADC_REQ_TR_N
#define XMC_POSIF_0_SR1         XMC_VADC_REQ_TR_O
#define XMC_REQ_GT_SEL          XMC_VADC_REQ_TR_P

#endif


#if (UC_SERIES == XMC14)

/*  Group request source Gating input connection mappings */
#define XMC_CCU_40_ST3          XMC_VADC_REQ_GT_A
#define XMC_CCU_40_ST2          XMC_VADC_REQ_GT_B
#define XMC_CCU_40_ST1          XMC_VADC_REQ_GT_C
#define XMC_CCU_40_ST0          XMC_VADC_REQ_GT_D
#define XMC_CCU_80_ST3_A        XMC_VADC_REQ_GT_E
#define XMC_CCU_81_ST3          XMC_VADC_REQ_GT_F
#define XMC_LEDTS0_FN           XMC_VADC_REQ_GT_I
#define XMC_LEDTS1_FN           XMC_VADC_REQ_GT_J
#define XMC_ERU_0_PDOUT2        XMC_VADC_REQ_GT_K
#define XMC_ERU_0_PDOUT3        XMC_VADC_REQ_GT_L
#define XMC_CCU_80_ST0          XMC_VADC_REQ_GT_M
#define XMC_CCU_80_ST1          XMC_VADC_REQ_GT_N
#define XMC_ERU_0_PDOUT0        XMC_VADC_REQ_GT_O
#define XMC_ERU_0_PDOUT1        XMC_VADC_REQ_GT_P

/* Group request source Trigger input connection mappings */
#define XMC_CCU_40_SR2          XMC_VADC_REQ_TR_A
#define XMC_CCU_40_SR3          XMC_VADC_REQ_TR_B
#define XMC_BCCU0_TRIGOUT       XMC_VADC_REQ_TR_F
#define XMC_ERU_0_IOUT2         XMC_VADC_REQ_TR_G
#define XMC_ERU_0_IOUT3         XMC_VADC_REQ_TR_H
#define XMC_CCU_80_SR2          XMC_VADC_REQ_TR_I
#define XMC_CCU_80_SR3          XMC_VADC_REQ_TR_J
#define XMC_ERU_0_IOUT0         XMC_VADC_REQ_TR_M
#define XMC_ERU_0_IOUT1         XMC_VADC_REQ_TR_N
#define XMC_POSIF_0_SR1         XMC_VADC_REQ_TR_O
#define XMC_REQ_GT_SEL          XMC_VADC_REQ_TR_P

#endif


#if ( (UC_SERIES == XMC42)||(UC_SERIES == XMC41) || (UC_SERIES == XMC43) )

/*  Group request source Gating input connection mappings */
#define XMC_CCU_40_ST3          XMC_VADC_REQ_GT_A
#define XMC_CCU_41_ST2          XMC_VADC_REQ_GT_B
#define XMC_CCU_40_SR0          XMC_VADC_REQ_GT_C
#define XMC_CCU_41_SR1          XMC_VADC_REQ_GT_D
#define XMC_CCU_80_ST3_A        XMC_VADC_REQ_GT_E
#define XMC_CCU_80_ST3_B        XMC_VADC_REQ_GT_F
#define XMC_DAC_0_SGN           XMC_VADC_REQ_GT_I
#define XMC_DAC_1_SGN           XMC_VADC_REQ_GT_I
#define XMC_LEDTS_FN            XMC_VADC_REQ_GT_J
#define XMC_VADC_G0_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G1_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G0_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_VADC_G1_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_CCU_80_SR0          XMC_VADC_REQ_GT_M
#define XMC_CCU_80_SR1          XMC_VADC_REQ_GT_N
#define XMC_ERU_1_PDOUT0        XMC_VADC_REQ_GT_O
#define XMC_ERU_1_PDOUT1        XMC_VADC_REQ_GT_P

/* Group request source Trigger input connection mappings */
#define XMC_CCU_40_SR2          XMC_VADC_REQ_TR_A
#define XMC_CCU_40_SR3          XMC_VADC_REQ_TR_B
#define XMC_CCU_41_SR2          XMC_VADC_REQ_TR_C
#define XMC_CCU_41_SR3          XMC_VADC_REQ_TR_D
#define XMC_CCU_80_SR2          XMC_VADC_REQ_TR_I
#define XMC_CCU_80_SR3          XMC_VADC_REQ_TR_J
#define XMC_ERU_1_IOUT0         XMC_VADC_REQ_TR_M
#define XMC_ERU_1_IOUT1         XMC_VADC_REQ_TR_N
#if (UC_SERIES == XMC43)
#define XMC_ERU_1_IOUT2         XMC_VADC_REQ_TR_N
#endif
#if (UC_SERIES != XMC43)
#define XMC_POSIF_0_SR1         XMC_VADC_REQ_TR_O
#endif
#define XMC_REQ_GT_SEL          XMC_VADC_REQ_TR_P

#endif

#if ( UC_SERIES == XMC44 ) || ( UC_SERIES == XMC48)

/*  Group request source Gating input connection mappings */
#define XMC_CCU_40_ST3          XMC_VADC_REQ_GT_A
#define XMC_CCU_41_ST2          XMC_VADC_REQ_GT_B
#define XMC_CCU_40_SR0          XMC_VADC_REQ_GT_C
#define XMC_CCU_41_SR1          XMC_VADC_REQ_GT_D
#define XMC_CCU_80_ST3_A        XMC_VADC_REQ_GT_E
#define XMC_CCU_80_ST3_B        XMC_VADC_REQ_GT_F
#define XMC_CCU_81_ST3_A        XMC_VADC_REQ_GT_G
#define XMC_CCU_81_ST3_B        XMC_VADC_REQ_GT_H
#define XMC_DAC_0_SGN           XMC_VADC_REQ_GT_I
#define XMC_DAC_1_SGN           XMC_VADC_REQ_GT_I
#define XMC_LEDTS_FN            XMC_VADC_REQ_GT_J
#define XMC_VADC_G0_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G1_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G2_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G3_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G0_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_VADC_G1_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_VADC_G2_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_VADC_G3_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_CCU_80_SR0          XMC_VADC_REQ_GT_M
#define XMC_CCU_80_SR1          XMC_VADC_REQ_GT_N
#define XMC_ERU_1_PDOUT0        XMC_VADC_REQ_GT_O
#define XMC_ERU_1_PDOUT1        XMC_VADC_REQ_GT_P

/* Group request source Trigger input connection mappings */
#define XMC_CCU_40_SR2          XMC_VADC_REQ_TR_A
#define XMC_CCU_40_SR3          XMC_VADC_REQ_TR_B
#define XMC_CCU_41_SR2          XMC_VADC_REQ_TR_C
#define XMC_CCU_41_SR3          XMC_VADC_REQ_TR_D
#define XMC_CCU_42_SR3          XMC_VADC_REQ_TR_E
#define XMC_CCU_43_SR3          XMC_VADC_REQ_TR_F
#define XMC_CCU_80_SR2          XMC_VADC_REQ_TR_I
#define XMC_CCU_80_SR3          XMC_VADC_REQ_TR_J
#define XMC_CCU_81_SR2          XMC_VADC_REQ_TR_K
#define XMC_CCU_81_SR3          XMC_VADC_REQ_TR_L
#define XMC_ERU_1_IOUT0         XMC_VADC_REQ_TR_M
#define XMC_ERU_1_IOUT1         XMC_VADC_REQ_TR_N
#define XMC_ERU_1_IOUT2         XMC_VADC_REQ_TR_N
#define XMC_POSIF_0_SR1         XMC_VADC_REQ_TR_O
#define XMC_POSIF_1_SR1         XMC_VADC_REQ_TR_O
#define XMC_REQ_GT_SEL          XMC_VADC_REQ_TR_P

#endif

#if ( UC_SERIES == XMC45 )

/*  Group request source Gating input connection mappings */
#define XMC_CCU_40_ST3          XMC_VADC_REQ_GT_A
#define XMC_CCU_41_ST2          XMC_VADC_REQ_GT_B
#define XMC_CCU_40_SR0          XMC_VADC_REQ_GT_C
#define XMC_CCU_41_SR1          XMC_VADC_REQ_GT_D
#define XMC_CCU_80_ST3_A        XMC_VADC_REQ_GT_E
#define XMC_CCU_80_ST3_B        XMC_VADC_REQ_GT_F
#define XMC_CCU_81_ST3_A        XMC_VADC_REQ_GT_G
#define XMC_CCU_81_ST3_B        XMC_VADC_REQ_GT_H
#define XMC_DAC_0_SGN           XMC_VADC_REQ_GT_I
#define XMC_DAC_1_SGN           XMC_VADC_REQ_GT_I
#define XMC_LEDTS_FN            XMC_VADC_REQ_GT_J
#define XMC_VADC_G0_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G1_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G2_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G3_BLOUT0      XMC_VADC_REQ_GT_K
#define XMC_VADC_G0_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_VADC_G1_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_VADC_G2_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_VADC_G3_SAMPLE      XMC_VADC_REQ_GT_L
#define XMC_CCU_80_SR0          XMC_VADC_REQ_GT_M
#define XMC_CCU_80_SR1          XMC_VADC_REQ_GT_N
#define XMC_ERU_1_PDOUT0        XMC_VADC_REQ_GT_O
#define XMC_ERU_1_PDOUT1        XMC_VADC_REQ_GT_P

/* Group request source Trigger input connection mappings */
#define XMC_CCU_40_SR2          XMC_VADC_REQ_TR_A
#define XMC_CCU_40_SR3          XMC_VADC_REQ_TR_B
#define XMC_CCU_41_SR2          XMC_VADC_REQ_TR_C
#define XMC_CCU_41_SR3          XMC_VADC_REQ_TR_D
#define XMC_CCU_42_SR3          XMC_VADC_REQ_TR_E
#define XMC_CCU_43_SR3          XMC_VADC_REQ_TR_F
#define XMC_CCU_80_SR2          XMC_VADC_REQ_TR_I
#define XMC_CCU_80_SR3          XMC_VADC_REQ_TR_J
#define XMC_CCU_81_SR2          XMC_VADC_REQ_TR_K
#define XMC_CCU_81_SR3          XMC_VADC_REQ_TR_L
#define XMC_ERU_1_IOUT0         XMC_VADC_REQ_TR_M
#define XMC_ERU_1_IOUT1         XMC_VADC_REQ_TR_N
#define XMC_ERU_1_IOUT2         XMC_VADC_REQ_TR_N
#define XMC_POSIF_0_SR1         XMC_VADC_REQ_TR_O
#define XMC_POSIF_1_SR1         XMC_VADC_REQ_TR_O
#define XMC_REQ_GT_SEL          XMC_VADC_REQ_TR_P

#endif

#ifdef __cplusplus
}
#endif

#endif
