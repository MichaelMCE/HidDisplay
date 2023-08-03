/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once


#include "usb_desc.h"

#define PACKET_SIZE			(512)
#define TRANSMIT_BUFFERS	(8)			// total buffer size = TRANSMIT_BUFFERS * PACKET_SIZE
#define RECIEVE_BUFFERS		(511)		// total buffer size = RECIEVE_BUFFERS * PACKET_SIZE

#ifndef RAWHID_TX_ENDPOINT
#define RAWHID_TX_ENDPOINT	(0x03)		//	0x83
#endif

#ifndef RAWHID_RX_ENDPOINT
#define RAWHID_RX_ENDPOINT	(0x04)
#endif

#ifndef RAWHID_RX_SIZE_480
#define RAWHID_RX_SIZE_480	PACKET_SIZE
#endif

#ifndef RAWHID_TX_SIZE_480
#define RAWHID_TX_SIZE_480	PACKET_SIZE
#endif


//#if defined(RAWHID_INTERFACE)

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif



void usb_rawhid_configure (void);

int usb_rawhid_recv (void *buffer, uint32_t timeout);
int usb_rawhid_recv2 (void **buffer, uint32_t timeout);

int usb_rawhid_send (const void *buffer, uint32_t timeout);

//int usb_rawhid_rxSize (void);
//int usb_rawhid_txSize (void);

int usb_rawhid_available (void);

#ifdef __cplusplus
}
#endif


// C++ interface
#ifdef __cplusplus
class usb_rawhid_classex
{
public:
	int available (void) {return usb_rawhid_available(); }

   // int rxSize (void) {return usb_rawhid_rxSize(); }
   // int txSize (void) {return usb_rawhid_txSize(); }
    
   	int send (const void *buffer, uint16_t timeout) { return usb_rawhid_send(buffer, timeout); }
    
	int recv  (void  *buffer, uint16_t timeout) { return usb_rawhid_recv (buffer, timeout); }
    int recv2 (void **buffer, uint16_t timeout) { return usb_rawhid_recv2(buffer, timeout); }
    
};

#endif // __cplusplus

//#endif // RAWHID_INTERFACE
