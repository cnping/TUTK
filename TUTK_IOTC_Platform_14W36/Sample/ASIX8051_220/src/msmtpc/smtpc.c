/*
*********************************************************************************
*     Copyright (c) 2010   ASIX Electronic Corporation      All rights reserved.
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
 * $Log: smtpc.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "adapter.h"
#include "smtpc.h"
#include "tcpip.h"
/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */
S8_T each3toc;	/* Buffer each 3 byte to coding Base64 format*/
S8_T b64[4];	/* memory output 4 base64 data*/


/* STATIC VARIABLE DECLARATIONS */
static U8_T *smtpc_Buf;
static U8_T *smtpc_Subject;
static SMTPC_CONN XDATA smtpc_Conns;
static U8_T XDATA smtpc_InterAppId;

/* LOCAL SUBPROGRAM DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_Init
 * Purpose: to initial the FTP client connection information.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SMTPC_Init(U8_T *pBuf, U8_T *pSubject)
{
	smtpc_Conns.State = SMTP_STATE_NONE;
	smtpc_Conns.ServerIp = 0;
	smtpc_InterAppId = TCPIP_Bind(NULL, SMTPC_Event, SMTPC_Receive);
    smtpc_Buf = pBuf;
    smtpc_Subject = pSubject;
} /* End of SMTPC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SMTPC_Event(U8_T id, U8_T event)
{
	if (id != 0)
		return;

	if (event <= TCPIP_CONNECT_ACTIVE)
		smtpc_Conns.State = event;
	else if (event == TCPIP_CONNECT_ACTIVE)
		smtpc_Conns.State = SMTP_STATE_CONNECTED;
	else if (event == TCPIP_CONNECT_XMIT_COMPLETE)
	{
		if (smtpc_Conns.State == SMTP_STATE_SEND_MESSAGE)
		{
			smtpc_Buf[0] = 0xd;
			smtpc_Buf[1] = 0xa;
			smtpc_Buf[2] = '.';
			smtpc_Buf[3] = 0xd;
			smtpc_Buf[4] = 0xa;
			TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, 5, TCPIP_SEND_NOT_FINAL);
			smtpc_Conns.State = SMTP_STATE_MESSAGE_SENT;
		}
	}

} /* End of SMTPC_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SMTPC_Receive(U8_T XDATA* pbuf, U16_T length, U8_T id)
{
	U16_T		codes, len;
	U8_T		*point, *pData;

	if (id != 0)
		return;

	codes = (pbuf[0] - '0') * 100 + (pbuf[1] - '0') * 10 + (pbuf[2] - '0');
	if (smtpc_Conns.State == SMTP_STATE_CONNECTED)
	{
		if (codes != 220)
			goto sendquit;
			
		smtpc_Buf[0] = 'H';
		smtpc_Buf[1] = 'E';
		smtpc_Buf[2] = 'L';
		smtpc_Buf[3] = 'O';
		smtpc_Buf[4] = ' ';

		len = 5;
		pbuf += 4;
		point = &smtpc_Buf[5];
		while ((*pbuf != ' ') && (length > 0))
		{
			*point++ = *pbuf++;
			len++;
			length--;
		}

		*point++ = 0x0d;
		*point = 0x0a;
		len += 2;
		TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, len, TCPIP_SEND_NOT_FINAL);
		smtpc_Conns.State = SMTP_STATE_HELO_SENT;
	}
	else if (smtpc_Conns.State == SMTP_STATE_HELO_SENT)
	{
		if (codes != 250)
			goto sendquit;
	
		smtpc_Buf[0] = 'M';
		smtpc_Buf[1] = 'A';
		smtpc_Buf[2] = 'I';
		smtpc_Buf[3] = 'L';
		smtpc_Buf[4] = ' ';
		smtpc_Buf[5] = 'F';
		smtpc_Buf[6] = 'R';
		smtpc_Buf[7] = 'O';
		smtpc_Buf[8] = 'M';
		smtpc_Buf[9] = ':';
		smtpc_Buf[10] = '<';

		len = 11;
		pData = smtpc_Conns.From;
		point = &smtpc_Buf[11];
		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}

		*point++ = '>';
		*point++ = 0x0d;
		*point = 0x0a;
		len += 3;
		TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, len, TCPIP_SEND_NOT_FINAL);
		smtpc_Conns.State = SMTP_STATE_FROM_SENT;
	}
	else if ((smtpc_Conns.State >= SMTP_STATE_FROM_SENT) &&	(smtpc_Conns.State < SMTP_STATE_RCV3_SENT))
	{
		if (smtpc_Conns.State == SMTP_STATE_FROM_SENT)
		{
			if (smtpc_Conns.To1[0])
			{
				pData = smtpc_Conns.To1;
				smtpc_Conns.State = SMTP_STATE_RCV1_SENT;
			}
			else if (smtpc_Conns.To2[0])
			{
				pData = smtpc_Conns.To2;
				smtpc_Conns.State = SMTP_STATE_RCV2_SENT;
			}
			else if (smtpc_Conns.To3[0])
			{
				pData = smtpc_Conns.To3;
				smtpc_Conns.State = SMTP_STATE_RCV3_SENT;
			}
			else
				goto sendquit;				
		}
		else if (smtpc_Conns.State == SMTP_STATE_RCV1_SENT)
		{
			if (smtpc_Conns.To2[0])
			{
				pData = smtpc_Conns.To2;
				smtpc_Conns.State = SMTP_STATE_RCV2_SENT;
			}
			else if (smtpc_Conns.To3[0])
			{
				pData = smtpc_Conns.To3;
				smtpc_Conns.State = SMTP_STATE_RCV3_SENT;
			}
			else
				goto senddata;
		}
		else if (smtpc_Conns.State == SMTP_STATE_RCV2_SENT)
		{
			if (smtpc_Conns.To3[0])
			{
				pData = smtpc_Conns.To3;
				smtpc_Conns.State = SMTP_STATE_RCV3_SENT;
			}
			else
				goto senddata;
		}

		smtpc_Buf[0] = 'R';
		smtpc_Buf[1] = 'C';
		smtpc_Buf[2] = 'P';
		smtpc_Buf[3] = 'T';
		smtpc_Buf[4] = ' ';
		smtpc_Buf[5] = 'T';
		smtpc_Buf[6] = 'O';
		smtpc_Buf[7] = ':';
		smtpc_Buf[8] = '<';

		len = 9;
		point = &smtpc_Buf[9];
		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}

		*point++ = '>';
		*point++ = 0x0d;
		*point = 0x0a;
		len += 3;
		TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, len, TCPIP_SEND_NOT_FINAL);
	}
	else if (smtpc_Conns.State == SMTP_STATE_RCV3_SENT)
	{
senddata:
		smtpc_Buf[0] = 'D';
		smtpc_Buf[1] = 'A';
		smtpc_Buf[2] = 'T';
		smtpc_Buf[3] = 'A';
		smtpc_Buf[4] = 0x0d;
		smtpc_Buf[5] = 0x0a;
		len = 6;
		TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, len, TCPIP_SEND_NOT_FINAL);
		smtpc_Conns.State = SMTP_STATE_DATA_SENT;
	}
	else if (smtpc_Conns.State == SMTP_STATE_DATA_SENT)
	{
		if (codes != 354)
			goto sendquit;
			
		smtpc_Conns.State = SMTP_STATE_WAIT_MESSAGE;//wait for user send the message.
	}
	else if (smtpc_Conns.State == SMTP_STATE_MESSAGE_SENT)
	{
sendquit:
		smtpc_Buf[0] = 'Q';
		smtpc_Buf[1] = 'U';
		smtpc_Buf[2] = 'I';
		smtpc_Buf[3] = 'T';
		smtpc_Buf[4] = 0x0d;
		smtpc_Buf[5] = 0x0a;
		len = 6;
		TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, len, TCPIP_SEND_NOT_FINAL);
		smtpc_Conns.State = SMTP_STATE_QUIT_SENT;
	}

} /* End of SMTPC_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_SendMessage
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SMTPC_SendMessage(U8_T* pBuf, U16_T length)
{
	U16_T		len, j;
	U8_T		*point, *pData, *ptemp;
	S8_T*		pimage;		/* point to image data */
	U8_T		char_count, i;
	U8_T		multi = 0;

	/* From header */
	smtpc_Buf[0] = 'F';
	smtpc_Buf[1] = 'r';
	smtpc_Buf[2] = 'o';
	smtpc_Buf[3] = 'm';
	smtpc_Buf[4] = ':';
	smtpc_Buf[5] = ' ';
	smtpc_Buf[6] = '"';

	len = 7;
	pData = smtpc_Conns.From;
	point = &smtpc_Buf[7];
	while (*pData != 0)
	{
		*point++ = *pData++;
		len++;
	}

	*point++ = '"';
	*point++ = ' ';
	*point++ = '<';
	len += 3;
	pData = smtpc_Conns.From;
	while (*pData != 0)
	{
		*point++ = *pData++;
		len++;
	}

	*point++ = '>';
	*point++ = 0x0d;
	*point++ = 0x0a;
	len += 3;

	/* To header */
	*point++ = 'T';
	*point++ = 'o';
	*point++ = ':';
	*point++ = ' ';
	len += 4;

	for (i = 0; i < 3; i++)
	{
		if (i == 0)
		{
			if (smtpc_Conns.To1[0])
			{
				pData = smtpc_Conns.To1;
				ptemp = smtpc_Conns.To1;
			}
			else
				continue;
		}
		else if (i == 1)
		{
			if (smtpc_Conns.To2[0])
			{
				pData = smtpc_Conns.To2;
				ptemp = smtpc_Conns.To2;
			}
			else
				continue;
		}
		else
		{
			if (smtpc_Conns.To3[0])
			{
				pData = smtpc_Conns.To3;
				ptemp = smtpc_Conns.To3;
			}
			else
				continue;
		}

		if (multi == 1)
		{
			*point++ = ',';
			len++;
		}

		*point++ = '"';
		len++;

		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}

		*point++ = '"';
		*point++ = ' ';
		*point++ = '<';
		len += 3;

		pData = ptemp;
		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}

		*point++ = '>';
		len += 1;
		multi = 1;
	}

	*point++ = 0x0d;
	*point++ = 0x0a;
	len += 2;
	/* Subject header */
	*point++ = 'S';
	*point++ = 'u';
	*point++ = 'b';
	*point++ = 'j';
	*point++ = 'e';
	*point++ = 'c';
	*point++ = 't';
	*point++ = ':';
	*point++ = ' ';
	len += 9;

	pData = smtpc_Subject;
	while (*pData != 0)
	{
		if (*pData == 'Z')
		{
			*point++ = 9;
			pData++;
		}
		else
			*point++ = *pData++;
		len++;
	}
	
	//Initial MIME64
	Cmime64_Init();
	char_count = 0;

	pimage = pBuf;
	for(j=0;j<length;j++)
	{
		cmime64((S8_T*)pimage);
		each3toc++;
		if(each3toc == 3)
		{
			if(char_count == SMTP_MAX_LENGTH)
			{
				*point++ = '\r';
				*point++ = '\n';
				len += 2;
				char_count = 0;
			}

			for(i=0;i<4;i++)
			{
				*point++ = b64[3-i];/*output b64[3..0]array for original 4 byte*/
				len++;
			}
			char_count +=4;
			each3toc = 0;
		} 
		pimage++;
	}
	*point++ = '\r';
	*point++ = '\n';
	len += 2;

	TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, len, TCPIP_SEND_NOT_FINAL);
	smtpc_Conns.State = SMTP_STATE_SEND_MESSAGE;

} /* End of SMTPC_SendMessage() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_Start
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SMTPC_Start(U32_T ip, U8_T* from, U8_T* to1, U8_T* to2, U8_T* to3)
{
	if (smtpc_Conns.State != SMTP_STATE_NONE)
	{
		return;
	}

	smtpc_Conns.ServerIp = ip;
	/* Create SMTP client port */
	if ((smtpc_Conns.TcpSocket = TCPIP_TcpNew(smtpc_InterAppId, 0,smtpc_Conns.ServerIp,
		0, SMTP_SERVER_PORT)) == TCPIP_NO_NEW_CONN)
	{
		return;
	}

	smtpc_Conns.From = from;
	smtpc_Conns.To1 = to1;
	smtpc_Conns.To2 = to2;
	smtpc_Conns.To3 = to3;
	smtpc_Conns.State = SMTP_STATE_INITIAL;

	TCPIP_TcpConnect(smtpc_Conns.TcpSocket);

} /* End of SMTPC_Start() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_GetState
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T SMTPC_GetState(void)
{
	return smtpc_Conns.State; 

} /* End of SMTPC_GetState() */


/* End of smtpc.c */