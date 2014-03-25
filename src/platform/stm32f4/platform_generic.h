// Generic platform-wide header

#ifndef __PLATFORM_GENERIC_H__
#define __PLATFORM_GENERIC_H__

#if defined(ELUA_BOARD_EXTERNAL_CLOCK_HZ)
 #if defined(ELUA_BOARD_INTERNAL_CLOCK_HZ)
  #error You must not specify both external and internal clock frequencies in 'clocks = { ... }'
 #endif
#elif defined(ELUA_BOARD_INTERNAL_CLOCK_HZ)
 #define ELUA_BOARD_EXTERNAL_CLOCK_HZ ELUA_BOARD_INTERNAL_CLOCK_HZ
#else
 #error You must specify either an external or internal clock frequency in 'clocks = { ... }'
#endif

#define PLATFORM_HAS_SYSTIMER
#define ENABLE_JTAG_SWD

#define GPIO_SOURCE2PIN(n)    (1 << (n))

// Aliases/macros for STM32F4_CON_RX/TX_PORT/PIN

#define GPIO0                 GPIOA
#define GPIO1                 GPIOB
#define GPIO2                 GPIOC
#define GPIO3                 GPIOD
#define GPIO4                 GPIOE
#define GPIO5                 GPIOF
#define GPIO6                 GPIOG

#define CON_GPIO_PORT_MACRO(x) CON_GPIO_PORT_MACRO_HELPER(x)
#define CON_GPIO_PORT_MACRO_HELPER(x) GPIO##x
#define CON_GPIO_PIN_MACRO(x)  CON_GPIO_PIN_MACRO_HELPER(x)
#define CON_GPIO_PIN_MACRO_HELPER(x)  GPIO_Pin_##x
#define CON_GPIO_SOURCE_MACRO(x) CON_GPIO_SOURCE_MACRO_HELPER(x)
#define CON_GPIO_SOURCE_MACRO_HELPER(x) GPIO_PinSource##x

#endif // #ifndef __PLATFORM_GENERIC_H__

