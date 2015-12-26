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
 * Module Name: TELNET
 * Purpose: The purpose of this package provides the services to TELNET
 * Author:
 * Date:
 * Notes:
 * $Log$
*=============================================================================
*/

#ifndef __TELNET_H__
#define __TELNET_H__
#if (INCLUDE_TELNET_SERVER)
/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define MAX_TELNET_CONNECT 1
#define DATABUFSIZE  128
#define IACBUFSIZE   128
/*
 * Definitions for the TELNET protocol.
 */
#define	IAC	  255		/* interpret as command: */
#define	DONT  254		/* you are not to use option */
#define	DO    253		/* please, you use option */
#define	WONT  252		/* I won't use option */
#define	WILL  251		/* I will use option */
#define	SB    250		/* interpret as subnegotiation */
#define	GA    249		/* you may reverse the line */
#define	EL    248		/* erase the current line */
#define	EC    247		/* erase the current character */
#define	AYT   246		/* are you there */
#define	AO    245		/* abort output--but let prog finish */
#define	IP    244		/* interrupt process--permanently */
#define	BREAK 243		/* break */
#define	DM    242		/* data mark--for connect. cleaning */
#define	NOP	  241		/* nop */
#define	SE    240		/* end sub negotiation */
#define EOR   239             /* end of record (transparent mode) */
#define	ABORT 238		/* Abort process */
#define	SUSP  237		/* Suspend process */
#define	xEOF  236		/* End of file: EOF is already used... */
#define SYNCH 242		/* for telfunc calls */

/* telnet options */
#define TELOPT_BINARY 0	/* 8-bit data path */
#define TELOPT_ECHO   1	/* echo */
#define	TELOPT_RCP    2	/* prepare to reconnect */
#define	TELOPT_SGA    3	/* suppress go ahead */
#define	TELOPT_NAMS   4	/* approximate message size */
#define	TELOPT_STATUS 5	/* give status */
#define	TELOPT_TM     6 /* timing mark */
#define	TELOPT_RCTE	  7 /* remote controlled transmission and echo */
#define TELOPT_NAOL   8 /* negotiate about output line width */
#define TELOPT_NAOP   9 /* negotiate about output page size */
#define TELOPT_NAOCRD 10 /* negotiate about CR disposition */
#define TELOPT_NAOHTS 11 /* negotiate about horizontal tabstops */
#define TELOPT_NAOHTD 12 /* negotiate about horizontal tab disposition */
#define TELOPT_NAOFFD 13 /* negotiate about formfeed disposition */
#define TELOPT_NAOVTS 14 /* negotiate about vertical tab stops */
#define TELOPT_NAOVTD 15 /* negotiate about vertical tab disposition */
#define TELOPT_NAOLFD 16 /* negotiate about output LF disposition */
#define TELOPT_XASCII 17 /* extended ascii character set */
#define	TELOPT_LOGOUT 18 /* force logout */
#define	TELOPT_BM     19 /* byte macro */
#define	TELOPT_DET    20 /* data entry terminal */
#define	TELOPT_SUPDUP 21 /* supdup protocol */
#define	TELOPT_SUPDUPOUTPUT 22 /* supdup output */
#define	TELOPT_SNDLOC 23 /* send location */
#define	TELOPT_TTYPE  24 /* terminal type */
#define	TELOPT_EOR    25 /* end or record */
#define	TELOPT_TUID   26 /* TACACS user identification */
#define	TELOPT_OUTMRK 27 /* output marking */
#define	TELOPT_TTYLOC 28 /* terminal location number */
#define	TELOPT_3270REGIME 29 /* 3270 regime */
#define	TELOPT_X3PAD      30 /* X.3 PAD */
#define	TELOPT_NAWS       31 /* window size */
#define	TELOPT_TSPEED     32 /* terminal speed */
#define	TELOPT_LFLOW      33 /* remote flow control */
#define TELOPT_LINEMODE   34 /* Linemode option */
#define TELOPT_XDISPLOC   35 /* X Display Location */
#define TELOPT_OLD_ENVIRON 36 /* Old - Environment variables */
#define	TELOPT_AUTHENTICATION 37 /* Authenticate */
#define	TELOPT_ENCRYPT     38 /* Encryption option */
#define TELOPT_NEW_ENVIRON 39 /* New - Environment variables */
#define	TELOPT_EXOPL 255 /* extended-options-list */

enum {
    TS_CR,
    TS_DATA,
    TS_IAC,
    TS_OPT,
    TS_SUB1,
    TS_SUB2,
};

enum TELNET_STATE_Ind 
{
    TELNET_STATE_FREE,
    TELNET_STATE_ACTIVE,
    TELNET_STATE_CONNECTING,
    TELNET_STATE_CONNECTED,
    TELNET_STATE_CLOSING
};

/* TYPE DECLARATIONS */
typedef struct _TELNET_CONTROL
{
    /* same buffer used both for network and console read/write */
    U8_T TelState; /* telnet negotiation state from network input */
    U8_T TelWish;  /* DO, DONT, WILL, WONT */
    U8_T TelFlags;
    U8_T SubState;
    /* buffer to handle telnet negotiations */
    U8_T IACBuf[IACBUFSIZE];
    U8_T IACLen; /* could even use byte */
    U8_T Rsvd; /* 16 bit alignment */
} TELNET_CONTROL;

typedef struct _TELNET_SERVER_CONN
{
	U32_T	Ip;
	U16_T	Port;
	U8_T	State;
	U8_T	TcpSocket;
    TELNET_CONTROL TnServer;
} TELNET_SERVER_CONN;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */ 
S8_T TELNET_GetChar(void);
S8_T TELNET_PutChar(S8_T);
void TELNET_NotifyClose(void);
void TELNET_Event(U8_T id, U8_T event);
U8_T TELNET_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket);
void TELNET_Receive(U8_T XDATA* pData, U16_T length, U8_T conn_id);
void TELNET_Init(void);
void TELNET_Task(void);
#endif /* #if (INCLUDE_TELNET_SERVER) */

#endif /* __TELNET_H__ */

/* End of telnet.h */