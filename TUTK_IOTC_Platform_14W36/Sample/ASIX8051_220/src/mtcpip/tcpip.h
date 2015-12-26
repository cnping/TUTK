/*
 ******************************************************************************
 *     Copyright (c) 2010	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name:tcpip.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: tcpip.h,v $
 *=============================================================================
 */

#ifndef __TCPIP_H__
#define __TCPIP_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define TCPIP_MAX_APP_CONN			16
#define TCPIP_MAX_TCP_LISTEN		12
#define TCPIP_MAX_UDP_LISTEN		12
#define TCPIP_MAX_TCP_CONN			16
#define TCPIP_MAX_UDP_CONN			16

#define TCPIP_NO_NEW_CONN			0xff

#define	TCPIP_CONNECT_CANCEL		0
#define TCPIP_CONNECT_WAIT			1
#define TCPIP_CONNECT_ACTIVE		2
#define TCPIP_CONNECT_XMIT_COMPLETE	3
#define TCPIP_CONNECT_BUSY			0xf1

#define TCPIP_SEND_NOT_FINAL		0
#define TCPIP_SEND_FINAL			1
#define TCPIP_SEND_NOT_PUSH			2

#define TCPIP_KEEPALIVE_ON			1
#define TCPIP_KEEPALIVE_OFF			0

#define TCPIP_DONT_FRAGMENT			1
#define TCPIP_FRAGMENT				0

#define TCPIP_PROTOCOL_ICMP			1
#define TCPIP_PROTOCOL_TCP			6
#define TCPIP_PROTOCOL_UDP			17

#define TCPIP_NON_TRANSPARENT_MODE	0
#define TCPIP_TRANSPARENT_MODE		1

#define TCPIP_TIME_TO_LIVE			255	/* TTL item of ip layer */

/* GLOBAL VARIABLES */

/* for uip */
extern U8_T xdata uip_buf[];
extern volatile U16_T idata uip_len;
extern volatile U8_T *uip_appdata;
extern U8_T xdata uip_getgatewaymacaddrflag;

#define TCPIP_GetRcvBuffer()		(uip_buf)
#define TCPIP_GetXmitBuffer()		(uip_buf)
#define TCPIP_SetRcvLength(len)		(uip_len = len)
#define TCPIP_SetXmitLength(len)	(uip_len = len)
#define TCPIP_GetXmitLength()		(uip_len)
#define TCPIP_GetPayloadBuffer()	(uip_appdata)
#define TCPIP_GetGwMacAddrFlag()	(uip_getgatewaymacaddrflag)
#define TCPIP_SetGwMacAddrFlag(a)	(uip_getgatewaymacaddrflag = a)

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void TCPIP_Init(U8_T);
U8_T TCPIP_Bind(U8_T (* )(U32_T XDATA*, U16_T, U8_T), void (* )(U8_T, U8_T), void (* )(U8_T XDATA*, U16_T, U8_T));
void TCPIP_Unbind(U8_T);

/* for tcp */
U8_T TCPIP_TcpListen(U16_T, U8_T);
void TCPIP_TcpUnlisten(U16_T);
U8_T TCPIP_TcpNew(U8_T, U8_T, U32_T, U16_T, U16_T);
void TCPIP_TcpConnect(U8_T);
void TCPIP_TcpKeepAlive(U8_T, U8_T);
void TCPIP_TcpClose(U8_T);
U16_T TCPIP_QueryTcpLocalPort(U8_T);
void TCPIP_TcpSend(U8_T, U8_T*, U16_T, U8_T);
void TCPIP_TcpSetTos(U8_T, U8_T);

/* for udp */
U8_T TCPIP_UdpListen(U16_T, U8_T);
void TCPIP_UdpUnlisten(U16_T);
U8_T TCPIP_UdpNew(U8_T, U8_T, U32_T, U16_T, U16_T);
void TCPIP_UdpClose(U8_T);
U16_T TCPIP_QueryUdpLocalPort(U8_T);
void TCPIP_UdpSend(U8_T, U8_T*,U8_T, U8_T*, U16_T);
void TCPIP_UdpSetTos(U8_T, U8_T);

/* for other purpose */
void TCPIP_AssignLowlayerXmitFunc(void (* )(U16_T));
void TCPIP_AssignPingRespFunc(void (* )(void));
void TCPIP_PeriodicCheck(void);
void TCPIP_SetPppoeMode(U8_T);
U32_T TCPIP_GetIPAddr(void);
U32_T TCPIP_GetSubnetMask(void);
U32_T TCPIP_GetGateway(void);
void TCPIP_SetIPAddr(U32_T);
void TCPIP_SetSubnetMask(U32_T);
void TCPIP_SetGateway(U32_T);
void TCPIP_SetMacAddr(U8_T*);
void TCPIP_Receive(void);
void TCPIP_DontFragment(U8_T);

/* for uip */
U16_T htons(U16_T);

/* for uip_arp */
void uip_arp_ipin(void);
void uip_arp_arpin(void);
void uip_arp_out(void);
U8_T uip_findarptable(U16_T*);

/* for debug */
//void TCPIP_Debug(void);

#endif /* End of __TCPIP_H__ */


/* End of tcpip.h */