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
 * Module Name:adapter.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: adapter.h,v $
 *=============================================================================
 */

#ifndef __ADAPTER_H__
#define __ADAPTER_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "adapter_cfg.h"
#include "stoe_cfg.h"

/* NAMING CONSTANT DECLARATIONS */
#define ETH_STOP_ARP_REQUEST_TO_GATEWAY_AFTER_REPLY			1
#define ETH_CONTINUE_ARP_REQUEST_TO_GATEWAY_AFTER_REPLY		0

/* GLOBAL VARIABLES */
extern U8_T ETH_IsPppoe;
extern U8_T ETH_DoDhcp;
extern U8_T ETH_PppoeHeaderLen;
extern U8_T ETH_DhcpArpReply;
#define ETH_GetPppoeFlag() (ETH_IsPppoe)
#define ETH_SetPppoeFlag(a) (ETH_IsPppoe = a)
#define ETH_GetDhcpFlag() (ETH_DoDhcp)
#define ETH_SetDhcpFlag(a) (ETH_DoDhcp = a)
#define ETH_GetPppoeHeaderLen() (ETH_PppoeHeaderLen)
#define ETH_SetPppoeHeaderLen(a) (ETH_PppoeHeaderLen = a)
#define ETH_SetDhcpArpReplyFlag(a) (ETH_DhcpArpReply = a)
#define ETH_CheckDhcpArpReply() (ETH_DhcpArpReply)

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/*-------------------------------------------------------------*/
void ETH_Init(void);
void ETH_Start(void);
void ETH_Send(U16_T);
void ETH_SendPkt(U8_T XDATA*, U16_T);
void ETH_SendArpToGateway(BOOL);
void ETH_SendArpRequest(U32_T dstIp);
void ETH_RestartSendArpToGateway(void);
void ETH_SendDhcpArpRequest(void);


#endif /* End of __ADAPTER_H__ */


/* End of adapter.h */