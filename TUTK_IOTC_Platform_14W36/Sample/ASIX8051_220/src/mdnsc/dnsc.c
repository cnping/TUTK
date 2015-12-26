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
 /*============================================================================
 * Module Name: dnsc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: dnsc.c,v $
 *
 *=============================================================================
 */
#define DNSC_DEBUG		1

/* INCLUDE FILE DECLARATIONS */
#include "main.h"

#if (INCLUDE_DNS_CLIENT)
#include "dnsc.h"
#include "tcpip.h"
#include "mstimer.h"
#if (DNSC_DEBUG)
#include "uart0.h"
#include "printd.h"
#endif
#include <stdio.h>
#include <string.h>
/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static DNSC_INFO dnscInfo;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void dnsc_Event(U8_T, U8_T);
static void dnsc_Receive(U8_T XDATA*, U16_T, U8_T);
static U8_T dnsc_Send(U8_T InterUdpId, char *pName, U32_T dnsIp);
static U8_T dnsc_PrepareQueryPacket(S8_T *pName, U8_T *pBuf);
static U32_T dnsc_ParseResponsePacket(U8_T *pBuf, U16_T bufLen);

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_Init()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */

void DNSC_Init(void)
{
	memset((U8_T*)&dnscInfo, 0 , sizeof(DNSC_INFO));
	dnscInfo.InterAppID = TCPIP_Bind(NULL, dnsc_Event, dnsc_Receive);
	printd("DNS client Init ok.\n\r");
	
} /* End of DNSC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: dnsc_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void dnsc_Event(U8_T id, U8_T event)
{
	if (id) return;

	if (event == TCPIP_CONNECT_CANCEL)
		dnscInfo.TaskState = DNSC_TASK_CLOSE;

} /* End of dnsc_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: dnsc_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void dnsc_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
	if ((dnscInfo.TaskState != DNSC_TASK_WAIT_RESPONSE) && id)
		return;
	printd("dnsc_Receive id = %bu, %bu\n", id, dnscInfo.TaskState);
	dnscInfo.QueryIP = dnsc_ParseResponsePacket(pData, length);
	dnscInfo.TaskState = DNSC_TASK_RESPONSE;

} /* End of dnsc_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: dnsc_Send
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T dnsc_Send(U8_T InterUdpId, S8_T *pName, U32_T dnsIp)
{
	U8_T pkt[512];
	U8_T len;
	U8_T i;
	char strIp[16] = {0};

	if(strlen(pName) <= 255)
	{	
		len = dnsc_PrepareQueryPacket(pName, &pkt[0]);
	
		i = sprintf(strIp, "%d.", ((dnsIp >> 24) & 0x000000FF));
		i += sprintf(strIp + i, "%d.", ((dnsIp >> 16) & 0x000000FF));
		i += sprintf(strIp + i, "%d.", ((dnsIp >> 8) & 0x000000FF));
		sprintf(strIp + i, "%d", (dnsIp & 0x000000FF));
	
		TCPIP_UdpSend(InterUdpId, 0, 0, pkt, len);
		return 0;
	}
	return 1;
} /* End of dnsc_Send() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: dnsc_PrepareQueryPacket
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T dnsc_PrepareQueryPacket(S8_T *pName, U8_T *pBuf)
{
	U8_T DotOffset, DataOffset, DataCnt, NameLen = strlen(pName);
	DNSC_HEADER *pDNSC_HD;
	DNSC_QUESTION *pDNSC_Q;

	// Prepare DNS header
	pDNSC_HD = (DNSC_HEADER*)pBuf;
	pDNSC_HD->ID = htons(0xABCD);
	pDNSC_HD->Flag = DNS_FLAG_RD;
	pDNSC_HD->QDCount = htons(0x0001);
	pDNSC_HD->ANCount = htons(0x0000);
	pDNSC_HD->NSCount = htons(0x0000);
	pDNSC_HD->ARCount = htons(0x0000);
	pBuf = pBuf + sizeof(DNSC_HEADER);

	// Prepare Question field
	DotOffset = 0;
	DataCnt = 0;
	for (DataOffset = 0 ; DataOffset < NameLen ; DataOffset ++)//Standard DNS name notation.
	{
		if (pName[DataOffset] == '.')
		{
			pBuf[DotOffset] = DataCnt;
			DataCnt = 0;			
			DotOffset = DataOffset + 1;
		}
		else
		{
			pBuf[DataOffset + 1] = pName[DataOffset];
			DataCnt ++;
	}
	}
	pBuf[DotOffset] = DataCnt;	
	pBuf[DataOffset + 1] = 0;

	pDNSC_Q = (DNSC_QUESTION*)&(pBuf[DataOffset + 2]);
	pDNSC_Q->Type = htons(DNS_TYPE_A);
	pDNSC_Q->Class = htons(DNS_CLASS_IN);

	return (NameLen + 2 + sizeof(DNSC_HEADER) + 4);
} /* End of dnsc_PrepareQueryPacket() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: dnsc_ParseResponsePacket
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U32_T dnsc_ParseResponsePacket(U8_T *pBuf, U16_T bufLen)
{	
	U16_T len;
	U16_T i;
	U32_T ip = 0x00000000;
	DNSC_HEADER *pDNSC_HD;
	DNSC_ANSWER *pDNSC_A;	

	if (bufLen < sizeof(DNSC_HEADER))
	{
		return ip;
	}

	pDNSC_HD = (DNSC_HEADER*)pBuf;
	len = sizeof(DNSC_HEADER);
	
	if (((pDNSC_HD->Flag) & DNS_FLAG_RCODE) == 0) // No error
	{
		if (pDNSC_HD->QDCount > 0)
		{
			for (i = 0; i < pDNSC_HD->QDCount; i++)
			{
				// Handle QDCOUNT field at here

				while (*(pBuf + len++) != 0)
				{
					// Handle QNAME field at here
					// ...					
				}
			
				// Handle QTYPE and QCLASS fields at here
				// ...
				len += sizeof(DNSC_QUESTION);
			}
		}

		if (pDNSC_HD->ANCount > 0)
		{
			for (i = 0; i < pDNSC_HD->ANCount; i++)
			{
				// Handle ANCOUNT field at here

				// Currently only handles first valid answer
				pDNSC_A = (DNSC_ANSWER*)(pBuf + len);
				if ((pDNSC_A->Type == DNS_TYPE_A) && (pDNSC_A->Class == DNS_CLASS_IN))
				{						
					if (pDNSC_A->ResurLen == 4)
					{									
						ip = ((((U32_T)pDNSC_A->Resur[0] << 16) & 0xFFFF0000) + 
						          ((U32_T)pDNSC_A->Resur[1] & 0x0000FFFF));
					}
				}
				else
				{
					// Shift to offset of next answer
					// Header length (Bytes)
					// Name:2  Type:2  Class:2  TTL:4  Data Len:2  Data:variable     
					len += 10; // Length of Name + Type + Class + TTL
					len += ((U16_T)((*(pBuf + len) << 8) & 0xFF00) + (U16_T)*(pBuf + len + 1) + 2);						
				}
			}
		}

		if (pDNSC_HD->ANCount > 0)
		{
			// Handle NSCOUNT field at here
			// ...
		}

		if (pDNSC_HD->ANCount > 0)
		{
			// Handle ARCOUNT field at here
			// ...
		}
	}
	
	return ip;
} /* End of dnsc_ParseResponsePacket() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_SetServerIP
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DNSC_SetServerIP(U32_T ip)
{
	dnscInfo.ServerIP = ip;	
	printd("DNS SERVER = %lx\n", ip);
} /* End of DNSC_SetServerIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_Timer()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DNSC_Timer()
{
 	static U32_T TimerStop = 0, TimerStart = 0, TimerElapse;
	U8_T i;
	
	TimerStop = SWTIMER_Tick();

	if (TimerStop >= TimerStart)
		TimerElapse = TimerStop - TimerStart;
	else
		TimerElapse = TimerStop + (0xFFFFFFFF - TimerStart);
							
	if (TimerElapse > SWTIMER_COUNT_SECOND)
	{
		TimerStart = TimerStop;	
		
		if (dnscInfo.WaitTimer) 
			dnscInfo.WaitTimer --;
			
		for (i = 0; i< MAX_DNSC_RECORDE_CNT; i ++)	//Add for recorder expire function.
		{
			if (dnscInfo.Table[i].TimerToLive)
			{
				dnscInfo.Table[i].TimerToLive --;
			}
		}
	}
}  /* End of DNSC_Timer() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_STATE DNSC_Start()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
DNSC_STATE DNSC_Start(U8_T *pHostName)
{
	U8_T Index, timeTemp, i;	
	
	for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)//Set the host name to query
	{
		if (dnscInfo.Table[Index].Result == DNSC_QUERY_FREE)
			break;
	}

	if (Index != MAX_DNSC_RECORDE_CNT)
	{
		strcpy(dnscInfo.Table[Index].HostName, pHostName);
		dnscInfo.Table[Index].Result = DNSC_QUERY_WAIT;
		return DNSC_QUERY_WAIT;
	}

	timeTemp = 0xff;	//Add for recorder expire function.
	i = 0;
	for (Index = 0; Index < MAX_DNSC_RECORDE_CNT; Index ++)
	{
		if ((dnscInfo.Table[Index].TimerToLive < timeTemp) && (dnscInfo.Table[Index].Result == DNSC_QUERY_OK))
		{
			timeTemp = dnscInfo.Table[Index].TimerToLive;
			i = Index;
		}
	}
	strcpy(dnscInfo.Table[i].HostName, pHostName);
	dnscInfo.Table[i].Result = DNSC_QUERY_WAIT;

	return DNSC_QUERY_WAIT;
}  /* End of DNSC_QueryStart() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_STATE DNSC_Query()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
DNSC_STATE DNSC_Query(U8_T *pHostName, U32_T *pHostIP)
{
	U8_T Index;

	for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)//Find host IP in record table.
	{
		//printd("Compare-> %s, %s, %bu\n", pHostName, dnscInfo.Table[Index].HostName, dnscInfo.Table[Index].Result);
		//printd("Index = %bu, %bu\n", Index, dnscInfo.Table[Index].Result);
		if ((!strcmp(pHostName, dnscInfo.Table[Index].HostName)) && (dnscInfo.Table[Index].Result == DNSC_QUERY_OK))
			break;
	}
	
	if (Index != MAX_DNSC_RECORDE_CNT)
	{
		*pHostIP = dnscInfo.Table[Index].HostIP;
		printd("return DNSC_QUERY_OK!!!\n");
		return DNSC_QUERY_OK;
	}

	for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)//Check this host IP query.
	{
		if ((!strcmp(pHostName, dnscInfo.Table[Index].HostName)) && (dnscInfo.Table[Index].Result == DNSC_QUERY_WAIT))
			break;
	}
	
	if (Index != MAX_DNSC_RECORDE_CNT)
	{
		return DNSC_QUERY_WAIT;
	}	
	
	return DNSC_QUERY_FREE;
} /* End of DNSC_Query() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: void DNSC_Task()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DNSC_Task()
{
	static U8_T Index;

	DNSC_Timer();

	if ((dnscInfo.WaitTimer != 0) && ((dnscInfo.TaskState != DNSC_TASK_WAIT_RESPONSE) || (dnscInfo.TaskState != DNSC_TASK_CLOSE)))//Check excution condition
		return;
	
	switch (dnscInfo.TaskState)
	{
	case DNSC_TASK_IDLE://Function start or initial.
		dnscInfo.TaskState = DNSC_TASK_START;
		printd("DNSC_TASK_IDLE\n");
		break;
		
	case DNSC_TASK_START://Find the query request
		for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)
		{
			if (dnscInfo.Table[Index].Result == DNSC_QUERY_WAIT)
				break;
		}
		if (Index != MAX_DNSC_RECORDE_CNT)
		{
			dnscInfo.RetryCnt = MAX_DNSC_RETRY_CNT;
			dnscInfo.TaskState = DNSC_TASK_QUERY;
		}
		//printd("DNSC_TASK_START\n");
		break;
		
	case DNSC_TASK_QUERY://Start query
		if ((dnscInfo.UdpSocket = TCPIP_UdpNew(dnscInfo.InterAppID, 0, dnscInfo.ServerIP, 0, DNS_SERVER_PORT)) == TCPIP_NO_NEW_CONN)
		{
			dnscInfo.Table[Index].Result = DNSC_QUERY_FREE;
			dnscInfo.TaskState = DNSC_TASK_START;
		}
        else
        {
			if (dnsc_Send(dnscInfo.UdpSocket, dnscInfo.Table[Index].HostName, dnscInfo.ServerIP))
			{
				dnscInfo.TaskState = DNSC_TASK_CLOSE;		
				TCPIP_UdpClose(dnscInfo.UdpSocket);
			}
			else
			{	
				dnscInfo.WaitTimer = MAX_DNSC_TIMER_OUT;//Set time out (unit = sec)
				dnscInfo.TaskState = DNSC_TASK_WAIT_RESPONSE;
				printd("SEND DNSC_TASK_QUERY\n");
			}
        }
        
		break;
		
	case DNSC_TASK_WAIT_RESPONSE://Wait response or timeout
		if (dnscInfo.WaitTimer == 0)
		{
#if (DNSC_DEBUG)
			printd("DNSC: Query Host Name %s IP timeout!\n\r", dnscInfo.Table[Index].HostName);
#endif			
			dnscInfo.TaskState = DNSC_TASK_CLOSE;
			TCPIP_UdpClose(dnscInfo.UdpSocket);
		}
		else
		printd("DNSC_TASK_WAIT_RESPONSE\n");
		break;
		
	case DNSC_TASK_RESPONSE://Receive response packet OK
		if (dnscInfo.QueryIP == 0)
			dnscInfo.TaskState = DNSC_TASK_CLOSE;				
		else
		{
			dnscInfo.Table[Index].HostIP = dnscInfo.QueryIP;			
			dnscInfo.Table[Index].Result = DNSC_QUERY_OK;			
			dnscInfo.Table[Index].TimerToLive = MAX_DNSC_RECORDE_EXPIRE_TIME;	//Add for recorder expire function.
			dnscInfo.TaskState = DNSC_TASK_START;
			printd("DNSC_TASK_RESPONSE\n");
		}
		
		break;
			
	case DNSC_TASK_CLOSE://Retry again
		if (dnscInfo.RetryCnt == 0)
		{
			dnscInfo.Table[Index].Result = DNSC_QUERY_FREE;
			dnscInfo.TaskState = DNSC_TASK_START;					
			dnscInfo.WaitTimer = 0;
		}
		else
		{
			dnscInfo.RetryCnt --;		
			dnscInfo.TaskState = DNSC_TASK_QUERY;
			dnscInfo.WaitTimer = MAX_DNSC_TIMER_OUT;
		}
		
		printd("DNSC_TASK_CLOSE\n");
		break;
		
	default:
		dnscInfo.WaitTimer = 0;	
		dnscInfo.TaskState = DNSC_TASK_IDLE;
		//printd("DNSC_TASK_IDLE\n");
		break;	
	};
} /* End of DNSC_Task() */

#endif /* INCLUDE_DNS_CLIENT */

/* End of dnsc.c */


