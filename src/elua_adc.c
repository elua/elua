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
  buf_set( BUF_ID_ADC, id, ADC_BUF_SIZE, BUF_DSIZE_U16 );
}

int adc_update_smoothing( unsigned id, u8 loglen )
{
  elua_adc_state *s = adc_get_ch_state( id );
  
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

    // Zero out and mark as empty
    adc_flush_smoothing( id );
  }
  return PLATFORM_OK;
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

  if ( buf_get_count( BUF_ID_ADC, id ) > 0 )
  {
    if ( ( s->logsmoothlen > 0)  && ( s->smooth_ready == 1 ) )
    {
      adc_smooth_data( id );
      
      if ( s->reqsamples > 0)
        s->reqsamples -- ;
      
      return (u16) s->smoothsum >> s->logsmoothlen;
      
    }
    else if ( s->logsmoothlen == 0 )
    {
      buf_read( BUF_ID_ADC, id, ( t_buf_data* )&sample );

      if ( s->reqsamples > 0)
        s->reqsamples -- ;

      return sample;
    }
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

u16 adc_samples_requested( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  return s->reqsamples;
}

u16 adc_samples_available( unsigned id ) 
{
  return ( u16 ) buf_get_count( BUF_ID_ADC, id );
}

void adc_wait_pending( unsigned id )
{
  elua_adc_state *s = adc_get_ch_state( id );
  
  if ( s->nonblocking == 0 && s->op_pending == 1 )
  {
    while ( s->op_pending == 1 ) { ; }
  }
}


#endif