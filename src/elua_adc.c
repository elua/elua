#include "platform_conf.h"
#ifdef BUILD_ADC

#include "buf.h"
#include "type.h"
#include "elua_adc.h"
#include "platform.h"
#include <stdlib.h>

#define SMOOTH_REALSIZE( s ) ( ( u16 )1 << ( s->logsmoothlen ) )

// Primary set of pointers to channel states
elua_adc_state adc_state[ NUM_ADC ];

elua_adc_state *adc_get_ch_state( unsigned id )
{
  return &adc_state[ id ];
}

// Initialize Configuration and Buffers
void adc_init_state( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  
  // Initialize Configuration
  s->op_pending = 0;
  s->blocking = 1;
  s->clocked = 0;
  s->smooth_ready = 0;
  s->reqsamples = 0;
  s->freerunning = 0;
  
  s->id = id;
  s->timer_id = 0;
  s->logsmoothlen = 0;
  s->smoothidx = 0;
  
  // Data Configuration
  s->smoothsum = 0;
  
  // Buffer initialization
  buf_set( BUF_ID_ADC, id, ADC_BUF_SIZE, BUF_DSIZE_U16 );
  
  // Set to run as fast as possible
  platform_adc_setclock( id, 0 );
}

// Update smoothing buffer length
// If new length differs from existing length and no operations are pending, attempt to resize
// Whether size is new or not, re-initialize buffers so that they are ready for new data.
int adc_update_smoothing( unsigned id, u8 loglen ) 
{
  elua_adc_state *s = adc_get_ch_state( id );
  
  // Only attempt resize if samples aren't currently being collected
  if ( s->op_pending == 0 )
  {
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
    buf_flush( BUF_ID_ADC, id );
  
    return PLATFORM_OK;
  }
  else
    return PLATFORM_ERR;
}

// Load oldest sample from the buffer, replace oldest value in smoothing ring
// buffer with this new sample.  Subtract previous oldest sample value from
// sum and add new sample to sum.
void adc_smooth_data( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  u16 sample;
  
  if( s->smoothidx == SMOOTH_REALSIZE( s ) )
  {
    s->smoothidx = 0;
    s->smooth_ready = 1;
  }

  // Subtract Oldest Value from Sum
  s->smoothsum -= s->smoothbuf[ s->smoothidx ];

  // Replace Oldest Value in Buffer
  buf_read( BUF_ID_ADC, id, ( t_buf_data* )&sample );
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
  elua_adc_state *s = adc_get_ch_state( id );
  u16 sample = 0;

  if ( buf_get_count( BUF_ID_ADC, id ) > 0 )
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
      buf_read( BUF_ID_ADC, id, ( t_buf_data* )&sample );

      if ( s->reqsamples > 0)
        s->reqsamples -- ;
    }
  }
  return sample;
}

// Zero out and reset smoothing buffer
void adc_flush_smoothing( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  u16 i;
  
  s->smoothidx = 0;
  s->smoothsum = 0;
  s->smooth_ready = 0;
  
  if ( s->logsmoothlen > 0 )
  {
    for( i = 0; i < ( SMOOTH_REALSIZE( s ) ); i ++ )
      s->smoothbuf[ i ] = 0;
  }
}

// Number of samples requested that have not yet been removed from the buffer
u16 adc_samples_requested( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  return s->reqsamples;
}

// Return count of available samples in the buffer
u16 adc_samples_available( unsigned id ) 
{
  return ( u16 ) buf_get_count( BUF_ID_ADC, id );
}

// If blocking is enabled, wait until we have enough samples or the current
//  sampling event has finished
void adc_wait_samples( unsigned id, unsigned samples )
{
  elua_adc_state *s = adc_get_ch_state( id );
  
  if( buf_get_count( BUF_ID_ADC, id ) < samples && s->blocking == 1 )
  {
    while( s->op_pending == 1 && buf_get_count( BUF_ID_ADC, id ) < samples ) { ; }
  }
}

#endif

