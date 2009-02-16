#ifndef __ELUA_ADC_H__
#define __ELUA_ADC_H__

#include "type.h"

typedef struct 
{
  // Status Bit Flags
  volatile u8     op_pending: 1, // Is there a pending conversion?
                  nonblocking: 1, // Are we in blocking or non-blocking mode? (0 - blocking, 1 - nonblocking)
                  burst: 1, // Acquiring in burst mode
                  smooth_ready: 1; // Has smoothing filter warmed up (i.e. smoothlen samples collected)
                    
  unsigned        id, timer_id;

  u8              logsmoothlen;
  volatile u8     smoothidx;
  volatile u32    smoothsum;
  u16             *smoothbuf;

  volatile u8     reqsamples;
} elua_adc_state;


void adc_smooth_data( unsigned id );
elua_adc_state *adc_get_ch_state( unsigned id );
u16 adc_get_processed_sample( unsigned id );
void adc_init_state( unsigned id );
int adc_update_smoothing( unsigned id, u8 len );
void adc_flush_smoothing( unsigned id );
u8 adc_samples_requested( unsigned id );
u8 adc_samples_ready( unsigned id );

#endif