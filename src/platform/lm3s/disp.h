// LM3S OLED display support

#ifndef __DISP_H__
#define __DISP_H__

void lm3s_disp_init( unsigned long freq );
void lm3s_disp_clear( void );
void lm3s_disp_stringDraw( const char *str, unsigned long x, unsigned long y, unsigned char level );
void lm3s_disp_imageDraw( const unsigned char *img, unsigned long x, unsigned long y,
                              unsigned long width, unsigned long height );                             
void lm3s_disp_enable( unsigned long freq );
void lm3s_disp_disable( void );
void lm3s_disp_displayOn( void );
void lm3s_disp_displayOff( void );

#endif
