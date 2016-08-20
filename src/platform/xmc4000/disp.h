
// OLED display support

#ifndef __DISP_H__
#define __DISP_H__

void xmc45_disp_init( unsigned long freq );
void xmc45_disp_clear( void );
void xmc45_disp_stringDraw( const char *str, unsigned long x, unsigned long y, unsigned char level );

#endif
