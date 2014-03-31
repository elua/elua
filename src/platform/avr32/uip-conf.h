/**
 * uip-conf.h - Project Specific Configuration File
 *
 * uIP has a number of configuration options that can be overridden
 * for each project. These are kept in a project-specific uip-conf.h
 * file and all configuration names have the prefix UIP_CONF.
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * Modified for eLua
 */

#ifndef __UIP_AVR32_CONF_H__
#define __UIP_AVR32_CONF_H__

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
// FIXME: Either above comment is wrong, or following should be 0
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
#define UIP_CONF_BYTE_ORDER         UIP_BIG_ENDIAN

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

#define CLOCK_SECOND                1000000UL

#endif // __UIP_CONF_H_
