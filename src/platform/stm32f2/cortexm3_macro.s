/*;******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
;* File Name          : cortexm3_macro.s
;* Author             : MCD Application Team
;* Version            : V2.0.3
;* Date               : 09/22/2008
;* Description        : Instruction wrappers for special Cortex-M3 instructions.
;*                      to be used with RIDE7 toolchain.
;*******************************************************************************
; THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
; WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
; AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
; INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
; CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
; INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
;******************************************************************************/
 .cpu cortex-m3
.fpu softvfp   
.syntax unified
.thumb
.text	

  /*; Exported functions*/	    
  .globl __WFI
  .globl __WFE
  .globl __SEV
  .globl __ISB 
  .globl __DSB
  .globl __DMB
  .globl __SVC
  .globl __MRS_CONTROL
  .globl __MSR_CONTROL
  .globl __MRS_PSP
  .globl __MSR_PSP
  .globl __MRS_MSP
  .globl __MSR_MSP   
  .globl __RESETPRIMASK   
  .globl __SETPRIMASK
  .globl __READ_PRIMASK  
  .globl __RESETFAULTMASK
  .globl __SETFAULTMASK
  .globl __READ_FAULTMASK
  .globl __BASEPRICONFIG
  .globl __GetBASEPRI
  .globl __REV_HalfWord
  .globl __REV_Word  

/*;*****************************************************************************
; Function Name  : __WFI
; Description    : Assembler function for the WFI instruction.
; Input          : None
; Return         : None
;******************************************************************************/
.thumb_func
__WFI: 
 
    WFI
    BX r14

/*;*****************************************************************************
; Function Name  : __WFE
; Description    : Assembler function for the WFE instruction.
; Input          : None
; Return         : None
;******************************************************************************/
.thumb_func
__WFE:

    WFE
    BX r14

/*;*****************************************************************************
; Function Name  : __SEV
; Description    : Assembler function for the SEV instruction.
; Input          : None
; Return         : None
;******************************************************************************/
.thumb_func
__SEV:

    SEV
    BX r14

/*;*****************************************************************************
; Function Name  : __ISB
; Description    : Assembler function for the ISB instruction.
; Input          : None
; Return         : None
;******************************************************************************/
.thumb_func
__ISB:

    ISB
    BX r14

/*;*****************************************************************************
; Function Name  : __DSB
; Description    : Assembler function for the DSB instruction.
; Input          : None
; Return         : None
;******************************************************************************/
.thumb_func
__DSB:

    DSB
    BX r14

/*;*****************************************************************************
; Function Name  : __DMB
; Description    : Assembler function for the DMB instruction.
; Input          : None
; Return         : None
;******************************************************************************/
.thumb_func
__DMB:

    DMB
    BX r14

/*;*****************************************************************************
; Function Name  : __SVC
; Description    : Assembler function for the SVC instruction.
; Input          : None
; Return         : None
;******************************************************************************/
.thumb_func
__SVC:

    SVC 0x01
    BX r14

/*;*****************************************************************************
; Function Name  : __MRS_CONTROL
; Description    : Assembler function for the MRS instruction.
; Input          : None
; Return         : - r4 : Cortex-M3 CONTROL register value.
;******************************************************************************/
.thumb_func
__MRS_CONTROL:

  MRS  r0,control
  BX r14

/*;*****************************************************************************
; Function Name  : __MSR_CONTROL
; Description    : Assembler function for the MSR instruction.
; Input          : - R0 : Cortex-M3 CONTROL register new value.  
; Return         : None
;******************************************************************************/
.thumb_func
__MSR_CONTROL:

  MSR control, r0
  ISB
  BX r14
/*;*****************************************************************************
; Function Name  : __MRS_PSP
; Description    : Assembler function for the MRS instruction.
; Input          : None
; Return         : - r0 : Process Stack value.
;******************************************************************************/
.thumb_func
__MRS_PSP:

  MRS r0, psp
  BX r14

/*;*****************************************************************************
; Function Name  : __MSR_PSP
; Description    : Assembler function for the MSR instruction.
; Input          : - r0 : Process Stack new value.  
; Return         : None
;******************************************************************************/
.thumb_func
__MSR_PSP:
 
    MSR psp,  r0      /* set Process Stack value*/
    BX r14

/*;*****************************************************************************
; Function Name  : __MRS_MSP
; Description    : Assembler function for the MRS instruction.
; Input          : None
; Return         : - r0 : Main Stack value.
;******************************************************************************/
.thumb_func
__MRS_MSP:

  MRS r0, msp
  BX r14

/*;*****************************************************************************
; Function Name  : __MSR_MSP
; Description    : Assembler function for the MSR instruction.
; Input          : - r0 : Main Stack new value.  
; Return         : None
;******************************************************************************/
.thumb_func
__MSR_MSP: 
 
    MSR msp, r0  /*; set Main Stack value*/
    BX r14

/*;*****************************************************************************
; Function Name  : __RESETPRIMASK
; Description    : Assembler function to reset the PRIMASK.
; Input          : None 
; Return         : None
;******************************************************************************/
.thumb_func
__RESETPRIMASK:

  CPSIE i
  BX r14

/*;*****************************************************************************
; Function Name  : __SETPRIMASK
; Description    : Assembler function to set the PRIMASK.
; Input          : None 
; Return         : None
;******************************************************************************/
.thumb_func
__SETPRIMASK:

  CPSID i
  BX r14
  
/*;*****************************************************************************
; Function Name  : __READ_PRIMASK
; Description    :Assembler function to get the PRIMASK value.
; Input          : None 
; Return         : - r0 : PRIMASK register value 
;******************************************************************************/
.thumb_func
__READ_PRIMASK:

  MRS r0, PRIMASK
  BX r14
    
/*;*****************************************************************************
; Function Name  : __RESETFAULTMASK
; Description    : Assembler function to reset the FAULTMASK.
; Input          : None 
; Return         : None
;******************************************************************************/
.thumb_func
__RESETFAULTMASK:

  CPSIE f
  BX r14
  
/*;*****************************************************************************
; Function Name  : __SETFAULTMASK
; Description    : Assembler function to set the FAULTMASK.
; Input          : None 
; Return         : None
;******************************************************************************/
.thumb_func
__SETFAULTMASK:

  CPSID f
  BX r14

/*;*****************************************************************************
; Function Name  : __READ_FAULTMASK
; Description    : Assembler function to get the FAULTMASK value.
; Input          : None 
; Return         : - r0 : FAULTMASK register value 
;******************************************************************************/
.thumb_func
__READ_FAULTMASK:

  MRS r0, FAULTMASK
  BX r14
  
/*;*****************************************************************************
; Function Name  : __BASEPRICONFIG
; Description    : Assembler function to set the Base Priority.
; Input          : - r0 : Base Priority new value  
; Return         : None
;******************************************************************************/
.thumb_func
__BASEPRICONFIG:

  MSR basepri, r0
  BX r14

/*;*****************************************************************************
; Function Name  : __GetBASEPRI
; Description    : Assembler function to get the Base Priority value.
; Input          : None 
; Return         : - r0 : Base Priority value 
;******************************************************************************/
.thumb_func
__GetBASEPRI:

  MRS r0, basepri_max
  BX r14
/*;*****************************************************************************
; Function Name  : __REV_HalfWord
; Description    : Reverses the byte order in HalfWord(16-bit) input variable.
; Input          : - r0 : specifies the input variable
; Return         : - r0 : holds tve variable value after byte reversing.
;******************************************************************************/
.thumb_func
__REV_HalfWord: 
 
  REV16 r0, r0
  BX r14

/*;*****************************************************************************
; Function Name  : __REV_Word
; Description    : Reverses the byte order in Word(32-bit) input variable.
; Input          : - r0 : specifies the input variable
; Return         : - r0 : holds tve variable value after byte reversing.
;******************************************************************************/
.thumb_func
__REV_Word: 
 
  REV r0, r0
  BX r14
  
.end  

  
/*;******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
