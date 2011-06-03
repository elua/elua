#ifndef _RFS_TRANSPORTS_H
#define _RFS_TRANSPORTS_H

typedef void ( *p_read_request )( void );
typedef void ( *p_send_response )( void );
typedef void ( *p_cleanup )( void );
typedef struct
{
  p_read_request f_read_request;
  p_send_response f_send_response;
  p_cleanup f_cleanup;
} RFS_TRANSPORT_DATA;

#define   MAX_PACKET_SIZE     4096

extern const RFS_TRANSPORT_DATA *p_transport_data; 
extern const RFS_TRANSPORT_DATA mem_transport_data;
extern const RFS_TRANSPORT_DATA udp_transport_data;
extern const RFS_TRANSPORT_DATA ser_transport_data;
extern u8 rfs_buffer[ MAX_PACKET_SIZE + ELUARPC_WRITE_REQUEST_EXTRA ];

#endif