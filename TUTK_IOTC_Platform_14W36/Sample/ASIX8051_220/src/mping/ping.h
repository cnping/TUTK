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
 * Module Name:ping.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: ping.h,v $
 *
 *=============================================================================
 */

#ifndef __PING_H__
#define __PING_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */

/* TYPE DECLARATIONS */
typedef struct _PING_CONFIG_SET
{
	U32_T TargetIP;
	U16_T PatternLen;
	U16_T EchoTime;		
	U8_T TTL;
	U8_T TxCount;
	U8_T RxCount;
	
	U16_T PingID;
	U16_T Sequence;	
} tsPING_CONFIG_SET;

struct _PING_LINK
{
	struct _PING_LINK *pPre;
	struct _PING_LINK *pNext;
	U16_T WaitTime;
	U16_T ReplyLen;
	U8_T Buf[1];
};
typedef struct _PING_LINK tsPING_LINK;

/* GLOBAL VARIABLES */
extern U8_T XDATA PingFlag;/* for mwif compatable */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void PING_Init(void);
void PING_Task(void);
S8_T PING_CmdEnQueue(tsPING_LINK *pEntity);
S8_T PING_CmdDeQueue(tsPING_LINK *pEntity);

#endif /* End of __PING_H__ */

/* End of ping.h */
