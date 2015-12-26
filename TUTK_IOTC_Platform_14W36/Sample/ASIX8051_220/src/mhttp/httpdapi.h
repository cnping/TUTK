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
 * Module Name: httpapi.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: httpapi.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __HTTPAPI_H__
#define __HTTPAPI_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "httpd.h"

/* TYPE DECLARATIONS */

/* NAMING CONSTANT DECLARATIONS */
/* Error Code Declarations */
#define HTTPAPI_OK          			0x00
#define HTTPAPI_ERR         			0xff

/* Status Code Declarations */
#define	HTTPAPI_400_BadRequest			0
#define	HTTPAPI_401_Unauthorized		1
#define	HTTPAPI_404_NotFound			2
#define	HTTPAPI_405_MethodNotAllowed	3
#define	HTTPAPI_406_NotAccepTable		4
	
/* Build Element Support Type */
#define HTTPAPI_FmtOnly					0
#define HTTPAPI_Hidden					1

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
// Generic data convertion //
U16_T HTTPAPI_Str2Short(U8_T* pBuf, U8_T len);
U8_T HTTPAPI_Short2Str(U16_T port, U8_T* pBuf);
U8_T HTTPAPI_Ulong2IpAddr(U32_T ip, U8_T* pbuf);
U32_T HTTPAPI_IpAddr2Ulong(U8_T* pBuf, U8_T len);
U8_T HTTPAPI_Num2HexText(U8_T *pHexText, U8_T *pNum, U8_T NumLen);
U8_T HTTPAPI_HexText2Num(U8_T *pNum, U8_T *pHexText, U8_T HexTextLen);
U16_T HTTPAPI_MIMEbase64Decode(U8_T *pDst, U8_T *pSur, U16_T len);

// Parse request data //
U8_T HTTPAPI_ParseAuthenHeader(U8_T *pDst, U8_T *pSur, U8_T **pUserName, U8_T **pPassWord);

// Build response data //
U16_T HTTPAPI_BuildGenericHeaders(U8_T **pDst, U8_T fid);
U16_T HTTPAPI_BuildMessageBody(U8_T **pDst, U8_T Opt, U8_T *pName, U8_T *pId, const char *fmt, ...);

// Send response data //
void HTTPAPI_ResponseAuthenMessage(U8_T *pDst, U8_T *pMsg, HTTP_SERVER_CONN *pConn);
void HTTPAPI_ResponseRedirectMesssge(U8_T *pDst, U8_T fid, HTTP_SERVER_CONN *pConn);
void HTTPAPI_ResponseErrMessage(U8_T *pDst, U8_T status, HTTP_SERVER_CONN *pConn);
void HTTPAPI_ResponseResource(U8_T *pSur, U16_T len, U8_T fid, HTTP_SERVER_CONN *pConn);
#endif /* End of __HTTPAPI_H__ */


/* End of httpapi.h */
