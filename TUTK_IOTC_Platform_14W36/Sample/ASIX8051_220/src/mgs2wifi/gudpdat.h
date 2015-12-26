/*
 ******************************************************************************
 *     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name:gudpdat.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpdat.h,v $
 *
 *=============================================================================
 */

#ifndef __GGUDPDAT_H__
#define __GGUDPDAT_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define GUDPDAT_TX_BUF_SIZE				1472

/* TYPE DECLARATIONS */
typedef struct _GUDPDAT_CONN
{
    U32_T Ip;
    U16_T Port;
	U8_T  State;
	U8_T  UdpSocket;
    U32_T Timer;
    U32_T TxWaitTime;
	U32_T UrRxBytes;
    U32_T UrTxBytes;
    U16_T TxDatLen;
    U8_T  TxBuf[GUDPDAT_TX_BUF_SIZE];
#if GS2W_ENABLE_FLOW_CONTROL
    U8_T  FlowControlXonRx, FlowControlModemStatus, FlowControlModemCtrl;
#endif
} GUDPDAT_CONN;

/* GLOBAL VARIABLES */
extern U16_T GTCPDAT_GetRandomPortNum(void);

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void GUDPDAT_Task(void);
void GUDPDAT_Init(U16_T);
U8_T GUDPDAT_NewConn(U32_T XDATA*, U16_T, U8_T);
void GUDPDAT_Event(U8_T, U8_T);
void GUDPDAT_Receive(U8_T XDATA*, U16_T, U8_T);
U32_T GUDPDAT_GetTxBytes(U8_T id);
U32_T GUDPDAT_GetRxBytes(U8_T id);

#endif /* End of __GGUDPDAT_H__ */

/* End of gudpdat.h */
