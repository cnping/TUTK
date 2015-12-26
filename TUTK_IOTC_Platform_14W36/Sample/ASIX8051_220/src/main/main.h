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
 * Module Name:main.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: main.h,v $
 *=============================================================================
 */

#ifndef __MAIN_H__
#define __MAIN_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define COMMAND_UNKNOW		0
#define COMMAND_HAD_DONE	1

/* Protocol modules - 1:include 0:exclude */
#define INCLUDE_DNS_CLIENT  1
#define INCLUDE_DHCP_CLIENT 1
#define INCLUDE_DHCP_SERVER 1
#define INCLUDE_TELNET_SERVER 1
#define INCLUDE_EVENT_DETECT 0
#define INCLUDE_HTTP_SERVER 1

#define MAX_DHCP_DECLINE_COUNT				10
/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/*-------------------------------------------------------------*/
#if (INCLUDE_DHCP_CLIENT)
void MAIN_RestartDhcpClient();
#endif

#endif /* End of __MAIN_H__ */

/* End of main.h */