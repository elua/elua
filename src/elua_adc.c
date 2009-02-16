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
  s->nonblocking = 0;
  s->burst = 0;
  s->smooth_ready = 0;
  s->reqsamples = 0;
  
  s->id = id;
  s->timer_id = 0;
  s->logsmoothlen = 0;
  s->smoothidx = 0;
  
  // Data Configuration
  s->smoothsum = 0;
  
  // Buffer initialization
  buf_set( BUF_ID_ADC, id, ADC_BUF_SIZE, sizeof( u16 ) );
}

int adc_update_smoothing( unsigned id, u8 len )
{
  elua_adc_state *s = adc_get_ch_state( id );
  
  if( len != s->logsmoothlen )
  {
    s->logsmoothlen = intlog2( len );
    
    if ( len != 0 )
    {
      // Allocate and zero new smoothing buffer
      if( ( s->smoothbuf = ( u16* )realloc( s->smoothbuf, ( SMOOTH_REALSIZE( s ) ) * sizeof( u16 ) ) ) == NULL )
      {
        return 1;
      }
    }
    else
    {
      if ( s->smoothbuf != NULL )
        free( s->smoothbuf );
    }
    
    // Zero out and mark as empty
    adc_flush_smoothing( id );
  }
  return 0;
}

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

u16 adc_get_processed_sample( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  u16 sample;
  
  // If smoothing is enabled AND we already warmed up, or have enough to warm up
  // smooth data and output latest sample
  if ( ( s->logsmoothlen > 0)  && ( s->smooth_ready ) )
  {
    adc_smooth_data( id );
    return (u16) s->smoothsum >> s->logsmoothlen;
  }
  else if ( s->logsmoothlen == 0 && buf_get_count( BUF_ID_ADC, id ) > 0 )
  {
    buf_read( BUF_ID_ADC, id, ( t_buf_data* )&sample );
    
    if ( s->reqsamples > 0)
      s->reqsamples -- ;
    
    return sample;
  }
  else
    return 0;
}

void adc_flush_smoothing( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  u8 i;
  
  s->smoothidx = 0;
  s->smoothsum = 0;
  s->smooth_ready = 0;
  
  if ( s->logsmoothlen > 0 )
  {
    for( i = 0; i < ( SMOOTH_REALSIZE( s ) ); i ++ )
      s->smoothbuf[ i ] = 0;
  }
}

u8 adc_samples_requested( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  return s->reqsamples;
}

u8 adc_samples_ready( unsigned id ) 
{
  return ( u8 ) buf_get_count( BUF_ID_ADC, id );
}


#endif