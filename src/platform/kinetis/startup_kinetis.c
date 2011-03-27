/* Kinetis startup code */

#include "stacks.h"
#include "platform_conf.h"
#include "common_kinetis.h"
#include "wdog.h"

#define WEAK __attribute__ ((weak))

//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
/* System exception vector handler */
void WEAK 		Reset_Handler(void);             /* Reset Handler */
void WEAK 		NMI_Handler(void);               /* NMI Handler */
void WEAK 		HardFault_Handler(void);         /* Hard Fault Handler */
void WEAK 		MemManage_Handler(void);         /* MPU Fault Handler */
void WEAK 		BusFault_Handler(void);          /* Bus Fault Handler */
void WEAK 		UsageFault_Handler(void);        /* Usage Fault Handler */
void WEAK 		SVC_Handler(void);               /* SVCall Handler */
void WEAK 		DebugMon_Handler(void);          /* Debug Monitor Handler */
void WEAK 		PendSV_Handler(void);            /* PendSV Handler */
void WEAK 		SysTick_Handler(void);           /* SysTick Handler */

/* External interrupt vector handler */
void WEAK      	WDT_IRQHandler(void);            /* Watchdog Timer */
void WEAK      	TIMER0_IRQHandler(void);         /* Timer0 */
void WEAK      	TIMER1_IRQHandler(void);         /* Timer1 */
void WEAK      	TIMER2_IRQHandler(void);         /* Timer2 */
void WEAK      	TIMER3_IRQHandler(void);         /* Timer3 */
void WEAK      	UART0_IRQHandler(void);          /* UART0 */
void WEAK      	UART1_IRQHandler(void);          /* UART1 */
void WEAK      	UART2_IRQHandler(void);          /* UART2 */
void WEAK      	UART3_IRQHandler(void);          /* UART3 */
void WEAK      	PWM1_IRQHandler(void);           /* PWM1 */
void WEAK      	I2C0_IRQHandler(void);           /* I2C0 */
void WEAK      	I2C1_IRQHandler(void);           /* I2C1 */
void WEAK      	I2C2_IRQHandler(void);           /* I2C2 */
void WEAK      	SPI_IRQHandler(void);            /* SPI */
void WEAK      	SSP0_IRQHandler(void);           /* SSP0 */
void WEAK      	SSP1_IRQHandler(void);           /* SSP1 */
void WEAK      	PLL0_IRQHandler(void);           /* PLL0 (Main PLL) */
void WEAK      	RTC_IRQHandler(void);            /* Real Time Clock */
void WEAK      	EINT0_IRQHandler(void);          /* External Interrupt 0 */
void WEAK      	EINT1_IRQHandler(void);          /* External Interrupt 1 */
void WEAK      	EINT2_IRQHandler(void);          /* External Interrupt 2 */
void WEAK      	EINT3_IRQHandler(void);          /* External Interrupt 3 */
void WEAK      	ADC_IRQHandler(void);            /* A/D Converter */
void WEAK      	BOD_IRQHandler(void);            /* Brown Out Detect */
void WEAK      	USB_IRQHandler(void);            /* USB */
void WEAK      	CAN_IRQHandler(void);            /* CAN */
void WEAK      	DMA_IRQHandler(void);            /* GP DMA */
void WEAK      	I2S_IRQHandler(void);            /* I2S */
void WEAK      	ENET_IRQHandler(void);           /* Ethernet */
void WEAK      	RIT_IRQHandler(void);            /* Repetitive Interrupt Timer */
void WEAK      	MCPWM_IRQHandler(void);          /* Motor Control PWM */
void WEAK      	QEI_IRQHandler(void);            /* Quadrature Encoder Interface */
void WEAK      	PLL1_IRQHandler(void);           /* PLL1 (USB PLL) */



/* Exported types --------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
extern unsigned long _etext;
extern unsigned long _sidata;		/* start address for the initialization values of the .data section. defined in linker script */
extern unsigned long _sdata;		/* start address for the .data section. defined in linker script */
extern unsigned long _edata;		/* end address for the .data section. defined in linker script */

extern unsigned long _sbss;			/* start address for the .bss section. defined in linker script */
extern unsigned long _ebss;			/* end address for the .bss section. defined in linker script */

extern void _estack;		/* init value for the stack pointer. defined in linker script */



/* Private typedef -----------------------------------------------------------*/
/* function prototypes ------------------------------------------------------*/
void Reset_Handler(void) __attribute__((__interrupt__)); 
extern int main(void);

//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
void Default_Handler(void) {
	// Go into an infinite loop.
	//
	while (1) {
	}
}

/******************************************************************************
*
* The minimal vector table for a Cortex M3.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/

__attribute__ ((section(".isr_vector"))) void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))( SRAM_BASE + SRAM_SIZE ), /* The initial stack pointer */
    Reset_Handler,             /* Reset Handler */
    NMI_Handler,               /* NMI Handler */
    HardFault_Handler,         /* Hard Fault Handler */
    MemManage_Handler,         /* MPU Fault Handler */
    BusFault_Handler,          /* Bus Fault Handler */
    UsageFault_Handler,        /* Usage Fault Handler */
    0,                         /* Reserved */
    0,                         /* Reserved */
    0,                         /* Reserved */
    0,                         /* Reserved */
    SVC_Handler,               /* SVCall Handler */
    DebugMon_Handler,          /* Debug Monitor Handler */
    0,                         /* Reserved */
    PendSV_Handler,            /* PendSV Handler */
    SysTick_Handler,           /* SysTick Handler */ 
    
    // External Interrupts
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    
    // This is actually the flash permissions configuration 
    (void (*)(void))0xFFFFFFFF,
    (void (*)(void))0xFFFFFFFF,
    (void (*)(void))0xFFFFFFFF,
    (void (*)(void))0xFFFFFFFE                                                       
};

/*******************************************************************************
* Function Name  : Reset_Handler
* Description    : This is the code that gets called when the processor first starts execution
*		       following a reset event.  Only the absolutely necessary set is performed,
*		       after which the application supplied main() routine is called.
* Input          :
* Output         :
* Return         :
*******************************************************************************/
void Reset_Handler(void)
{
    unsigned long *pulSrc, *pulDest;
    
    wdog_disable();

    pulSrc = &_sidata;
    for(pulDest = &_sdata; pulDest < &_edata; )
    {
        *(pulDest++) = *(pulSrc++);
    }
    //
    // Zero fill the bss segment.
    //
    for(pulDest = &_sbss; pulDest < &_ebss; )
    {
        *(pulDest++) = 0;
    }

    //
    // Call the application's entry point.
    //
    main();
    while( 1 );
}

//*****************************************************************************
//
// Provide weak aliases for each Exception handler to the Default_Handler.
// As they are weak aliases, any function with the same name will override
// this definition.
//
//*****************************************************************************
#pragma weak MemManage_Handler = Default_Handler          /* MPU Fault Handler */
#pragma weak BusFault_Handler = Default_Handler           /* Bus Fault Handler */
#pragma weak UsageFault_Handler = Default_Handler         /* Usage Fault Handler */
#pragma weak SVC_Handler = Default_Handler                /* SVCall Handler */
#pragma weak DebugMon_Handler = Default_Handler           /* Debug Monitor Handler */
#pragma weak PendSV_Handler = Default_Handler             /* PendSV Handler */
#pragma weak SysTick_Handler = Default_Handler            /* SysTick Handler */

/* External interrupt vector handler */
#pragma weak WDT_IRQHandler = Default_Handler            /* Watchdog Timer */
#pragma weak TIMER0_IRQHandler = Default_Handler         /* Timer0 */
#pragma weak TIMER1_IRQHandler = Default_Handler         /* Timer1 */
#pragma weak TIMER2_IRQHandler = Default_Handler         /* Timer2 */
#pragma weak TIMER3_IRQHandler = Default_Handler         /* Timer3 */
#pragma weak UART0_IRQHandler = Default_Handler          /* UART0 */
#pragma weak UART1_IRQHandler = Default_Handler          /* UART1 */
#pragma weak UART2_IRQHandler = Default_Handler          /* UART2 */
#pragma weak UART3_IRQHandler = Default_Handler          /* UART3 */
#pragma weak PWM1_IRQHandler = Default_Handler           /* PWM1 */
#pragma weak I2C0_IRQHandler = Default_Handler           /* I2C0 */
#pragma weak I2C1_IRQHandler = Default_Handler           /* I2C1 */
#pragma weak I2C2_IRQHandler = Default_Handler           /* I2C2 */
#pragma weak SPI_IRQHandler = Default_Handler            /* SPI */
#pragma weak SSP0_IRQHandler = Default_Handler           /* SSP0 */
#pragma weak SSP1_IRQHandler = Default_Handler           /* SSP1 */
#pragma weak PLL0_IRQHandler = Default_Handler           /* PLL0 (Main PLL) */
#pragma weak RTC_IRQHandler = Default_Handler            /* Real Time Clock */
#pragma weak EINT0_IRQHandler = Default_Handler          /* External Interrupt 0 */
#pragma weak EINT1_IRQHandler = Default_Handler          /* External Interrupt 1 */
#pragma weak EINT2_IRQHandler = Default_Handler          /* External Interrupt 2 */
#pragma weak EINT3_IRQHandler = Default_Handler          /* External Interrupt 3 */
#pragma weak ADC_IRQHandler = Default_Handler            /* A/D Converter */
#pragma weak BOD_IRQHandler = Default_Handler            /* Brown Out Detect */
#pragma weak USB_IRQHandler = Default_Handler            /* USB */
#pragma weak CAN_IRQHandler = Default_Handler            /* CAN */
#pragma weak DMA_IRQHandler = Default_Handler            /* GP DMA */
#pragma weak I2S_IRQHandler = Default_Handler            /* I2S */
#pragma weak ENET_IRQHandler = Default_Handler           /* Ethernet */
#pragma weak RIT_IRQHandler = Default_Handler            /* Repetitive Interrupt Timer */
#pragma weak MCPWM_IRQHandler = Default_Handler          /* Motor Control PWM */
#pragma weak QEI_IRQHandler = Default_Handler            /* Quadrature Encoder Interface */
#pragma weak PLL1_IRQHandler = Default_Handler           /* PLL1 (USB PLL) */
