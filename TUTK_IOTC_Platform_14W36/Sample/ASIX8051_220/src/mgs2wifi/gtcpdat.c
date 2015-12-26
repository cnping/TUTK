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
 /*============================================================================
 * Module Name: gtcpdat.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gtcpdat.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "adapter.h"
#include "gs2w.h"
#include "gconfig.h"
#include "gtcpdat.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart0.h"
#include "hsuart.h"
#include "hsuart2.h"
#include "printd.h"
#include <string.h>

/* NAMING CONSTANT DECLARATIONS */
#define GTCPDAT_MAX_CONNS				1
#define GTCPDAT_NO_NEW_CONN				0xFF

#define GTCPDAT_STATE_FREE				0
#define	GTCPDAT_STATE_WAIT				1
#define	GTCPDAT_STATE_CONNECTED			2

#define GTCPDAT_DATA_INDICATOR   		0x32
#define GTCPDAT_FLOW_CONTROL_INDICATOR	0x33

#define GTCPDAT_MAIN_CONNECT_TIME       (3 * 60 * (1000/SWTIMER_INTERVAL))   /* 3 minutes */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static GTCPDAT_CONN gtcpdat_Conns[GTCPDAT_MAX_CONNS];
static U8_T gtcpdat_InterAppId;
static U16_T gtcpdat_EthernetTxTimer;
static U8_T gtcpdat_TcpClient;
static U32_T elapse, time;

#if GS2W_ENABLE_FLOW_CONTROL			
static U8_T txBuf[6];
#endif

/* LOCAL SUBPROGRAM DECLARATIONS */
#if GS2W_ENABLE_FLOW_CONTROL			
static void gtcpdat_HandleFlowControlPacket(U8_T XDATA* pData, U16_T length, U8_T id);
static void gtcpdat_SetFlowControl(U8_T fCtrl);
#endif

static void gtcpdat_MaintainConnection(U8_T id);
static void gtcpdat_GetDataFromUr(U8_T id);
static void gtcpdat_SendData(U8_T);

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_GetRandomPortNum
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GTCPDAT_GetRandomPortNum(void)
{
    U16_T port;

    port = (U16_T)SWTIMER_Tick();

    if (port < 5000)
        port += 5000;

    return port;
} /* End of GTCPDAT_GetRandomPortNum() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GTCPDAT_Task(void)
{
    U8_T i;

    for (i = 0; i < GTCPDAT_MAX_CONNS; i++)
    {
        switch (gtcpdat_Conns[i].State)
        {
            case GTCPDAT_STATE_FREE:
                if (gtcpdat_TcpClient == 1)
                {
                    gtcpdat_Conns[i].Ip = GCONFIG_GetClientDestIP();
                    /* get destination ip */
                    if (gtcpdat_Conns[i].Ip == 0)
                        break;
                    gtcpdat_Conns[i].State = GTCPDAT_STATE_WAIT;
                    gtcpdat_Conns[i].Timer = SWTIMER_Tick();
                    gtcpdat_Conns[i].Port = GCONFIG_GetClientDestPort();
                    gtcpdat_Conns[i].TcpSocket = TCPIP_TcpNew(gtcpdat_InterAppId,
                                                              i,
                                                              gtcpdat_Conns[i].Ip,
                                                              GTCPDAT_GetRandomPortNum(),
                                                              gtcpdat_Conns[i].Port);

                    TCPIP_TcpConnect(gtcpdat_Conns[i].TcpSocket);
                    GS2W_SetTaskState(GS2W_STATE_TCP_DATA_PROCESS);

                    printd("Make a TCP connection with host ip %bu %bu %bu %bu at port %u\n\r",
                          (U8_T) ((gtcpdat_Conns[i].Ip >> 24) & 0x000000FF), (U8_T) ((gtcpdat_Conns[i].Ip >> 16) & 0x000000FF), 
                          (U8_T) ((gtcpdat_Conns[i].Ip >> 8) & 0x000000FF), (U8_T) (gtcpdat_Conns[i].Ip & 0x000000FF),
                          gtcpdat_Conns[i].Port);
                }
                break;
            case GTCPDAT_STATE_CONNECTED:
                {
#if GS2W_ENABLE_FLOW_CONTROL

                U8_T  modemStatus, modemCtrl;

                /* Flow control header: type    length   payload */
                /*                      ------  -------  ------- */
                /*                      1 byte  2 bytes   any    */
                /* length = length of payload                    */
                if (gtcpdat_Conns[i].LastTxPktAck == 1)
                {

                    modemStatus = HSUR2_GetModemStatus(TRUE);
                    modemCtrl = (HSUR2_GetModemControl() & 0x3);

                    if (gtcpdat_Conns[i].FlowControlModemStatus != modemStatus 
                        || modemCtrl != gtcpdat_Conns[i].FlowControlModemCtrl)
                    {
                       gtcpdat_Conns[i].FlowControlModemStatus = modemStatus;
                       gtcpdat_Conns[i].FlowControlModemCtrl = modemCtrl;
                       txBuf[0] = GTCPDAT_FLOW_CONTROL_INDICATOR;
                       txBuf[1] = 0;
                       txBuf[2] = 3;
                       txBuf[3] = MODEM_STATUS_CHANGE;
                       txBuf[4] = modemStatus;
                       txBuf[5] = modemCtrl;
                       gtcpdat_Conns[i].LastTxPktAck = 0;
                       TCPIP_TcpSend(gtcpdat_Conns[i].TcpSocket, &txBuf[0], 6, TCPIP_SEND_NOT_FINAL);
                    }
                }

                if (gtcpdat_Conns[i].TxDatLen == 0)
                {
                    gtcpdat_Conns[i].TxBuf[gtcpdat_Conns[i].TxDatLen++] = GTCPDAT_DATA_INDICATOR; // TCP data packet indicator
                    gtcpdat_Conns[i].TxDatLen += 2; /* reserved for length */
                }

                if (gtcpdat_Conns[i].FlowControlXonRx == TRUE)
                {
#endif
	                gtcpdat_GetDataFromUr(i);
                    gtcpdat_SendData(i);
#if GS2W_ENABLE_FLOW_CONTROL
                } // FlowControlXonRx
#endif
                gtcpdat_MaintainConnection(i);
                }
                break;
            default:
                break;
        }
    }
} /* End of GTCPDAT_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gtcpdat_MaintainConnection()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void gtcpdat_MaintainConnection(U8_T id)
{
    time = SWTIMER_Tick();

    if (time >= gtcpdat_Conns[id].Timer)
        elapse = time - gtcpdat_Conns[id].Timer;
    else
        elapse = (0xFFFFFFFF - gtcpdat_Conns[id].Timer) + time;

    if (elapse >= GTCPDAT_MAIN_CONNECT_TIME)
    {
        if ((STOE_GetIPAddr() & STOE_GetSubnetMask()) != (gtcpdat_Conns[id].Ip & STOE_GetSubnetMask()))
            ETH_SendArpRequest(STOE_GetGateway());
        else
            ETH_SendArpRequest(gtcpdat_Conns[id].Ip);
        gtcpdat_Conns[id].Timer = time;
    }
} /* End of gtcpdat_MaintainConnection() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gtcpdat_SendData()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void gtcpdat_SendData(U8_T id)
{
    GTCPDAT_CONN *tcpConn = &gtcpdat_Conns[id];

#if GS2W_ENABLE_FLOW_CONTROL
    if (tcpConn->LastTxPktAck == 1 && tcpConn->TxDatLen > 3)
#else
    if (tcpConn->LastTxPktAck == 1 && tcpConn->TxDatLen > 0)
#endif
    {
        if (tcpConn->TxDatLen >= GTCPDAT_MAX_TCP_DATA_LEN)
        {
            tcpConn->LastTxPktAck = 0;
#if GS2W_ENABLE_FLOW_CONTROL
            /* Fill in the length of payload */
            tcpConn->TxBuf[1] = (U8_T)((GTCPDAT_MAX_TCP_DATA_LEN-3) >> 8);
            tcpConn->TxBuf[2] = (U8_T)((GTCPDAT_MAX_TCP_DATA_LEN-3) & 0x00FF);
            tcpConn->TxSentLen = GTCPDAT_MAX_TCP_DATA_LEN;
#else
            tcpConn->TxSentLen = tcpConn->TxDatLen;
#endif
            TCPIP_TcpSend(tcpConn->TcpSocket, &tcpConn->TxBuf[0], tcpConn->TxSentLen, TCPIP_SEND_NOT_FINAL);   	
            tcpConn->TxWaitTime = 0;
            
        }
        else
        {
            time = SWTIMER_Tick();

            if (tcpConn->TxWaitTime == 0)
            {
                tcpConn->TxWaitTime = time;
            }
            else
            {
                if (time >= tcpConn->TxWaitTime)
                    elapse = time - tcpConn->TxWaitTime;
                else
                    elapse = (0xFFFFFFFF - tcpConn->TxWaitTime) + time;

                if (((elapse * SWTIMER_INTERVAL) >= gtcpdat_EthernetTxTimer))
                {
                    tcpConn->LastTxPktAck = 0;
#if GS2W_ENABLE_FLOW_CONTROL
                     /* Fill in the length of payload */
                    tcpConn->TxBuf[1] = (U8_T)((tcpConn->TxDatLen-3) >> 8);
                    tcpConn->TxBuf[2] = (U8_T)((tcpConn->TxDatLen-3) & 0x00FF);
#endif
                    TCPIP_TcpSend(tcpConn->TcpSocket, &tcpConn->TxBuf[0], tcpConn->TxDatLen, TCPIP_SEND_NOT_FINAL);	
                    tcpConn->TxWaitTime = 0;
                    tcpConn->TxSentLen = tcpConn->TxDatLen;
                }
            }
        }
    }
} /* End of gtcpdat_SendData() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gtcpdat_GetDataFromUr()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void gtcpdat_GetDataFromUr(U8_T id)
{
    U8_T *pUpBuf;
    U16_T availCount;
    GTCPDAT_CONN *tcpConn = &gtcpdat_Conns[id];

    availCount = HSUR2_GetRxDmaAvailCount();
    if (availCount == 0 || tcpConn->TxDatLen >= GTCPDAT_TX_BUF_SIZE)
        return;

    pUpBuf = &tcpConn->TxBuf[tcpConn->TxDatLen];

    if ((availCount+tcpConn->TxDatLen) >= GTCPDAT_TX_BUF_SIZE)
        availCount = GTCPDAT_TX_BUF_SIZE - tcpConn->TxDatLen;

    HSUR2_CopyUartToApp(pUpBuf, availCount);
    tcpConn->TxDatLen += availCount;
    tcpConn->UrRxBytes += availCount;

} /* End of gtcpdat_GetDataFromUr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_Init()
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GTCPDAT_Init(U16_T localPort)
{
	U8_T	i;

	gtcpdat_TcpClient = 0;

	for (i = 0; i < GTCPDAT_MAX_CONNS; i++)
    {
		gtcpdat_Conns[i].State = GTCPDAT_STATE_FREE;
        gtcpdat_Conns[i].UrRxBytes = 0;
        gtcpdat_Conns[i].UrTxBytes = 0;
        gtcpdat_Conns[i].TxDatLen = 0;
        gtcpdat_Conns[i].TxSentLen = 0;
        gtcpdat_Conns[i].LastTxPktAck = 0;
#if GS2W_ENABLE_FLOW_CONTROL			
        gtcpdat_Conns[i].FlowControlXonRx = TRUE;
        gtcpdat_Conns[i].FlowControlModemStatus = HSUR2_GetModemStatus(TRUE);
        gtcpdat_Conns[i].FlowControlModemCtrl = (HSUR2_GetModemControl() & 0x3);
#endif
    }

	gtcpdat_InterAppId = TCPIP_Bind(GTCPDAT_NewConn, GTCPDAT_Event, GTCPDAT_Receive);
	
	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_CLIENT) != GCONFIG_NETWORK_CLIENT)
	{	
		/* unicast packet */
		TCPIP_TcpListen(localPort, gtcpdat_InterAppId);
		
		printd("TCP data server init ok.\n\r");
	}
	else
	{
		gtcpdat_TcpClient = 1;
	
		printd("TCP data client init ok.\n\r");	
	}
	
	gtcpdat_EthernetTxTimer = GCONFIG_GetEthernetTxTimer();

} /* End of GTCPDAT_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GTCPDAT_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;

	for (i = 0; i < GTCPDAT_MAX_CONNS; i++)
	{
		if (gtcpdat_Conns[i].State == GTCPDAT_STATE_FREE)
		{
			gtcpdat_Conns[i].State = GTCPDAT_STATE_WAIT;
			gtcpdat_Conns[i].Timer = SWTIMER_Tick();
			gtcpdat_Conns[i].Ip = *pip;
			gtcpdat_Conns[i].Port = remotePort;
			gtcpdat_Conns[i].TcpSocket = socket;
            gtcpdat_Conns[i].TxDatLen = 0;
            gtcpdat_Conns[i].TxSentLen = 0;
            gtcpdat_Conns[i].LastTxPktAck = 1;
			printd("New TCP connection: id=%bu, remotePort=%u, socket=%bu\n\r", i, remotePort, socket);
			return i;
		}
	}
	
	return GTCPDAT_NO_NEW_CONN;

} /* End of GTCPDAT_NewConn() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GTCPDAT_Event(U8_T id, U8_T event)
{
    GTCPDAT_CONN *tcpConn = &gtcpdat_Conns[id];

	if (event == TCPIP_CONNECT_ACTIVE)
	{
		if (gtcpdat_Conns[id].State == GTCPDAT_STATE_WAIT)
		{
			gtcpdat_Conns[id].State = GTCPDAT_STATE_CONNECTED;
			GS2W_SetTaskState(GS2W_STATE_TCP_DATA_PROCESS);
            /* Enable keep-alive */
            TCPIP_TcpKeepAlive(gtcpdat_Conns[id].TcpSocket, TCPIP_KEEPALIVE_ON);
			gtcpdat_Conns[id].LastTxPktAck = 1;
			gtcpdat_Conns[id].TxDatLen = 0;
			gtcpdat_Conns[id].TxWaitTime = 0;
			gtcpdat_Conns[id].TxSentLen = 0;
		}
	}
	else if (event == TCPIP_CONNECT_CANCEL)
	{
		if (gtcpdat_Conns[id].State == GTCPDAT_STATE_CONNECTED ||
            gtcpdat_Conns[id].State == GTCPDAT_STATE_WAIT)
		{
			gtcpdat_Conns[id].State = GTCPDAT_STATE_FREE;
			GS2W_SetTaskState(GS2W_STATE_IDLE);			
		}
	}
	else if (event == TCPIP_CONNECT_XMIT_COMPLETE)
	{
		tcpConn->LastTxPktAck = 1;
        if ( tcpConn->TxSentLen > 0)
        {
            tcpConn->TxDatLen -= tcpConn->TxSentLen;
#if GS2W_ENABLE_FLOW_CONTROL
            if (tcpConn->TxDatLen > 0)
            {
                tcpConn->TxBuf[0] = GTCPDAT_DATA_INDICATOR;
                /* reserved the length field */
                memcpy(&tcpConn->TxBuf[3], &tcpConn->TxBuf[tcpConn->TxSentLen], tcpConn->TxDatLen);
                tcpConn->TxDatLen += 3;
                gtcpdat_GetDataFromUr(id);
            }
#else
            if (tcpConn->TxDatLen > 0)
                memcpy(&tcpConn->TxBuf[0], &tcpConn->TxBuf[tcpConn->TxSentLen], tcpConn->TxDatLen);
            gtcpdat_GetDataFromUr(id);
#endif
            tcpConn->TxSentLen = 0;
        }
	}
} /* End of GTCPDAT_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GTCPDAT_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
	if (gtcpdat_Conns[id].State == GTCPDAT_STATE_CONNECTED)
	{
		if (length <= GTCPDAT_MAX_TCP_RCV_DATA_LEN) // Is valid length
		{
#if GS2W_ENABLE_FLOW_CONTROL
MoreTcpData:
			if (length <= 6 && *pData == GTCPDAT_FLOW_CONTROL_INDICATOR)
			{
				gtcpdat_HandleFlowControlPacket(pData, length, id);
			}
			else if (length > 3 && *pData == GTCPDAT_DATA_INDICATOR) 
			{
                U16_T dataLen = (*(pData+1) << 8) + *(pData+2) + 3;

                if (dataLen > length) 
                    dataLen = length;

				HSUR2_XmitOutEnq((pData + 3), (dataLen - 3));
				gtcpdat_Conns[id].UrTxBytes += (dataLen - 3);
                
                length -= dataLen;
                if (length > 0)
                {
                    /* point to the next packet header */
                    pData += dataLen;
                    goto MoreTcpData;
                }
			}
#else
			HSUR2_XmitOutEnq(pData, length);
			gtcpdat_Conns[id].UrTxBytes += length;
#endif
		}
	}	
} /* End of GTCPDAT_Receive() */

#if GS2W_ENABLE_FLOW_CONTROL			
/*
 * ----------------------------------------------------------------------------
 * Function Name: gtcpdat_HandleFlowControlPacket
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gtcpdat_HandleFlowControlPacket(U8_T XDATA* pData, U16_T length, U8_T id)
{
	U8_T command = *(pData + 3);
	U8_T dat1;
	U8_T dat2;

	if (length >= 5)
	{
		dat1 = *(pData + 4);

		if (length == 6)
			dat2 = *(pData + 5);
	}

	switch (command)
	{
		case IOCTL_SERIAL_GET_DTRRTS:
            if (gtcpdat_Conns[id].LastTxPktAck == 1)
            {
                gtcpdat_Conns[id].LastTxPktAck = 0;
                txBuf[0] = GTCPDAT_FLOW_CONTROL_INDICATOR;
                txBuf[1] = 0;
                txBuf[2] = 2;
                txBuf[3] = IOCTL_SERIAL_GET_DTRRTS_REPLY;
                txBuf[4] = HSUR2_GetModemControl();
                TCPIP_TcpSend(gtcpdat_Conns[id].TcpSocket, &txBuf[0], 5, TCPIP_SEND_NOT_FINAL);
            }
			break;
		case IOCTL_SERIAL_GET_MODEM_CONTROL:
			if (gtcpdat_Conns[id].LastTxPktAck == 1)
            {
                gtcpdat_Conns[id].LastTxPktAck = 0;
                txBuf[0] = GTCPDAT_FLOW_CONTROL_INDICATOR;
                txBuf[1] = 0;
                txBuf[2] = 2;
                txBuf[3] = IOCTL_SERIAL_GET_MODEM_CONTROL_REPLY;
                txBuf[4] = HSUR2_GetModemControl();
                TCPIP_TcpSend(gtcpdat_Conns[id].TcpSocket, &txBuf[0], 5, TCPIP_SEND_NOT_FINAL);
            }
			break;
		case IOCTL_SERIAL_GET_MODEM_STATUS:
			if (gtcpdat_Conns[id].LastTxPktAck == 1)
            {
                gtcpdat_Conns[id].LastTxPktAck = 0;
                txBuf[0] = GTCPDAT_FLOW_CONTROL_INDICATOR;
                txBuf[1] = 0;
                txBuf[2] = 2;
                txBuf[3] = IOCTL_SERIAL_GET_MODEM_STATUS_REPLY;
                txBuf[4] = HSUR2_GetModemStatus(TRUE);
                TCPIP_TcpSend(gtcpdat_Conns[id].TcpSocket, &txBuf[0], 5, TCPIP_SEND_NOT_FINAL);
            }
			break;
		case IOCTL_SERIAL_CLR_DTR:
			HSUR2_ClearDTR();
			break;
		case IOCTL_SERIAL_CLR_RTS:
			HSUR2_ClearRTS();		
			break;
		case IOCTL_SERIAL_SET_DTR:
			HSUR2_SetDTR();
			break;
		case IOCTL_SERIAL_SET_RTS:
			HSUR2_SetRTS();
			break;
		case IOCTL_SERIAL_SET_BAUD_RATE:			
            HSUR2_SetupPort(GCONFIG_GetBaudRate(dat1), dat2);
			break;
		case IOCTL_SERIAL_SET_BREAK_OFF:
			dat1 = HSUR2_GetLineControl();
			HSUR2_SetLineControl(dat1 & 0xBF); // bit 6 - break control bit (1:on, 0:off)
			break;
		case IOCTL_SERIAL_SET_BREAK_ON:
			dat1 = HSUR2_GetLineControl();
			HSUR2_SetLineControl(dat1 | 0x40); // bit 6 - break control bit (1:on, 0:off)
			break;
		case IOCTL_SERIAL_SET_FIFO_CONTROL:
			HSUR2_SetFifoControl(dat1);
			break;
		case IOCTL_SERIAL_SET_LINE_CONTROL:
			HSUR2_SetLineControl(dat1);
			break;
		case IOCTL_SERIAL_SET_MODEM_CONTROL:
			HSUR2_SetModemControl(dat1);
			break;
		case IOCTL_SERIAL_SET_XOFF:
			gtcpdat_Conns[id].FlowControlXonRx = FALSE;
			break;
		case IOCTL_SERIAL_SET_XON:
			gtcpdat_Conns[id].FlowControlXonRx = TRUE;
			break;
        case IOCTL_SERIAL_SET_HANDFLOW:
            gtcpdat_SetFlowControl(dat1);
			break;
		case IOCTL_SERIAL_XOFF_COUNTER:
			//...
			break;

		default:
			printd("Unknown flow control command!\n\r");
			break;
	}
} /* End of gtcpdat_HandleFlowControlPacket() */

/*
 *--------------------------------------------------------------------------------
 * void gtcpdat_SetFlowControl(U8_T)
 * Purpose: 
 * Params : fCtrl: the flow control to be set.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void gtcpdat_SetFlowControl(U8_T fCtrl)
{
    switch (fCtrl)
    {
        case SERIAL_HANDFLOW_NONE:
            HSUR2_AutoCtsRtsFlowCtrl(0);
            HSUR2_AutoSoftwareFlowCtrl(0, 0, 0, 0, 0);
            break;
        case SERIAL_HANDFLOW_HW:
            HSUR2_AutoCtsRtsFlowCtrl(1);
            break;
        case SERIAL_HANDFLOW_XON_XOFF:
            HSUR2_AutoSoftwareFlowCtrl(1, (HSAFCSR_ASFCE_ENB), ASCII_XON, ASCII_XOFF, 0);
            break;
        default:
            break;
    }
}
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_GetTxBytes
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GTCPDAT_GetTxBytes(U8_T id)
{
    if (id < GTCPDAT_MAX_CONNS)
        return gtcpdat_Conns[id].UrTxBytes;

    return 0;
} /* End of GTCPDAT_GetTxBytes() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_GetRxBytes
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GTCPDAT_GetRxBytes(U8_T id)
{
    if (id < GTCPDAT_MAX_CONNS)
        return gtcpdat_Conns[id].UrRxBytes;

    return 0;
} /* End of GTCPDAT_GetRxBytes() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_GetTcpTxReady
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GTCPDAT_GetTcpTxReady(U8_T id)
{
    if (id < GTCPDAT_MAX_CONNS)
        return gtcpdat_Conns[id].LastTxPktAck;

    return 0;
} /* End of GTCPDAT_GetTcpTxReady() */

/* End of gtcpdat.c */