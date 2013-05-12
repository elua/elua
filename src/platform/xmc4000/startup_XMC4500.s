/*****************************************************************************/
/* Startup_XMC4500.s: Startup file for XMC4500 device series                 */
/*****************************************************************************/

/* ********************* Version History *********************************** */
/* ***************************************************************************
V1.0 , July 2011, First version for XIP profile
V1.1 , Oct  2011, Program loading code included (GH: b to main changed)
V1.2 , Nov, 01, 2011 GH :Removed second definition of section .Xmc4500.reset
                         at line 186. 
V1.3 , Nov, 16, 2011 GH :Removed PMU0_1_IRQHandler and respective weak function
                         declaration.
V1.4 , Dec, 16, 2011 PKB:Jump to __Xmc4500_start_c reinstated for RTOS integration
V1.5 , Jan, 10, 2012 PKB:Migrated to GCC from ARM
V1.6 , Jan, 16, 2012 PKB:Branch prediction turned off, Parity errors cleared.
V1.7 , Apr, 17, 2012 PKB:Added decision function for PLL initialization  
V1.8 , Apr, 20, 2012 PKB:Handshake with DAVE code engine added  
**************************************************************************** */
/**
* @file     Startup_XMC4500.s
*           XMC4000 Device Series
* @version  V1.8
* @date     Apr 2012
*
Copyright (C) 2012 Infineon Technologies AG. All rights reserved.
*
*
* @par
* Infineon Technologies AG (Infineon) is supplying this software for use with 
* Infineon's microcontrollers.  This file can be freely distributed
* within development tools that are supporting such microcontrollers.
*
* @par
* THIS SOFTWARE IS PROVIDED AS IS.  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
* ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
******************************************************************************/

/* ================== START OF VECTOR TABLE DEFINITION ====================== */
/* Vector Table - This gets programed into VTOR register by onchip BootROM */
    .syntax unified

    .section ".Xmc4500.reset"
    .globl  __Xmc4500_interrupt_vector_cortex_m
    .type   __Xmc4500_interrupt_vector_cortex_m, %object

__Xmc4500_interrupt_vector_cortex_m:
    .long   __Xmc4500_stack             /* Top of Stack                 */
    .long   __Xmc4500_reset_cortex_m    /* Reset Handler                */
    .long   NMI_Handler                 /* NMI Handler                  */
    .long   HardFault_Handler           /* Hard Fault Handler           */
    .long   MemManage_Handler           /* MPU Fault Handler            */
    .long   BusFault_Handler            /* Bus Fault Handler            */
    .long   UsageFault_Handler          /* Usage Fault Handler          */
    .long   0                           /* Reserved                     */
    .long   0                           /* Reserved                     */
    .long   0                           /* Reserved                     */
    .long   0                           /* Reserved                     */
    .long   SVC_Handler                 /* SVCall Handler               */
    .long   DebugMon_Handler            /* Debug Monitor Handler        */
    .long   0                           /* Reserved                     */
    .long   PendSV_Handler              /* PendSV Handler               */
    .long   SysTick_Handler             /* SysTick Handler              */

    /* Interrupt Handlers for Service Requests (SR) from XMC4500 Peripherals */
    .long   SCU_0_IRQHandler            /* Handler name for SR SCU_0     */
    .long   ERU0_0_IRQHandler           /* Handler name for SR ERU0_0    */
    .long   ERU0_1_IRQHandler           /* Handler name for SR ERU0_1    */
    .long   ERU0_2_IRQHandler           /* Handler name for SR ERU0_2    */
    .long   ERU0_3_IRQHandler           /* Handler name for SR ERU0_3    */ 
    .long   ERU1_0_IRQHandler           /* Handler name for SR ERU1_0    */
    .long   ERU1_1_IRQHandler           /* Handler name for SR ERU1_1    */
    .long   ERU1_2_IRQHandler           /* Handler name for SR ERU1_2    */
    .long   ERU1_3_IRQHandler           /* Handler name for SR ERU1_3    */
    .long   0                           /* Not Available                 */
    .long   0                           /* Not Available                 */
    .long   0                           /* Not Available                 */
    .long   PMU0_0_IRQHandler           /* Handler name for SR PMU0_0    */
    .long   0                           /* Not Available                 */
    .long   VADC0_C0_0_IRQHandler       /* Handler name for SR VADC0_C0_0  */
    .long   VADC0_C0_1_IRQHandler       /* Handler name for SR VADC0_C0_1  */
    .long   VADC0_C0_2_IRQHandler       /* Handler name for SR VADC0_C0_1  */
    .long   VADC0_C0_3_IRQHandler       /* Handler name for SR VADC0_C0_3  */
    .long   VADC0_G0_0_IRQHandler       /* Handler name for SR VADC0_G0_0  */
    .long   VADC0_G0_1_IRQHandler       /* Handler name for SR VADC0_G0_1  */
    .long   VADC0_G0_2_IRQHandler       /* Handler name for SR VADC0_G0_2  */
    .long   VADC0_G0_3_IRQHandler       /* Handler name for SR VADC0_G0_3  */
    .long   VADC0_G1_0_IRQHandler       /* Handler name for SR VADC0_G1_0  */
    .long   VADC0_G1_1_IRQHandler       /* Handler name for SR VADC0_G1_1  */
    .long   VADC0_G1_2_IRQHandler       /* Handler name for SR VADC0_G1_2  */
    .long   VADC0_G1_3_IRQHandler       /* Handler name for SR VADC0_G1_3  */
    .long   VADC0_G2_0_IRQHandler       /* Handler name for SR VADC0_G2_0  */
    .long   VADC0_G2_1_IRQHandler       /* Handler name for SR VADC0_G2_1  */
    .long   VADC0_G2_2_IRQHandler       /* Handler name for SR VADC0_G2_2  */
    .long   VADC0_G2_3_IRQHandler       /* Handler name for SR VADC0_G2_3  */
    .long   VADC0_G3_0_IRQHandler       /* Handler name for SR VADC0_G3_0  */
    .long   VADC0_G3_1_IRQHandler       /* Handler name for SR VADC0_G3_1  */
    .long   VADC0_G3_2_IRQHandler       /* Handler name for SR VADC0_G3_2  */
    .long   VADC0_G3_3_IRQHandler       /* Handler name for SR VADC0_G3_3  */
    .long   DSD0_0_IRQHandler           /* Handler name for SR DSD0_0    */
    .long   DSD0_1_IRQHandler           /* Handler name for SR DSD0_1    */
    .long   DSD0_2_IRQHandler           /* Handler name for SR DSD0_2    */
    .long   DSD0_3_IRQHandler           /* Handler name for SR DSD0_3    */
    .long   DSD0_4_IRQHandler           /* Handler name for SR DSD0_4    */
    .long   DSD0_5_IRQHandler           /* Handler name for SR DSD0_5    */
    .long   DSD0_6_IRQHandler           /* Handler name for SR DSD0_6    */
    .long   DSD0_7_IRQHandler           /* Handler name for SR DSD0_7    */
    .long   DAC0_0_IRQHandler           /* Handler name for SR DAC0_0    */
    .long   DAC0_1_IRQHandler           /* Handler name for SR DAC0_0    */
    .long   CCU40_0_IRQHandler          /* Handler name for SR CCU40_0   */
    .long   CCU40_1_IRQHandler          /* Handler name for SR CCU40_1   */
    .long   CCU40_2_IRQHandler          /* Handler name for SR CCU40_2   */
    .long   CCU40_3_IRQHandler          /* Handler name for SR CCU40_3   */
    .long   CCU41_0_IRQHandler          /* Handler name for SR CCU41_0   */
    .long   CCU41_1_IRQHandler          /* Handler name for SR CCU41_1   */
    .long   CCU41_2_IRQHandler          /* Handler name for SR CCU41_2   */
    .long   CCU41_3_IRQHandler          /* Handler name for SR CCU41_3   */
    .long   CCU42_0_IRQHandler          /* Handler name for SR CCU42_0   */
    .long   CCU42_1_IRQHandler          /* Handler name for SR CCU42_1   */
    .long   CCU42_2_IRQHandler          /* Handler name for SR CCU42_2   */
    .long   CCU42_3_IRQHandler          /* Handler name for SR CCU42_3   */
    .long   CCU43_0_IRQHandler          /* Handler name for SR CCU43_0   */
    .long   CCU43_1_IRQHandler          /* Handler name for SR CCU43_1   */
    .long   CCU43_2_IRQHandler          /* Handler name for SR CCU43_2   */
    .long   CCU43_3_IRQHandler          /* Handler name for SR CCU43_3   */
    .long   CCU80_0_IRQHandler          /* Handler name for SR CCU80_0   */
    .long   CCU80_1_IRQHandler          /* Handler name for SR CCU80_1   */
    .long   CCU80_2_IRQHandler          /* Handler name for SR CCU80_2   */
    .long   CCU80_3_IRQHandler          /* Handler name for SR CCU80_3   */
    .long   CCU81_0_IRQHandler          /* Handler name for SR CCU81_0   */
    .long   CCU81_1_IRQHandler          /* Handler name for SR CCU81_1   */
    .long   CCU81_2_IRQHandler          /* Handler name for SR CCU81_2   */
    .long   CCU81_3_IRQHandler          /* Handler name for SR CCU81_3   */
    .long   POSIF0_0_IRQHandler         /* Handler name for SR POSIF0_0  */
    .long   POSIF0_1_IRQHandler         /* Handler name for SR POSIF0_1  */
    .long   POSIF1_0_IRQHandler         /* Handler name for SR POSIF1_0  */
    .long   POSIF1_1_IRQHandler         /* Handler name for SR POSIF1_1  */
    .long   0                           /* Not Available                 */
    .long   0                           /* Not Available                 */
    .long   0                           /* Not Available                 */
    .long   0                           /* Not Available                 */
    .long   CAN0_0_IRQHandler           /* Handler name for SR CAN0_0    */
    .long   CAN0_1_IRQHandler           /* Handler name for SR CAN0_1    */
    .long   CAN0_2_IRQHandler           /* Handler name for SR CAN0_2    */
    .long   CAN0_3_IRQHandler           /* Handler name for SR CAN0_3    */
    .long   CAN0_4_IRQHandler           /* Handler name for SR CAN0_4    */
    .long   CAN0_5_IRQHandler           /* Handler name for SR CAN0_5    */
    .long   CAN0_6_IRQHandler           /* Handler name for SR CAN0_6    */
    .long   CAN0_7_IRQHandler           /* Handler name for SR CAN0_7    */
    .long   USIC0_0_IRQHandler          /* Handler name for SR USIC0_0   */
    .long   USIC0_1_IRQHandler          /* Handler name for SR USIC0_1   */
    .long   USIC0_2_IRQHandler          /* Handler name for SR USIC0_2   */
    .long   USIC0_3_IRQHandler          /* Handler name for SR USIC0_3   */
    .long   USIC0_4_IRQHandler          /* Handler name for SR USIC0_4   */
    .long   USIC0_5_IRQHandler          /* Handler name for SR USIC0_5   */
    .long   USIC1_0_IRQHandler          /* Handler name for SR USIC1_0   */
    .long   USIC1_1_IRQHandler          /* Handler name for SR USIC1_1   */
    .long   USIC1_2_IRQHandler          /* Handler name for SR USIC1_2   */
    .long   USIC1_3_IRQHandler          /* Handler name for SR USIC1_3   */
    .long   USIC1_4_IRQHandler          /* Handler name for SR USIC1_4   */
    .long   USIC1_5_IRQHandler          /* Handler name for SR USIC1_5   */
    .long   USIC2_0_IRQHandler          /* Handler name for SR USIC2_0   */
    .long   USIC2_1_IRQHandler          /* Handler name for SR USIC2_1   */
    .long   USIC2_2_IRQHandler          /* Handler name for SR USIC2_2   */
    .long   USIC2_3_IRQHandler          /* Handler name for SR USIC2_3   */
    .long   USIC2_4_IRQHandler          /* Handler name for SR USIC2_4   */
    .long   USIC2_5_IRQHandler          /* Handler name for SR USIC2_5   */
    .long   LEDTS0_0_IRQHandler         /* Handler name for SR LEDTS0_0  */
    .long   0                           /* Not Available                 */
    .long   FCE0_0_IRQHandler           /* Handler name for SR FCE0_0    */
    .long   GPDMA0_0_IRQHandler         /* Handler name for SR GPDMA0_0  */
    .long   SDMMC0_0_IRQHandler         /* Handler name for SR SDMMC0_0  */
    .long   USB0_0_IRQHandler           /* Handler name for SR USB0_0    */
    .long   ETH0_0_IRQHandler           /* Handler name for SR ETH0_0    */
    .long   0                           /* Not Available                 */
    .long   GPDMA1_0_IRQHandler         /* Handler name for SR GPDMA1_0  */
    .long   0                           /* Not Available                 */


    .size  __Xmc4500_interrupt_vector_cortex_m, . - __Xmc4500_interrupt_vector_cortex_m
/* ================== END OF VECTOR TABLE DEFINITION ======================= */

/* ================== START OF VECTOR ROUTINES ============================= */
    .thumb
/* ======================================================================== */
/* Reset Handler */

    .thumb_func
    .globl  __Xmc4500_reset_cortex_m
    .type   __Xmc4500_reset_cortex_m, %function
__Xmc4500_reset_cortex_m:
    .fnstart

    /* Disable Branch prediction */
    LDR R0,=PREF_PCON
    LDR R1,[R0]
    ORR R1,R1,#0x00010000
    STR R1,[R0]
    
    /* Clear existing parity errors if any */
    LDR R0,=SCU_GCU_PEFLAG
    LDR R1,=0xFFFFFFFF
    STR R1,[R0]

    /* Disable parity */
    LDR R0,=SCU_GCU_PEEN
    MOV R1,#0
    STR R1,[R0]

    /* C routines are likely to be called. Setup the stack now */
    /* This is already setup by BootROM,hence this step is optional */ 
    LDR SP,=__Xmc4500_stack

    /* Clock tree is not initialized. Set the status now  */    
    LDR R1, =SetInitStatus
    MOV R0,#0
    BLX R1

    /* Clock tree, External memory setup etc may be done here */    
    LDR     R0, =SystemInit
    BLX     R0

/* 
   SystemInit_DAVE3() is provided by DAVE3 code generation engine. It is  
   weakly defined here though for a potential override.
*/
    LDR     R0, =SystemInit_DAVE3 	
    BLX     R0

    B       __Xmc4500_Program_Loader 
    
    .pool
    .cantunwind
    .fnend
    .size   __Xmc4500_reset_cortex_m,.-__Xmc4500_reset_cortex_m
/* ======================================================================== */
/* __Xmc4500_reset must yield control to __Xmc4500_Program_Loader before control
   to C land is given */
   .section .Xmc4500.postreset,"x",%progbits
   __Xmc4500_Program_Loader:
   .fnstart
   /* Memories are accessible now*/
   
   /* DATA COPY */
   /* R0 = Start address, R1 = Destination address, R2 = Size */
   LDR R0, =eROData
   LDR R1, =__Xmc4500_sData
   LDR R2, =__Xmc4500_Data_Size

   /* Is there anything to be copied? */
   CMP R2,#0
   BEQ SKIPCOPY
   
   /* For bytecount less than 4, at least 1 word must be copied */
   CMP R2,#4
   BCS STARTCOPY
   
   /* Byte count < 4 ; so bump it up */
   MOV R2,#4

STARTCOPY:
   /* 
      R2 contains byte count. Change it to word count. It is ensured in the 
      linker script that the length is always word aligned.
   */
   LSR R2,R2,#2 /* Divide by 4 to obtain word count */

   /* The proverbial loop from the schooldays */
COPYLOOP:
   LDR R3,[R0]
   STR R3,[R1]
   SUBS R2,#1
   BEQ SKIPCOPY
   ADD R0,#4
   ADD R1,#4
   B COPYLOOP
    
SKIPCOPY:
   /* BSS CLEAR */
   LDR R0, =__Xmc4500_sBSS     /* Start of BSS */
   LDR R1, =__Xmc4500_BSS_Size /* BSS size in bytes */

   /* Find out if there are items assigned to BSS */   
   CMP R1,#0 
   BEQ SKIPCLEAR

   /* At least 1 word must be copied */
   CMP R1,#4
   BCS STARTCLEAR
   
   /* Byte count < 4 ; so bump it up to a word*/
   MOV R1,#4

STARTCLEAR:
   LSR R1,R1,#2            /* BSS size in words */
   
   MOV R2,#0
CLEARLOOP:
   STR R2,[R0]
   SUBS R1,#1
   BEQ SKIPCLEAR
   ADD R0,#4
   B CLEARLOOP
    
SKIPCLEAR:
   /* Remap vector table */
   /* This is already setup by BootROM,hence this step is optional */ 
   LDR R0, =__Xmc4500_interrupt_vector_cortex_m 
   LDR R1, =SCB_VTOR
   STR R0,[R1]
   
   /* Update System Clock */
   LDR R0,=SystemCoreClockUpdate
   BLX R0

   /* Clock tree is initialized. Set the status now  */    
   LDR R1, =SetInitStatus
   MOV R0,#1
   BLX R1

   /* Reset stack pointer before zipping off to user application, Optional */
   LDR SP,=__Xmc4500_stack 
   MOV R0,#0
   MOV R1,#0
   LDR PC, =Platform_Main	
   .pool
   .cantunwind
   .fnend
   .size   __Xmc4500_Program_Loader,.-__Xmc4500_Program_Loader
/* ======================================================================== */
/* ========== START OF EXCEPTION HANDLER DEFINITION ======================== */

/* Default exception Handlers - Users may override this default functionality by
   defining handlers of the same name in their C code */
    .thumb
    .text

    .weak   NMI_Handler
    .type   NMI_Handler, %function
NMI_Handler:
    B       .
    .size   NMI_Handler, . - NMI_Handler
/* ======================================================================== */
    .weak   HardFault_Handler
    .type   HardFault_Handler, %function
HardFault_Handler:
    B       .		
    .size   HardFault_Handler, . - HardFault_Handler
/* ======================================================================== */
    .weak   MemManage_Handler
    .type   MemManage_Handler, %function
MemManage_Handler:
    B       .
    .size   MemManage_Handler, . - MemManage_Handler
/* ======================================================================== */
    .weak   BusFault_Handler
    .type   BusFault_Handler, %function
BusFault_Handler:
    B       .
    .size   BusFault_Handler, . - BusFault_Handler
/* ======================================================================== */
    .weak   UsageFault_Handler
    .type   UsageFault_Handler, %function
UsageFault_Handler:
    B       .
    .size   UsageFault_Handler, . - UsageFault_Handler
/* ======================================================================== */
    .weak   SVC_Handler
    .type   SVC_Handler, %function
SVC_Handler:
    B       .
    .size   SVC_Handler, . - SVC_Handler
/* ======================================================================== */
    .weak   DebugMon_Handler
    .type   DebugMon_Handler, %function
DebugMon_Handler:
    B       .
    .size   DebugMon_Handler, . - DebugMon_Handler
/* ======================================================================== */
    .weak   PendSV_Handler
    .type   PendSV_Handler, %function
PendSV_Handler:
    B       .
    .size   PendSV_Handler, . - PendSV_Handler
/* ======================================================================== */
    .weak   SysTick_Handler
    .type   SysTick_Handler, %function
SysTick_Handler:
    B       .
    .size   SysTick_Handler, . - SysTick_Handler

/* ============= END OF EXCEPTION HANDLER DEFINITION ======================== */

/* ============= START OF INTERRUPT HANDLER DEFINITION ====================== */

/* IRQ Handlers */
    .weak   SCU_0_IRQHandler
    .type   SCU_0_IRQHandler, %function
SCU_0_IRQHandler:
    B       .
    .size   SCU_0_IRQHandler, . - SCU_0_IRQHandler
/* ======================================================================== */
    .weak   ERU0_0_IRQHandler
    .type   ERU0_0_IRQHandler, %function
ERU0_0_IRQHandler:
    B       .
    .size   ERU0_0_IRQHandler, . - ERU0_0_IRQHandler
/* ======================================================================== */
    .weak   ERU0_1_IRQHandler
    .type   ERU0_1_IRQHandler, %function
ERU0_1_IRQHandler:
    B       .
    .size   ERU0_1_IRQHandler, . - ERU0_1_IRQHandler
/* ======================================================================== */
    .weak   ERU0_2_IRQHandler
    .type   ERU0_2_IRQHandler, %function
ERU0_2_IRQHandler:
    B       .
    .size   ERU0_2_IRQHandler, . - ERU0_2_IRQHandler
/* ======================================================================== */
    .weak   ERU0_3_IRQHandler
    .type   ERU0_3_IRQHandler, %function
ERU0_3_IRQHandler:
    B       .
    .size   ERU0_3_IRQHandler, . - ERU0_3_IRQHandler
/* ======================================================================== */
    .weak   ERU1_0_IRQHandler
    .type   ERU1_0_IRQHandler, %function
ERU1_0_IRQHandler:
    B       .
    .size   ERU1_0_IRQHandler, . - ERU1_0_IRQHandler

    .weak   ERU1_1_IRQHandler
    .type   ERU1_1_IRQHandler, %function
ERU1_1_IRQHandler:
    B       .
    .size   ERU1_1_IRQHandler, . - ERU1_1_IRQHandler

    .weak   ERU1_2_IRQHandler 
    .type   ERU1_2_IRQHandler , %function
ERU1_2_IRQHandler:
    B       .
    .size   ERU1_2_IRQHandler , . - ERU1_2_IRQHandler 

    .weak   ERU1_3_IRQHandler 
    .type   ERU1_3_IRQHandler , %function
ERU1_3_IRQHandler:
    B       .
    .size   ERU1_3_IRQHandler , . - ERU1_3_IRQHandler 

    .weak   PMU0_0_IRQHandler
    .type   PMU0_0_IRQHandler, %function
PMU0_0_IRQHandler:
    B       .
    .size   PMU0_0_IRQHandler, . - PMU0_0_IRQHandler

    .weak   VADC0_C0_0_IRQHandler 
    .type   VADC0_C0_0_IRQHandler , %function
VADC0_C0_0_IRQHandler:
    B       .
    .size   VADC0_C0_0_IRQHandler , . - VADC0_C0_0_IRQHandler 

    .weak   VADC0_C0_1_IRQHandler 
    .type   VADC0_C0_1_IRQHandler , %function
VADC0_C0_1_IRQHandler:
    B       .
    .size   VADC0_C0_1_IRQHandler , . - VADC0_C0_1_IRQHandler 

    .weak   VADC0_C0_2_IRQHandler
    .type   VADC0_C0_2_IRQHandler, %function
VADC0_C0_2_IRQHandler:
    B       .
    .size   VADC0_C0_2_IRQHandler, . - VADC0_C0_2_IRQHandler

    .weak   VADC0_C0_3_IRQHandler
    .type   VADC0_C0_3_IRQHandler, %function
VADC0_C0_3_IRQHandler:
    B       .
    .size   VADC0_C0_3_IRQHandler, . - VADC0_C0_3_IRQHandler

    .weak   VADC0_G0_0_IRQHandler
    .type   VADC0_G0_0_IRQHandler, %function
VADC0_G0_0_IRQHandler:
    B       .
    .size   VADC0_G0_0_IRQHandler, . - VADC0_G0_0_IRQHandler

    .weak   VADC0_G0_1_IRQHandler
    .type   VADC0_G0_1_IRQHandler, %function
VADC0_G0_1_IRQHandler:
    B       .
    .size   VADC0_G0_1_IRQHandler, . - VADC0_G0_1_IRQHandler

    .weak   VADC0_G0_2_IRQHandler
    .type   VADC0_G0_2_IRQHandler, %function
VADC0_G0_2_IRQHandler:
    B       .
    .size   VADC0_G0_2_IRQHandler, . - VADC0_G0_2_IRQHandler

    .weak   VADC0_G0_3_IRQHandler
    .type   VADC0_G0_3_IRQHandler, %function
VADC0_G0_3_IRQHandler:
    B       .
    .size   VADC0_G0_3_IRQHandler, . - VADC0_G0_3_IRQHandler

    .weak   VADC0_G1_0_IRQHandler
    .type   VADC0_G1_0_IRQHandler, %function
VADC0_G1_0_IRQHandler:
    B       .
    .size   VADC0_G1_0_IRQHandler, . - VADC0_G1_0_IRQHandler

    .weak   VADC0_G1_1_IRQHandler
    .type   VADC0_G1_1_IRQHandler, %function
VADC0_G1_1_IRQHandler:
    B       .
    .size   VADC0_G1_1_IRQHandler, . - VADC0_G1_1_IRQHandler

    .weak   VADC0_G1_2_IRQHandler
    .type   VADC0_G1_2_IRQHandler, %function
VADC0_G1_2_IRQHandler:
    B       .
    .size   VADC0_G1_2_IRQHandler, . - VADC0_G1_2_IRQHandler

    .weak   VADC0_G1_3_IRQHandler
    .type   VADC0_G1_3_IRQHandler, %function
VADC0_G1_3_IRQHandler:
    B       .
    .size   VADC0_G1_3_IRQHandler, . - VADC0_G1_3_IRQHandler

    .weak   VADC0_G2_0_IRQHandler
    .type   VADC0_G2_0_IRQHandler, %function
VADC0_G2_0_IRQHandler:
    B       .
    .size   VADC0_G2_0_IRQHandler, . - VADC0_G2_0_IRQHandler

    .weak   VADC0_G2_1_IRQHandler
    .type   VADC0_G2_1_IRQHandler, %function
VADC0_G2_1_IRQHandler:
    B       .
    .size   VADC0_G2_1_IRQHandler, . - VADC0_G2_1_IRQHandler

    .weak   VADC0_G2_2_IRQHandler
    .type   VADC0_G2_2_IRQHandler, %function
VADC0_G2_2_IRQHandler:
    B       .
    .size   VADC0_G2_2_IRQHandler, . - VADC0_G2_2_IRQHandler

    .weak   VADC0_G2_3_IRQHandler
    .type   VADC0_G2_3_IRQHandler, %function
VADC0_G2_3_IRQHandler:
    B       .
    .size   VADC0_G2_3_IRQHandler, . - VADC0_G2_3_IRQHandler

    .weak   VADC0_G3_0_IRQHandler
    .type   VADC0_G3_0_IRQHandler, %function
VADC0_G3_0_IRQHandler:
    B       .
    .size   VADC0_G3_0_IRQHandler, . - VADC0_G3_0_IRQHandler

    .weak   VADC0_G3_1_IRQHandler
    .type   VADC0_G3_1_IRQHandler, %function
VADC0_G3_1_IRQHandler:
    B       .
    .size   VADC0_G3_1_IRQHandler, . - VADC0_G3_1_IRQHandler

    .weak   VADC0_G3_2_IRQHandler
    .type   VADC0_G3_2_IRQHandler, %function
VADC0_G3_2_IRQHandler:
    B       .
    .size   VADC0_G3_2_IRQHandler, . - VADC0_G3_2_IRQHandler

    .weak   VADC0_G3_3_IRQHandler
    .type   VADC0_G3_3_IRQHandler, %function
VADC0_G3_3_IRQHandler:
    B       .
    .size   VADC0_G3_3_IRQHandler, . - VADC0_G3_3_IRQHandler

    .weak   DSD0_0_IRQHandler
    .type   DSD0_0_IRQHandler, %function
DSD0_0_IRQHandler:
    B       .
    .size   DSD0_0_IRQHandler, . - DSD0_0_IRQHandler

    .weak   DSD0_1_IRQHandler
    .type   DSD0_1_IRQHandler, %function
DSD0_1_IRQHandler:
    B       .
    .size   DSD0_1_IRQHandler, . - DSD0_1_IRQHandler

    .weak   DSD0_2_IRQHandler
    .type   DSD0_2_IRQHandler, %function
DSD0_2_IRQHandler:
    B       .
    .size   DSD0_2_IRQHandler, . - DSD0_2_IRQHandler

    .weak   DSD0_3_IRQHandler
    .type   DSD0_3_IRQHandler, %function
DSD0_3_IRQHandler:
    B       .
    .size   DSD0_3_IRQHandler, . - DSD0_3_IRQHandler

    .weak   DSD0_4_IRQHandler
    .type   DSD0_4_IRQHandler, %function
DSD0_4_IRQHandler:
    B       .
    .size   DSD0_4_IRQHandler, . - DSD0_4_IRQHandler

    .weak   DSD0_5_IRQHandler
    .type   DSD0_5_IRQHandler, %function
DSD0_5_IRQHandler:
    B       .
    .size   DSD0_5_IRQHandler, . - DSD0_5_IRQHandler

    .weak   DSD0_6_IRQHandler
    .type   DSD0_6_IRQHandler, %function
DSD0_6_IRQHandler:
    B       .
    .size   DSD0_6_IRQHandler, . - DSD0_6_IRQHandler

    .weak   DSD0_7_IRQHandler
    .type   DSD0_7_IRQHandler, %function
DSD0_7_IRQHandler:
    B       .
    .size   DSD0_7_IRQHandler, . - DSD0_7_IRQHandler

    .weak   DAC0_0_IRQHandler
    .type   DAC0_0_IRQHandler, %function
DAC0_0_IRQHandler:
    B       .
    .size   DAC0_0_IRQHandler, . - DAC0_0_IRQHandler

    .weak   DAC0_1_IRQHandler
    .type   DAC0_1_IRQHandler, %function
DAC0_1_IRQHandler:
    B       .
    .size   DAC0_1_IRQHandler, . - DAC0_1_IRQHandler

    .weak   CCU40_0_IRQHandler
    .type   CCU40_0_IRQHandler, %function
CCU40_0_IRQHandler:
    B       .
    .size   CCU40_0_IRQHandler, . - CCU40_0_IRQHandler

    .weak   CCU40_1_IRQHandler
    .type   CCU40_1_IRQHandler, %function
CCU40_1_IRQHandler:
    B       .
    .size   CCU40_1_IRQHandler, . - CCU40_1_IRQHandler

    .weak   CCU40_2_IRQHandler
    .type   CCU40_2_IRQHandler, %function
CCU40_2_IRQHandler:
    B       .
    .size   CCU40_2_IRQHandler, . - CCU40_2_IRQHandler

    .weak   CCU40_3_IRQHandler
    .type   CCU40_3_IRQHandler, %function
CCU40_3_IRQHandler:
    B       .
    .size   CCU40_3_IRQHandler, . - CCU40_3_IRQHandler

    .weak   CCU41_0_IRQHandler
    .type   CCU41_0_IRQHandler, %function
CCU41_0_IRQHandler:
    B       .
    .size   CCU41_0_IRQHandler, . - CCU41_0_IRQHandler

    .weak   CCU41_1_IRQHandler
    .type   CCU41_1_IRQHandler, %function
CCU41_1_IRQHandler:
    B       .
    .size   CCU41_1_IRQHandler, . - CCU41_1_IRQHandler

    .weak   CCU41_2_IRQHandler
    .type   CCU41_2_IRQHandler, %function
CCU41_2_IRQHandler:
    B       .
    .size   CCU41_2_IRQHandler, . - CCU41_2_IRQHandler

    .weak   CCU41_3_IRQHandler
    .type   CCU41_3_IRQHandler, %function
CCU41_3_IRQHandler:
    B       .
    .size   CCU41_3_IRQHandler, . - CCU41_3_IRQHandler

    .weak   CCU42_0_IRQHandler
    .type   CCU42_0_IRQHandler, %function
CCU42_0_IRQHandler:
    B       .
    .size   CCU42_0_IRQHandler, . - CCU42_0_IRQHandler

    .weak   CCU42_1_IRQHandler
    .type   CCU42_1_IRQHandler, %function
CCU42_1_IRQHandler:
    B       .
    .size   CCU42_1_IRQHandler, . - CCU42_1_IRQHandler

    .weak   CCU42_2_IRQHandler
    .type   CCU42_2_IRQHandler, %function
CCU42_2_IRQHandler:
    B       .
    .size   CCU42_2_IRQHandler, . - CCU42_2_IRQHandler

    .weak   CCU42_3_IRQHandler
    .type   CCU42_3_IRQHandler, %function
CCU42_3_IRQHandler:
    B       .
    .size   CCU42_3_IRQHandler, . - CCU42_3_IRQHandler

    .weak   CCU43_0_IRQHandler
    .type   CCU43_0_IRQHandler, %function
CCU43_0_IRQHandler:
    B       .
    .size   CCU43_0_IRQHandler, . - CCU43_0_IRQHandler

    .weak   CCU43_1_IRQHandler
    .type   CCU43_1_IRQHandler, %function
CCU43_1_IRQHandler:
    B       .
    .size   CCU43_1_IRQHandler, . - CCU43_1_IRQHandler

    .weak   CCU43_2_IRQHandler
    .type   CCU43_2_IRQHandler, %function
CCU43_2_IRQHandler:
    B       .
    .size   CCU43_2_IRQHandler, . - CCU43_2_IRQHandler

    .weak   CCU43_3_IRQHandler
    .type   CCU43_3_IRQHandler, %function
CCU43_3_IRQHandler:
    B       .
    .size   CCU43_3_IRQHandler, . - CCU43_3_IRQHandler

    .weak   CCU80_0_IRQHandler
    .type   CCU80_0_IRQHandler, %function
CCU80_0_IRQHandler:
    B       .
    .size   CCU80_0_IRQHandler, . - CCU80_0_IRQHandler

    .weak   CCU80_1_IRQHandler
    .type   CCU80_1_IRQHandler, %function
CCU80_1_IRQHandler:
    B       .
    .size   CCU80_1_IRQHandler, . - CCU80_1_IRQHandler

    .weak   CCU80_2_IRQHandler
    .type   CCU80_2_IRQHandler, %function
CCU80_2_IRQHandler:
    B       .
    .size   CCU80_2_IRQHandler, . - CCU80_2_IRQHandler

    .weak   CCU80_3_IRQHandler
    .type   CCU80_3_IRQHandler, %function
CCU80_3_IRQHandler:
    B       .
    .size   CCU80_3_IRQHandler, . - CCU80_3_IRQHandler

    .weak   CCU81_0_IRQHandler
    .type   CCU81_0_IRQHandler, %function
CCU81_0_IRQHandler:
    B       .
    .size   CCU81_0_IRQHandler, . - CCU81_0_IRQHandler

    .weak   CCU81_1_IRQHandler
    .type   CCU81_1_IRQHandler, %function
CCU81_1_IRQHandler:
    B       .
    .size   CCU81_1_IRQHandler, . - CCU81_1_IRQHandler

    .weak   CCU81_2_IRQHandler
    .type   CCU81_2_IRQHandler, %function
CCU81_2_IRQHandler:
    B       .
    .size   CCU81_2_IRQHandler, . - CCU81_2_IRQHandler

    .weak   CCU81_3_IRQHandler
    .type   CCU81_3_IRQHandler, %function
CCU81_3_IRQHandler:
    B       .
    .size   CCU81_3_IRQHandler, . - CCU81_3_IRQHandler

    .weak   POSIF0_0_IRQHandler
    .type   POSIF0_0_IRQHandler, %function
POSIF0_0_IRQHandler:
    B       .
    .size   POSIF0_0_IRQHandler, . - POSIF0_0_IRQHandler

    .weak   POSIF0_1_IRQHandler
    .type   POSIF0_1_IRQHandler, %function
POSIF0_1_IRQHandler:
    B       .
    .size   POSIF0_1_IRQHandler, . - POSIF0_1_IRQHandler

    .weak   POSIF1_0_IRQHandler
    .type   POSIF1_0_IRQHandler, %function
POSIF1_0_IRQHandler:
    B       .
    .size   POSIF1_0_IRQHandler, . - POSIF1_0_IRQHandler

    .weak   POSIF1_1_IRQHandler
    .type   POSIF1_1_IRQHandler, %function
POSIF1_1_IRQHandler:
    B       .
    .size   POSIF1_1_IRQHandler, . - POSIF1_1_IRQHandler

    .weak   CAN0_0_IRQHandler
    .type   CAN0_0_IRQHandler, %function
CAN0_0_IRQHandler:
    B       .
    .size   CAN0_0_IRQHandler, . - CAN0_0_IRQHandler

    .weak   CAN0_1_IRQHandler
    .type   CAN0_1_IRQHandler, %function
CAN0_1_IRQHandler:
    B       .
    .size   CAN0_1_IRQHandler, . - CAN0_1_IRQHandler

    .weak   CAN0_2_IRQHandler
    .type   CAN0_2_IRQHandler, %function
CAN0_2_IRQHandler:
    B       .
    .size   CAN0_2_IRQHandler, . - CAN0_2_IRQHandler

    .weak   CAN0_3_IRQHandler
    .type   CAN0_3_IRQHandler, %function
CAN0_3_IRQHandler:
    B       .
    .size   CAN0_3_IRQHandler, . - CAN0_3_IRQHandler

    .weak   CAN0_4_IRQHandler
    .type   CAN0_4_IRQHandler, %function
CAN0_4_IRQHandler:
    B       .
    .size   CAN0_4_IRQHandler, . - CAN0_4_IRQHandler

    .weak   CAN0_5_IRQHandler
    .type   CAN0_5_IRQHandler, %function
CAN0_5_IRQHandler:
    B       .
    .size   CAN0_5_IRQHandler, . - CAN0_5_IRQHandler

    .weak   CAN0_6_IRQHandler
    .type   CAN0_6_IRQHandler, %function
CAN0_6_IRQHandler:
    B       .
    .size   CAN0_6_IRQHandler, . - CAN0_6_IRQHandler

    .weak   CAN0_7_IRQHandler
    .type   CAN0_7_IRQHandler, %function
CAN0_7_IRQHandler:
    B       .
    .size   CAN0_7_IRQHandler, . - CAN0_7_IRQHandler

    .weak   USIC0_0_IRQHandler
    .type   USIC0_0_IRQHandler, %function
USIC0_0_IRQHandler:
    B       .
    .size   USIC0_0_IRQHandler, . - USIC0_0_IRQHandler

    .weak   USIC0_1_IRQHandler
    .type   USIC0_1_IRQHandler, %function
USIC0_1_IRQHandler:
    B       .
    .size   USIC0_1_IRQHandler, . - USIC0_1_IRQHandler

    .weak   USIC0_2_IRQHandler
    .type   USIC0_2_IRQHandler, %function
USIC0_2_IRQHandler:
    B       .
    .size   USIC0_2_IRQHandler, . - USIC0_2_IRQHandler

    .weak   USIC0_3_IRQHandler
    .type   USIC0_3_IRQHandler, %function
USIC0_3_IRQHandler:
    B       .
    .size   USIC0_3_IRQHandler, . - USIC0_3_IRQHandler

    .weak   USIC0_4_IRQHandler
    .type   USIC0_4_IRQHandler, %function
USIC0_4_IRQHandler:
    B       .
    .size   USIC0_4_IRQHandler, . - USIC0_4_IRQHandler

    .weak   USIC0_5_IRQHandler
    .type   USIC0_5_IRQHandler, %function
USIC0_5_IRQHandler:
    B       .
    .size   USIC0_5_IRQHandler, . - USIC0_5_IRQHandler

    .weak   USIC1_0_IRQHandler
    .type   USIC1_0_IRQHandler, %function
USIC1_0_IRQHandler:
    B       .
    .size   USIC1_0_IRQHandler, . - USIC1_0_IRQHandler

    .weak   USIC1_1_IRQHandler
    .type   USIC1_1_IRQHandler, %function
USIC1_1_IRQHandler:
    B       .
    .size   USIC1_1_IRQHandler, . - USIC1_1_IRQHandler

    .weak   USIC1_2_IRQHandler
    .type   USIC1_2_IRQHandler, %function
USIC1_2_IRQHandler:
    B       .
    .size   USIC1_2_IRQHandler, . - USIC1_2_IRQHandler

    .weak   USIC1_3_IRQHandler
    .type   USIC1_3_IRQHandler, %function
USIC1_3_IRQHandler:
    B       .
    .size   USIC1_3_IRQHandler, . - USIC1_3_IRQHandler

    .weak   USIC1_4_IRQHandler
    .type   USIC1_4_IRQHandler, %function
USIC1_4_IRQHandler:
    B       .
    .size   USIC1_4_IRQHandler, . - USIC1_4_IRQHandler

    .weak   USIC1_5_IRQHandler
    .type   USIC1_5_IRQHandler, %function
USIC1_5_IRQHandler:
    B       .
    .size   USIC1_5_IRQHandler, . - USIC1_5_IRQHandler

    .weak   USIC2_0_IRQHandler
    .type   USIC2_0_IRQHandler, %function
USIC2_0_IRQHandler:
    B       .
    .size   USIC2_0_IRQHandler, . - USIC2_0_IRQHandler

    .weak   USIC2_1_IRQHandler
    .type   USIC2_1_IRQHandler, %function
USIC2_1_IRQHandler:
    B       .
    .size   USIC2_1_IRQHandler, . - USIC2_1_IRQHandler

    .weak   USIC2_2_IRQHandler
    .type   USIC2_2_IRQHandler, %function
USIC2_2_IRQHandler:
    B       .
    .size   USIC2_2_IRQHandler, . - USIC2_2_IRQHandler

    .weak   USIC2_3_IRQHandler
    .type   USIC2_3_IRQHandler, %function
USIC2_3_IRQHandler:
    B       .
    .size   USIC2_3_IRQHandler, . - USIC2_3_IRQHandler

    .weak   USIC2_4_IRQHandler
    .type   USIC2_4_IRQHandler, %function
USIC2_4_IRQHandler:
    B       .
    .size   USIC2_4_IRQHandler, . - USIC2_4_IRQHandler

    .weak   USIC2_5_IRQHandler
    .type   USIC2_5_IRQHandler, %function
USIC2_5_IRQHandler:
    B       .
    .size   USIC2_5_IRQHandler, . - USIC2_5_IRQHandler

    .weak   LEDTS0_0_IRQHandler
    .type   LEDTS0_0_IRQHandler, %function
LEDTS0_0_IRQHandler:
    B       .
    .size   LEDTS0_0_IRQHandler, . - LEDTS0_0_IRQHandler

    .weak   FCE0_0_IRQHandler
    .type   FCE0_0_IRQHandler, %function
FCE0_0_IRQHandler:
    B       .
    .size   FCE0_0_IRQHandler, . - FCE0_0_IRQHandler

    .weak   GPDMA0_0_IRQHandler
    .type   GPDMA0_0_IRQHandler, %function
GPDMA0_0_IRQHandler:
    B       .
    .size   GPDMA0_0_IRQHandler, . - GPDMA0_0_IRQHandler

    .weak   SDMMC0_0_IRQHandler
    .type   SDMMC0_0_IRQHandler, %function
SDMMC0_0_IRQHandler:
    B       .
    .size   SDMMC0_0_IRQHandler, . - SDMMC0_0_IRQHandler

    .weak   USB0_0_IRQHandler
    .type   USB0_0_IRQHandler, %function
USB0_0_IRQHandler:
    B       .
    .size   USB0_0_IRQHandler, . - USB0_0_IRQHandler

    .weak   ETH0_0_IRQHandler
    .type   ETH0_0_IRQHandler, %function
ETH0_0_IRQHandler:
    B       .
    .size   ETH0_0_IRQHandler, . - ETH0_0_IRQHandler

    .weak   GPDMA1_0_IRQHandler
    .type   GPDMA1_0_IRQHandler, %function
GPDMA1_0_IRQHandler:
    B       .
    .size   GPDMA1_0_IRQHandler, . - GPDMA1_0_IRQHandler
/* ======================================================================== */
/* ======================================================================== */

/* ============= END OF INTERRUPT HANDLER DEFINITION ======================== */

/* ============= IFX implementation of Platform_Main (For RTOS) =============== */
    .weak   Platform_Main
    .type   Platform_Main, %function
Platform_Main:
    LDR PC,=main
    .size   Platform_Main, . - Platform_Main

/* ========= Decision function queried by CMSIS startup for PLL setup ======== */
/* In the absence of DAVE code engine, CMSIS SystemInit() must perform clock 
   tree setup. 
   
   This decision routine defined here will always return TRUE.
   
   When overridden by a definition defined in DAVE code engine, this routine
   returns FALSE indicating that the code engine has performed the clock setup
*/   
    .weak   AllowPLLInitByStartup
    .type   AllowPLLInitByStartup, %function
AllowPLLInitByStartup:
    MOV R0,#1
    BX LR
    .size   AllowPLLInitByStartup, . - AllowPLLInitByStartup

/* ========= Clock tree initialization status handling mechanism ======== */
/* 
   Clock tree setup is done before DATA+BSS are initialized. Program cannot
   use global/static variables. Clock app (from DAVE code engine) uses this
   interface to prevent multiple initialization.
*/ 

/* DAVE code engine will override this weak definition */
    .weak   SetInitStatus
    .type   SetInitStatus, %function
SetInitStatus:
    MOV R0,R0 /* NOP */
    BX LR
    .size   SetInitStatus, . - SetInitStatus


/* ======  Definition of the default weak SystemInit_DAVE3 function =========
	This function will be called by the CMSIS SystemInit function. 

	If DAVE3 requires an extended SystemInit it will create its own 
	SystemInit_DAVE3 which will override this weak definition. Example includes
	setting up of external memory interfaces.
*/
     .section ".XmcStartup"
     .weak SystemInit_DAVE3
     .type SystemInit_DAVE3, %function
SystemInit_DAVE3:
     NOP
     BX LR
     .size SystemInit_DAVE3, . - SystemInit_DAVE3
/* ======================================================================== */
/* ======================================================================== */

/* ======================== Data references =============================== */
.equ  SCB_VTOR,       0xE000ED08
.equ  PREF_PCON,      0x58004000
.equ  SCU_GCU_PEEN,   0x5000413C
.equ  SCU_GCU_PEFLAG, 0x50004150

    .end
