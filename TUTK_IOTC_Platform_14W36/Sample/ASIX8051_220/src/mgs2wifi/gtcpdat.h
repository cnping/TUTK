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
 * Module Name:gtcpdat.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gtcpdat.h,v $
 *
 *=============================================================================
 */

#ifndef __GGTCPDAT_H__
#define __GGTCPDAT_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define GTCPDAT_MAX_TCP_DATA_LEN        1344
#define GTCPDAT_TX_BUF_SIZE       	    (GTCPDAT_MAX_TCP_DATA_LEN * 2)
#define GTCPDAT_MAX_TCP_RCV_DATA_LEN    1460

/* TYPE DECLARATIONS */
typedef struct _GTCPDAT_CONN
{
	U32_T Ip;
	U16_T Port;
	U8_T  State;
	U8_T  TcpSocket;
	U32_T Timer;
    U32_T TxWaitTime;
	U32_T UrRxBytes;
    U32_T UrTxBytes;
    U16_T TxDatLen;
    U16_T TxSentLen; /* tcp packet length was sent */
    U8_T  TxBuf[GTCPDAT_TX_BUF_SIZE];
    U8_T  LastTxPktAck;               /* 1: complete 0: not complete */
#if GS2W_ENABLE_FLOW_CONTROL
    U8_T  FlowControlXonRx, FlowControlModemStatus, FlowControlModemCtrl;
#endif

} GTCPDAT_CONN;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void GTCPDAT_Task(void);
void GTCPDAT_Init(U16_T);
U8_T GTCPDAT_NewConn(U32_T XDATA*, U16_T, U8_T);
void GTCPDAT_Event(U8_T, U8_T);
void GTCPDAT_Receive(U8_T XDATA*, U16_T, U8_T);
U16_T GTCPDAT_GetRandomPortNum(void);
U32_T GTCPDAT_GetTxBytes(U8_T id);
U32_T GTCPDAT_GetRxBytes(U8_T id);
U8_T GTCPDAT_GetTcpTxReady(U8_T id);

#endif /* End of __GGTCPDAT_H__ */

/* End of gtcpdat.h */
