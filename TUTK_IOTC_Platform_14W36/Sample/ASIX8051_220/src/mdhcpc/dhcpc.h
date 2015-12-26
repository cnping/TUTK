/*
 *********************************************************************************
 *     Copyright (c) 2010	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : uip_dhcp.h
 * Purpose     : A header file of DHCP module over uIP.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * $Log: dhcpc.h,v $
 * no message
 *
 *================================================================================
 */
#ifndef UIP_DHCP_H
#define UIP_DHCP_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"


/* MACRO DECLARATIONS */
#define		DHCP_CHADDR_LEN				16
#define		DHCP_SNAME_LEN				64
#define		DHCP_FILE_LEN				128
#define		DHCP_OPTIONS_LEN			312
#define		DHCP_MAGIC_COOKIE_LEN		4
#define		DHCP_FIXED_LEN				236

#define		DHCP_REQUEST_TIMEOUT		(2 * SWTIMER_COUNT_SECOND)
#define		DHCP_MAX_RETRY				6

#define		DHCP_CLIENT_XID				0x12340000
#define		DHCP_CLIENT_BROADCAST		0xFFFFFFFF

#define		DHCP_IDLE_STATE				0
#define		DHCP_INITIAL_STATE			BIT0
#define		DHCP_DISCOVER_STATE			BIT1
#define		DHCP_RCV_OFFER_STATE		BIT2
#define		DHCP_REQUEST_STATE			BIT3
#define		DHCP_RCV_ACK_STATE			BIT4
#define		DHCP_DISCOVER_TX_STATE		BIT5
#define		DHCP_REQUEST_TX_STATE		BIT6

#define		DHCP_CLIENT_PORT			68
#define		DHCP_SERVER_PORT			67

#define		DHCP_C_TO_S					1
#define		DHCP_S_TO_C					2
#define		DHCP_HTYPE_ETHERNET			1
#define		DHCP_HLEN_ETHERNET			6
#define		DHCP_SERVER_BROADCAST		1
#define		DHCP_MSG_LEN				236

#define		DHCP_DISCOVER				1
#define		DHCP_OFFER					2
#define		DHCP_REQUEST				3
#define		DHCP_DECLINE				4
#define		DHCP_ACK					5
#define		DHCP_NAK					6
#define		DHCP_RELEASE				7

#define		DHCP_OPTION_SUBNET_MASK		1
#define		DHCP_OPTION_ROUTER			3
#define		DHCP_OPTION_DNS_SERVER		6
#define		DHCP_OPTION_REQ_IPADDR		50
#define		DHCP_OPTION_MSG_TYPE		53
#define		DHCP_OPTION_SERVER_ID		54
#define		DHCP_OPTION_PAR_REQ_LIST	55
#define		DHCP_OPTION_END				255

#define		DHCP_FIXED_FORMAT_LEN		240
#define		DHCP_MIN_OPTIONS_LEN		64

#define		IP_HEADER_LEN				20
#define		UDP_HEADER_LEN				8

/* TYPE DECLARATIONS */
typedef struct _dhcp_header {
	U8_T	Op;
    U8_T	Htype;
    U8_T	Hlen;
    U8_T	Hops;
    U32_T	Xid;
    U16_T	Seconds;
    U16_T	Flags;
    U32_T	CiAddr;
    U32_T	YiAddr;
    U32_T	SiAddr;
    U32_T	GiAddr;
    U8_T	ChAddr[DHCP_CHADDR_LEN];
    U8_T	Sname[DHCP_SNAME_LEN];
    U8_T	File[DHCP_FILE_LEN];
    U8_T	Options[DHCP_OPTIONS_LEN];
}	DHCP_HEADER;

typedef struct _dhcp_info{
	S8_T		State;
	U8_T		AppId;
	U8_T		SocketId;
	DHCP_HEADER	*dhcpPktPtr;
	U8_T		*pOpt;
	U16_T		OptionsCounter;
	U16_T		Secs;
	U16_T		TimeoutInterval; // second
	U8_T		ServerId[4];
	U8_T		ClientOfferedIp[4];
	U32_T		IpAddr;
	U32_T		NetMask;
	U32_T		DnsAddr;
	U32_T		DefaultRouter;
} DHCP_INFO;


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
BOOL DHCP_Init(void);
void DHCP_Start(void);
void DHCP_Send(void);
S8_T DHCP_GetState(void);
void DHCP_EventHandle(U8_T, U8_T);
void DHCP_ReceiveHandle(U8_T XDATA*, U16_T, U8_T);
void DHCP_RejectOffer(void);
void DHCP_ResetState(void);

#endif /* End of UIP_DHCP_H */