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
 * Module Name: mwifapp.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: mwifapp.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __MWIF_APP_H__
#define __MWIF_APP_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES */

/* TYPE DECLARATIONS */
#if IO_CPU_TYPE
#if MWIF_TASK_HOOKUP

#define MWIFAPP_MAX_IO_CMD_TIMEOUT  (75)
#define MWIFAPP_MAX_SITESURVEY_TABLE_MEMBER (20)
#define MWIFAPP_MAX_SSID_LENGTH	(33)

#define MWIFAPP_UPLAYER_NO_WAIT	0
#define MWIFAPP_UPLAYER_WAIT	1
#define MWIFAPP_UPLAYER_BUSY	2

typedef struct _MWIFAPP_LINK
{
	struct _MWIFAPP_LINK *pPre;
	struct _MWIFAPP_LINK *pNext;
	U16_T WaitTime;
	U16_T ReplyLen;
	U8_T Buf[1];
} MWIFAPP_LINK;

typedef struct _MWIFAPP_LINKLIST
{
	MWIFAPP_LINK *pHead;
	MWIFAPP_LINK *pTail;
}MWIFAPP_LINKLIST;

typedef struct _MWIFAPP_WiFi_INFO
{
	U8_T WcpuFwVersion[30];
	U8_T WcpuMAC[6];
}MWIFAPP_WiFi_INFO;

typedef struct _MWIFAPP_SITE_SURVEY_REPORT
{
	U8_T Index;
	U8_T BSSID[6];
	U8_T Type;
	U8_T Channel;
	U8_T RSSI;	
	U8_T Security;		
	U8_T WebPageFlag;		
	U8_T SSID[MWIFAPP_MAX_SSID_LENGTH];
} MWIFAPP_SITE_SURVEY_REPORT;

extern MWIFAPP_WiFi_INFO MWIFAPP_WiFiInfo;
extern MWIFAPP_SITE_SURVEY_REPORT MWIFAPP_SiteSurveyTable[];

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void MWIFAPP_TaskInit(void);
void MWIFAPP_Task(void);
U8_T MWIFAPP_TaskCmdRcv(U8_T XDATA *pBuf);
S8_T MWIFAPP_TaskCmdEnQueue(MWIFAPP_LINK *pEntity);
S8_T MWIFAPP_TaskCmdDeQueue(MWIFAPP_LINK *pEntity);
#endif /* End of #if (MWIFAPP_TASK_FUNC_ENABLE) */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void MWIFAPP_TaskIOWiFiConnect(void);
void MWIFAPP_TaskIOJoinBss(U8_T value);
void MWIFAPP_TaskIOReadBeaconInterval(void);
void MWIFAPP_TaskIOReadEncryptType(void);
void MWIFAPP_TaskIOReadChannel(U8_T* pbuf);
void MWIFAPP_TaskIOWiFiMacAddrSet(U8_T XDATA *pbuf);
void MWIFAPP_TaskIOWiFiDriverVer(U8_T XDATA *pbuf);

#endif /* End of #if (IO_CPU_TYPE) */
#endif /* End of __MWIF_APP_H__ */

/* End of mwif.h */
