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
 * Module Name: gudpdat.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpdat.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "adapter.h"
#include "gs2w.h"
#include "gconfig.h"
#include "gudpdat.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart0.h"
#include "hsuart.h"
#include "hsuart2.h"
#include "printd.h"
#include <string.h>

/* NAMING CONSTANT DECLARATIONS */
#define GUDPDAT_MAX_DATA_LEN		    1472 // maximun UDP payload length
#define GUDPDAT_MAX_CONNS				1 // maximun UDP connections, current version only supports 
                                          // one connection
#define GUDPDAT_NO_NEW_CONN				0xFF

#define GUDPDAT_STATE_FREE				0
#define GUDPDAT_STATE_CONNECTING        1
#define	GUDPDAT_STATE_WAIT				2
#define	GUDPDAT_STATE_CONNECTED			3

#define GUDPDAT_CLOSE_INDICATOR	    	0x30
#define GUDPDAT_CONNECT_INDICATOR   	0x31
#define GUDPDAT_DATA_INDICATOR   		0x32
#define GUDPDAT_FLOW_CONTROL_INDICATOR	0x33
#define GUDPDAT_MAIN_CONNECT_TIME       (3 * 60 * (1000/SWTIMER_INTERVAL))   /* 3 minutes */

/* MACRO DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static GUDPDAT_CONN gudpdat_Conns[GUDPDAT_MAX_CONNS];
static U8_T gudpdat_InterAppId;
//static U16_T gudpdat_Port;
static U16_T gudpdat_EthernetTxTimer;
static U8_T gudpdat_UdpClient;
static U8_T gudpdat_UdpAutoConnect;
static U32_T elapse, time;
static U8_T txBuf[6];

/* LOCAL SUBPROGRAM DECLARATIONS */
#if GS2W_ENABLE_FLOW_CONTROL			
static void gudpdat_HandleFlowControlPacket(U8_T XDATA *pData, U16_T length, U8_T id);
static void gudpdat_SetFlowControl(U8_T fCtrl);
#endif
static void gudpdat_GetDataFromUr(U8_T id);
static void gudpdat_SendData(U8_T id);
static void gudpdat_MaintainConnection(U8_T id);

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPDAT_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPDAT_Task(void) 
{
	U8_T i;
    U32_T dip;
    U16_T dport;

    for (i = 0; i < GUDPDAT_MAX_CONNS; i++)
    {
        switch(gudpdat_Conns[i].State)
        {
            case GUDPDAT_STATE_FREE:
                if (gudpdat_UdpClient == TRUE)
                {
                    dip = GCONFIG_GetClientDestIP();
		            dport = GCONFIG_GetClientDestPort(); 
        
                    /* get destination ip */
                    if (dip == 0)
                        break;
        
		            printd("Make a UDP connection with host ip:%bu %bu %bu %bu port:%u\n\r",
		                   (U8_T) ((dip >> 24) & 0x000000FF), (U8_T) ((dip >> 16) & 0x000000FF), 
				           (U8_T) ((dip >> 8) & 0x000000FF), (U8_T) (dip & 0x000000FF), (U16_T) dport);
		
				    gudpdat_Conns[i].State = GUDPDAT_STATE_CONNECTING;
                    gudpdat_Conns[i].Timer = SWTIMER_Tick();
                    gudpdat_Conns[i].Ip = dip;
                    gudpdat_Conns[i].Port = dport;
				    gudpdat_Conns[i].UdpSocket = TCPIP_UdpNew(gudpdat_InterAppId,
                                                              i,	
				                                              dip,
														      GTCPDAT_GetRandomPortNum(),
														      dport);
                    /* Send ARP request to build IP/MAC entry in ARP table */
                    if ((STOE_GetIPAddr() & STOE_GetSubnetMask()) != (dip & STOE_GetSubnetMask()))
                        dip = STOE_GetGateway();

                    ETH_SendArpRequest(dip);
                    ETH_SendArpRequest(dip);
				    GS2W_SetTaskState(GS2W_STATE_UDP_DATA_PROCESS);
                }
                else  if (gudpdat_UdpAutoConnect == TRUE)
                {
                    dip = GCONFIG_GetUdpAutoConnectClientIp();
		            dport = GCONFIG_GetUdpAutoConnectClientPort(); 
        
                    /* get destination ip */
                    if (dip == 0)
                        break;
        
		            gudpdat_UdpAutoConnect = FALSE;
				    gudpdat_Conns[i].State = GUDPDAT_STATE_WAIT;
                    gudpdat_Conns[i].Timer = SWTIMER_Tick();
                    gudpdat_Conns[i].Ip = dip;
                    gudpdat_Conns[i].Port = dport;
				    gudpdat_Conns[i].UdpSocket = TCPIP_UdpNew(gudpdat_InterAppId,
                                                              i,	
				                                              dip,
														      GTCPDAT_GetRandomPortNum(),
														      dport);
                    /* Send ARP request to build IP/MAC entry in ARP table */
                    if ((STOE_GetIPAddr() & STOE_GetSubnetMask()) != (dip & STOE_GetSubnetMask()))
                        dip = STOE_GetGateway();

                    ETH_SendArpRequest(dip);
                    ETH_SendArpRequest(dip);
				    GS2W_SetTaskState(GS2W_STATE_UDP_DATA_PROCESS);
                }
                break;
            case GUDPDAT_STATE_CONNECTING:
                if (gudpdat_UdpClient == TRUE)
                {
#if GS2W_ENABLE_FLOW_CONTROL
                   dip = gudpdat_Conns[i].Ip;
                   if ((STOE_GetIPAddr() & STOE_GetSubnetMask()) != (dip & STOE_GetSubnetMask()))
                        dip = STOE_GetGateway();

                   if (uip_findarptable((U16_T *)&dip) == 0xFF)
                        break;
 
                    /* Send connected message */
                    txBuf[0] = GUDPDAT_CONNECT_INDICATOR;
                    txBuf[1] = 0;
                    txBuf[2] = 0;
                    TCPIP_UdpSend(gudpdat_Conns[i].UdpSocket, 0, 0, txBuf, 3);
#endif
                    gudpdat_Conns[i].State = GUDPDAT_STATE_CONNECTED;
                }
                break;

            case GUDPDAT_STATE_CONNECTED:
		        {
#if GS2W_ENABLE_FLOW_CONTROL
			        U8_T  modemStatus, modemCtrl;
			
                    /* Flow control header: type    length   payload */
                    /*                      ------  -------  ------- */
                    /*                      1 byte  2 bytes   any    */
                    /* length = length of payload                    */
			        modemStatus = HSUR2_GetModemStatus(TRUE);
                    modemCtrl = (HSUR2_GetModemControl() & 0x3);
			        if (gudpdat_Conns[i].FlowControlModemStatus != modemStatus || 
                        modemCtrl != gudpdat_Conns[i].FlowControlModemCtrl)			
			        {
				        gudpdat_Conns[i].FlowControlModemStatus = modemStatus;
                        gudpdat_Conns[i].FlowControlModemCtrl = modemCtrl;
				        txBuf[0] = GUDPDAT_FLOW_CONTROL_INDICATOR;
                        txBuf[1] = 0;
                        txBuf[2] = 3;
				        txBuf[3] = MODEM_STATUS_CHANGE;
				        txBuf[4] = modemStatus;
				        txBuf[5] = modemCtrl;
				        TCPIP_UdpSend(gudpdat_Conns[i].UdpSocket, 0, 0, txBuf, 6);
			        }

			        if (gudpdat_Conns[i].TxDatLen == 0)
			        {
				        gudpdat_Conns[i].TxBuf[gudpdat_Conns[i].TxDatLen++] = GUDPDAT_DATA_INDICATOR; // UDP data packet indicator
                        gudpdat_Conns[i].TxDatLen += 2;
			        }

			        if (gudpdat_Conns[i].FlowControlXonRx == TRUE)
			        {
#endif
				        gudpdat_GetDataFromUr(i);
                        gudpdat_SendData(i);
#if GS2W_ENABLE_FLOW_CONTROL			
		 	        }
#endif
                    /* Maintain the connection: send ARP request each GUDPDAT_MAIN_CONNECT_TIME */
                    gudpdat_MaintainConnection(i);
                }
                break;

            default:
                break;
        }
    }
} /* End of GUDPDAT_Task() */

/*
 * ----------------------------------------------------------------------------
 * static void gudpdat_GetDataFromUr(U8_T id)
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void gudpdat_GetDataFromUr(U8_T id)
{
    U8_T *pUpBuf;
    U16_T availCount;
    GUDPDAT_CONN *pConn= &gudpdat_Conns[id];

    availCount = HSUR2_GetRxDmaAvailCount();
    if (availCount == 0 || pConn->TxDatLen >= GUDPDAT_TX_BUF_SIZE)
        return;

    pUpBuf = &pConn->TxBuf[pConn->TxDatLen];

    if ((availCount+pConn->TxDatLen) >= GUDPDAT_MAX_DATA_LEN)
        availCount = GUDPDAT_MAX_DATA_LEN - pConn->TxDatLen;

    HSUR2_CopyUartToApp(pUpBuf, availCount);
    pConn->TxDatLen += availCount;
    pConn->UrRxBytes += availCount;

} /* End of gudpdat_GetDataFromUr(U8_T id) */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpdat_MaintainConnection()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void gudpdat_MaintainConnection(U8_T id)
{
    time = SWTIMER_Tick();

    if (time >= gudpdat_Conns[id].Timer)
        elapse = time - gudpdat_Conns[id].Timer;
    else
        elapse = (0xFFFFFFFF - gudpdat_Conns[id].Timer) + time;

    if (elapse >= GUDPDAT_MAIN_CONNECT_TIME)
    {
        if ((STOE_GetIPAddr() & STOE_GetSubnetMask()) != (gudpdat_Conns[id].Ip & STOE_GetSubnetMask()))
            ETH_SendArpRequest(STOE_GetGateway());
        else
            ETH_SendArpRequest(gudpdat_Conns[id].Ip);
        gudpdat_Conns[id].Timer = time;
    }
} /* End of gudpdat_MaintainConnection() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpdat_SendData()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void gudpdat_SendData(U8_T id)
{
    GUDPDAT_CONN *pConn= &gudpdat_Conns[id];

    if (pConn->TxDatLen >= GUDPDAT_MAX_DATA_LEN)
	{
#if GS2W_ENABLE_FLOW_CONTROL
        /* Fill in the length of payload */
        pConn->TxBuf[1] = (U8_T)((GUDPDAT_MAX_DATA_LEN-3) >> 8);
        pConn->TxBuf[2] = (U8_T)((GUDPDAT_MAX_DATA_LEN-3) & 0x00FF);
#endif
        TCPIP_UdpSend(pConn->UdpSocket, 0, 0, pConn->TxBuf, GUDPDAT_MAX_DATA_LEN);
					
        pConn->TxDatLen = 0;				
		pConn->TxWaitTime = 0;
    }
#if GS2W_ENABLE_FLOW_CONTROL
    else if (pConn->TxDatLen > 3) // both client and server have an indicator
#else
    else if (pConn->TxDatLen > 0)
#endif
    {
        time = SWTIMER_Tick();

        if (pConn->TxWaitTime == 0)
        {
	        pConn->TxWaitTime = time;
        }
        else
        {
	        if (time >= pConn->TxWaitTime)
		        elapse = time - pConn->TxWaitTime;
	        else
		        elapse = (0xFFFFFFFF - pConn->TxWaitTime) + time;
	
	        if ((elapse * SWTIMER_INTERVAL) >= gudpdat_EthernetTxTimer)
	        {
#if GS2W_ENABLE_FLOW_CONTROL
                /* Fill in the length of payload */
                pConn->TxBuf[1] = (U8_T)((pConn->TxDatLen-3) >> 8);
                pConn->TxBuf[2] = (U8_T)((pConn->TxDatLen-3) & 0x00FF);
#endif
		        TCPIP_UdpSend(pConn->UdpSocket, 0, 0, pConn->TxBuf, pConn->TxDatLen);
                pConn->TxDatLen = 0;
                pConn->TxWaitTime = 0;	
            }
        }
    }
} /* End of gudpdat_SendData() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPDAT_Init()
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPDAT_Init(U16_T localPort)
{
	U8_T	i;

	gudpdat_UdpClient = FALSE;
	gudpdat_UdpAutoConnect = FALSE;

	for (i = 0; i < GUDPDAT_MAX_CONNS; i++)
    {
		gudpdat_Conns[i].State = GUDPDAT_STATE_FREE;
        gudpdat_Conns[i].UrRxBytes = 0;
        gudpdat_Conns[i].UrTxBytes = 0;
        gudpdat_Conns[i].TxDatLen = 0;
        gudpdat_Conns[i].TxWaitTime = 0;
#if GS2W_ENABLE_FLOW_CONTROL
	    gudpdat_Conns[i].FlowControlModemStatus = HSUR2_GetModemStatus(TRUE);
        gudpdat_Conns[i].FlowControlModemCtrl = (HSUR2_GetModemControl() & 0x3);
        gudpdat_Conns[i].FlowControlXonRx = TRUE;
#endif
    }

	gudpdat_InterAppId = TCPIP_Bind(GUDPDAT_NewConn, GUDPDAT_Event, GUDPDAT_Receive);

	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_CLIENT) != GCONFIG_NETWORK_CLIENT)
	{
		TCPIP_UdpListen(localPort, gudpdat_InterAppId); 		
		printd("UDP data server init ok.\n\r");		
		gudpdat_UdpAutoConnect = GCONFIG_EnableUdpAutoConnectAfterReboot();
	}
	else
	{
		gudpdat_UdpClient = TRUE;	
		printd("UDP data client init ok.\n\r");			
	}

	gudpdat_EthernetTxTimer = GCONFIG_GetEthernetTxTimer();

} /* End of GUDPDAT_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPDAT_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GUDPDAT_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;

	if (GCONFIG_EnableUdpAutoConnectAfterReboot() == TRUE)
	{
		GCONFIG_SetUdpAutoConnectClientIp(*pip);
		GCONFIG_SetUdpAutoConnectClientPort(remotePort);
		GCONFIG_WriteConfigData();
	}

#if (GUDPDAT_MAX_CONNS == 1)
	if (gudpdat_Conns[0].State == GUDPDAT_STATE_CONNECTED)
	{
		gudpdat_Conns[0].State = GUDPDAT_STATE_FREE;
		TCPIP_UdpClose(gudpdat_Conns[0].UdpSocket);
	}
#endif

	for (i = 0; i < GUDPDAT_MAX_CONNS; i++)
	{
		if (gudpdat_Conns[i].State == GUDPDAT_STATE_FREE)
		{
#if GS2W_ENABLE_FLOW_CONTROL
			if (gudpdat_UdpAutoConnect == TRUE)
			{
				gudpdat_UdpAutoConnect = FALSE;
				GS2W_SetTaskState(GS2W_STATE_UDP_DATA_PROCESS);			
				gudpdat_Conns[i].State = GUDPDAT_STATE_CONNECTED;
			}
			else
			{
				gudpdat_Conns[i].State = GUDPDAT_STATE_WAIT;
			}
#else
			GS2W_SetTaskState(GS2W_STATE_UDP_DATA_PROCESS);			
			gudpdat_Conns[i].State = GUDPDAT_STATE_CONNECTED;

			if (gudpdat_UdpAutoConnect == TRUE)
				gudpdat_UdpAutoConnect = FALSE;
#endif
			gudpdat_Conns[i].UdpSocket = socket;
            gudpdat_Conns[i].Timer = SWTIMER_Tick();
            gudpdat_Conns[i].Ip = *pip;
            gudpdat_Conns[i].Port = remotePort;
			printd("New UDP connection: id=%bu, remotePort=%u, socket=%bu\n\r", i, remotePort, socket);

			return i;
		}
	}
	
	return GUDPDAT_NO_NEW_CONN;

} /* End of GUDPDAT_NewConn() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPDAT_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPDAT_Event(U8_T id, U8_T event)
{
    GUDPDAT_CONN *pConn= &gudpdat_Conns[id];

	if (event == TCPIP_CONNECT_ACTIVE)
	{
#if GS2W_ENABLE_FLOW_CONTROL
		if (pConn->State == GUDPDAT_STATE_WAIT)
		{
			pConn->State = GUDPDAT_STATE_CONNECTED;
			GS2W_SetTaskState(GS2W_STATE_UDP_DATA_PROCESS);
		}
#endif
	}
	else if (event == TCPIP_CONNECT_CANCEL)
	{
#if GS2W_ENABLE_FLOW_CONTROL
		if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_CLIENT) == GCONFIG_NETWORK_CLIENT)
		{
			txBuf[0] = GUDPDAT_CLOSE_INDICATOR; // close indication
            txBuf[1] = 0;
            txBuf[2] = 0;
			TCPIP_UdpSend(pConn->UdpSocket, 0, 0, txBuf, 3);
		}
#endif
		pConn->State = GUDPDAT_STATE_FREE;
		GS2W_SetTaskState(GS2W_STATE_IDLE);
		TCPIP_UdpClose(pConn->UdpSocket);
	}

} /* End of GUDPDAT_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPDAT_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPDAT_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
    GUDPDAT_CONN *pConn= &gudpdat_Conns[id];
#if GS2W_ENABLE_FLOW_CONTROL
MoreUdpData:
	if (length > 3 && *pData == GUDPDAT_DATA_INDICATOR)
	{
        U16_T dataLen = (*(pData+1) << 8) + *(pData+2) + 3;

		if (pConn->State != GUDPDAT_STATE_CONNECTED)
		{
			pConn->State = GUDPDAT_STATE_CONNECTED;
			GS2W_SetTaskState(GS2W_STATE_UDP_DATA_PROCESS);
			pConn->TxDatLen = 0;
			pConn->TxWaitTime = 0;			
		}
        
        if (dataLen > length) 
            dataLen = length;

            HSUR2_XmitOutEnq((pData + 3), (dataLen - 3));
			pConn->UrTxBytes += (dataLen - 3);

        length -= dataLen;
        if (length > 0)
        {
            /* point to the next packet header */
            pData += dataLen;
            goto MoreUdpData;
        }
	}
	else if (length <= 6 && *pData == GUDPDAT_FLOW_CONTROL_INDICATOR)
	{
		gudpdat_HandleFlowControlPacket(pData, length, id);
	}
	else if (length == 3)
    {
        if (*pData == GUDPDAT_CONNECT_INDICATOR)
        {
            pConn->State = GUDPDAT_STATE_CONNECTED;
            GS2W_SetTaskState(GS2W_STATE_UDP_DATA_PROCESS);
            pConn->TxDatLen = 0;
            pConn->TxWaitTime = 0;			
        }
        else if (*pData == GUDPDAT_CLOSE_INDICATOR)
        {
            pConn->State = GUDPDAT_STATE_FREE;
            GS2W_SetTaskState(GS2W_STATE_IDLE);	
            printd("TCPIP_UdpClose(id=%bu, socket=%bu)\n\r", id, pConn->UdpSocket);
            TCPIP_UdpClose(pConn->UdpSocket);
        }
    }
#else
	HSUR2_XmitOutEnq(pData, length);
	pConn->UrTxBytes += length;
#endif
} /* End of GUDPDAT_Receive() */

#if GS2W_ENABLE_FLOW_CONTROL			
/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpdat_HandleFlowControlPacket
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpdat_HandleFlowControlPacket(U8_T XDATA* pData, U16_T length, U8_T id)
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
		case IOCTL_SERIAL_GET_DTRRTS: // 0x01
			txBuf[0] = GUDPDAT_FLOW_CONTROL_INDICATOR;
            txBuf[1] = 0;
            txBuf[2] = 2;
			txBuf[3] = IOCTL_SERIAL_GET_DTRRTS_REPLY; // 0xA1
			txBuf[4] = HSUR2_GetModemControl();
			TCPIP_UdpSend(gudpdat_Conns[id].UdpSocket, 0, 0, &txBuf, 5);
			break;
		case IOCTL_SERIAL_GET_MODEM_CONTROL: // 0x02
			txBuf[0] = GUDPDAT_FLOW_CONTROL_INDICATOR;
            txBuf[1] = 0;
            txBuf[2] = 2;
			txBuf[3] = IOCTL_SERIAL_GET_MODEM_CONTROL_REPLY; // 0xA2
			txBuf[4] = HSUR2_GetModemControl();
			TCPIP_UdpSend(gudpdat_Conns[id].UdpSocket, 0, 0, &txBuf, 5);
			break;
		case IOCTL_SERIAL_GET_MODEM_STATUS: // 0x03
			txBuf[0] = GUDPDAT_FLOW_CONTROL_INDICATOR;
            txBuf[1] = 0;
            txBuf[2] = 2;
			txBuf[3] = IOCTL_SERIAL_GET_MODEM_STATUS_REPLY; // 0xA3
			txBuf[4] = HSUR2_GetModemStatus(TRUE);
			TCPIP_UdpSend(gudpdat_Conns[id].UdpSocket, 0, 0, &txBuf, 5);
			break;
		case IOCTL_SERIAL_CLR_DTR: // 0x11
			HSUR2_ClearDTR();
			break;
		case IOCTL_SERIAL_CLR_RTS: // 0x12
			HSUR2_ClearRTS();		
			break;
		case IOCTL_SERIAL_SET_DTR: // 0x13
			HSUR2_SetDTR();
			break;
		case IOCTL_SERIAL_SET_RTS: // 0x14
			HSUR2_SetRTS();
			break;
		case IOCTL_SERIAL_SET_BAUD_RATE: // 0x15			
            HSUR2_SetupPort(GCONFIG_GetBaudRate(dat1), dat2);
			break;
		case IOCTL_SERIAL_SET_BREAK_OFF: // 0x16
			dat1 = HSUR2_GetLineControl();
			HSUR2_SetLineControl(dat1 & 0xBF); // bit 6 - break control bit (1:on, 0:off)
			break;
		case IOCTL_SERIAL_SET_BREAK_ON: // 0x17
			dat1 = HSUR2_GetLineControl();
			HSUR2_SetLineControl(dat1 | 0x40); // bit 6 - break control bit (1:on, 0:off)
			break;
		case IOCTL_SERIAL_SET_FIFO_CONTROL: // 0x18
			HSUR2_SetFifoControl(dat1);
			break;
		case IOCTL_SERIAL_SET_LINE_CONTROL: // 0x19
			HSUR2_SetLineControl(dat1);
			break;
		case IOCTL_SERIAL_SET_MODEM_CONTROL: // 0x1A
			HSUR2_SetModemControl(dat1);
			break;
		case IOCTL_SERIAL_SET_XOFF: // 0x21
			gudpdat_Conns[id].FlowControlXonRx = FALSE;
			break;
		case IOCTL_SERIAL_SET_XON: // 0x22
			gudpdat_Conns[id].FlowControlXonRx = TRUE;
			break;
        case IOCTL_SERIAL_SET_HANDFLOW:
            gudpdat_SetFlowControl(dat1);
			break;
		case IOCTL_SERIAL_XOFF_COUNTER: // 0x23
			// Not support
			break;

		default:
			printd("Unknown flow control command!\n\r");
			break;
	}
} /* End of gudpdat_HandleFlowControlPacket() */

/*
 *--------------------------------------------------------------------------------
 * void gudpdat_SetFlowControl(U8_T)
 * Purpose: 
 * Params : fCtrl: the flow control to be set.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void gudpdat_SetFlowControl(U8_T fCtrl)
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
 * Function Name: GUDPDAT_GetTxBytes
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GUDPDAT_GetTxBytes(U8_T id)
{
    if (id < GUDPDAT_MAX_CONNS)
        return gudpdat_Conns[id].UrTxBytes;

    return 0;
} /* End of GUDPDAT_GetTxBytes() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPDAT_GetRxBytes
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GUDPDAT_GetRxBytes(U8_T id)
{
    if (id < GUDPDAT_MAX_CONNS)
        return gudpdat_Conns[id].UrRxBytes;

    return 0;
} /* End of GUDPDAT_GetRxBytes() */

/* End of gudpdat.c */
