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
 /*============================================================================
 * Module name: smtpc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: smtpc.h,v $
 *
 *=============================================================================
 */

#ifndef __SMTPC_H__
#define __SMTPC_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "CMIME64.h"

/* NAMING CONSTANT DECLARATIONS */
#define SMTP_SERVER_PORT		25
#define SMTP_MAX_LENGTH			76

/*-------------------------------------------------------------*/
/* NAMING CONSTANT DECLARATIONS */
#define SMTP_STATE_NONE			0
#define SMTP_STATE_INITIAL		1
#define SMTP_STATE_CONNECTED	2  
#define SMTP_STATE_HELO_SENT	3
#define SMTP_STATE_FROM_SENT	4
#define SMTP_STATE_RCV1_SENT	5
#define SMTP_STATE_RCV2_SENT	6
#define SMTP_STATE_RCV3_SENT	7
#define SMTP_STATE_DATA_SENT	8
#define SMTP_STATE_WAIT_MESSAGE	9
#define SMTP_STATE_SEND_MESSAGE	10
#define SMTP_STATE_MESSAGE_SENT	11
#define SMTP_STATE_QUIT_SENT	12

/* TYPE DECLARATIONS */
typedef struct _SMTPC_CONN
{
	U8_T	State;
	U8_T	TcpSocket;
	U32_T	ServerIp;
	U8_T*	From;
	U8_T*	To1;
	U8_T*	To2;
	U8_T*	To3;

} SMTPC_CONN;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void SMTPC_Init(U8_T *pBuf, U8_T *pSubject);
void SMTPC_Event(U8_T, U8_T);
void SMTPC_Receive(U8_T XDATA*, U16_T, U8_T);
void SMTPC_Start(U32_T, U8_T*, U8_T*, U8_T*, U8_T*);
void SMTPC_SendMessage(U8_T*, U16_T);
U8_T SMTPC_GetState(void);


#endif /* __SMTPC_H__ */

