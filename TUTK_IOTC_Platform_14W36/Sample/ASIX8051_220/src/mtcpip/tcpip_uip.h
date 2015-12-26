/*
 ******************************************************************************
 *     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name:tcpip_uip.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: tcpip_uip.h,v $
 *=============================================================================
 */

#ifndef __TCPIP_UIP_H__
#define __TCPIP_UIP_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define TCPIP_STATE_FREE			0
#define TCPIP_STATE_WAIT_CONNECT	1
#define TCPIP_STATE_CONNECT			2
#define TCPIP_STATE_WAIT_FIN		3
#define TCPIP_STATE_LAST_ACK		4

#define TCPIP_KEEPALIVE_PERIOD		(240 * 2) /* 2 mimute */
#define TCPIP_CONN_TIMEOUT			(240 * 8) /* 8 minute */

/* UIP_APPCALL: the name of the application function. This function
   must return void and take no arguments (i.e., C type "void
   appfunc(void)"). */
#ifndef UIP_APPCALL
#define UIP_APPCALL()	TCPIP_CallTcpApp()
#endif

/* TYPE DECLARATIONS */
/*-------------------------------------------------------------*/
typedef struct _TCPIP_APPLICATION
{
	U8_T	Valid;
	U8_T	(*NewConnHandle)(U32_T XDATA*, U16_T, U8_T);
	void	(*EventHandle)(U8_T, U8_T);
	void	(*ReceiveHandle)(U8_T XDATA*, U16_T, U8_T);

} TCPIP_APPLICATION;

/*-------------------------------------------------------------*/
typedef struct _TCPIP_LISTEN
{
	U16_T	LocalPort;
	U8_T	InterfaceId;/* TCPIP_APPLICATION Connect ID */

} TCPIP_LISTEN;

/*-------------------------------------------------------------*/
typedef struct _TCPIP_UDP_CONNECT
{
	U8_T	State;
	U8_T	InterfaceId;/* TCPIP_APPLICATION Connect ID */
	U8_T	AppId;		/* Uplayer Connect ID */
	U32_T	Ip;
	U16_T	LocalPort;
	U16_T	RemotePort;
	U8_T	HeadLen;
	U16_T	PayloadLen;
	U8_T*	PHead;		/* point to up-layer protocol header */
	U8_T*	PPayload;	/* point to up_layer protocol payload */
	void XDATA*	PSocket;/* UDP socket */

} TCPIP_UDP_CONNECT;

/*-------------------------------------------------------------*/
typedef struct _TCPIP_TCP_CONNECT
{
	U8_T	State;
	U8_T	InterfaceId;/* TCPIP_APPLICATION Connect ID */
	U8_T	AppId;		/* Uplayer Connect ID */
	U8_T	FinalFlag;
	U32_T	Ip;
	void XDATA*	PSocket;
	U16_T	XmitLen0;
	U16_T	XmitLen1;
	U16_T	XmitLen2;
	U16_T	XmitLen3;
	U16_T	LeftLen;
	U8_T* 	PData0;
	U8_T* 	PData1;
	U8_T* 	PData2;
	U8_T* 	PData3;
	U8_T* 	PNextData;
	U8_T	Reserved;

} TCPIP_TCP_CONNECT;

/* GLOBAL VARIABLES */
extern U8_T TCPIP_PppoeHeaderLen;
extern U8_T TCPIP_IsPppoe;
extern U8_T TCPIP_DontFrag;
extern U8_T TCPIP_EthHeadLen;
extern U16_T TCPIP_MssLen;
extern void (*TCPIP_PingRespCallBack)(void);


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void TCPIP_CallTcpApp(void);
void TCPIP_CallUdpApp(void);


#endif /* End of __TCPIP_UIP_H__ */


/* End of tcpip_uip.h */