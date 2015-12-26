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
 * Module Name: mwifapp.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: mwifapp.c,v $
 * no message
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "mcpu.h"
#include "mwioctl.h"
#include "mwifapp.h"
#include "stoe_cfg.h"
#include "adapter.h"
#include "dhcpc.h"
#if MWIF_TASK_HOOKUP
#include "mstimer.h"
#include <string.h>
#endif

#if IO_CPU_TYPE
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)

#if MWIF_TASK_HOOKUP
/* GLOBAL VARIABLE DECLARATIONS */
MWIFAPP_WiFi_INFO MWIFAPP_WiFiInfo;
MWIFAPP_SITE_SURVEY_REPORT MWIFAPP_SiteSurveyTable[MWIFAPP_MAX_SITESURVEY_TABLE_MEMBER];

/* STATIC VARIABLE DECLARATIONS */
static MWIFAPP_LINKLIST mwifapp_cmdFifo, mwifapp_waitFifo;
static U32_T mwifapp_Tp = 0, mwifapp_Ts = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static U8_T mwifapp_TaskChkQueueEmpty(MWIFAPP_LINKLIST XDATA *pFIFO);
static void mwifapp_TaskEnQueue(MWIFAPP_LINKLIST *pFIFO, MWIFAPP_LINK *pEntity);
static MWIFAPP_LINK *mwifapp_TaskDeQueue(MWIFAPP_LINKLIST *pFIFO);
static MWIFAPP_LINK *mwifapp_TaskDeQueueByOption(MWIFAPP_LINKLIST *pFIFO, MWIFAPP_LINK *pEntity);
#endif
/* LOCAL SUBPROGRAM BODIES */
/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskIOJoinBss()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_TaskIOJoinBss(U8_T value)
{

} /* End of MWIFAPP_TaskIOJoinBss() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskIOReadBeaconInterval()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_TaskIOReadBeaconInterval(void)
{

} /* End of MWIFAPP_TaskIOReadBeaconInterval() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskIOReadEncryptType()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_TaskIOReadEncryptType(void)
{

} /* End of MWIFAPP_TaskIOReadEncryptType() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskIOReadChannel()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_TaskIOReadChannel(U8_T* pbuf)
{

} /* End of MWIFAPP_TaskIOReadChannel() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskIOWiFiConnect()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_TaskIOWiFiConnect(void)
{

} /* End of MWIFAPP_TaskIOWiFiConnect() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskIOWiFiMacAddrSet()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_TaskIOWiFiMacAddrSet(U8_T XDATA *pbuf)
{
    memcpy(MWIFAPP_WiFiInfo.WcpuMAC, &pbuf[4], 6);
} /* End of MWIFAPP_TaskIOWiFiMacAddrSet() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskIOWiFiDriverVer()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_TaskIOWiFiDriverVer(U8_T XDATA *pbuf)
{
	U8_T len;
   
	len = pbuf[2];
 	if (len > 29)
        len = 29;
    memcpy(MWIFAPP_WiFiInfo.WcpuFwVersion, &pbuf[3], len);
    MWIFAPP_WiFiInfo.WcpuFwVersion[len] = '\0';
} /* End of MWIFAPP_TaskIOWiFiDriverVer() */

#if MWIF_TASK_HOOKUP

/*
 * ----------------------------------------------------------------------------
 * Function Name: mwifapp_TaskChkQueueEmpty()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T mwifapp_TaskChkQueueEmpty(MWIFAPP_LINKLIST XDATA *pFIFO)
{
	return ((pFIFO->pHead == 0 && pFIFO->pTail == 0) ? 1:0);
} // End of mwifapp_TaskChkQueueEmpty() // 

/*
 * ----------------------------------------------------------------------------
 * Function Name: mwifapp_TaskEnQueue()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void mwifapp_TaskEnQueue(MWIFAPP_LINKLIST *pFIFO, MWIFAPP_LINK *pEntity)
{
	if (mwifapp_TaskChkQueueEmpty(pFIFO))
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
} // End of mwifapp_TaskEnQueue() // 

/*
 * ----------------------------------------------------------------------------
 * Function Name: mwifapp_TaskDeQueue()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
MWIFAPP_LINK *mwifapp_TaskDeQueue(MWIFAPP_LINKLIST *pFIFO)
{
	MWIFAPP_LINK *pTmp;
	
	if (mwifapp_TaskChkQueueEmpty(pFIFO))
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
} // End of mwifapp_TaskDeQueue() //

/*
 * ----------------------------------------------------------------------------
 * Function Name: mwifapp_TaskDeQueueByOption()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
MWIFAPP_LINK *mwifapp_TaskDeQueueByOption(MWIFAPP_LINKLIST *pFIFO, MWIFAPP_LINK *pEntity)
{
	MWIFAPP_LINK *pTmp;
	U8_T i;
	
	if (mwifapp_TaskChkQueueEmpty(pFIFO))
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
} // End of mwifapp_TaskDeQueueByOption() //

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskCmdEnQueue()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S8_T MWIFAPP_TaskCmdEnQueue(MWIFAPP_LINK *pEntity)
{
	pEntity->WaitTime = MWIFAPP_MAX_IO_CMD_TIMEOUT;
	pEntity->ReplyLen = 0;

	mwifapp_TaskEnQueue(&mwifapp_cmdFifo, pEntity);
	return 0;
} // End of MWIFAPP_TaskCmdEnQueue() //

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskCmdDeQueue()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S8_T MWIFAPP_TaskCmdDeQueue(MWIFAPP_LINK *pEntity)
{
	if (mwifapp_TaskDeQueueByOption(&mwifapp_waitFifo, pEntity))
		return 0;
	else
		return -1;
} // End of MWIFAPP_TaskCmdDeQueue() //

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskCmdRcv()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T MWIFAPP_TaskCmdRcv(U8_T XDATA *pBuf)
{
	if (mwifapp_TaskChkQueueEmpty(&mwifapp_waitFifo))
	{
		return MWIFAPP_UPLAYER_NO_WAIT;
	}
	else if (!(mwifapp_waitFifo.pHead->ReplyLen))
	{
		mwifapp_waitFifo.pHead->ReplyLen = pBuf[2]+3;
		memcpy(mwifapp_waitFifo.pHead->Buf, pBuf, mwifapp_waitFifo.pHead->ReplyLen);
		return MWIFAPP_UPLAYER_WAIT;
	}
	return MWIFAPP_UPLAYER_BUSY;
} // End of MWIFAPP_TaskCmdRcv() // 

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_Task()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_Task(void)
{
	// Check command queue //	
	if (!mwifapp_TaskChkQueueEmpty(&mwifapp_cmdFifo) && mwifapp_TaskChkQueueEmpty(&mwifapp_waitFifo))
	{
		MWIF_Send(mwifapp_cmdFifo.pHead->Buf[0], (U8_T*)&(mwifapp_cmdFifo.pHead->Buf[3]), mwifapp_cmdFifo.pHead->Buf[2]);
		mwifapp_TaskEnQueue(&mwifapp_waitFifo, mwifapp_TaskDeQueue(&mwifapp_cmdFifo));
	}
	
	mwifapp_Tp = SWTIMER_Tick();
	if (mwifapp_Ts != mwifapp_Tp)
	{
		mwifapp_Ts = mwifapp_Tp;	
		if (!mwifapp_TaskChkQueueEmpty(&mwifapp_waitFifo))
		{
			if (mwifapp_waitFifo.pHead->WaitTime)
				mwifapp_waitFifo.pHead->WaitTime --;
			if (!(mwifapp_waitFifo.pHead->WaitTime))
				mwifapp_TaskDeQueue(&mwifapp_waitFifo);		
		}
	}
} /* End of MWIFAPP_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIFAPP_TaskInit()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MWIFAPP_TaskInit(void)
{
	memset((U8_T*)&mwifapp_cmdFifo, 0, sizeof(MWIFAPP_LINKLIST));
	memset((U8_T*)&mwifapp_waitFifo, 0, sizeof(MWIFAPP_LINKLIST));
	memset((U8_T*)&MWIFAPP_WiFiInfo, 0, sizeof(MWIFAPP_WiFi_INFO));	
} /* End of MWIFAPP_TaskInit() */

#endif /* End of #if (MWIFAPP_TASK_FUNC_ENABLE) */
#endif /* End of #if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI) */
#endif /* End of #if (IO_CPU_TYPE) */
/* End of mwifapp.c */