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
 * Module Name: httpd.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: httpd.c,v $
 *
 *=============================================================================
 */
#define HTTPD_DEBUG		1

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "mcpu.h"
#include "httpd.h"
#include "filesys.h"
#include "adapter.h"
#include "tcpip.h"
#include "mstimer.h"
#include "httpdapi.h"
#include "httpdap.h"
#include "uart0.h"
#include "printd.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* NAMING CONSTANT DECLARATIONS */
#define HTTP_TEMP_BUFFER_SIZE	512

/* GLOBAL VARIABLES DECLARATIONS */
entry entries[20];
U8_T num_parms;
U8_T HTTP_PostTable[MAX_POST_COUNT];
U8_T HTTP_PostCount = 0;

HTTP_SERVER_INFO httpInfo;
U8_T HTTP_TmpBuf[HTTP_TEMP_BUFFER_SIZE];

/* LOCAL VARIABLES DECLARATIONS */
static HTTP_SERVER_CONN XDATA HTTP_Connects[MAX_HTTP_CONNECT];
static U8_T CODE HtmlHeader[] = {"HTTP/1.0 200 OK\r\nCache-control: no-cache\r\nConnection: Keep-Alive\r\nContent-type: text/html\r\n\r\n"};
static U8_T XDATA HtmlHeaderLen = (sizeof(HtmlHeader) - 1);
static U8_T XDATA CSSHeader[] = {"HTTP/1.0 200 OK\r\nCache-control: no-cache\r\nConnection: Keep-Alive\r\nContent-type: text/css\r\n\r\n"};
static U8_T XDATA CSSHeaderLen = (sizeof(CSSHeader) - 1);
static U8_T CODE JsHeader[] = {"HTTP/1.0 200 OK\r\nCache-control: no-cache\r\nConnection: Keep-Alive\r\nContent-type: application/javascript\r\n\r\n"};
static U8_T XDATA JsHeaderLen = (sizeof(JsHeader) - 1);
static U8_T XDATA ImageHeader[] = {"HTTP/1.0 200 OK\r\nCache-control: no-cache\r\nConnection: Keep-Alive\r\nContent-Length: xxxxx\r\nContent-type: image/jpeg\r\n\r\n"};
static U8_T XDATA ImageHeaderLen = (sizeof(ImageHeader) - 1);
static U8_T XDATA ImageByteCountOffset = 82;

static U8_T http_InterfaceId = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static U8_T http_NewConfig(void);
static void	http_DivideHtmlFile(HTTP_SERVER_CONN XDATA*, U8_T);

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTP_Init
 * Purpose: Initialize HTTP server. 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTP_Init(void)
{
	U8_T			i;

	for (i = 0; i < MAX_HTTP_CONNECT; i++)
	{
		HTTP_Connects[i].State = HTTP_STATE_FREE;
		HTTP_Connects[i].FileId = 0xff;
		HTTP_Connects[i].ContinueFlag = 0;
	}

	http_InterfaceId = TCPIP_Bind(HTTP_NewConn, HTTP_Event, HTTP_Receive);
	TCPIP_TcpListen(HTTP_SERVER_PORT ,http_InterfaceId);

	FSYS_Init();

	memset(&httpInfo, sizeof(HTTP_SERVER_INFO), 0);
	HTTPAP_Init(&httpInfo);

} /* End of HTTP_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTP_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T HTTP_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;

	for (i = 0; i < MAX_HTTP_CONNECT; i++)
	{
		if (HTTP_Connects[i].State == HTTP_STATE_FREE)
		{
			HTTP_Connects[i].State = HTTP_STATE_ACTIVE;
			HTTP_Connects[i].Timer = (U16_T)SWTIMER_Tick();
			HTTP_Connects[i].Ip = *pip;
			HTTP_Connects[i].Port = remotePort;
			HTTP_Connects[i].TcpSocket = socket;

			return i;
		}
	}
	return TCPIP_NO_NEW_CONN;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTP_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTP_Event(U8_T id, U8_T event)
{
	U8_T	fileId = HTTP_Connects[id].FileId;

	if (event < TCPIP_CONNECT_XMIT_COMPLETE)
	{
		HTTP_Connects[id].State = event;
		/* Add for send web page final process start */
		if (event == TCPIP_CONNECT_CANCEL)
		{
           	if (FSYS_Manage[fileId].FType == FILE_TYPE_HTML || FSYS_Manage[fileId].FType == FILE_TYPE_CGI)
			{
				if (HTTPAP_ResponseComplete(&(HTTP_Connects[id]), &httpInfo) == HTTP_POST_FAILURE)
					return;
			}
		}
		/* Add for send web page final process end */
	}
	else if (event == TCPIP_CONNECT_XMIT_COMPLETE)
	{
		U8_T*			pSour;
		static U16_T	dataLen;

		if (HTTP_Connects[id].State == HTTP_STATE_SEND_HEADER)
		{
			{
				if (FSYS_Manage[fileId].FType & FILE_TYPE_BIN)
				{
					HTTP_Connects[id].Divide.Fragment = 0;
				}
				else
				{
					http_DivideHtmlFile(&HTTP_Connects[id], fileId);
					
					HTTPAP_DivideHtmlFile(&HTTP_Connects[id], fileId);
				}

			 	pSour = FSYS_Manage[fileId].PBuf;
				dataLen = FSYS_Manage[fileId].FileSize;

				if (HTTP_Connects[id].Divide.Fragment == 0)
				{
					TCPIP_TcpSend(HTTP_Connects[id].TcpSocket, pSour, dataLen, TCPIP_SEND_FINAL);
					HTTP_Connects[id].State = HTTP_STATE_SEND_FINAL;
				}
				else
				{
					HTTP_Connects[id].State = HTTP_STATE_SEND_DATA;
					HTTP_Connects[id].Divide.PData = pSour;
					HTTP_Connects[id].Divide.LeftLen = dataLen;
					HTTP_Connects[id].Divide.CurIndex = 0;
					HTTP_Connects[id].Divide.PadFlag = 0;
					goto SENDHTML;
				}
			}
		}
		else if (HTTP_Connects[id].State == HTTP_STATE_SEND_DATA)
		{
SENDHTML:
			{
				BUF_TEXT XDATA*		pText;
				BUF_TAG XDATA*		pTag;
				U8_T*				pSour = HTTP_Connects[id].Divide.PData;
				static U8_T XDATA	pTrue[] = " checked";
				static U8_T XDATA	pSele[] = " selected";
				static U16_T	leftLen;
				static U8_T		index;

				leftLen = HTTP_Connects[id].Divide.LeftLen;
				index = HTTP_Connects[id].Divide.CurIndex;

				if (!HTTP_Connects[id].Divide.PadFlag)
				{
					if (index == 0)
						dataLen = HTTP_Connects[id].Divide.Offset[index];
					else if (index == HTTP_Connects[id].Divide.Fragment)
						dataLen = leftLen;
					else
						dataLen = HTTP_Connects[id].Divide.Offset[index] -
							HTTP_Connects[id].Divide.Offset[index - 1];

					if (index == HTTP_Connects[id].Divide.Fragment)
					{
						TCPIP_TcpSend(HTTP_Connects[id].TcpSocket, pSour, dataLen, TCPIP_SEND_FINAL);
						HTTP_Connects[id].State = HTTP_STATE_SEND_FINAL;
					}
					else
					{
						TCPIP_TcpSend(HTTP_Connects[id].TcpSocket, pSour, dataLen, TCPIP_SEND_NOT_FINAL);
						HTTP_Connects[id].Divide.PData += dataLen;
						HTTP_Connects[id].Divide.LeftLen -= dataLen;
						HTTP_Connects[id].Divide.PadFlag = 1;
					}
				}
				else
				{
					if (HTTP_Connects[id].Divide.PostType[index] ==POST_TYPE_RADIO)
					{
						if (HTTP_Connects[id].Divide.SetFlag[index] == 1)
							TCPIP_TcpSend(HTTP_Connects[id].TcpSocket, pTrue, 8, TCPIP_SEND_NOT_FINAL);
						else if (HTTP_Connects[id].Divide.SetFlag[index] == 2)
						{
							if (*pSour == ' ')
							{
								HTTP_Connects[id].Divide.PData += 8;
								HTTP_Connects[id].Divide.Offset[index] += 8;
								HTTP_Connects[id].Divide.LeftLen -= 8;
							}
							else
							{
								HTTP_Connects[id].Divide.PData += 7;
								HTTP_Connects[id].Divide.Offset[index] += 7;
								HTTP_Connects[id].Divide.LeftLen -= 7;
							}
							HTTP_Connects[id].Divide.PadFlag = 0;
							HTTP_Connects[id].Divide.CurIndex++;
							goto SENDHTML;
						}
					}
					else if (HTTP_Connects[id].Divide.PostType[index] == POST_TYPE_TEXT)
					{
						pText = POST_Record[HTTP_Connects[id].Divide.RecordIndex[index]].PValue;
						leftLen -= pText->DefaultLength;
						if (!leftLen)
						{
							TCPIP_TcpSend(HTTP_Connects[id].TcpSocket, pText->CurrValue, pText->CurrLength, TCPIP_SEND_FINAL);
							HTTP_Connects[id].State = HTTP_STATE_SEND_FINAL;
							return;
						}

						TCPIP_TcpSend(HTTP_Connects[id].TcpSocket, pText->CurrValue, pText->CurrLength, TCPIP_SEND_NOT_FINAL);
						HTTP_Connects[id].Divide.PData += pText->DefaultLength;
						HTTP_Connects[id].Divide.Offset[index] += pText->DefaultLength;
						HTTP_Connects[id].Divide.LeftLen =leftLen;
					}
					else if (HTTP_Connects[id].Divide.PostType[index] == POST_TYPE_TAG)
					{
						pTag = POST_Record[HTTP_Connects[id].Divide.RecordIndex[index]].PValue;
						leftLen -= pTag->DefaultLength;
						if(pTag->CurrLength)
						  TCPIP_TcpSend(HTTP_Connects[id].TcpSocket, pTag->CurrValue, pTag->CurrLength, TCPIP_SEND_NOT_FINAL);
						HTTP_Connects[id].Divide.PData += pTag->DefaultLength;
						HTTP_Connects[id].Divide.Offset[index] += pTag->DefaultLength;
						HTTP_Connects[id].Divide.LeftLen =leftLen;
						if(!pTag->CurrLength)
						{
							HTTP_Connects[id].Divide.PadFlag = 0;
							HTTP_Connects[id].Divide.CurIndex++;
							goto SENDHTML;
						}
					} 
					else if (HTTP_Connects[id].Divide.PostType[index] == POST_TYPE_SELECT)
					{
						if (HTTP_Connects[id].Divide.SetFlag[index] == 1)
						{
							TCPIP_TcpSend(HTTP_Connects[id].TcpSocket, pSele, 9, TCPIP_SEND_NOT_FINAL);
						}
						else if (HTTP_Connects[id].Divide.SetFlag[index] == 2)
						{
							HTTP_Connects[id].Divide.PData += 9;
							HTTP_Connects[id].Divide.Offset[index] += 9;
							HTTP_Connects[id].Divide.LeftLen -= 9;

							HTTP_Connects[id].Divide.PadFlag = 0;
							HTTP_Connects[id].Divide.CurIndex++;
							goto SENDHTML;
						}
						else
						{
							HTTPAP_SendMoreWebFragment(index, &HTTP_Connects[id]);
						}
					}
					HTTP_Connects[id].Divide.PadFlag = 0;
					HTTP_Connects[id].Divide.CurIndex++;
				}
			}
		}
		else if	(HTTP_Connects[id].State == HTTP_STATE_SEND_FINAL)
		{
//			HTTP_Connects[id].State = HTTP_STATE_FREE;
		}
	}

} /* End of HTTP_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: StringCompare
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T StringCompare(U8_T XDATA* pData, U8_T XDATA* pStr, U8_T len)
{
	U8_T	index;

	for (index = 0; index < len; index++)
	{
		if (*pData++ != *pStr++)
			break;
	}
	if (index == len)
		return 1;

	return 0;
} /* End of StringCompare() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: http_GetFile
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T http_GetFile(U8_T XDATA** pData, U8_T XDATA** pFName, U8_T XDATA** pFNameExt, U16_T* length)
{
	U8_T	c, find = 0;
	U8_T	parse_state = 0;

	c = **pData;
	while ((c != ' ') && (c != '\0') && (c != '\r') && (c != '\n'))
	{
		switch(parse_state)
		{
		case 0:
			if (c == '/' || c == '\\')
			{
				parse_state = 1;
				*pFName = *pData + 1;
			}
			break;
		case 1:
			if (c == '?')
			{
				find = 2;
				return find;
			}
			else if (c == '&')
			{
				**pData = '\0';
				return find;
			}
			else
			{
				if ((c == '+') || (c == '='))
					return 0;

				/* a path, change it to '_' */
				else if ((c == '/') || (c == '\\'))
				{
					**pData = '_';
				}
				/* Remember where file extension starts. */
				else if (c == '.')
				{
					*pFNameExt = *pData + 1;
					find = 1;
				}
			}
			break;
		}
		(*pData)++;
		(*length)--;
		c = **pData;
	}

	return find;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: http_UserPost
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T http_UserPost(U8_T XDATA* pData, U16_T length, U8_T fileId, U8_T continueFlag)
{
	BUF_SUBMIT XDATA*	pSubmit;
	BUF_TEXT XDATA*		pText;
	U8_T				j;
#if (MAX_POST_BUF_RADIO)
	BUF_RADIO XDATA*	pRadio;
#endif
#if (MAX_POST_BUF_SELECT)
	BUF_SELECT XDATA*	pSelect;
#endif
	U16_T				i;
	U8_T				x, y, a, nameLen;
	U8_T				status = 0, final = 0;

 	U8_T index;
 	U8_T StrTemp[MAX_POST_VALUE_LEN] ;
  	U8_T tempchar;

	if (continueFlag)
		goto MAPRECORD;

	if (length < 150)
	{
		return HTTP_POST_FAILURE;
	}

	HTTP_PostCount = 0;
	if (*pData == '?')
	{
		length -= 1;
		pData += 1;
		goto MAPRECORD;
	}

	/* Parse the start address of post value from trailer to header */
	for (i = length - 4; ;i --)
	{
		if (pData[i] == '\r' && pData[i+1] == '\n' && pData[i+2] == '\r' && pData[i+3] == '\n')
		{
			pData += i;
			length -= i;
			break;
		}
		else if (!i)
			return HTTP_POST_FAILURE;
	}

	i = 0;
	while(i < length)
	{
		while (1)
		{
			if ((pData[0]=='\r')&&(pData[1]=='\n')&&(pData[2]=='\r')&&(pData[3]=='\n'))
			{
				pData += 4;
				i += 4;
				if (i == length)
					return HTTP_POST_CONTINUE;
					
				break;
			}
			else
			{
				pData++;
				i++;
				if ((i + 3) >= length)
					return HTTP_POST_FAILURE;
			}
		}
MAPRECORD:
		for (x = 0; x < MAX_POST_RECORDS; x++)
		{
			if (POST_Record[x].Occupy == 1)
			{
				// check file id
				if (fileId != POST_Record[x].FileIndex)
				{
					if (POST_Record[x].FileIndex != 0xff)
					{
						continue;
					}
				}

				for (y = 0; y < POST_Record[x].NameLen; y++)
				{
					if (POST_Record[x].Name[y] != pData[y])
						break;
				}
				if (y == POST_Record[x].NameLen)
				{
					if (pData[y] == '=')
					{
						nameLen = (y + 1);
						pData += nameLen;
						i += nameLen;

						switch (POST_Record[x].PostType)
						{
						case POST_TYPE_SUBMIT:
							pSubmit = POST_Record[x].PValue;
							for (a = 0; a < pSubmit->DefaultVlaueLen; a++)
							{
								if (pSubmit->Value[a] != pData[a])
									break;
							}
							if (a == pSubmit->DefaultVlaueLen)
							{
								HTTP_PostTable[HTTP_PostCount] = x;
								HTTP_PostCount++;
#if (HTTPD_DEBUG)
								printd ("SUBMIT\n\r");
#endif
								if (pSubmit->IsApply)
								{
									if (status)
									{
										return HTTP_POST_FAILURE;
									}
									else
										return HTTP_POST_SUCCESS;
								}
								else
									return HTTP_POST_CANCEL;
							}
							else
							{
								pData -= nameLen;
								i -= nameLen;

								goto MAPRECORD;
							}
							break;
						case POST_TYPE_TEXT:
							pText = POST_Record[x].PValue;
							memset( StrTemp, 0, sizeof(StrTemp));
    						index=0;
							for (a = 0; a < MAX_POST_VALUE_LEN; a++)
							{
								if (pData[a] == '&')
									break;
								else if (pData[a] == ' ')
								{
									final = 1;
									break;
								}

								StrTemp[index] = pData[a];
								if(pData[a] == '%')
								{	
       								 if(isdigit(pData[a+1]))
          								tempchar = (pData[a+1] - '0') << 4;
        							 else
          								tempchar = ((pData[a+1] - 'A') + 10) << 4;
			  
        							 if(isdigit(pData[a+2]))
          								tempchar += (pData[a+2] - '0');
        							else
          								tempchar += ((pData[a+2] - 'A') + 10);

        						    StrTemp[index] = tempchar;
        							a+=2;				
      							}
								index++;
							}

							if (pText == NULL)
							{
								return HTTP_POST_FAILURE;
							}
							else
							{
								memcpy(pText->UserValue, StrTemp, strlen(StrTemp)+1);
								pText->UserLength = strlen(StrTemp);
							}

							if (strlen(StrTemp) == MAX_POST_VALUE_LEN)
							{
								if ((pData[a] != '&') && (pData[a] != ' '))
								{
#if (HTTPD_DEBUG)
									printd ("TEXT_FALSE\n\r");
#endif
									status = 1;
									pData += a;
									i += a;

									while ((*pData != '&') && (*pData != 0xa) && (*pData != 0xd))
									{
										pData++;
										i++;
									}

									pData++;
									i++;

								   	goto MAPRECORD;
								}
								else if (pData[a] == ' ')
									final = 1;
							}
#if (HTTPD_DEBUG)
							printd ("TEXT_TRUE\n\r");
#endif
							HTTP_PostTable[HTTP_PostCount] = x;
							HTTP_PostCount++;
							if (final == 1)
							{
								if (status)
								{
									return HTTP_POST_FAILURE;
								}
								else
									return HTTP_POST_SUCCESS;
							}
							pData += (a + 1);
							i += (a + 1);

							goto MAPRECORD;
							break;
#if (MAX_POST_BUF_PASSWORD)
						case POST_TYPE_PASSWORD:
							pText = POST_Record[x].PValue;
							memset(StrTemp, 0, sizeof(StrTemp));
    						index=0;
							for (a = 0; a < MAX_POST_VALUE_LEN; a++)
							{
								if (pData[a] == '&')
									break;
								else if (pData[a] == ' ')
								{
									final = 1;
									break;
								}

								StrTemp[index] = pData[a];
								index++;
							}

							if (pText == NULL)
							{
								return HTTP_POST_FAILURE;
							}
							else
							{
								memcpy(pText->UserValue, StrTemp, strlen(StrTemp)+1);
								pText->UserLength = strlen(StrTemp);
							}

							if (strlen(StrTemp) == MAX_POST_VALUE_LEN)
							{
								if ((pData[a] != '&') && (pData[a] != ' '))
								{
#if (HTTPD_DEBUG)
									printd ("TEXT_FALSE\n\r");
#endif
									status = 1;
									pData += a;
									i += a;

									while ((*pData != '&') && (*pData != 0xa) && (*pData != 0xd))
									{
										pData++;
										i++;
									}

									pData++;
									i++;

								   	goto MAPRECORD;
								}
								else if (pData[a] == ' ')
									final = 1;
							}
#if (HTTPD_DEBUG)
							printd ("PASSWORD_TRUE\n\r");
#endif
							HTTP_PostTable[HTTP_PostCount] = x;
							HTTP_PostCount++;
							if (final == 1)
							{
								if (status)
								{
									return HTTP_POST_FAILURE;
								}
								else
									return HTTP_POST_SUCCESS;
							}
							pData += (a + 1);
							i += (a + 1);

							goto MAPRECORD;
							break;
#endif
#if (MAX_POST_BUF_RADIO)
						case POST_TYPE_RADIO:
							pRadio = POST_Record[x].PValue;
							for (j = 0; j < pRadio->Count; j++)
							{
								for (a = 0; a < pRadio->Length[j]; a++)
								{
									if (pRadio->Value[j][a] != pData[a])
										break;
								}
								if (a == pRadio->Length[j])
								{
									pData += (a + 1);
									i += (a + 1);
									pRadio->UserSet = j;
									HTTP_PostTable[HTTP_PostCount] = x;
									HTTP_PostCount++;
#if (HTTPD_DEBUG)
									printd ("Radio item : %bx\n\r", j);
#endif
									goto MAPRECORD;
								}
							}
							return HTTP_POST_FAILURE;
							break;
#endif
#if (MAX_POST_BUF_SELECT)
						case POST_TYPE_SELECT:
#if (HTTPD_DEBUG)
							printd("POST_TYPE_SELECT\n\r");
#endif
							pSelect = POST_Record[x].PValue;

							pSelect->UserSet = (*pData) - 0x30;

							if (pSelect->UserSet >= pSelect->Count)
								return HTTP_POST_FAILURE;

							pData++;
							i++;

							if (*pData != '&')
							{
								/* support select item > 10 */
								if ((*pData >= '0') && (*pData <= '9'))
								{
									pSelect->UserSet *= 10;
									pSelect->UserSet += ((*pData) - 0x30);
									if (pSelect->UserSet >= pSelect->Count)
										return HTTP_POST_FAILURE;

									pData++;
									i++;
								}

								if (*pData != '&')
								{
									while ((*pData != '&') && (*pData != 0xa) && (*pData != 0xd))
									{
										pData++;
										i++;
									}

									pData++;
									i++;

								   	goto MAPRECORD;
								}
							}

							pData++;
							i++;
							HTTP_PostTable[HTTP_PostCount] = x;
							HTTP_PostCount++;

							goto MAPRECORD;
							break;
#endif
						default:
							break;
						}
					}
				}
			}
		}
	}
} /* End of http_UserPost() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: http_PostVal
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T http_PostVal(U8_T XDATA* pData, U16_T length, U8_T Continue_Flag)
{
	U8_T*	pTemp;
	U8_T	idx, index;
	U16_T	temp;
	U8_T	StrTemp[NORM_POST_VALUE_LEN] ;
	U8_T	tempchar;
  
	num_parms=0;
	pData[length]=0x0;

    if(Continue_Flag)
		goto HTTP_POST_VAL_START;
		
	if (*pData == '?')
	{
		pData += 1;
		goto HTTP_POST_VAL_START;
	}		

	/* Parse the start address of post value from trailer to header */
	for (temp = length - 4; ;temp --)
	{
		if (pData[temp] == '\r' && pData[temp+1] == '\n' && pData[temp+2] == '\r' && pData[temp+3] == '\n')
			break;
		else if (!temp)
			return HTTP_POST_FAILURE;
	}
	pData+=temp+4;

HTTP_POST_VAL_START:
	idx=0;
	while (*pData != 0x0)
	{
		entries[idx].name = pData;
		pTemp = strchr(pData, '=');
		if (!pTemp)
			break;

		pData=pTemp;
		*pData++ = 0x0;
		entries[idx].val = pData;

		while (*pData != 0x0)
		{
			if (*pData == '\r')
			{
				*pData++= 0x0;
			}
			if (*pData == '&')
			{
				*pData++ = 0x0;    
				if (*pData == 0x0d)
					pData +=2;  //for \r\n
				break;
			}
			pData++;
		}

		/* convert  '+' to spaces */
		pTemp = entries[idx].val;
		for (; *pTemp != 0x0; pTemp++)
			if (*pTemp == '+')
				*pTemp = ' '; 
                
		/* convert  '%XX' */
		memset( StrTemp, 0, sizeof(StrTemp));
		index=0;
		for (pTemp = entries[idx].val ; *pTemp != 0x0 ; pTemp++)
		{
			StrTemp[index] = *pTemp;
			if(*pTemp == '%')
			{	
				if(isdigit(*(pTemp+1)))
					tempchar = (*(pTemp+1) - '0') << 4;
				else
					tempchar = ((*(pTemp+1) - 'A') + 10) << 4;

				if(isdigit(*(pTemp+2)))
					tempchar += (*(pTemp+2) - '0');
				else
					tempchar += ((*(pTemp+2) - 'A') + 10);

				StrTemp[index] = tempchar;
				pTemp+=2;				
			}
			index ++;
		}
		strncpy(entries[idx].val, StrTemp, strlen(StrTemp)+1 );
		idx++;
	}

	if(idx==0)
		return HTTP_POST_CONTINUE;

	num_parms=idx;
	return  HTTP_POST_SUCCESS;
}
/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTP_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTP_Receive(U8_T XDATA* pData, U16_T length, U8_T conn_id)
{
	HTTP_SERVER_CONN XDATA*	pHttpConn = &HTTP_Connects[conn_id];
	U8_T XDATA				str_post[] = {"POST"};
	U8_T					command, status, fileId, index, fileStatus;
	U8_T XDATA*				pFName;
	U8_T XDATA*				pFNameExt;
	U8_T CODE*				pSour;
	U8_T XDATA*				pExpanSour;
	U16_T					data_len;
	U8_T					i;
		
    num_parms = 0;
	
	httpInfo.pRequestBuf = pData;
		
	if (pHttpConn->State < HTTP_STATE_ACTIVE)
	{
		return;
	}

	if (pHttpConn->ContinueFlag)
	{
		fileId = pHttpConn->ContinueFileId;
		goto POST_START;
	}

	if (length < 14)
	{
        /* Auto response warning message */
		HTTPAPI_ResponseErrMessage(HTTP_TmpBuf, HTTPAPI_400_BadRequest, pHttpConn);	
		return;
	}

	if (StringCompare(pData, str_post, 4))
	{
		command = HTTP_CMD_POST;
		pData += 5;
		length -= 5;
	}
	else
	{
		command = HTTP_CMD_GET;
		pData += 4;
		length -= 4;
	}

	/* skip space */
	while (*pData == ' ')
	{
		pData++;
		length--;
	}

	fileId = MAX_STORE_FILE_NUM;
	fileStatus = http_GetFile(&pData, &pFName, &pFNameExt, &length);

	if (fileStatus)
		fileId = FSYS_FindFile(pFName);

	if (fileStatus == 2) /* for GET /XXX.XXX?--- mode */
		goto POST_START;

	if (command == HTTP_CMD_POST)
	{
POST_START:
		if (fileId == MAX_STORE_FILE_NUM)
		{
        	/* Auto response warning message */
			HTTPAPI_ResponseErrMessage(HTTP_TmpBuf, HTTPAPI_404_NotFound, pHttpConn);
			return;
		}

		if(FSYS_Manage[fileId].FType == FILE_TYPE_CGI)
		{
			status = http_PostVal(pData, length, pHttpConn->ContinueFlag);

			if (status == HTTP_POST_CONTINUE)
			{
#if (HTTPD_DEBUG)
				printd ("POST continue.\n\r");
#endif
				pHttpConn->ContinueFlag = 1;
				pHttpConn->ContinueFileId = fileId;
				return;
			}
#if (HTTPD_DEBUG)
			i=0;
			while(i<num_parms)
			{
				printd("entry[%02bx]=%s,%s\n",i,entries[i].name,entries[i].val);
				i++;
			}
#endif
			pHttpConn->ContinueFlag = 0;

			if (status == HTTP_POST_FAILURE)
			{
#if (HTTPD_DEBUG)
				printd ("POST error data0.\n\r");
#endif
        		/* Auto response warning message */
				HTTPAPI_ResponseErrMessage(HTTP_TmpBuf, HTTPAPI_400_BadRequest, pHttpConn);	
				return;
			}
		}	
		else
		{
/* Add for post method process start */
			{
				pHttpConn->FileId = fileId;
				HTTPAP_PostMethodPreParseProcess(pHttpConn, &httpInfo);
				fileId = pHttpConn->FileId;
           	}
/* Add for post method process end */

			/* look for Referer */
			status = http_UserPost(pData, length, FSYS_Manage[fileId].CgiRef,
				pHttpConn->ContinueFlag);

/* Add for post method process start */
			if (status == HTTP_POST_SUCCESS)
			{
				pHttpConn->FileId = fileId;
				status = HTTPAP_PostMethodDeferParseProcess(pHttpConn, &httpInfo);
				fileId = pHttpConn->FileId;
 			}
/* Add for post method process end */

			if (status == HTTP_POST_SUCCESS)
			{
				pHttpConn->ContinueFlag = 0;

/* Add for post method process start */
				{
					pHttpConn->FileId = fileId;
					if (HTTPAP_PostMethodPreUpdateProcess(pHttpConn, &httpInfo) == HTTPAPI_OK)
					{
						fileId = pHttpConn->FileId;

						status = http_NewConfig();
					
       		            pHttpConn->FileId = fileId;
           		        pHttpConn->Method = command;
               		    if (HTTPAP_PostMethodDeferProcess(HTTP_POST_SUCCESS, pHttpConn, &httpInfo) == HTTP_POST_ERR)
						{
                   		    return;
						}
					}
					fileId = pHttpConn->FileId;
				}
/* Add for post method process end */
			}
			else if (status == HTTP_POST_CONTINUE)
			{
				pHttpConn->ContinueFlag = 1;
				pHttpConn->ContinueFileId = fileId;
				return;
			}
			else if (status == HTTP_POST_FAILURE)
			{
				pHttpConn->ContinueFlag = 0;
#if (HTTPD_DEBUG)
				printd ("POST error data1.\n\r");
#endif
				{
	                pHttpConn->FileId = fileId;
		            pHttpConn->Method = command;
			        if (HTTPAP_PostMethodDeferProcess(HTTP_POST_FAILURE, pHttpConn, &httpInfo) == HTTP_POST_ERR)
  					{
		       			/* Auto response warning message */
						HTTPAPI_ResponseErrMessage(HTTP_TmpBuf, HTTPAPI_400_BadRequest, pHttpConn);	
						return;
					}
					fileId = pHttpConn->FileId;
				}
			}
		}
	}

	if (command == HTTP_CMD_GET)
	{
		if (fileStatus)
		{
			if (fileId == MAX_STORE_FILE_NUM)
			{
				/* not find file, send reset */
        		/* Auto response warning message */
				HTTPAPI_ResponseErrMessage(HTTP_TmpBuf, HTTPAPI_404_NotFound, pHttpConn);	
				return;
			}

            /* Add for get method process start */
            if (FSYS_Manage[fileId].FType == FILE_TYPE_HTML)
            {
                pHttpConn->FileId = fileId;
	            pHttpConn->Method = command;
		        if (HTTPAP_GetMethodProcess(pHttpConn, &httpInfo) == HTTP_POST_ERR)
			        return;

				fileId = pHttpConn->FileId;
	        }
            /* Add for get method process end */
		}
		else
		{
            /* Auto redirect to home page */
			HTTPAPI_ResponseRedirectMesssge(HTTP_TmpBuf, httpInfo.HomePage, pHttpConn);	
			return;
		}
	}

    //call cgi function
	if(FSYS_Manage[fileId].CgiCall)
	{
		void (*pF)(void);
		pF= FSYS_Manage[fileId].CgiCall;
		pF();
	}
	
	// update data in some web page
	{
		U8_T i;

		for (i = 0; i < MAX_POST_RECORDS; i++)
		{
			if (fileId == POST_Record[i].FileIndex)
			{
				if (POST_Record[i].UpdateSelf == TRUE)
				{
					void *pValue = POST_Record[i].PValue;
					void (*f)(void *pWebData);
					f = fun_tbl[i];
					f(pValue);
				}
			}
		}
	}

	pHttpConn->FileId = fileId;

	/* begin send html and jpg header */
	if (FSYS_Manage[fileId].FType & FILE_TYPE_BIN)
	{
		data_len = (U32_T)FSYS_Manage[fileId].FileSize;
		index = 5;

		while (index > 0)
		{
			index--;
			ImageHeader[ImageByteCountOffset + index] = (U8_T)((data_len%10) + 0x30);
			data_len /= 10;
		}

		if (FSYS_Manage[fileId].FType == FILE_TYPE_CSS)
		{

			data_len = (U32_T)CSSHeaderLen;
			pExpanSour = CSSHeader;
		}
		else /* only for ASIX IPCam */
		{ 
			data_len = (U32_T)ImageHeaderLen;
			pExpanSour = ImageHeader;
		}
		TCPIP_TcpSend(pHttpConn->TcpSocket, pExpanSour, data_len, TCPIP_SEND_NOT_FINAL);
	}
	else
	{
		if (FSYS_Manage[fileId].FType == FILE_TYPE_JS)
		{

			data_len = (U32_T)JsHeaderLen;
			pSour = JsHeader;
		}
		else
		{ 
			data_len = (U32_T)HtmlHeaderLen;
			pSour = HtmlHeader;
		}
		TCPIP_TcpSend(pHttpConn->TcpSocket, pSour, data_len, TCPIP_SEND_NOT_FINAL);
	}

	pHttpConn->State = HTTP_STATE_SEND_HEADER;

} /* End of HTTP_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: http_DivideHtmlFile
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void http_DivideHtmlFile(HTTP_SERVER_CONN XDATA* pHttpConn, U8_T id)
{
#if (MAX_POST_BUF_RADIO)
	BUF_RADIO XDATA*	pRadio;
#endif
	BUF_TEXT XDATA*		pText;
#if (MAX_POST_BUF_SELECT)
	BUF_SELECT XDATA*	pSelect;
#endif
	BUF_TAG XDATA*		pTag;
	U8_T				tableIndex = 0;
	U8_T				i, k;

	{
		for (i = 0; i < MAX_POST_RECORDS; i++)
		{
			if (id != POST_Record[i].FileIndex)
				continue;

			if (POST_Record[i].PostType == POST_TYPE_TEXT)
			{
				U8_T*	pData;

				pText = POST_Record[i].PValue;
				if (!pText || (pText->DefaultLength == 0) || (pText->CurrLength == 0) ||
					((pText->CurrLength == 1) && (pText->DefaultLength == 1)))
				{
					continue;
				}

				pData = FSYS_Manage[id].PBuf + pText->Offset;
				if (pText->CurrLength == pText->DefaultLength)
				{
					for (k = 0; k < pText->CurrLength; k++)
					{
						if (pData[k] != pText->CurrValue[k])
							break;
					}

					if (k == pText->CurrLength)
						continue;
				}

				pHttpConn->Divide.Offset[tableIndex] = pText->Offset;
				pHttpConn->Divide.RecordIndex[tableIndex] = i;
				pHttpConn->Divide.PostType[tableIndex] = POST_TYPE_TEXT;

				tableIndex++;
			}
			else if (POST_Record[i].PostType == POST_TYPE_TAG)
			{
				
				pTag = POST_Record[i].PValue;
				if (pTag->DefaultLength == 0)
					continue;
				pHttpConn->Divide.Offset[tableIndex] = pTag->Offset;
				pHttpConn->Divide.RecordIndex[tableIndex] = i;
				pHttpConn->Divide.PostType[tableIndex] = POST_TYPE_TAG;

				tableIndex++;
			}
#if (MAX_POST_BUF_RADIO)
			else if (POST_Record[i].PostType == POST_TYPE_RADIO)
			{
				pRadio = POST_Record[i].PValue;
				if (pRadio->DefaultSet == pRadio->CurrentSet)
					continue;

				for (k = 0; k < pRadio->Count; k++)
				{
					pHttpConn->Divide.Offset[tableIndex] = pRadio->Offset[k];
					pHttpConn->Divide.RecordIndex[tableIndex] = i;
					pHttpConn->Divide.PostType[tableIndex] = POST_TYPE_RADIO;
					if (pRadio->CurrentSet == k)
					{
						pHttpConn->Divide.SetFlag[tableIndex] = 1;
						tableIndex++;
					}
					else if (pRadio->DefaultSet == k)
					{
						pHttpConn->Divide.SetFlag[tableIndex] = 2;
						tableIndex++;
					}
				}
			}
#endif
#if (MAX_POST_BUF_SELECT)
			else if (POST_Record[i].PostType == POST_TYPE_SELECT)
			{
				pSelect = POST_Record[i].PValue;
				if ((pSelect->DefaultSet == pSelect->CurrentSet) ||
						(pSelect->CurrentSet >= pSelect->Count))
				{
					continue;
				}
				for (k = 0; k < pSelect->Count; k++)
				{
					if (pSelect->DefaultSet == k)
					{
						pHttpConn->Divide.Offset[tableIndex] = pSelect->Offset[k];
						pHttpConn->Divide.RecordIndex[tableIndex] = i;
						pHttpConn->Divide.PostType[tableIndex] = POST_TYPE_SELECT;
						pHttpConn->Divide.SetFlag[tableIndex] = 2;
						tableIndex++;
					}
					else if (pSelect->CurrentSet == k)
					{
						pHttpConn->Divide.Offset[tableIndex] = pSelect->Offset[k];
						pHttpConn->Divide.RecordIndex[tableIndex] = i;
						pHttpConn->Divide.PostType[tableIndex] = POST_TYPE_SELECT;
						pHttpConn->Divide.SetFlag[tableIndex] = 1;
						tableIndex++;
					}
				}			
			}
#endif
			else
				continue;
		}
	}
	pHttpConn->Divide.Fragment = tableIndex;

} /* End of http_DivideHtmlFile() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: http_NewConfig
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T http_NewConfig(void)
{
#if (MAX_POST_BUF_RADIO)
	BUF_RADIO XDATA		*pRadio;
#endif
	BUF_TEXT XDATA		*pText;
#if (MAX_POST_BUF_PASSWORD)
	BUF_PASSWORD XDATA	*pPassword;
#endif
#if (MAX_POST_BUF_SELECT)
	BUF_SELECT XDATA	*pSelect;
#endif
	static U8_T			i; /* static for avoiding callback function issue */
	U8_T				change;
	void				(*f)(void *pWebData);

#if (HTTPD_DEBUG)
	printd ("POST count = %bx\n\r", HTTP_PostCount);
#endif
	for (i = 0; i < HTTP_PostCount; i++)
	{
#if (HTTPD_DEBUG)
		printd ("Post Table %bx\n\r", HTTP_PostTable[i]);
#endif
		if (POST_Record[HTTP_PostTable[i]].PostType == POST_TYPE_TEXT)
		{
			change = 0;
			pText = POST_Record[HTTP_PostTable[i]].PValue;

			if (pText == NULL)
				continue;

			if ((pText->CurrLength - 1) == pText->UserLength)
			{
				if (memcmp(pText->CurrValue, pText->UserValue, pText->CurrLength - 1))
					change = 1;
			}
			else
				change = 1;

			if (!change)
				continue;

			memcpy(pText->CurrValue, pText->UserValue, pText->UserLength);
			pText->CurrValue[pText->UserLength] = '"';
			pText->CurrLength =  pText->UserLength + 1 ;
			pText->UserLength++;

			f = fun_tbl[HTTP_PostTable[i]];
			f(pText);
		}
#if (MAX_POST_BUF_PASSWORD)		
		else if (POST_Record[HTTP_PostTable[i]].PostType == POST_TYPE_PASSWORD)
		{
			change = 0;
			pPassword = POST_Record[HTTP_PostTable[i]].PValue;

			if (pPassword == NULL)
				continue;

			if ((pPassword->CurrLength - 1) == pPassword->UserLength)
			{
				if (memcmp(pPassword->CurrValue, pPassword->UserValue, pPassword->CurrLength - 1))
					change = 1;
			}
			else
				change = 1;

			if (!change)
				continue;

			memcpy(pPassword->CurrValue, pPassword->UserValue, pPassword->UserLength);
			pPassword->CurrValue[pPassword->UserLength] = '"';
			pPassword->CurrLength =  pPassword->UserLength + 1 ;
			pPassword->UserLength++;

			f = fun_tbl[HTTP_PostTable[i]];
			f(pPassword);			
		}
#endif		
#if (MAX_POST_BUF_RADIO)
		else if (POST_Record[HTTP_PostTable[i]].PostType == POST_TYPE_RADIO)
		{
			change = 0;
			pRadio = POST_Record[HTTP_PostTable[i]].PValue;
			if (pRadio->UserSet != pRadio->CurrentSet)
			{
				pRadio->CurrentSet = pRadio->UserSet;
				change = 1;
			}

			if (!change)
				continue;

			f = fun_tbl[HTTP_PostTable[i]];
			f(&pRadio->CurrentSet);
		}
#endif
#if (MAX_POST_BUF_SELECT)
		else if (POST_Record[HTTP_PostTable[i]].PostType == POST_TYPE_SELECT)
		{
			change = 0;
			pSelect = POST_Record[HTTP_PostTable[i]].PValue;

			if (pSelect->UserSet != pSelect->CurrentSet)
			{
				pSelect->CurrentSet = pSelect->UserSet;
				change = 1;
			}

			if (!change)
				continue;

			f = fun_tbl[HTTP_PostTable[i]];
			f(&pSelect->CurrentSet);
		}
#endif
		else 
			continue;
	}

	return HTTP_POST_SUCCESS;

} /* End of http_NewConfig() */


/* End of httpd.c */
