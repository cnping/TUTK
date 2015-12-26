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
 * Module Name: httpapi.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: httpapi.c,v $
 * no message
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "httpdapi.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "tcpip.h"
#include "filesys.h"

/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */

/* LOCAL SUBPROGRAM DECLARATIONS */
static U8_T httpapi_StrCpyReturnLen(U8_T **pDst, U8_T *pSur);
static U8_T httpapi_BuildContentTypeHeader(U8_T **pDst, U8_T opt);

/*
 * ----------------------------------------------------------------------------
 * Function Name: httpapi_StrCpyReturnLen()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T httpapi_StrCpyReturnLen(U8_T **pDst, U8_T *pSur)
{
	U8_T len;
	
	for (len=0; pSur[len] != '\0'; len++)
	{
		(*pDst)[len] = pSur[len];
	}
	(*pDst) += len;
	return len;
} /* End of httpapi_StrCpyReturnLen() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_ResponseErrMessage()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTPAPI_ResponseErrMessage(U8_T *pDst, U8_T status, HTTP_SERVER_CONN *pConn)
{
	U16_T len;
	U8_T *pSur = pDst, *pStatusCode;

	switch (status)
	{
	case HTTPAPI_400_BadRequest:
		pStatusCode = "400 Bad_Request"; break;
	case HTTPAPI_404_NotFound:
	default:	
		pStatusCode = "404 Not_Found"; break;
	case HTTPAPI_405_MethodNotAllowed:
		pStatusCode = "405 Method_Not_Allowed"; break;
	case HTTPAPI_406_NotAccepTable:
		pStatusCode = "406 Not_Acceptable"; break;
	}
		
	/* Build response start line */
	len = httpapi_StrCpyReturnLen(&pDst, "HTTP/1.1");
	len += httpapi_StrCpyReturnLen(&pDst," ");	
	len += httpapi_StrCpyReturnLen(&pDst, pStatusCode);
	len += httpapi_StrCpyReturnLen(&pDst,"\r\n");
			
	/* Build generic headers */
	len += httpapi_StrCpyReturnLen(&pDst, "Cache-control: no-cache\r\n");
	len += httpapi_StrCpyReturnLen(&pDst, "Connection: keep-alive\r\n");
	len += httpapi_StrCpyReturnLen(&pDst, "Content-type: text/html\r\n");
	
	/* Build empty line */
	len += httpapi_StrCpyReturnLen(&pDst, "\r\n");
	
	/* Build error message */
	len += httpapi_StrCpyReturnLen(&pDst, "<html><head><title>");	
	len += httpapi_StrCpyReturnLen(&pDst, "Document_Error");
	len += httpapi_StrCpyReturnLen(&pDst, "</title></head><body><h2>Access Error: ");
	len += httpapi_StrCpyReturnLen(&pDst, pStatusCode);
	len += httpapi_StrCpyReturnLen(&pDst, "</h2></body></html>");
			
	/* Send response message */
	TCPIP_TcpSend(pConn->TcpSocket, pSur, len, TCPIP_SEND_FINAL);
	pConn->State = HTTP_STATE_SEND_FINAL;		
} /* End of HTTPAPI_ResponseErrMessage() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_ResponseRedirectMesssge()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTPAPI_ResponseRedirectMesssge(U8_T *pDst, U8_T fid, HTTP_SERVER_CONN *pConn)
{
	U16_T len;
	U8_T *pSur = pDst;
	
	/* Build response start line */
	len = httpapi_StrCpyReturnLen(&pDst, "HTTP/1.1");
	len += httpapi_StrCpyReturnLen(&pDst," ");	
	len += httpapi_StrCpyReturnLen(&pDst, "302 Re-direction");
	len += httpapi_StrCpyReturnLen(&pDst,"\r\n");
	
	/* Build generic headers */
	len += httpapi_StrCpyReturnLen(&pDst, "Cache-control: no-cache\r\n");
	len += httpapi_StrCpyReturnLen(&pDst, "Connection: keep-alive\r\n");
	len += httpapi_StrCpyReturnLen(&pDst, "Content-type: text/html\r\n");

	/* Build redirection header */
	len += httpapi_StrCpyReturnLen(&pDst, "Location: ");
	len += httpapi_StrCpyReturnLen(&pDst, FSYS_Manage[fid].FName);
	len += httpapi_StrCpyReturnLen(&pDst, "\r\n");
	
	/* Build empty line */
	len += httpapi_StrCpyReturnLen(&pDst, "\r\n");
	
	/* Send response message */
	TCPIP_TcpSend(pConn->TcpSocket, pSur, len, TCPIP_SEND_FINAL);
	pConn->State = HTTP_STATE_SEND_FINAL;	
} /* End of HTTPAPI_ResponseRedirectMesssge() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_ResponseAuthenMessage()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTPAPI_ResponseAuthenMessage(U8_T *pDst, U8_T *pMsg, HTTP_SERVER_CONN *pConn)
{
	U16_T len;
	U8_T *pSur = pDst;
	
	/* Build response start line */
	len = httpapi_StrCpyReturnLen(&pDst, "HTTP/1.1");
	len += httpapi_StrCpyReturnLen(&pDst," ");	
	len += httpapi_StrCpyReturnLen(&pDst, "401 Unauthorized");
	len += httpapi_StrCpyReturnLen(&pDst,"\r\n");
	
	/* Build generic headers */
	len += httpapi_StrCpyReturnLen(&pDst, "Cache-control: no-cache\r\n");
	len += httpapi_StrCpyReturnLen(&pDst, "Connection: keep-alive\r\n");
	len += httpapi_StrCpyReturnLen(&pDst, "Content-type: text/html\r\n");
	len += httpapi_StrCpyReturnLen(&pDst, "WWW-Authenticate: BASIC realm=\"");
	len += httpapi_StrCpyReturnLen(&pDst, pMsg);
	len += httpapi_StrCpyReturnLen(&pDst, "\"\r\n");

	/* Build empty line */
	len += httpapi_StrCpyReturnLen(&pDst, "\r\n");
	
	/* Send response message */
	TCPIP_TcpSend(pConn->TcpSocket, pSur, len, TCPIP_SEND_FINAL);
	pConn->State = HTTP_STATE_SEND_FINAL;
} /* End of HTTPAPI_ResponseAuthenMessage() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_BuildGenericHeaders()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T HTTPAPI_BuildGenericHeaders(U8_T **pDst, U8_T fid)
{
	U16_T len;
	
	/* Build start line */	
	len = httpapi_StrCpyReturnLen(pDst, "HTTP/1.1");
	len += httpapi_StrCpyReturnLen(pDst," ");	
	len += httpapi_StrCpyReturnLen(pDst, "200 Ok");
	len += httpapi_StrCpyReturnLen(pDst,"\r\n");
	
	/* Build message headers */	
	len += httpapi_StrCpyReturnLen(pDst, "Cache-control: no-cache\r\n");
	len += httpapi_StrCpyReturnLen(pDst, "Connection: keep-alive\r\n");
	len += httpapi_StrCpyReturnLen(pDst, "Content-type: ");
	switch (FSYS_Manage[fid].FType)
	{
	default:			
	case FILE_TYPE_HTML:
		len += httpapi_StrCpyReturnLen(pDst, "text/html");	break;
	case FILE_TYPE_BIN:
		len += httpapi_StrCpyReturnLen(pDst, "image/jpeg"); break;	
	case FILE_TYPE_JS:
		len += httpapi_StrCpyReturnLen(pDst, "application/javascript"); break;	
	case FILE_TYPE_CSS:
		len += httpapi_StrCpyReturnLen(pDst, "text/css"); break;	
	}	
	len += httpapi_StrCpyReturnLen(pDst, "\r\n");	
	
	/* Build empty line */
	len += httpapi_StrCpyReturnLen(pDst, "\r\n");	
	
	return len;
} /* End of HTTPAPI_BuildGenericHeaders() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_BuildMessageBody()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T HTTPAPI_BuildMessageBody(U8_T **pDst, U8_T Opt, U8_T *pName, U8_T *pId, const char *fmt, ...)
{
	U16_T len = 0, lentmp = 0;
   	va_list args;

	switch (Opt)
	{
	/* Build pure text message */	
	default:
	case HTTPAPI_FmtOnly:
		if (fmt)
		{	
			va_start(args, fmt);
			len = vsprintf(*pDst,fmt,args);
			va_end(args);
			(*pDst) += len;
		}
		break;
		
	/* Build hidden element */
	case HTTPAPI_Hidden:
		len += httpapi_StrCpyReturnLen(pDst, "<input type=\"hidden\" ");
		if (pName)
		{
			len += httpapi_StrCpyReturnLen(pDst, "name=\"");
			len += httpapi_StrCpyReturnLen(pDst, pName);		
			len += httpapi_StrCpyReturnLen(pDst, "\" ");
		}
		if (pId)
		{
			len += httpapi_StrCpyReturnLen(pDst, "id=\"");
			len += httpapi_StrCpyReturnLen(pDst, pId);		
			len += httpapi_StrCpyReturnLen(pDst, "\" ");
		}
		if (fmt)
		{
			len += httpapi_StrCpyReturnLen(pDst, "value=\"");
			va_start(args, fmt);
			lentmp = vsprintf(*pDst,fmt,args);
			va_end(args);
			len += lentmp;
			(*pDst) += lentmp;			
			len += httpapi_StrCpyReturnLen(pDst, "\" ");
		}
		len += httpapi_StrCpyReturnLen(pDst, "/>");		
		break;
	}
	
	return len;
} /* End of HTTPAPI_BuildMessageBody() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_ResponseResource()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTPAPI_ResponseResource(U8_T *pSur, U16_T len, U8_T fid, HTTP_SERVER_CONN *pConn)
{
	pConn->FileId = fid;
	
	/* Send response message */
	TCPIP_TcpSend(pConn->TcpSocket, pSur, len, TCPIP_SEND_NOT_FINAL);
	pConn->State = HTTP_STATE_SEND_HEADER;	
} /* End of HTTPAPI_ResponseResource() */


/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_Str2Short()
 * Purpose: Transfer a string to ushort number.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T HTTPAPI_Str2Short(U8_T* pBuf, U8_T len)
{
	U32_T	value = 0;

	while (len--)
	{
		value *= 10;
 
		if ((*pBuf < 0x3a) && (*pBuf > 0x2f))
			value += (*pBuf - 0x30);
		else
			return 0xffff;

		pBuf++;
	}
	if (value & 0xFFFF0000) return 0xFFFF;
	return (U16_T)(value & 0xFFFF);

} /* End of HTTPAPI_Str2Short() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_Short2Str
 * Purpose: Transfer a unsigned short number to decimal string.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T HTTPAPI_Short2Str(U16_T value, U8_T* pBuf)
{
	U8_T	loop, index = 0;
	U8_T	tmp[5];

	while (value)
	{
		tmp[index] = value % 10;
		index++;
		value /= 10;
	}

	for (loop = 0; loop < index; loop++)
	{
		pBuf[loop] = tmp[index -loop -1] + 0x30;
	}

	return index;

} /* End of HTTPAPI_Short2Str() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_Ulong2IpAddr()
 * Purpose: Convertion a unsigne long to dotted decimal notation string
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T HTTPAPI_Ulong2IpAddr(U32_T ip, U8_T* pbuf)
{
	U8_T*	point = (U8_T*)&ip;
	U8_T	i = sizeof (U32_T);
	U8_T	temp, value[3];
	U8_T	j, len = 0;

	while (i--)
	{
		temp = *point++;
		if (temp == 0)
		{
			*pbuf++ = 0x30;
			len++;
		}
		else
		{
			j = 3;
			while (j--)
			{
				value[j] = (temp % 10) + 0x30;
				temp /= 10;
			}

			if (value[0] != '0')
			{
				*pbuf++ = value[0];
				*pbuf++ = value[1];
				*pbuf++ = value[2];
				len += 3;
			}
			else if (value[1] != '0')
			{
				*pbuf++ = value[1];
				*pbuf++ = value[2];
				len += 2;
			}
			else
			{
				*pbuf++ = value[2];
				len++;
			}
		}

		if (i)
		{
			*pbuf++ = '.';
			len++;
		}
	}

	return len;
			
} /* End of HTTPAPI_Ulong2IpAddr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_IpAddr2Ulong()
 * Purpose: Convertion dotted decimal notation string to a unsigne long
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T HTTPAPI_IpAddr2Ulong(U8_T* pBuf, U8_T len)
{
	U32_T	ip = 0;
	U16_T	value16;
	U8_T*	point = (U8_T*)&ip;
	U8_T	count = 0;

	while (1)
	{
		value16 = 0;

		while ((*pBuf != '.') && len)
		{
			*point *= 10;
			value16 *= 10;
  
			if ((*pBuf < 0x3a) && (*pBuf > 0x2f))
			{
				*point += (*pBuf - 0x30);
				value16 += (*pBuf - 0x30);
			}
			else
				return 0xffffffff;

			pBuf++;
			len--;
		}

		if (value16 > 255)
			return 0xffffffff;

		if (len == 0)
			break;

		pBuf++;
		len--;
		count++;
		point++;

		if (count > 3)
			return 0xffffffff;
	}

	if (count != 3)
		return 0xffffffff;

	return ip;
} /* End of HTTPAPI_IpAddr2Ulong() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: U8_T HTTPAPI_Num2HexText()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T HTTPAPI_Num2HexText(U8_T *pHexText, U8_T *pNum, U8_T NumLen)
{
	U8_T Char, i, HexTextLen;
	
	for (i = 0, HexTextLen = 0; i < NumLen; i ++)
	{
		Char = pNum[i] >> 4;
		if (Char > 9)
			Char += 0x57;
		else
			Char += 0x30;
		pHexText[HexTextLen++] = Char;
	
		Char = (pNum[i]) & 0x0f;
		if (Char > 9)
			Char += 0x57;
		else
			Char += 0x30;
		pHexText[HexTextLen++] = Char;
	}
	pHexText[HexTextLen] = '\0';
	
	return HexTextLen;
} /* End of HTTPAPI_Num2HexText() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: U8_T HTTPAPI_HexText2Num()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T HTTPAPI_HexText2Num(U8_T *pNum, U8_T *pHexText, U8_T HexTextLen)
{
	U8_T i, NumLen;
	
	for (i = 0, NumLen = 0; i < HexTextLen; i ++)
	{
		if (pHexText[i] >= '0' && pHexText[i] <= '9')
			pNum[NumLen] = (pHexText[i++] - 0x30) << 4;
		else if (pHexText[i] >= 'a' && pHexText[i] <= 'f')
			pNum[NumLen] = (pHexText[i++] - 0x61) << 4;		
		else if (pHexText[i] >= 'A' && pHexText[i] <= 'F')
			pNum[NumLen] = (pHexText[i++] - 0x41) << 4;
		else
			return 0;// not a valid character //
			
		if (!(i < HexTextLen))
		{
			NumLen ++;
			break;
		}
		
		if (pHexText[i] >= '0' && pHexText[i] <= '9')
			pNum[NumLen++] |= (pHexText[i] - 0x30);
		else if (pHexText[i] >= 'a' && pHexText[i] <= 'f')
			pNum[NumLen++] |= (pHexText[i] - 0x61);		
		else if (pHexText[i] >= 'A' && pHexText[i] <= 'F')
			pNum[NumLen++] |= (pHexText[i] - 0x41);
		else
			return 0;// not a valid character //
	}
		
	return NumLen;
} /* End of HTTPAPI_HexText2Num() */


/*
 * ----------------------------------------------------------------------------
 * Function Name: U16_T HTTPAPI_MIMEbase64Decode()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T HTTPAPI_MIMEbase64Decode(U8_T *pDst, U8_T *pSur, U16_T len)
{
	U8_T i, tmp[4];
	U16_T j = 0;

	while (len)
	{
		// get 4 bytes from source buffer //
		for (i = 0; i < 4; i ++)
		{
			if (len)
			{
				tmp[i] = *pSur++;
				len --;
			}
			else
				tmp[i] = 0;
		}

		// decode start //
		for (i = 0; i < 4; i++)
		{
			if (tmp[i] > ('A'-1) && tmp[i] < ('Z'+1))
				tmp[i] -= 65;
			else if (tmp[i] > ('a'-1) && tmp[i] < ('z'+1))
				tmp[i] -= 71;
			else if (tmp[i] > ('0'-1) && tmp[i] < ('9'+1))
				tmp[i] += 4;
			else if (tmp[i] == '+')
				tmp[i] += 19;
			else if (tmp[i] == '/')
				tmp[i] += 16;
			else if (tmp[i] == '=')
				tmp[i] = 0;
		}
		
		// export data to destination buffer //
		*pDst++ = (tmp[0] << 2) | (tmp[1] >> 4);
		if (*(pDst - 1) != 0) j ++;
		*pDst++ = (tmp[1] << 4) | (tmp[2] >> 2);
		if (*(pDst - 1) != 0) j ++;		
		*pDst++ = (tmp[2] << 6) | tmp[3];
		if (*(pDst - 1) != 0) j ++;		
	}
	return j;
} /* End of HTTPAPI_MIMEbase64Decode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: httpapi_SearchStrStopByTag()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T httpapi_SearchStrStopByTag(U8_T *pSur, U8_T *pRef, U8_T *pStop)
{
	U16_T i;
	U8_T c;

	for (i = 0; i < 1600; i ++)
	{
		for (c = 0; pRef[c] != '\0'; c ++)
		{
			if (pSur[i+c] != pRef[c])
				break;
		}
		if (pRef[c] == '\0')
			return i;// found return offset //
			
		for (c = 0; pStop[c] != '\0'; c ++)
		{
			if (pSur[i+c] != pStop[c])
				break;
		}
		if (pStop[c] == '\0')
			return 0xffff;		
	}
	return 0xffff;
} /* End of httpapi_SearchStrStopByTag() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTPAPI_ParseAuthenHeader()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T HTTPAPI_ParseAuthenHeader(U8_T *pDst, U8_T *pSur, U8_T **pUserName, U8_T **pPassWord)
{
	U16_T offset;

	offset = httpapi_SearchStrStopByTag(pSur, "Authorization: Basic ", "\r\n\r\n");
	
	if (offset != 0xffff)
	{
		pSur += offset + 21;
		offset = httpapi_SearchStrStopByTag(pSur, "\r\n", "\r\n\r\n");
		if (offset == 0xffff) return HTTPAPI_ERR;
		offset = HTTPAPI_MIMEbase64Decode(pDst, pSur, offset);
		
		*pUserName = pDst; // set username pointer //
		pDst[offset] = '\0';
		offset = httpapi_SearchStrStopByTag(pDst, ":", " ");
		pDst[offset] = '\0';
		*pPassWord = &pDst[offset+1]; // set password pointer //
		return HTTPAPI_OK;
	}
	return HTTPAPI_ERR;// not found //
} /* End of HTTPAPI_ParseAuthenHeader() */

/* End of httpapi.c */
