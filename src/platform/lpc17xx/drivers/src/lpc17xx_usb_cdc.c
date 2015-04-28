/*
	LPCUSB, an USB device driver for LPC microcontrollers
	Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright
	   notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.
	3. The name of the author may not be used to endorse or promote products
	   derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "platform_conf.h"

#ifdef BUILD_USB_CDC

#include <stdio.h>
#include <string.h>			// memcpy

#include "LPC17xx.h"
#include "type.h"

#include "usbapi.h"
#include "usbdebug.h"

#ifdef CDC_BUF_SIZE
 #define VCOM_FIFO_SIZE (1<<CDC_BUF_SIZE)
#else
 #define VCOM_FIFO_SIZE 128
#endif

typedef struct {
        int             head;
        int     tail;
        u8              *buf;
} fifo_t;

void fifo_init(fifo_t *fifo, u8 *buf)
{
	fifo->head = 0;
	fifo->tail = 0;
	fifo->buf = buf;
}


BOOL fifo_put(fifo_t *fifo, u8 c)
{
	int next;

	// check if FIFO has room
	next = (fifo->head + 1) % VCOM_FIFO_SIZE;
	if (next == fifo->tail) {
		// full
		return FALSE;
	}

	fifo->buf[fifo->head] = c;
	fifo->head = next;

	return TRUE;
}


BOOL fifo_get(fifo_t *fifo, u8 *pc)
{
	int next;

	// check if FIFO has data
	if (fifo->head == fifo->tail) {
		return FALSE;
	}

	next = (fifo->tail + 1) % VCOM_FIFO_SIZE;

	*pc = fifo->buf[fifo->tail];
	fifo->tail = next;

	return TRUE;
}


int fifo_avail(fifo_t *fifo)
{
	return (VCOM_FIFO_SIZE + fifo->head - fifo->tail) % VCOM_FIFO_SIZE;
}


int fifo_free(fifo_t *fifo)
{
	return (VCOM_FIFO_SIZE - 1 - fifo_avail(fifo));
}




#define BAUD_RATE	115200

#define INT_IN_EP		0x81
#define BULK_OUT_EP		0x05
#define BULK_IN_EP		0x82

#define MAX_PACKET_SIZE	64

#define LE_WORD(x)		((x)&0xFF),((x)>>8)

// CDC definitions
#define CS_INTERFACE			0x24
#define CS_ENDPOINT				0x25

#define	SET_LINE_CODING			0x20
#define	GET_LINE_CODING			0x21
#define	SET_CONTROL_LINE_STATE	0x22

// data structure for GET_LINE_CODING / SET_LINE_CODING class requests
typedef struct {
	u32		dwDTERate;
	u8		bCharFormat;
	u8		bParityType;
	u8		bDataBits;
} TLineCoding;

static TLineCoding LineCoding = {115200, 0, 0, 8};
static u8 abBulkBuf[64];
static u8 abClassReqData[8];

static u8 txdata[VCOM_FIFO_SIZE];
static u8 rxdata[VCOM_FIFO_SIZE];

static fifo_t txfifo;
static fifo_t rxfifo;

// forward declaration of interrupt handler
void USBIntHandler(void);

static const u8 abDescriptors[] = {

// device descriptor
	0x12,
	DESC_DEVICE,
	LE_WORD(0x0101),			// bcdUSB
	0x02,						// bDeviceClass
	0x00,						// bDeviceSubClass
	0x00,						// bDeviceProtocol
	MAX_PACKET_SIZE0,			// bMaxPacketSize
	LE_WORD(0x1fc9),			// idVendor
	LE_WORD(0x2047),			// idProduct
	LE_WORD(0x0100),			// bcdDevice
	0x01,						// iManufacturer
	0x02,						// iProduct
	0x03,						// iSerialNumber
	0x01,						// bNumConfigurations

// configuration descriptor
	0x09,
	DESC_CONFIGURATION,
	LE_WORD(67),				// wTotalLength
	0x02,						// bNumInterfaces
	0x01,						// bConfigurationValue
	0x00,						// iConfiguration
	0xC0,						// bmAttributes
	0x32,						// bMaxPower
// control class interface
	0x09,
	DESC_INTERFACE,
	0x00,						// bInterfaceNumber
	0x00,						// bAlternateSetting
	0x01,						// bNumEndPoints
	0x02,						// bInterfaceClass
	0x02,						// bInterfaceSubClass
	0x01,						// bInterfaceProtocol, linux requires value of 1 for the cdc_acm module
	0x00,						// iInterface
// header functional descriptor
	0x05,
	CS_INTERFACE,
	0x00,
	LE_WORD(0x0110),
// call management functional descriptor
	0x05,
	CS_INTERFACE,
	0x01,
	0x01,						// bmCapabilities = device handles call management
	0x01,						// bDataInterface
// ACM functional descriptor
	0x04,
	CS_INTERFACE,
	0x02,
	0x02,						// bmCapabilities
// union functional descriptor
	0x05,
	CS_INTERFACE,
	0x06,
	0x00,						// bMasterInterface
	0x01,						// bSlaveInterface0
// notification EP
	0x07,
	DESC_ENDPOINT,
	INT_IN_EP,					// bEndpointAddress
	0x03,						// bmAttributes = intr
	LE_WORD(8),					// wMaxPacketSize
	0x0A,						// bInterval
// data class interface descriptor
	0x09,
	DESC_INTERFACE,
	0x01,						// bInterfaceNumber
	0x00,						// bAlternateSetting
	0x02,						// bNumEndPoints
	0x0A,						// bInterfaceClass = data
	0x00,						// bInterfaceSubClass
	0x00,						// bInterfaceProtocol
	0x00,						// iInterface
// data EP OUT
	0x07,
	DESC_ENDPOINT,
	BULK_OUT_EP,				// bEndpointAddress
	0x02,						// bmAttributes = bulk
	LE_WORD(MAX_PACKET_SIZE),	// wMaxPacketSize
	0x00,						// bInterval
// data EP in
	0x07,
	DESC_ENDPOINT,
	BULK_IN_EP,					// bEndpointAddress
	0x02,						// bmAttributes = bulk
	LE_WORD(MAX_PACKET_SIZE),	// wMaxPacketSize
	0x00,						// bInterval

	// string descriptors
	0x04,
	DESC_STRING,
	LE_WORD(0x0409),

	0x0E,
	DESC_STRING,
	'L', 0, 'P', 0, 'C', 0, 'U', 0, 'S', 0, 'B', 0,

	0x14,
	DESC_STRING,
	'U', 0, 'S', 0, 'B', 0, 'S', 0, 'e', 0, 'r', 0, 'i', 0, 'a', 0, 'l', 0,

	0x12,
	DESC_STRING,
	'D', 0, 'E', 0, 'A', 0, 'D', 0, 'C', 0, '0', 0, 'D', 0, 'E', 0,

// terminating zero
	0
};


/**
	Local function to handle incoming bulk data

	@param [in] bEP
	@param [in] bEPStatus
 */
static void BulkOut(u8 bEP, u8 bEPStatus)
{
	int i, iLen;

	if (fifo_free(&rxfifo) < MAX_PACKET_SIZE) {
		// may not fit into fifo
		return;
	}

	// get data from USB into intermediate buffer
	iLen = USBHwEPRead(bEP, abBulkBuf, sizeof(abBulkBuf));
	for (i = 0; i < iLen; i++) {
		// put into FIFO
		if (!fifo_put(&rxfifo, abBulkBuf[i])) {
			// overflow... :(
			ASSERT(FALSE);
			break;
		}
	}
}


/**
	Local function to handle outgoing bulk data

	@param [in] bEP
	@param [in] bEPStatus
 */
static void BulkIn(u8 bEP, u8 bEPStatus)
{
	int i, iLen;

	if (fifo_avail(&txfifo) == 0) {
		// no more data, disable further NAK interrupts until next USB frame
		USBHwNakIntEnable(0);
		return;
	}

	// get bytes from transmit FIFO into intermediate buffer
	for (i = 0; i < MAX_PACKET_SIZE; i++) {
		if (!fifo_get(&txfifo, &abBulkBuf[i])) {
			break;
		}
	}
	iLen = i;

	// send over USB
	if (iLen > 0) {
		USBHwEPWrite(bEP, abBulkBuf, iLen);
	}
}


/**
	Local function to handle the USB-CDC class requests

	@param [in] pSetup
	@param [out] piLen
	@param [out] ppbData
 */
static BOOL HandleClassRequest(TSetupPacket *pSetup, int *piLen, u8 **ppbData)
{
	switch (pSetup->bRequest) {

	// set line coding
	case SET_LINE_CODING:
DBG("SET_LINE_CODING\n");
		memcpy((u8 *)&LineCoding, *ppbData, 7);
		*piLen = 7;
DBG("dwDTERate=%u, bCharFormat=%u, bParityType=%u, bDataBits=%u\n",
	LineCoding.dwDTERate,
	LineCoding.bCharFormat,
	LineCoding.bParityType,
	LineCoding.bDataBits);
		break;

	// get line coding
	case GET_LINE_CODING:
DBG("GET_LINE_CODING\n");
		*ppbData = (u8 *)&LineCoding;
		*piLen = 7;
		break;

	// set control line state
	case SET_CONTROL_LINE_STATE:
		// bit0 = DTR, bit = RTS
DBG("SET_CONTROL_LINE_STATE %X\n", pSetup->wValue);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/**
	Initialises the VCOM port.
	Call this function before using VCOM_putchar or VCOM_getchar
 */
void VCOM_init(void)
{
	fifo_init(&txfifo, txdata);
	fifo_init(&rxfifo, rxdata);
}


/**
	Writes one character to VCOM port

	@param [in] c character to write
	@returns character written, or EOF if character could not be written
 */
int VCOM_putchar(int c)
{
	return fifo_put(&txfifo, c) ? c : EOF;
}


/**
	Reads one character from VCOM port

	@returns character read, or EOF if character could not be read
 */
int VCOM_getchar(void)
{
	u8 c;

	return fifo_get(&rxfifo, &c) ? c : EOF;
}


/**
	Interrupt handler

	Simply calls the USB ISR

	This gets installed by overriding a WEAK linker symbol

 */
void USB_IRQHandler(void)
{
	USBHwISR();
}


static void USBFrameHandler(u16 wFrame)
{
	if (fifo_avail(&txfifo) > 0) {
		// data available, enable NAK interrupt on bulk in
		USBHwNakIntEnable(INACK_BI);
	}
}

void platform_setup_usb_cdc(void)
{
	int c;

	printf("Initialising USB stack\n");

	// initialise stack
	USBInit();

	// register descriptors
	USBRegisterDescriptors(abDescriptors);

	// register class request handler
	USBRegisterRequestHandler(REQTYPE_TYPE_CLASS, HandleClassRequest, abClassReqData);

	// register endpoint handlers
	USBHwRegisterEPIntHandler(INT_IN_EP, NULL);
	USBHwRegisterEPIntHandler(BULK_IN_EP, BulkIn);
	USBHwRegisterEPIntHandler(BULK_OUT_EP, BulkOut);

	// register frame handler
	USBHwRegisterFrameHandler(USBFrameHandler);

	// enable bulk-in interrupts on NAKs
	USBHwNakIntEnable(INACK_BI);

	// initialise VCOM
	VCOM_init();
	printf("Starting USB communication\n");

	// enable IRQ
	NVIC_EnableIRQ(USB_IRQn);

	// connect to bus

	printf("Connecting to USB bus\n");
	USBHwConnect(TRUE);
}

#endif
