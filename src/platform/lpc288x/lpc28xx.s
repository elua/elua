@ LPC2888 startup file, adapted by BogdanM from the NXP appnotes

#include "stacks.h"

@ Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

.equ        Mode_USR,      0x10
.equ        Mode_FIQ,      0x11
.equ        Mode_IRQ,      0x12
.equ        Mode_SVC,      0x13
.equ        Mode_ABT,      0x17
.equ        Mode_UND,      0x1B
.equ        Mode_SYS,      0x1F

.equ        I_Bit,         0x80            @ when I bit is set, IRQ is disabled
.equ        F_Bit,         0x40            @ when F bit is set, FIQ is disabled

@----------------------- Memory Definitions ------------------------------------

.equ        IRAM_BASE,      0x00400000
.equ        IRAM_SIZE,      0x00010000
.equ        IRAM_END,       (IRAM_BASE + IRAM_SIZE)

@----------------------- Clock Definitions -------------------------------------

@ Clock User Interface
.equ        CGU_BASE,       0x80004C00      @ Clock Generation Unit   Base Address
                                            @ Low Power PLL
.equ        LPFIN_OFS,      0xE4            @ Input Select
.equ        LPPDN_OFS,      0xE8            @ Power Down
.equ        LPMBYP_OFS,     0xEC            @ Multiplier Bypass
.equ        LPLOCK_OFS,     0xF0            @ Lock Status
.equ        LPDBYP_OFS,     0xF4            @ Divisor Bypass
.equ        LPMSEL_OFS,     0xF8            @ Multiplication Factor
.equ        LPPSEL_OFS,     0xFC            @ Division Factor
                                            @ High Speed PLL
.equ        HPFIN_OFS,      0xAC            @ Input Select
.equ        HPNDEC_OFS,     0xB4            @ Initial Divider Control
.equ        HPMDEC_OFS,     0xB0            @ Multiplier Control
.equ        HPPDEC_OFS,     0xB8            @ Final Divider Control
.equ        HPMODE_OFS,     0xBC            @ Mode
.equ        HPSTAT_OFS,     0xC0            @ Status
.equ        HPREQ_OFS,      0xC8            @ Rate Change Request
.equ        HPACK_OFS,      0xC4            @ Rate Change Acknowledge
.equ        HPSELR_OFS,     0xD8            @ R Bandwidth
.equ        HPSELI_OFS,     0xDC            @ I Bandwidth
.equ        HPSELP_OFS,     0xE0            @ P Bandwidth

.equ        CGUSWBOX_BASE,  0x80004000      @ Clk Gen Unit Switchbox  Base Address
.equ        SYSSCR_OFS,     0x00
.equ        APB0SCR_OFS,    0x04
.equ        APB1SCR_OFS,    0x08
.equ        APB3SCR_OFS,    0x0C
.equ        DCDCSCR_OFS,    0x10
.equ        RTCSCR_OFS,     0x14
.equ        MCISCR_OFS,     0x18
.equ        UARTSCR_OFS,    0x1C
.equ        DAIOSCR_OFS,    0x20
.equ        DAISCR_OFS,     0x24
.equ        SYSFSR2_OFS,    0x58            
.equ        APB0FSR2_OFS,   0x5C
.equ        APB1FSR2_OFS,   0x60
.equ        APB3FSR2_OFS,   0x64
.equ        DCDCFSR2_OFS,   0x68
.equ        RTCFSR2_OFS,    0x6C
.equ        MCIFSR2_OFS,    0x70
.equ        UARTFSR2_OFS,   0x74
.equ        DAIOFSR2_OFS,   0x78
.equ        DAIFSR2_OFS,    0x7C


@ Constants
.equ        HPMODE_FR,      (1<<5)          @ High Speed PLL Free run mode selection bit
.equ        LPLOCK_ON,      1               @ Low Power PLL lock bit
.equ        HPLOCK_ON,      1               @ High Speed PLL lock bit
.equ        FREERUN_ON,     (1<<1)          @ Free Running active bit

@<e> Use Low Power PLL
@  <o1.0..2> LPFIN: Input Select Register
@                  <0=> 32 kHz oscillator
@                  <1=> Fast (12 MHz) oscillator
@                  <2=> MCLKI pin
@                  <3=> BCKI pin
@                  <4=> WSI pin
@                  <7=> High Speed PLL
@  <o2.0> LPMBYP: Multiplier Bypass Register
@  <o3.0> LPDBYP: Divisor Bypass Register
@  <o4.0..4> LPMSEL: Multiplication Factor <0-15>
@    <i> If LPMBYP = 0 then calculate LPMSEL to get desired Fclkout
@    <i> by using following formula Fclkout = Fclkin * (LPMSEL + 1)
@  <o5.0..4> LPPSEL: Division Factor <0-15>
@    <i> If LPDBYP = 0 then calculate LPPSEL to satisfy following 
@    <i> condition 160 MHz <= Fclkout * 2^(LPPSEL+1) <= 320 MHz
@</e>
.equ        LPFIN_Val,      0x00000001
.equ        LPMBYP_Val,     0x00000000
.equ        LPDBYP_Val,     0x00000000
.equ        LPMSEL_Val,     0x00000004
.equ        LPPSEL_Val,     0x00000001

@<h> Selection Stage Setting
@  <h> Frequency Select 2 Registers
@    <o0.0..3> SYSFSR2: Side 2 SYS Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o1.0..3> APB0FSR2: Side 2 APB0 Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o2.0..3> APB1FSR2: Side 2 APB1 Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o3.0..3> APB3FSR2: Side 2 APB3 Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o4.0..3> DCDCFSR2: Side 2 DCDC Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o5.0..3> RTCFSR2: Side 2 RTC Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o6.0..3> MCIFSR2: Side 2 MCI Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o7.0..3> UARTFSR2: Side 2 UART Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o8.0..3> DAIOFSR2: Side 2 DAIO Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@    <o9.0..3> DAIFSR2: Side 2 DAI Clock Selection
@                  <0=> 32 kHz oscillator <1=> Fast oscillator <2=> MCI Clock pin
@                  <3=> DAI BCLK pin      <4=> DAI WS pin      <7=> High Speed PLL
@                  <8=> Main PLL
@  </h>
@</h>
.equ        SYSFSR2_Val,    0x00000008
.equ        APB0FSR2_Val,   0x00000001
.equ        APB1FSR2_Val,   0x00000001
.equ        APB3FSR2_Val,   0x00000001
.equ        DCDCFSR2_Val,   0x00000001
.equ        RTCFSR2_Val,    0x00000000
.equ        MCIFSR2_Val,    0x00000001
.equ        UARTFSR2_Val,   0x00000008
.equ        DAIOFSR2_Val,   0x00000001
.equ        DAIFSR2_Val,    0x00000001

@ ==============================================================================
@ Interrupt vectors

@ Area Definition and Entry Point
@ Startup Code must be linked first at Address at which it expects to run.

            .arm
            .align 4
            .global HardReset

            .section  .vectors, "a"

HardReset:

@ Exception Vectors

Vectors:
            LDR     PC,Reset_Addr         
            LDR     PC,Undef_Addr
            LDR     PC,SWI_Addr
            LDR     PC,PAbt_Addr
            LDR     PC,DAbt_Addr
            NOP
            LDR     PC,IRQ_Addr     
            LDR     PC,FIQ_Addr
            
Reset_Addr: .word     Reset_Handler
Undef_Addr: .word     Undef_Handler
SWI_Addr:   .word     SWI_Handler
PAbt_Addr:  .word     PAbt_Handler
DAbt_Addr:  .word     DAbt_Handler
            .word     0               @ Reserved Address
IRQ_Addr:   .word     IRQ_Handler
FIQ_Addr:   .word     FIQ_Handler

Undef_Handler:  B       Undef_Handler
SWI_Handler:    B       SWI_Handler
PAbt_Handler:   B       PAbt_Handler
DAbt_Handler:   B       DAbt_Handler
FIQ_Handler:    B       FIQ_Handler
IRQ_Handler:    B       IRQ_Handler

@ ==============================================================================
@ Reset handler

            .text
                        
Reset_Handler:   

@ Clock Setup ------------------------------------------------------------------           

            @ Switch to selected clocks
            LDR     R0, =CGUSWBOX_BASE
            MOV     R1, #SYSFSR2_Val
            STR     R1, [R0, #SYSFSR2_OFS]                        
            MOV     R1, #APB0FSR2_Val
            STR     R1, [R0, #APB0FSR2_OFS]
            MOV     R1, #APB1FSR2_Val
            STR     R1, [R0, #APB1FSR2_OFS]
            MOV     R1, #APB3FSR2_Val
            STR     R1, [R0, #APB3FSR2_OFS]
            MOV     R1, #DCDCFSR2_Val
            STR     R1, [R0, #DCDCFSR2_OFS]
            MOV     R1, #RTCFSR2_Val
            STR     R1, [R0, #RTCFSR2_OFS]
            MOV     R1, #MCIFSR2_Val
            STR     R1, [R0, #MCIFSR2_OFS]
            MOV     R1, #UARTFSR2_Val
            STR     R1, [R0, #UARTFSR2_OFS]
            MOV     R1, #DAIOFSR2_Val
            STR     R1, [R0, #DAIOFSR2_OFS]
            MOV     R1, #DAIFSR2_Val
            STR     R1, [R0, #DAIFSR2_OFS]            



@ Setup Stack for each mode 

            LDR     R0, =IRAM_END

@ Enter IRQ Mode and set its Stack Pointer
            MSR     CPSR_c, #Mode_IRQ | I_Bit | F_Bit
            MOV     SP, R0
            SUB     R0, R0, #STACK_SIZE_IRQ

@ Enter Supervisor Mode and set its Stack Pointer
            MSR     CPSR_c, #Mode_SVC | I_Bit | F_Bit
            MOV     SP, R0

@ Relocate .data section (Copy from ROM to RAM)
            LDR     R1, =_efixed 
            LDR     R2, =_srelocate 
            LDR     R3, =_erelocate 
LoopRel:    CMP     R2, R3 
            BEQ     DataIsEmpty
            LDR     R0, [R1], #4 
            STR     R0, [R2], #4 
            B       LoopRel 
DataIsEmpty:
 
@  Clear .bss section (Zero init)
            MOV     R0, #0 
            LDR     R1, =_szero 
            LDR     R2, =_ezero 
            CMP     R1,R2
            BEQ     BSSIsEmpty
LoopZI:     CMP     R1, R2 
            STRLO   R0, [R1], #4 
            BLO     LoopZI 
BSSIsEmpty:          

@ Enter the main C code
            .extern main
            LDR     R0, =main
            MOV     LR, PC
            BX      R0
                
@ Should main() ever return, block in an infinite loop
forever:
            B       forever                
            
.end
