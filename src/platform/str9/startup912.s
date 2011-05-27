#*************************************************************************
# *** STR912 Startup Code For GNU Tools (executed after Reset) ***
#
# (C) Hitex (UK) Ltd. 2006
#
# Disclaimer: Whilst every effort has been made to ensure the correctness
# of this code, Hitex (UK) Ltd. cannot be held responsible for the consequences
# of its use.  Users should therefore verify its operation before including it 
# in any program.
#*************************************************************************
#
#
# ---------------------------------------------
# Include startup macros
# ---------------------------------------------

#include "stacks.h"

        .include "startup_generic.s"

#      
# Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs
#
        .equ    RAM_Size,      0x00018000      /* 96K */
        .equ    RAM_Base,      0x40000000

#*************************************************************************
# Control Startup Code Operation
#*************************************************************************
.equ   SRAM_SETUP  ,   1     /* Enable setup of SRAM */
.equ   FMI_SETUP   ,   1     /* Enable FMI Setup */
.equ   CLOCK_SETUP ,   1     /* Enable clock setup */

#*************************************************************************
# Hardware Definitions
#*************************************************************************

# Flash Memory Interface (FMI) definitions (Flash banks sizes and addresses)
.equ   FMI_BASE        ,     0x54000000      /* FMI Base Address (non-buffered) */
.equ   FMI_BBSR_OFS    ,     0x00            /* Boot Bank Size Register */
.equ   FMI_NBBSR_OFS   ,     0x04            /* Non-boot Bank Size Register      */
.equ   FMI_BBADR_OFS   ,     0x0C            /* Boot Bank Base Address Register       #!!! Documentation page 30,*/
.equ   FMI_NBBADR_OFS  ,     0x10            /* Non-boot Bank Base Address Register   #!!! adresseses do not correspond*/
.equ   FMI_CR_OFS      ,     0x18            /* Control Register */
.equ   FMI_SR_OFS      ,     0x1C            /* Status Register */

.equ   FMI_CR_Val      ,     0x00000018
.equ   FMI_BBSR_Val    ,     0x00000004
.equ   FMI_BBADR_Val   ,     0x00000000
.equ   FMI_NBBSR_Val   ,     0x00000002
.equ   FMI_NBBADR_Val  ,     0x00080000
.equ   FLASH_CFG_Val   ,     0x00001010
.equ   FMI_SR_Val      ,     0x00000003      /* Clear status errors (register not in STR912 manual! */


# System Control Unit (SCU) definitions
.equ   SCU_BASE        ,     0x5C002000      /* SCU Base Address (non-buffered)          */
.equ   SCU_CLKCNTR_OFS ,     0x00            /* Clock Control register Offset             */
.equ   SCU_PLLCONF_OFS ,     0x04            /* PLL Configuration register Offset         */
.equ   SCU_SYSTAT_OFS  ,     0x08            /* SCU status register offset                */
.equ   SCU_PCGR0_OFS   ,     0x14            /* Peripheral Clock Gating Register 0 Offset */
.equ   SCU_PCGR1_OFS   ,     0x18            /* Peripheral Clock Gating Register 1 Offset */
.equ   SCU_SCR0_OFS    ,     0x34            /* System Configuration Register 0 Offset      */


.equ   SCU_CLKCNTR_Val ,     0x00030000      /* Use PLL, external memory ratio/2 */
.equ   SCU_PLLCONF_Val ,     0x000AC019
.equ   SCU_PCGR0_Val   ,     0x000000DB      /* Setup ext mem clock, EMI, SRAM, Prefetch Queue/Branch cache, FMI */
.equ   SCU_PCGR1_Val   ,     0x00C40000      /* Setup GPIO8, 9 & 4                                     */
.equ   SCU_SCR0_Val    ,     0x00000196      /* Disable Prefetch Queue and Branch cache, SRAM = 96kb */
.equ   SCU_SYSSTAT_LOCK ,    0x01            /* Check for PLL locked                           */

# APB Bridge 1 & 2 definitions (Peripherals)
.equ   APB0_BUF_BASE   ,     0x48001802      /* APB Bridge 0 Buffered Base Address       */
.equ   APB0_NBUF_BASE  ,     0x58000000      /* APB Bridge 0 Non-buffered Base Address    */
.equ   APB1_BUF_BASE   ,     0x4C000000      /* APB Bridge 1 Buffered Base Address       */
.equ   APB1_NBUF_BASE  ,     0x5C000000      /* APB Bridge 1 Non-buffered Base Address    */

# Interrupt controller vector address
.equ   VECTOR_ADDRESS ,      0xFFFFF030      
    
#*************************************************************************
# Stack definitions
#*************************************************************************

        .equ    Top_Stack,     RAM_Base + RAM_Size

# NOTE: Startup Code must be linked first at Address at which it expects to run.

#*************************************************************************
# STARTUP EXECUTABLE CODE
#*************************************************************************

      .align 4
      .arm
      .extern main
      .global _startup

      .section .vectors, "a"

_startup:

#*************************************************************************
# Exception Vectors
#*************************************************************************
Vectors:
        LDR     PC, Reset_Addr      /* 0x0000 */
        LDR     PC, Undef_Addr      /* 0x0004 */
        LDR     PC, SWI_Addr        /* 0x0008 */
        LDR     PC, PAbt_Addr       /* 0x000C */
        LDR     PC, DAbt_Addr       /* 0x0010 */
        NOP                         /* 0x0014 Reserved Vector */
        LDR     PC, IRQ_Addr        /* IRQ handler */
        LDR     PC, FIQ_Addr        /* 0x001C FIQ has no VIC vector slot!   */

#*************************************************************************
# Interrupt Vectors
#*************************************************************************

Reset_Addr:     .word   Hard_Reset           /* CPU reset vector and entry point */
Undef_Addr:     .word   Undefined_Handler
SWI_Addr:       .word   SWI_Handler
PAbt_Addr:      .word   PAbt_Handler
DAbt_Addr:      .word   DAbt_Handler
                .word   0                      /* Reserved Address */
IRQ_Addr:       .word   IRQ_Handler            /* Does not get used due to "LDR PC, [PC, #-0xFF0]" above */
FIQ_Addr:       .word   FIQ_Handler

# Dummy Interrupt Vector Table (real service routines in INTERRUPT.C)

Undefined_Handler:  B       Undefined_Handler
SWI_Handler:        B       SWI_Handler
PAbt_Handler:       B       PAbt_Handler
DAbt_Handler:       B       DAbt_Handler
IRQ_Handler:        B       ASM_IRQ_Handler       /* should never get here as IRQ is via VIC slot... */
FIQ_Handler:        B       FIQ_Handler

# Generic IRQ handler
ASM_IRQ_Handler:
            sub       lr, lr ,#4
            stmfd     sp!, {r0-r3, r12, lr}
            ldr       r0, =VECTOR_ADDRESS
            ldr       r0, [r0] 
            mov       lr, pc                          
            bx        r0               
            ldmfd     sp!, {r0-r3, r12, pc}^
 

#*************************************************************************
# Reset Handler Entry Point
#*************************************************************************
Hard_Reset:  

           StartupDelay 900000     
                
#*************************************************************************
# Setup SRAM Size

                .IF      SRAM_SETUP == 1

                LDR     R0, =SCU_BASE
                LDR     R1, =SCU_SCR0_Val
                STR     R1, [R0, #SCU_SCR0_OFS]

                .ENDIF
                    
#*************************************************************************
# Setup Flash Memory Interface (FMI)

                .IF      FMI_SETUP == 1

                LDR     R0, =FMI_BASE
                LDR     R1, =FMI_BBSR_Val
                STR     R1, [R0, #FMI_BBSR_OFS]
                LDR     R1, =FMI_NBBSR_Val
                STR     R1, [R0, #FMI_NBBSR_OFS]
                LDR     R1, =(FMI_BBADR_Val >> 2)
                STR     R1, [R0, #FMI_BBADR_OFS]
                LDR     R1, =(FMI_NBBADR_Val >> 2)
                STR     R1, [R0, #FMI_NBBADR_OFS]
                LDR     R2, =FMI_CR_Val
                STR     R2, [R0, #FMI_CR_OFS]
                LDR     R2, =FMI_SR_Val
                STR     R2, [R0, #FMI_SR_OFS]

# Write "Write flash configuration" command (60h)
                MOV     R0, R1, LSL #2
                MOV     R1, #0x60
                STRH    R1, [R0, #0]

# Write "Write flash configuration confirm" command (03h)
                LDR     R2, =(FLASH_CFG_Val >> 2)
                ADD     R0, R0, R2
                MOV     R1, #0x03
                STRH    R1, [R0, #0]

                .ENDIF
                    
#*************************************************************************
# Setup Clock PLL

                .IF      CLOCK_SETUP == 1

                LDR     R0, =SCU_BASE
                LDR     R1, =0x00020002
                STR     R1, [R0, #SCU_CLKCNTR_OFS]    /* Select OSC as clock src */

                NOP     /* Wait for oscillator stabilisation */
                NOP     /* Must be more than 10 oscillator periods */
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                NOP
                LDR     R1, =0x0003C019               /* Disable PLL */
                STR     R1, [R0, #SCU_PLLCONF_OFS]
                LDR     R1, =SCU_PLLCONF_Val
                STR     R1, [R0, #SCU_PLLCONF_OFS]    /* Set new PLL values */

                .IF      (SCU_PLLCONF_Val & 0x8000)     /* See if PLL is being used */

                LDR     R1, =SCU_SYSSTAT_LOCK
PLL_LOCK_LOOP:
                LDR      R2,[R0, #SCU_SYSTAT_OFS]      /* Wait for PLL lock */
                ANDS   R2, R2, R1
                BEQ      PLL_LOCK_LOOP

                .ENDIF

                LDR     R1, =SCU_PLLCONF_Val
                STR     R1, [R0, #SCU_PLLCONF_OFS]
                LDR     R1, =SCU_CLKCNTR_Val
                STR     R1, [R0, #SCU_CLKCNTR_OFS]

                LDR     R1, =SCU_PCGR0_Val            /* Enable clock gating */
                STR     R1, [R0, #SCU_PCGR0_OFS]
                LDR     R1, =SCU_PCGR1_Val
                STR     R1, [R0, #SCU_PCGR1_OFS]
                    
                .ENDIF                    
                                        
#*************************************************************************
# Compiler Runtime Environment Setup
#*************************************************************************
# Note: R13 = SP

# Setup Stack for each mode
              LDR     R0, =Top_Stack

# Set up Interrupt Mode and set IRQ Mode Stack
              msr     CPSR_c, #Mode_IRQ|I_BIT|F_BIT
              mov     r13, r0
              sub     r0, r0, #STACK_SIZE_IRQ                  

#    Set up Supervisor Mode and set Supervisor Mode Stack
              msr     CPSR_c, #Mode_SVC|F_BIT
              mov     r13, r0

#*************************************************************************
# Initialise RAM For Compiler Variables
#*************************************************************************

              copy_section2 data, _efixed, _srelocate, _erelocate

#*************************************************************************
# Clear .bss section
#*************************************************************************

              clear_section bss, _szero, _ezero

#*************************************************************************
# Enter the C code
#*************************************************************************
# Jump to main()
       
       ldr    r0, =main
       mov    lr, pc
       bx     r0

forever:
       B      forever

#*************************************************************************
# END
#*************************************************************************   
        .end
