#include "platform_conf.h"
#ifdef BUILD_ADC

#include "buf.h"
#include "type.h"
#include "elua_adc.h"
#include "platform.h"
#include <stdlib.h>
#include "utils.h"

#define SMOOTH_REALSIZE( s ) ( ( u16 )1 << ( s->logsmoothlen ) )

// Primary set of pointers to channel states
elua_adc_ch_state adc_ch_state[ NUM_ADC ];
elua_adc_dev_state  adc_dev_state;

elua_adc_ch_state *adc_get_ch_state( unsigned id )
{
  return &adc_ch_state[ id ];
}

elua_adc_dev_state *adc_get_dev_state( unsigned dev_id )
{
  return &adc_dev_state;
}

// Rewrite device sequence
void adc_update_dev_sequence( unsigned dev_id  )
{
  elua_adc_dev_state *d = adc_get_dev_state( dev_id );
  elua_adc_ch_state *s;
  unsigned previd = d->ch_state[ d->seq_ctr ]->id;
  unsigned id;
  u8 tmp_seq_ctr = 0;
  int old_status;  
  if( d->ch_active != d->last_ch_active || d->force_reseq == 1 )
  {
    old_status = platform_cpu_set_global_interrupts( PLATFORM_CPU_DISABLE );
    // Update channel sequence
    d->seq_ctr = 0;
    for( id = 0; id < NUM_ADC; id ++ )
    {
      if ( ( d->ch_active & ( ( u32 )1 << ( id ) ) ) > 0 )
      {
        s = adc_get_ch_state( id );
        d->ch_state[ d->seq_ctr ] = s;
        s->value_ptr = &( d->sample_buf[ s->id ] );
        s->value_fresh = 0;
        if( s->id == previd )
          tmp_seq_ctr = d->seq_ctr;
        d->seq_ctr++;
      }
    }
    d->seq_len = d->seq_ctr;

    // Null out remainder of sequence
    while( d->seq_ctr < NUM_ADC )
      d->ch_state[ d->seq_ctr++ ] = NULL;
    
    d->seq_ctr = 0;

    // Sync up hardware
    platform_adc_update_sequence();
    
    d->last_ch_active = d->ch_active;
    d->seq_ctr = tmp_seq_ctr;
    d->force_reseq = 0;
    platform_cpu_set_global_interrupts( old_status );
  }
}

// Set up channel in preparation for sample acquisition
int adc_setup_channel( unsigned id, u8 logcount )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  int old_status = platform_cpu_get_global_interrupts();

#if defined( BUF_ENABLE_ADC )
  int res;

  old_status = platform_cpu_get_global_interrupts( ); // Had argument PLATFORM_CPU_DISABLE, but the prototype does not list an argument, and none of the 
  if( ((unsigned) ( (u16) 1 << logcount )) != buf_get_size( BUF_ID_ADC, id ) )
  {   
    res = buf_set( BUF_ID_ADC, id, logcount, BUF_DSIZE_U16 );
    if ( res != PLATFORM_OK )
      return res;
    buf_flush( BUF_ID_ADC, id );
  }
#endif

  s->reqsamples = ( u16 )1 << logcount;
  s->op_pending = 1;
  
  ACTIVATE_CHANNEL( d, id );
  platform_cpu_set_global_interrupts( old_status );
  
  return PLATFORM_OK;
}

// Initialize channel configuration and buffers
void adc_init_ch_state( unsigned id )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  elua_adc_dev_state *d = adc_get_dev_state( 0 );
  
  INACTIVATE_CHANNEL(d, id);
  
  // Initialize Configuration
  s->op_pending = 0;
  s->blocking = 1;
  s->smooth_ready = 0;
  s->reqsamples = 0;
  s->freerunning = 0;
  
  s->id = id;
  s->logsmoothlen = 0;
  s->smoothidx = 0;
  
  // Data Configuration
  s->smoothsum = 0;

#if defined( BUF_ENABLE_ADC )  
  // Buffer initialization
  buf_set( BUF_ID_ADC, id, ADC_BUF_SIZE, BUF_DSIZE_U16 );
#endif

  // Set to run as fast as possible
  platform_adc_set_clock( id, 0 );
}

void adc_init_dev_state( unsigned dev_id )
{
  elua_adc_dev_state *d = adc_get_dev_state( dev_id );
  d->seq_id = 0;
  d->running = 0;
  d->ch_active = 0;
  d->last_ch_active = 0;
  d->force_reseq = 0;
  d->skip_cycle = 0;
}

// Update smoothing buffer length
// If operations are pending, stop them. If new length differs from 
// existing length, attempt to resize. Whether size is new or not, 
// re-initialize buffers so that they are ready for new data.
int adc_update_smoothing( unsigned id, u8 loglen ) 
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  
  // Stop sampling if still running
  if ( s->op_pending == 1 )
  {
    platform_adc_stop( id );
  }
  
  // Update smoothing length if necessary
  if( loglen != s->logsmoothlen )
  {
    s->logsmoothlen = loglen;
  
    // Allocate and zero new smoothing buffer
    if( ( s->smoothbuf = ( u16* )realloc( s->smoothbuf, ( SMOOTH_REALSIZE( s ) ) * sizeof( u16 ) ) ) == NULL )
    {
      s->logsmoothlen = 0;
      s->smoothidx = s->smooth_ready = s->smoothsum = 0;
      if( loglen != 0 )
        return PLATFORM_ERR;
    }
  }

  // Even if buffer isn't actually reconfigured, flush contents
  adc_flush_smoothing( id );

#if defined( BUF_ENABLE_ADC )
  buf_flush( BUF_ID_ADC, id );
#endif

  return PLATFORM_OK;
}

// Load oldest sample from the buffer, replace oldest value in smoothing ring
// buffer with this new sample.  Subtract previous oldest sample value from
// sum and add new sample to sum.
void adc_smooth_data( unsigned id )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  u16 sample;
  
  if( s->smoothidx == SMOOTH_REALSIZE( s ) )
  {
    s->smoothidx = 0;
    s->smooth_ready = 1;
  }

  // Subtract Oldest Value from Sum
  s->smoothsum -= s->smoothbuf[ s->smoothidx ];

  // Replace Oldest Value in Buffer
#if defined( BUF_ENABLE_ADC )
  if ( s->value_fresh == 1 )
  {
    sample = *( s->value_ptr );
    s->value_fresh = 0;
  }
  else
    buf_read( BUF_ID_ADC, id, ( t_buf_data* )&sample );
#else
  sample = *( s->value_ptr );
  s->value_fresh = 0;
#endif
  s->smoothbuf[ s->smoothidx ] = sample;

  // Add New Sample to Sum
  s->smoothsum += s->smoothbuf[ s->smoothidx ];

  s->smoothidx++;
}

// Get samples from the buffer
// If samples are available and...
//  Smoothing is enabled
//    If smoothing is warmed up, put oldest buffer sample into smoothing
//    buffer and return result of this sample averaged with length-1
//    previous samples which had been loaded into the smoothing buffer.
//    Decrements count of requested samples.
//
//  Smoothing is off
//    Return results directly from the buffer. Decrements count of requested
//    samples.
//
// If samples are not available...
//  return 0
u16 adc_get_processed_sample( unsigned id )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  u16 sample = 0;

  if ( adc_samples_available( id ) > 0 )
  {
    if ( ( s->logsmoothlen > 0)  && ( s->smooth_ready == 1 ) )
    {
      adc_smooth_data( id );
      
      if ( s->reqsamples > 0)
        s->reqsamples -- ;
      
      sample = (u16) ( s->smoothsum >> s->logsmoothlen );
    }
    else if ( s->logsmoothlen == 0 )
    {
#if defined( BUF_ENABLE_ADC )
      if( s->value_fresh == 1 )
      {
        sample = *( s->value_ptr );
        s->value_fresh = 0;
      }
      else
        buf_read( BUF_ID_ADC, id, ( t_buf_data* )&sample );
#else
      sample = *( s->value_ptr );
      s->value_fresh = 0;
#endif
      if ( s->reqsamples > 0)
        s->reqsamples -- ;
    }
  }
  return sample;
}

// Zero out and reset smoothing buffer
void adc_flush_smoothing( unsigned id )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  u16 i = 0;
  
  s->smoothidx = 0;
  s->smoothsum = 0;
  s->smooth_ready = 0;
  
  if ( s->logsmoothlen > 0 )
    DUFF_DEVICE_8( SMOOTH_REALSIZE( s ),  s->smoothbuf[ i++ ] = 0 );
}

// Number of samples requested that have not yet been removed from the buffer
u16 adc_samples_requested( unsigned id )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  return s->reqsamples;
}

// Return count of available samples in the buffer
u16 adc_samples_available( unsigned id ) 
{
elua_adc_ch_state *s = adc_get_ch_state( id );

#if defined( BUF_ENABLE_ADC )
  u16 buffer_count = ( u16 )buf_get_count( BUF_ID_ADC, id );
  return ( ( buffer_count == 0 ) ? s->value_fresh : buffer_count );
#else
  return s->value_fresh;
#endif
}

// If blocking is enabled, wait until we have enough samples or the current
//  sampling event has finished, returns number of available samples when
//  function does exit
u16 adc_wait_samples( unsigned id, unsigned samples )
{
  elua_adc_ch_state *s = adc_get_ch_state( id );
  
  if( adc_samples_available( id ) < samples && s->blocking == 1 )
    while( s->op_pending == 1 && adc_samples_available( id ) < samples );
    
  return adc_samples_available( id );
}

#endif

