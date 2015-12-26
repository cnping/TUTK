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
 /*============================================================================
 * Module Name: ping.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: ping.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ping.h"
#include "stoe.h"
#include "adapter.h"
#include "tcpip.h"
#include "mstimer.h"
#include <string.h>
/* NAMING CONSTANT DECLARATIONS */
#define PING_PATTERN_DATA_START		(0x61)
#define PING_ICMP_ECHO_TYPE			(0x08)
#define PING_ICMP_ECHO_REPLY_TYPE	(0x00)

#define PING_DEFAULT_TIME_OUT 		(35)//Unit in 50ms
#define MAX_PING_PATTERN_LENGTH 	(64)

/* MACRO DECLARATIONS */

/* TYPE DECLARATIONS */
typedef struct _IP_HEADER
{
	U8_T	VerHlen;
	U8_T	TOS;
	U16_T	Length;
	U16_T	ID;
	U16_T	Frags;
	U8_T	TTL;
	U8_T	Protocol;
	U16_T	CheckSum;
	U32_T	SourIp;
	U32_T	DestIp;
	U8_T	Option_Data[1];
} tsIP_HEADER;

typedef struct _ICMP_HEADER
{
	U8_T	Type;
	U8_T	Code;
	U16_T	CheckSum;
	U16_T	Indentifier;
	U16_T	Sequence;
	U8_T	Data[1];
} tsICMP_HEADER;

typedef struct _PING_LINKLIST
{
	tsPING_LINK *pHead;
	tsPING_LINK *pTail;
}tsPING_LINKLIST;

/* GLOBAL VARIABLES DECLARATIONS */
U8_T XDATA PingFlag;/* for mwif compatable */

/* LOCAL VARIABLES DECLARATIONS */
static tsPING_LINKLIST CmdFIFO, WaitFIFO, ReplyFIFO;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void PING_Send(tsPING_CONFIG_SET *pEntity);
static void PING_CallBack(void);
static U8_T PING_ChkQueueEmpty(tsPING_LINKLIST *pFIFO);
static void PING_EnQueue(tsPING_LINKLIST *pFIFO, tsPING_LINK *pEntity);
static tsPING_LINK *PING_DeQueue(tsPING_LINKLIST *pFIFO);
static tsPING_LINK *PING_DeQueueByOption(tsPING_LINKLIST *pFIFO, tsPING_LINK *pEntity);

/*
 * ----------------------------------------------------------------------------
 * Function Name:void PING_Init()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_Init(void)
{
	memset((U8_T*)&CmdFIFO, 0, sizeof(tsPING_LINKLIST));
	memset((U8_T*)&WaitFIFO, 0, sizeof(tsPING_LINKLIST));
	memset((U8_T*)&ReplyFIFO, 0, sizeof(tsPING_LINKLIST));
	
	TCPIP_AssignPingRespFunc(PING_CallBack);
} /* End of PING_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name:void PING_Send()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_Send(tsPING_CONFIG_SET *pEntity)
{
	U16_T i, j;
	tsIP_HEADER XDATA *pIP;
	tsICMP_HEADER XDATA *pICMP;	

	/* append ICMP header */
	pICMP = (tsICMP_HEADER XDATA*)(TCPIP_GetXmitBuffer() + ETH_HEADER_LEN + 20);
	j = pEntity->PatternLen;
	for (i = 0; i < j; i ++)
	{
		pICMP->Data[i] = (U8_T)(i + PING_PATTERN_DATA_START);
	}
	pICMP->Type = PING_ICMP_ECHO_TYPE;
	pICMP->Code = 0;
	pICMP->CheckSum = 0x0000;
	pICMP->Indentifier = pEntity->PingID;
	pICMP->Sequence = ++(pEntity->Sequence);
	j += 8;//for ICMP header	

	/* append IP header */
	pIP = (tsIP_HEADER XDATA*)(TCPIP_GetXmitBuffer() + ETH_HEADER_LEN);
	
	pIP->VerHlen = 0x45;
	pIP->TOS = 0x00;
	pIP->Length = j + ((pIP->VerHlen & 0x0F)*4);
	pIP->ID = pICMP->Sequence;
	pIP->Frags = 0x0000;
	pIP->TTL = 0x80;
	pIP->Protocol = 0x01;
	pIP->CheckSum = 0x0000;
	pIP->SourIp = STOE_GetIPAddr();
	pIP->DestIp = pEntity->TargetIP;

	TCPIP_SetXmitLength(pIP->Length);
	
	if (TCPIP_GetXmitLength() > 0)
	{
		uip_appdata = (TCPIP_GetXmitBuffer() + ETH_HEADER_LEN + 40);
		uip_arp_out();		
		ETH_Send(0);
	}
} /* End of PING_Send() */

/*
 * ----------------------------------------------------------------------------
 * Function Name:void PING_CallBack()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_CallBack(void)
{
	tsIP_HEADER XDATA *pIP;
	tsICMP_HEADER XDATA *pICMP;	
	tsPING_LINK *pEntity;
	tsPING_CONFIG_SET *pTmp;
	U16_T i;

	if (PING_ChkQueueEmpty(&WaitFIFO))
		return;

	pIP = (tsIP_HEADER XDATA*)(TCPIP_GetRcvBuffer() + ETH_HEADER_LEN);
	pICMP = (tsICMP_HEADER XDATA*)(TCPIP_GetRcvBuffer() + ETH_HEADER_LEN + 20);
	
	// Search entity in wait queue //
	pEntity = WaitFIFO.pHead;
	for (i = 0; i < 100; i ++)
	{
		pTmp = (tsPING_CONFIG_SET *)(pEntity->Buf);
		if (pIP->SourIp == pTmp->TargetIP &&
			pICMP->Type == PING_ICMP_ECHO_REPLY_TYPE &&
			pICMP->Sequence == pTmp->Sequence &&
			pICMP->Indentifier == pTmp->PingID)
			break;
			
		if (pEntity != WaitFIFO.pTail)
			pEntity = pEntity->pNext;
		else
			return;
	}
	if (i == 100)
		return;
	
	for (i = 0; i < pTmp->PatternLen; i ++)
	{
		if (pICMP->Data[i] != (U8_T)(i + PING_PATTERN_DATA_START))
			return;
	}	
	
	// Mission complete //
	pTmp->EchoTime = PING_DEFAULT_TIME_OUT - pEntity->WaitTime;	
	pTmp->TTL = pIP->TTL;
	pTmp->RxCount ++;
	
	pEntity->ReplyLen = sizeof(tsPING_CONFIG_SET);
	pEntity = PING_DeQueueByOption(&WaitFIFO, pEntity);
	if (pEntity)
		PING_EnQueue(&ReplyFIFO, pEntity);
	
} /* End of PING_CallBack() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_ChkQueueEmpty()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T PING_ChkQueueEmpty(tsPING_LINKLIST *pFIFO)
{
	return ((pFIFO->pHead == 0 && pFIFO->pTail == 0) ? 1:0);
} // End of PING_ChkQueueEmpty() // 

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_EnQueue()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_EnQueue(tsPING_LINKLIST *pFIFO, tsPING_LINK *pEntity)
{
	if (PING_ChkQueueEmpty(pFIFO))
	{
		pFIFO->pHead = pEntity;
		pFIFO->pTail = pEntity;
	}
	else
	{
		pEntity->pPre = pFIFO->pTail;
		pFIFO->pTail->pNext = pEntity;
		pFIFO->pTail = pEntity;		
	}
} // End of PING_EnQueue() // 

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_DeQueue()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
tsPING_LINK *PING_DeQueue(tsPING_LINKLIST *pFIFO)
{
	tsPING_LINK *pTmp;
	
	if (PING_ChkQueueEmpty(pFIFO))
		return 0;
		
	pTmp = pFIFO->pHead;
	if (pFIFO->pHead == pFIFO->pTail)
	{
		pFIFO->pHead = 0;
		pFIFO->pTail = 0;
	}
	else
	{
		pFIFO->pHead = pFIFO->pHead->pNext;
	}
	return pTmp;		
} // End of PING_DeQueue() //

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_DeQueueByOption()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
tsPING_LINK *PING_DeQueueByOption(tsPING_LINKLIST *pFIFO, tsPING_LINK *pEntity)
{
	tsPING_LINK *pTmp;
	U8_T i;
	
	if (PING_ChkQueueEmpty(pFIFO))
		return 0;
		
	pTmp = pFIFO->pHead;
	for (i = 0; i < 100; i ++)
	{		
		if (pTmp == pEntity)
		{
			if (pFIFO->pHead == pFIFO->pTail)
			{
				pFIFO->pHead = 0;		
				pFIFO->pTail = 0;
				return pTmp;
			}
			else if (pTmp == pFIFO->pHead)
			{
				pFIFO->pHead = pFIFO->pHead->pNext;
				return pTmp;
			}
			else if (pTmp == pFIFO->pTail)
			{
				pFIFO->pTail = pFIFO->pTail->pPre;
				return pTmp;
			}
			else
			{
				pTmp->pPre->pNext = pTmp->pNext;
				pTmp->pNext->pPre = pTmp->pPre;
				return pTmp;			
			}
		}
		if (pTmp != pFIFO->pTail)
			pTmp = pTmp->pNext;		
		else
			break;
	}
	return 0;
} // End of PING_DeQueueByOption() //

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_CmdEnQueue()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S8_T PING_CmdEnQueue(tsPING_LINK *pEntity)
{
	static U16_T PingID;
	
	tsPING_CONFIG_SET *pCfig = pEntity->Buf;
	
	if (pCfig->PatternLen > MAX_PING_PATTERN_LENGTH)
		pCfig->PatternLen = MAX_PING_PATTERN_LENGTH;
		
	pEntity->WaitTime = PING_DEFAULT_TIME_OUT;
	pEntity->ReplyLen = 0;

	pCfig->TTL = 0;
	pCfig->PingID = ++PingID;
	
	PING_EnQueue(&CmdFIFO, pEntity);
	return 0;
} // End of PING_CmdEnQueue() //

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_CmdDeQueue()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S8_T PING_CmdDeQueue(tsPING_LINK *pEntity)
{
	if (PING_DeQueueByOption(&ReplyFIFO, pEntity))
		return 0;
	else
		return -1;
} // End of PING_CmdDeQueue() //

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_Task()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_Task(void)
{
 	static U32_T Tp = 0, Ts = 0;
	tsPING_LINK *pEntity, *pEntityTmp;
	U8_T i, ExitFlag;
		
	// Check command queue and send command //
	if (!PING_ChkQueueEmpty(&CmdFIFO))
	{
		PING_Send((tsPING_CONFIG_SET *)&(CmdFIFO.pHead->Buf));
		pEntity = PING_DeQueue(&CmdFIFO);
		PING_EnQueue(&WaitFIFO, pEntity);	
	}
			
	Tp = SWTIMER_Tick();
	if (Ts != Tp)
	{
		Ts = Tp;
		
		// Check wait queue timeout //
		if (!PING_ChkQueueEmpty(&WaitFIFO))
		{
			pEntity = WaitFIFO.pHead;
			ExitFlag = 0;
			for (i = 0; i < 100; i ++)
			{
				if (pEntity->WaitTime)
					pEntity->WaitTime --;
				
				pEntityTmp = pEntity;
				if (pEntity != WaitFIFO.pTail)
					pEntity = pEntity->pNext;
				else
					ExitFlag = 1;
					
				if (pEntityTmp->WaitTime == 0)
				{
					pEntityTmp = PING_DeQueueByOption(&WaitFIFO, pEntityTmp);
					if (pEntityTmp)
						PING_EnQueue(&ReplyFIFO, pEntityTmp);				
				}
				
				if (ExitFlag)
					break;
			}
		}
		
		// Reply queue timer countdown //
		if (!PING_ChkQueueEmpty(&ReplyFIFO))
		{
			pEntity = ReplyFIFO.pHead;
			for (i = 0; i < 100; i ++)
			{
				if (pEntity->WaitTime)
					pEntity->WaitTime --;		
				if (pEntity != ReplyFIFO.pTail)
					pEntity = pEntity->pNext;
				else
					break;
			}		
		}
	}
} /* End of PING_Task() */

/* End of ping.c */


