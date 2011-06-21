/*
 * pwm.h: Declarations for low-level part of the AVR32 PWM driver for eLua.
 *
 *      Martin Guy <martinwguy@gmail.com>, March 2011
 */

#include "platform.h"   // for u32

// Initialize the PWM system, called at startup
void pwm_init( void );

// Set the frequencies of the two linear dividers A and B
void pwm_set_linear_divider( unsigned prea, unsigned diva );

// Return the actual clock frequency obtained.
u32 pwm_get_clock_freq( void );

// Set the parameters determining period and duty cycle for a channel
void pwm_channel_set_period_and_duty_cycle( unsigned id, u32 period, u32 duty );

// Enable a PWM channel (set it running)
void pwm_channel_start( int id );

// Disable a PWM channel (stop it)
void pwm_channel_stop( int id );
