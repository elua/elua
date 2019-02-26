
/*
 * Taken from the "Microseconds delay lib for STM32 (or whatever ARM)
 * based on DWT". See https://github.com/keatis/dwt_delay/
 */

#ifndef DWT_H_
#define DWT_H_

#include <DAVE.h>

void dwt_init (void);
void dwt_delay (uint32_t us);

#endif /* DWT_H_ */
