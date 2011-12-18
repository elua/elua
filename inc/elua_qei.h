//  Quadrature Encoding Interface (QEI)

#ifndef ELUA_QEI_H
#define ELUA_QEI_H

#include "platform_conf.h"

/* Encoder 0 and 1 each with phase A and B */
enum valid_encoder_ids { ELUA_QEI_CH0 = 1, ELUA_QEI_CH1, ELUA_QEI_CH01 };
enum valid_encoder_phases { ELUA_QEI_PHA = 0, ELUA_QEI_PHAB };
/* Defines whether phases are switched before processing */
enum qei_swap_codes { ELUA_QEI_NO_SWAP, ELUA_QEI_SWAP };
/* Defines whether an index pulse is used */
enum qei_index_codes { ELUA_QEI_NO_INDEX, ELUA_QEI_INDEX };
/* Error Codes */
enum qei_error_codes { ELUA_QEI_ERR_OK = 0, ELUA_QEI_ERR_VEL_NOT_ENABLED, ELUA_QEI_ERR_ENC_NOT_ENABLED };

u8 qei_flag;
u8 qei_enc_flags[] = { 0x00, 0x02, 0x04, 0x06 };
u32 vel_ticks;

/* Function Prototypes */
void elua_qei_init( u8 enc_id, u8 phase, u8 swap, u8 index, u32 max_count );
void elua_qei_vel_init( u8 enc_id, u32 vel_period, int ppr, int edges );
void elua_qei_enable( u8 enc_id );
void elua_qei_disable( u8 enc_id );
u32 elua_qei_getPulses( u8 enc_id );
s32 elua_qei_getRPM( u8 enc_id );
u32 elua_qei_getPosition( u8 enc_id );

#endif
