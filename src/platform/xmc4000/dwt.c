
/*
 * Taken from the "Microseconds delay lib for STM32 (or whatever ARM)
 * based on DWT". See https://github.com/keatis/dwt_delay/
 */

#include "dwt.h"

/* Initialize DWT for micros delay */
void dwt_init (void) {
  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }
}

/* DWT delay routine */
void dwt_delay (uint32_t us) {
  int32_t target_tick = DWT->CYCCNT + us * (SystemCoreClock / 1000000);
  while (DWT->CYCCNT <= target_tick);
}
