//*****************************************************************************
//
// uip-conf.h - uIP Project Specific Configuration File
//
// Copyright (c) 2007-2008 Luminary Micro, Inc.  All rights reserved.
// 
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2752 of the Stellaris Peripheral Driver Library.
//
//*****************************************************************************

#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

//
// 8 bit datatype
// This typedef defines the 8-bit type used throughout uIP.
//
typedef unsigned char u8_t;

//
// 16 bit datatype
// This typedef defines the 16-bit type used throughout uIP.
//
typedef unsigned short u16_t;

//
// Statistics datatype
// This typedef defines the dataype used for keeping statistics in
// uIP.
//
typedef unsigned short uip_stats_t;

//
// Ping IP address assignment
// Use first incoming "ping" packet to derive host IP address
//
#define UIP_CONF_PINGADDRCONF       0

// 
// TCP support on or off
//
#define UIP_CONF_TCP                1

//
// UDP support on or off
//
#define UIP_CONF_UDP                1

//
// UDP checksums on or off
// (not currently supported ... should be 0)
//
#define UIP_CONF_UDP_CHECKSUMS      1

//
// UDP Maximum Connections
//
#define UIP_CONF_UDP_CONNS          4

//
// Maximum number of TCP connections.
//
#define UIP_CONF_MAX_CONNECTIONS    4

//
// Maximum number of listening TCP ports.
//
#define UIP_CONF_MAX_LISTENPORTS    4

//
// Size of advertised receiver's window
//
//#define UIP_CONF_RECEIVE_WINDOW     400

//
// Size of ARP table
//
#define UIP_CONF_ARPTAB_SIZE        4

//
// uIP buffer size.
//
#define UIP_CONF_BUFFER_SIZE        1024

//
// uIP statistics on or off
//
#define UIP_CONF_STATISTICS         0

//
// Logging on or off
//
#define UIP_CONF_LOGGING            0

//
// Broadcast Support
//
#define UIP_CONF_BROADCAST          1

//
// Link-Level Header length
//
#define UIP_CONF_LLH_LEN            14

//
// CPU byte order.
//
#define UIP_CONF_BYTE_ORDER         LITTLE_ENDIAN

//
// Here we include the header file for the application we are using in
// this example
#include "elua_uip.h"
#include "dhcpc.h"

//
// Define the uIP Application State type (both TCP and UDP)
//
typedef struct elua_uip_state uip_tcp_appstate_t;
typedef struct dhcpc_state uip_udp_appstate_t;

//
// UIP_APPCALL: the name of the application function. This function
// must return void and take no arguments (i.e., C type "void
// appfunc(void)").
//
#ifndef UIP_APPCALL
#define UIP_APPCALL                 elua_uip_appcall
#endif

#ifndef UIP_ADP_APPCALL
#define UIP_UDP_APPCALL             elua_uip_udp_appcall
#endif

// Added for eLua: DHCP TIMER ID
#define ELUA_DHCP_TIMER_ID          1
#define CLOCK_SECOND                1000000UL

#endif // __UIP_CONF_H_
