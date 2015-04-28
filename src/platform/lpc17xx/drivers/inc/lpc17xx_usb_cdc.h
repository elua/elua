#ifndef __LPC17XX_USB_CDC_H
#define __LPC17XX_USB_CDC_H

#include "type.h"


#ifdef __cplusplus
extern "C"
{
#endif

int VCOM_putchar(int c);
int VCOM_getchar(void);
void platform_setup_usb_cdc(void);


#ifdef __cplusplus
}
#endif


#endif /* __LPC17XX_USB_CDC_H */
