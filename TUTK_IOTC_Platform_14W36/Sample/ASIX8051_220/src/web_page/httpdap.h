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
 * Module Name: httpdap.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: httpdap.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __HTTPDAP_H__
#define __HTTPDAP_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "httpd.h"
#include "filesys.h"

/* NAMING CONSTANT DECLARATIONS */

/* TYPE DECLARATIONS */

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void HTTPAP_Init(HTTP_SERVER_INFO *pInfo);
U8_T HTTPAP_ResponseComplete(HTTP_SERVER_CONN *pConn, HTTP_SERVER_INFO *pInfo);
void HTTPAP_DivideHtmlFile(HTTP_SERVER_CONN *pConn, U8_T id);
void HTTPAP_SendMoreWebFragment(U8_T index, HTTP_SERVER_CONN *pConn);
U8_T HTTPAP_PostMethodPreParseProcess(HTTP_SERVER_CONN *pConn, HTTP_SERVER_INFO *pInfo);
U8_T HTTPAP_PostMethodDeferParseProcess(HTTP_SERVER_CONN *pConn, HTTP_SERVER_INFO *pInfo);
U8_T HTTPAP_PostMethodPreUpdateProcess(HTTP_SERVER_CONN *pConn, HTTP_SERVER_INFO *pInfo);
U8_T HTTPAP_PostMethodDeferProcess(U8_T postSuccessFlag, HTTP_SERVER_CONN *pConn, HTTP_SERVER_INFO *pInfo);
U8_T HTTPAP_GetMethodProcess(HTTP_SERVER_CONN *pConn, HTTP_SERVER_INFO *pInfo);

/*===========================================================================*/
/*============== Functions Supplied by Customers' Application ===============*/
/*===========================================================================*/
typedef struct _HTTPDAP_INFO
{
	U16_T	SecurityTimeExpired[MAX_HTTP_CONNECT];// Unit in 50ms //
	U32_T	SecurityRemoteIP[MAX_HTTP_CONNECT];
	
	U8_T	AccIPOnOff;
	U8_T	Cmd;
} HTTPDAP_INFO;

void HTTPAP_Task(void);

#endif /* End of __HTTPDAP_H__ */


/* End of httpdap.h */
