/*
 * pwm.c: The low-level part of the AVR32 PWM driver for eLua.
 *
 *      Martin Guy <martinwguy@gmail.com>, March 2011
 */

#include "pwm.h"
#include "platform_conf.h"    // for REQ_PBA_FREQ

/*
 * The AVR32 has 7 PWM channels, each of which chooses its clock from
 * 13 system clocks:
 * - The PBA clock (currently 15,000,000 Hz)
 * - The PBA clock divided by a power of two from 2 to 1024
 * - Two "linear dividers" that divide one of the above by a factor of 1-255
 *
 * The eLua API you set an clock rate independently per-channel but this is
 * not possible in practice. At best we can select one of PBA, PBA/2, PBA/4,
 * PBA/8... or the outputs of the two linear dividers at PBA/(1-1024 * 1-255).
 * ELua's AT91 PWM driver runs half the PWM channels from one of the linear
 * dividers and the other half from the other, so selecting a new clock
 * frequency for one channel changes the clock rate and operating frequency
 * for some but not all of the other channels.
 * Here, we just run them all from linear divider A.
 */

// Should we update the period/duty cycle while the PWM is running using the
// update register?
// If 0, we just disable the channel, update the values and enable it.
// If 1, there is a complex procedure which won't work until we figure out
// how to ignore the interrupts that it generates.
#define ASYNCHRONOUS_UPDATE 0

// Values to set into the CPD bit of the per-channel CMR registers
// to say whether we are updating the duty cycle or the period.
// These values seem not to be defined anywhere in the avr32 headers.
#define AVR32_PWM_CMR_CPD_UPDATE_CDTY 0
#define AVR32_PWM_CMR_CPD_UPDATE_CPRD 1

// Initialize the PWM system, called at startup
void pwm_init()
{
  unsigned i;

  /*
   * By default, we choose the base clock frequency so thet users can select
   * any of the PWM frequencies expressible in eLua (from 1 Hz upwards).
   * The PWM counters divide the base clock by up to (but not including)
   * 2^20, so we choose the highest base clock frequency less than 1048576.
   * Possible PBA clocks are all an integer multiple of 1000000Hz (except one)
   * which suggests using 1000000 Hz ad the PWM base clock.
   * The exception is 16.5 MHz when the CPU frequency is 66 MHz. With this
   * the naive formula PBA_FREQ/1000000 this gives a base clock of 1031250 Hz
   * which is still < 1048576 so is OK.
   */
  pwm_set_linear_divider( AVR32_PWM_MR_PREA_MCK, REQ_PBA_FREQ / 1000000 );

  // Set all PWM channels to use clock A
  // Also select the fastest cycle possible by default in case they start the
  // channel without having first set its period (which is invalid).
  for ( i = 0; i < NUM_PWM; i++ )
  {
    avr32_pwm_cmr_t cmr;    // Value to write to Channel Mode Register.
                            // As it is a word of bitfields, this way we
                            // write it as a single word write.

    cmr.cpre = AVR32_PWM_CMR0_CPRE_CLKA;    // Run from clock A
    cmr.calg = 0;    // "Left-aligned waveform". Actually, the active part
                     // occurs at the end of the cycle, not the beginning.
    cmr.cpol = 0;    // Low for duty period then high; low when not running.


    AVR32_PWM.channel[i].CMR = cmr;
    AVR32_PWM.channel[i].cprd = 1;
    AVR32_PWM.channel[i].cdty = 0;
  }

#if ASYNCHRONOUS_UPDATE
  // Enable all channels in the Interrupt Enable Register so that we can
  // use the update register to change period/duty while the channel is active.
  // Doesn't this generate real hard interrupts? Where do they get handled?
  AVR32_PWM.ier = ~( (~0) << NUM_PWM );    // Set bits 0 to (NUM_PWM-1)
#endif
}


// Set the two linear dividers' frequencies
void pwm_set_linear_divider( unsigned prea, unsigned diva )
{
  avr32_pwm_mr_t mr = AVR32_PWM.MR;

  mr.prea = prea;
  mr.diva = diva;
  AVR32_PWM.MR = mr;
}

// Return the actual PWM base clock frequency
// ( well, actually (u32)(floor(reality)) )
// This stuff should probably go in platform.c
u32 pwm_get_clock_freq( void )
{
  unsigned int prescaler = AVR32_PWM.MR.prea;
  unsigned int divisor = AVR32_PWM.MR.diva;

  if (divisor == 0)
  {
    // This clock is turned off. A frequency of 0 should surprise them.
    return 0;
  }

  return REQ_PBA_FREQ / ( ( 1<<prescaler ) * divisor );
}

/*
 * Set the period and duty cycle of a PWM channel in clock cycles.
 *
 * If the channel is not active, this is easy: write to CPRD and CDTY registers.
 *
 * If the channel is running, the docs say you cannot write to these registers
 * but have to use the CUPD update register and CPD field in the channel's
 * control register to say whether the CUPD data should be written to the
 * CPRD or CDTY fields.  This is fraught with problems:
 *
 * - You can only update one of the fields per output cycle, which means that,
 *   to update both, you have to update one, wait a cycle then update the other.
 *   Not only does this incur a delay of one cycle (which could be a second!)
 *   but also means that you get one intermediate cycle of junk output using
 *   the old frequency at the new duty period or vice versa.
 *   Worse, you have to be careful in which order you change them, since
 *   duty_period > cycle_period is not allowed by the hardware.
 *
 * - To know when one value has been flushed to its register, you have to
 *   enable the per-channel PWM interrupt, clear its status flag and wait until
 *   that interrupt status flag goes high, which happens at the end of each
 *   cycle (which is also the moment in which the update register is flushed).
 *   Problems:
 *   - You have to enable the interrupt for that channel even if you are polling
 *     the status bit.  Does this generate a real hard interrupt or not?
 *   - While polling, reading the interrupt status register clears ALL the
 *     channels' flags to zero.  The AVR32UC3A User's Guide polling example
 *     always waits for the update to complete before returning, which is
 *     inefficient. It would be better to check whether there is a pending
 *     update just before writing the new update value.  To handle several
 *     channels simultaneously, we need to cache the interrupt status flags
 *     for each channel, because the signal saying that an update on channel
 *     1 has completed may be read while updating channel 2.
 *
 * In practice, it seems you can just write to the CPRD and CDTY registers
 * while the PWM is running. You don't even have to disable it first like
 * they say.  So we do this.
 *
 * The reason to implement the update method is for applications which want
 * to change the duty cycle on a per-cycle basis, such as using PWM for
 * audio output, since with the asynchronous update mechanism you only get
 * to update one value per cycle.
 */

#if ASYNCHRONOUS_UPDATE

// A bitmask on 1<<channel_id, the same as the Interrupt Status Register.
// If a bit is zero, it means that a pending update on that channel
// has not yet been flushed to the actual register.
static unsigned update_has_flushed = ~0; // Initial value: No pending updates

// Forward declarations
static void pwm_channel_set_period    ( unsigned id, u32 period );
static void pwm_channel_set_duty_cycle( unsigned id, u32 duty );

#endif

void pwm_channel_set_period_and_duty_cycle( unsigned id, u32 period, u32 duty )
{
  // Fix invalid parameters
  if ( duty > period ) duty = period;

#if ASYNCHRONOUS_UPDATE
  // If the channel is not active...
  if ( ( AVR32_PWM.sr & ( 1 << id ) ) == 0 )
  {
     // ...write directly to the registers
     AVR32_PWM.channel[id].cprd = period;
     AVR32_PWM.channel[id].cdty = duty;
  }
  else
  {
    // ...otherwise we cannot write CPRD directly, but have to update them
    // through the update register.
    // You can't change both period and duty cycle in one cycle; you have to
    // update one, wait for the end of the output waveform and then update the
    // other.  This means you always get one cycle of junk between one update
    // and the other.
    // We can only choose whether to output one cycle at the old frequency
    // and the new duty cycle or at the old duty cycle and the new frequency.

    // In either case we must ensure that duty <= period for the junk cycle.
    // So one solution is, if the period decreases, update the duty cycle first
    // or if the period increases, update the period first.

    if ( period < AVR32_PWM.channel[id].cprd )
    {
      pwm_channel_set_duty_cycle( id, duty );
      pwm_channel_set_period( id, period );
    } else {
      pwm_channel_set_period( id, period );
      pwm_channel_set_duty_cycle( id, duty );
    }
  }
#else
  // No nasty asynchronous stuff. just disable, configure and re-enable.
  {
    int was_enabled = AVR32_PWM.sr & (1 << id);

    if (was_enabled) AVR32_PWM.dis = 1 << id;
    AVR32_PWM.channel[id].cprd = period;
    AVR32_PWM.channel[id].cdty = duty;
    if (was_enabled) AVR32_PWM.ena = 1 << id;
  }
#endif
}

#if ASYNCHRONOUS_UPDATE
static void pwm_channel_set_period( unsigned id, u32 period )
{
  // Only go through the update rigmarole if the value changed
  if ( period == AVR32_PWM.channel[id].cprd )
    return;

  // Make sure there is no pending update for this channel, otherwise we
  // would overwrite its data before the previous update has completed.
  while (! (update_has_flushed & (1 << id) ) )
    update_has_flushed |= AVR32_PWM.isr;

  // Select updating of the period and write into the update register
  AVR32_PWM.channel[id].CMR.cpd = AVR32_PWM_CMR_CPD_UPDATE_CPRD;
  update_has_flushed &= ~(1 << id);     // The update hasn't happened yet...
  AVR32_PWM.channel[id].cupd = period;  // Schedule the update to be performed
}

static void pwm_channel_set_duty_cycle( unsigned id, u32 duty )
{
  // Only go through the update rigmarole if the value changed
  if ( duty == AVR32_PWM.channel[id].cprd )
    return;

  // Make sure there is no pending update for this channel, otherwise we
  // would overwrite its data before the previous update has completed.
  while (! (update_has_flushed & (1 << id) ) )
    update_has_flushed |= AVR32_PWM.isr;

  // Select updating of the period and write into the update register
  AVR32_PWM.channel[id].CMR.cpd = AVR32_PWM_CMR_CPD_UPDATE_CDTY;
  update_has_flushed &= ~(1 << id);
  AVR32_PWM.channel[id].cupd = duty;
}
#endif

// Enable a PWM channel (set it running)
void pwm_channel_start( int id )
{
  AVR32_PWM.ena = 1 << id;
}

// Disable a PWM channel (stop it)
void pwm_channel_stop( int id )
{
  AVR32_PWM.dis = 1 << id;
}
