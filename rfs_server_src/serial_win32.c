// Serial inteface implementation for POSIX-compliant systems

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "type.h"
#include "serial.h"

#define WIN_ERROR     ( HANDLE )-1
#define WIN_MAX_PORT_NAME   1024
#define MAX_HANDLES   1024

static HANDLE sel_handlers[ MAX_HANDLES ];
static int sel_handler_map[ MAX_HANDLES ];

// Helper: set timeout
static int ser_win32_set_timeouts( HANDLE hComm, DWORD ri, DWORD rtm, DWORD rtc, DWORD wtm, DWORD wtc )
{   
  COMMTIMEOUTS timeouts;
  
  if( GetCommTimeouts( hComm, &timeouts ) == FALSE )
  {
    CloseHandle( hComm );
    return SER_ERR;
  }
  timeouts.ReadIntervalTimeout = ri;
  timeouts.ReadTotalTimeoutConstant = rtm;
  timeouts.ReadTotalTimeoutMultiplier = rtc;
  timeouts.WriteTotalTimeoutConstant = wtm;
  timeouts.WriteTotalTimeoutMultiplier = wtc;
	if( SetCommTimeouts( hComm, &timeouts ) == FALSE )
	{
	  CloseHandle( hComm );
	  return SER_ERR;
  }               
  
  return SER_OK;
}

// Helper: set communication timeout
static int ser_set_timeout_ms( HANDLE hComm, u32 timeout )
{ 
  if( timeout == SER_NO_TIMEOUT )
    return ser_win32_set_timeouts( hComm, MAXDWORD, 0, 0, 0, 0 );
  else if( timeout == SER_INF_TIMEOUT )
    return ser_win32_set_timeouts( hComm, 0, 0, 0, 0, 0 );
  else
    return ser_win32_set_timeouts( hComm, 0, 0, timeout, 0, 0 );
}

// Open the serial port
ser_handler ser_open( const char* sername )
{
  char portname[ WIN_MAX_PORT_NAME + 1 ];
  HANDLE hComm;
  ser_handler hnd;
  
  portname[ 0 ] = portname[ WIN_MAX_PORT_NAME ] = '\0';
  _snprintf( portname, WIN_MAX_PORT_NAME, "\\\\.\\%s", sername );
  hComm = CreateFile( portname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0 );
  if( hComm == INVALID_HANDLE_VALUE )
    return SER_HANDLER_INVALID;
  if( !SetupComm( hComm, 2048, 2048 ) )
    return SER_HANDLER_INVALID;
  if( ser_set_timeout_ms( hComm, SER_INF_TIMEOUT ) != SER_OK )
    return SER_HANDLER_INVALID;
  if( ( hnd = malloc( sizeof( SERIAL_DATA ) ) ) == NULL )
    return SER_HANDLER_INVALID;
  memset( hnd, 0, sizeof( SERIAL_DATA ) );
  hnd->hnd = hComm;
  hnd->fWaitingOnRead = FALSE;
  if( ( hnd->o.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) ) == NULL )
    return SER_HANDLER_INVALID;
  if( ( hnd->o_wr.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) ) == NULL )
    return SER_HANDLER_INVALID;    
  return hnd;
}

// Close the serial port
void ser_close( ser_handler id )
{
  CloseHandle( id->o.hEvent );
  CloseHandle( id->o_wr.hEvent );  
  CloseHandle( id->hnd );
  free( id );
}

int ser_setup( ser_handler id, u32 baud, int databits, int parity, int stopbits, int flow )
{
  HANDLE hComm = id->hnd;
  DCB dcb;
  
	if( GetCommState( hComm, &dcb ) == FALSE )
	{
		CloseHandle( hComm );
		return SER_ERR;
	}
  dcb.BaudRate = baud;
  dcb.ByteSize = databits;
  dcb.Parity = parity == SER_PARITY_NONE ? NOPARITY : ( parity == SER_PARITY_EVEN ? EVENPARITY : ODDPARITY );
  dcb.StopBits = stopbits == SER_STOPBITS_1 ? ONESTOPBIT : ( stopbits == SER_STOPBITS_1_5 ? ONE5STOPBITS : TWOSTOPBITS );
  dcb.fBinary = TRUE;
  dcb.fDsrSensitivity = FALSE;
  dcb.fParity = parity != SER_PARITY_NONE ? TRUE : FALSE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;
  dcb.fNull = FALSE;
  /**/ dcb.fAbortOnError = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fDsrSensitivity = FALSE;
  if( flow == SER_FLOW_NONE )
  {
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fOutxCtsFlow = FALSE;
  }
  else
  {
    dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
    dcb.fOutxCtsFlow = TRUE;
  }
  if( SetCommState( hComm, &dcb ) == 0 )
  {
    CloseHandle( hComm );
    return SER_ERR;
  }
  
  if( ser_win32_set_timeouts( hComm, 0, 0, 0, 0, 0 ) == SER_ERR )
  {
    CloseHandle( hComm );
    return SER_ERR;
  }
  
  FlushFileBuffers( hComm );

  return SER_OK;
}

// Read up to the specified number of bytes, return bytes actually read
u32 ser_read( ser_handler id, u8* dest, u32 maxsize, u32 timeout )
{
  HANDLE hComm = id->hnd;
  DWORD readbytes = 0;
  
  if( !id->fWaitingOnRead )
  {
    if( ReadFile( hComm, dest, maxsize, &readbytes, &id->o ) == FALSE )
    {
      if( GetLastError() != ERROR_IO_PENDING )   
        return 0;
      else
        id->fWaitingOnRead = TRUE;
    }
    else
      return readbytes;
  }
      
  if( id->fWaitingOnRead )
  {
    DWORD dwRes = WaitForSingleObject( id->o.hEvent, timeout == SER_INF_TIMEOUT ? INFINITE : timeout ); 
    if( dwRes == WAIT_OBJECT_0 ) 
    {
      if( !GetOverlappedResult( hComm, &id->o, &readbytes, TRUE ) )
        readbytes = 0;
    }
    else if( dwRes == WAIT_TIMEOUT )
    {
      CancelIo( hComm );
      GetOverlappedResult( hComm, &id->o, &readbytes, TRUE );
      readbytes = 0;
    }
    ResetEvent( id->o.hEvent );   
  }  
  id->fWaitingOnRead = FALSE;  
  return readbytes;
}

// Read a single byte and return it (or -1 for error)
int ser_read_byte( ser_handler id, u32 timeout )
{
  u8 data;
  int res = ser_read( id, &data, 1, timeout );

  return res == 1 ? data : -1;
}

// Write up to the specified number of bytes, return bytes actually written
u32 ser_write( ser_handler id, const u8 *src, u32 size )
{
  HANDLE hComm = id->hnd;
  DWORD written = 0;
  BOOL fWaitingOnWrite = FALSE;
	HANDLE temp = id->o_wr.hEvent;
	
	memset( &id->o_wr, 0, sizeof( OVERLAPPED ) );
	id->o_wr.hEvent = temp;
  if( WriteFile( hComm, src, size, &written, &id->o_wr ) == FALSE )
  {
    if( GetLastError() != ERROR_IO_PENDING )    
      return 0;
    else
      fWaitingOnWrite = TRUE;
  }
  else
    return written;

  if( fWaitingOnWrite )
  {                              
    DWORD dwRes = WaitForSingleObject( id->o_wr.hEvent, INFINITE );      
    if( dwRes == WAIT_OBJECT_0 )
      if( !GetOverlappedResult( hComm, &id->o_wr, &written, FALSE ) )
        written = 0;
    ResetEvent( id->o_wr.hEvent );        
  }

  return written; 
}

// Write a byte to the serial port
u32 ser_write_byte( ser_handler id, u8 data )
{
  return ser_write( id, &data, 1 );
}

// Perform 'select' on the specified handler(s), returning a single byte 
// if it could be read (plus the object ID in the upper 8 bits) and -1
// otherwise
int ser_select_byte( ser_handler *pobjects, unsigned nobjects, int timeout )
{
  int i, idx;
  DWORD readbytes;
  int res = -1;
  unsigned num_wait = 0;
  ser_handler hnd;
  HANDLE temp;
  
  // Try to read directly first
  for( i = 0; i < nobjects; i ++ )
  {   
    temp = pobjects[ i ]->o.hEvent;
    memset( &pobjects[ i ]->o, 0, sizeof( OVERLAPPED ) );
    pobjects[ i ]->o.hEvent = temp;
    if( !pobjects[ i ]->fWaitingOnRead )
    {
      if( ReadFile( pobjects[ i ]->hnd, &pobjects[ i ]->databuf, 1, &readbytes, &pobjects[ i ]->o ) == FALSE )
      {
        if( GetLastError() != ERROR_IO_PENDING )
          return -1;
        else
        {
          pobjects[ i ]->fWaitingOnRead = TRUE;
          sel_handler_map[ num_wait ] = i;
          sel_handlers[ num_wait ++ ] = pobjects[ i ]->o.hEvent;
        }
      }
      else
      {
        if( readbytes == 1 )
          return pobjects[ i ]->databuf | ( i << 8 );
        else
          return -1;
      }    
    }  
    else
    {
      sel_handler_map[ num_wait ] = i;
      sel_handlers[ num_wait ++ ] = pobjects[ i ]->o.hEvent;
    }  
  }
    
  if( num_wait > 0 )
  {
    idx = -1;
    DWORD dwRes = WaitForMultipleObjects( num_wait, sel_handlers, FALSE, timeout == SER_INF_TIMEOUT ? INFINITE : timeout );
    if( dwRes >= WAIT_OBJECT_0 && dwRes < WAIT_OBJECT_0 + num_wait ) 
    {
      i = idx = dwRes - WAIT_OBJECT_0;
      hnd = pobjects[ sel_handler_map[ i ] ];
      hnd->fWaitingOnRead = FALSE;
      if( GetOverlappedResult( hnd->hnd, &hnd->o, &readbytes, TRUE ) && readbytes == 1 )
        res = hnd->databuf | ( sel_handler_map[ i ] << 8 );
      ResetEvent( hnd->o.hEvent );        
    }
    else if( dwRes == WAIT_TIMEOUT )
    {
      for( i = 0; i < num_wait; i ++ )
      {
        hnd = pobjects[ sel_handler_map[ i ] ];      
        hnd->fWaitingOnRead = FALSE;      
        CancelIo( hnd->hnd );
        GetOverlappedResult( hnd->hnd, &hnd->o, &readbytes, TRUE );
        ResetEvent( hnd->o.hEvent );
      } 
    }  
  }  
    
  return res;    
}

