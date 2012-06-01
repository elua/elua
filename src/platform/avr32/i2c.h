// Declarations for the low-level AVR32 I2C driver for eLua

#include "type.h"	// for u32

u32 i2c_setup( u32 speed );                // speed is in Hz
void i2c_start_cond( void );
void i2c_stop_cond( void );
int i2c_write_byte( unsigned char byte );  // returns 0 if acked by slave
unsigned char i2c_read_byte( int nack );

// Higher-level interface
int i2c_send(u8 slave_address, const u8 *data, unsigned len, bool stop);
int i2c_recv(u8 slave_address, u8 *data, unsigned len, bool stop);
bool i2c_probe(u8 slave_address);
