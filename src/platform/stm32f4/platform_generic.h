// Generic platform-wide header

#ifndef __PLATFORM_GENERIC_H__
#define __PLATFORM_GENERIC_H__

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

