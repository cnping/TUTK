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
 * Module Name : dhcps.h
 * Purpose     : A header file of DHCP Server module.
 * Author      : 
 * Date        :
 * Notes       :
 * $Log: dhcps.h,v $
 *
 *================================================================================
 */
#ifndef DHCP_SERVER_H
#define DHCP_SERVER_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "main.h"
#if (INCLUDE_DHCP_SERVER)
/* NAMING CONSTANT DECLARATIONS */
#define 	DHCPS_MAX_STA_CONNS			4
#define		DHCPS_MAX_IP_POOL_NUM		20
#define		DHCPS_CHADDR_LEN			16
#define		DHCPS_SNAME_LEN				64
#define		DHCPS_FILE_LEN				128
#define		DHCPS_OPTIONS_LEN			312
#define		DHCPS_OFFSET_CHADDR			28
#define		DHCPS_OFFSET_SNAME			(DHCPS_OFFSET_CHADDR + DHCPS_CHADDR_LEN)
#define		DHCPS_OFFSET_FILE			(DHCPS_OFFSET_SNAME + DHCPS_SNAME_LEN)
#define		DHCPS_OFFSET_OPTIONS		(DHCPS_OFFSET_FILE + DHCPS_FILE_LEN)

#define		DHCPS_CONNECTION_TIMEOUT	(10 * SWTIMER_COUNT_SECOND)

#define		DHCPS_BROADCAST				0xFFFFFFFF

#define		DHCPS_STATE_IDLE			0
//#define		DHCPS_STATE_INITIAL			1
#define		DHCPS_STATE_ACTIVE			2
#define		DHCPS_STATE_DISCOVER		3
#define		DHCPS_STATE_OFFER			4
#define		DHCPS_STATE_REQUEST			5
#define		DHCPS_STATE_ACK				6

#define		DHCPS_CLIENT_PORT			68
#define		DHCPS_SERVER_PORT			67

#define		DHCP_C_TO_S					1
#define		DHCP_S_TO_C					2

#define		DHCPS_DISCOVER				1
#define		DHCPS_OFFER					2
#define		DHCPS_REQUEST				3
#define		DHCPS_DECLINE				4
#define		DHCPS_ACK					5
#define		DHCPS_NAK					6

#define		DHCPS_OPTION_SUBNET_MASK	1
#define		DHCPS_OPTION_ROUTER			3
#define		DHCPS_OPTION_DNS_SERVER		6
#define		DHCPS_OPTION_REQ_IPADDR		50
#define		DHCPS_OPTION_LEASE_TIME		51
#define		DHCPS_OPTION_MSG_TYPE		53
#define		DHCPS_OPTION_SERVER_ID		54
#define		DHCPS_OPTION_PAR_REQ_LIST	55
#define		DHCPS_OPTION_END			255

#define		DHCPS_MAC_ADDRESS_LEN		6
#define		DHCPS_IP_ADDRESS_LEN		4

/* TYPE DECLARATIONS */
typedef struct _DHCPS_HEADER
{
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
	U8_T	ChAddr[DHCPS_CHADDR_LEN];
	U8_T	Sname[DHCPS_SNAME_LEN];
	U8_T	File[DHCPS_FILE_LEN];
	U8_T	Options[DHCPS_OPTIONS_LEN];

} DHCPS_HEADER;

/*-------------------------------------------------------------*/
typedef struct _DHCP_SERVER_IP_POOL_TABLE
{
	U32_T	IpAddr;
    U32_T   Lease;
	U8_T	Occupy;
	U8_T	MacAddr[DHCPS_MAC_ADDRESS_LEN];

} DHCPS_POOL;

typedef struct _DHCP_SERVER_CONN_INFORMATION
{
	U16_T	Timer;
	U32_T	Xid;
	U8_T	ChMacAddr[DHCPS_MAC_ADDRESS_LEN];
	U8_T	IpPoolIndex;
	U8_T	State;

} DHCPS_CONNINFO;

/*-------------------------------------------------------------*/
typedef struct _DHCP_SERVER_CONN
{
	U32_T	Ip;
	U8_T	State;
	U8_T	SocketId;

} DHCPS_CONN;


/* MACRO DECLARATIONS */

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
BOOL DHCPS_Init(U32_T startIp, U32_T endIp, U32_T netmask, U32_T gateway, U16_T lease, U8_T status);
U8_T DHCPS_NewConnHandle(U32_T XDATA*, U16_T, U8_T);
void DHCPS_EventHandle(U8_T, U8_T);
void DHCPS_ReceiveHandle(U8_T XDATA*, U16_T, U8_T);
void DHCPS_ConnsCheck(void);
void DHCPS_GetServerSettings(U32_T *pStartIp, U32_T *pEndIp, U32_T *pNetmask, U32_T *pGateway, U16_T *pLease);
void DHCPS_SetServerSettings(U32_T startIp, U32_T endIp, U32_T netmask, U32_T gateway, U16_T lease);
void DHCPS_SetServerStatus(U8_T status);
U8_T DHCPS_GetServerStatus(void);
void DHCPS_SetDnsServer(U32_T dnsSrv);

#endif /* INCLUDE_DHCP_SERVER */

#endif /* End of DHCP_SERVER_H */