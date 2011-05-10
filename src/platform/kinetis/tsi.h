// STM32 encoder support

#ifndef __ENC_H__
#define __ENC_H__

void kin_tsi_init( unsigned id );
u16 kin_tsi_read( unsigned id );

#define KIN_TSI_DBOUNCE_COUNTS 0x10
#define KIN_TSI_TOUCH_LEVEL 0x200

#endif
