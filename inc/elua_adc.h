#ifndef __ELUA_ADC_H__
#define __ELUA_ADC_H__

#include "platform_conf.h"


typedef struct 
{
  // Status Bit Flags
  volatile u8     op_pending: 1, // Is there a pending conversion?
                  blocking: 1, // Are we in blocking or non-blocking mode? (0 - blocking, 1 - nonblocking)
                  freerunning: 1, // If true, we don't stop when we've acquired the requested number of samples
                  smooth_ready: 1, // Has smoothing filter warmed up (i.e. smoothlen samples collected)
                  value_fresh: 1; // Whether the value pointed to by value_ptr is fresh
                    
  unsigned        id;

  u8              logsmoothlen;
  volatile u16    smoothidx;
  volatile u32    smoothsum;
  u16             *smoothbuf;

  volatile u16    reqsamples;
  volatile u16    *value_ptr;
} elua_adc_ch_state;

typedef struct
{
  elua_adc_ch_state   *ch_state[ NUM_ADC ];
  volatile u16        sample_buf[ NUM_ADC ]; 
  volatile u8         clocked: 1,
                      force_reseq: 1,
                      skip_cycle: 1,
                      running: 1; // Whether or not sequence is running
  volatile u32        ch_active; // bits represent whether channel should be converted on this device
  volatile u32        last_ch_active; // keep copy of old configuration
  unsigned            timer_id, seq_id; // Timer bound to device, sequencer device id
  volatile u8         seq_ctr, seq_len;
} elua_adc_dev_state;

// Channel Management
#define ACTIVATE_CHANNEL( d, id ) ( d->ch_active |= ( ( u32 )1 << ( id ) ) )
#define INACTIVATE_CHANNEL( d, id ) ( d->ch_active &= ~( ( u32 )1 << ( id ) ) )
#define INCR_SEQCTR( d ) ( d->seq_ctr++; if( d->seq_ctr >= d->seq_len - 1) d->seq_ctr = 0; )

int adc_setup_channel( unsigned id, u8 logcount );
void adc_update_dev_sequence( unsigned dev_id );
void adc_init_dev_state( unsigned dev_id );
elua_adc_dev_state *adc_get_dev_state( unsigned dev_id );
void adc_smooth_data( unsigned id );
elua_adc_ch_state *adc_get_ch_state( unsigned id );
u16 adc_get_processed_sample( unsigned id );
void adc_init_ch_state( unsigned id );
int adc_update_smoothing( unsigned id, u8 loglen );
void adc_flush_smoothing( unsigned id );
u16 adc_samples_requested( unsigned id );
u16 adc_samples_available( unsigned id );
u16 adc_wait_samples( unsigned id, unsigned samples );

#endif

