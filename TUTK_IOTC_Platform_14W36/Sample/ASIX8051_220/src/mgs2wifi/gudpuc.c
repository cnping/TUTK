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
 * Module Name: gudpuc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpuc.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <string.h>
#include "ax22000.h"
#include "mcpu.h"
#include "adapter.h"
#include "gudpuc.h"
#include "gs2w.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart0.h"
#include "gconfig.h"
#include "mac.h"
#include "printd.h"
#include "gudpdat.h"
#include "gtcpdat.h"
#include "hsuart.h"
#include "hsuart2.h"


/* NAMING CONSTANT DECLARATIONS */
/* signature(8) commmand(1) option(1) devname(16) macAddr(6) */
#define GUDPUC_AUTHENTIC_HEADER_LEN 32
#define GUDPUC_MAX_CONNS			1
#define GUDPUC_NO_NEW_CONN			0xFF

#define GUDPUC_STATE_FREE			0
#define	GUDPUC_STATE_WAIT			1
#define	GUDPUC_STATE_CONNECTED		2

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static GUDPUC_CONN gudpuc_Conns[GUDPUC_MAX_CONNS];
static U8_T gudpuc_InterAppId;
GCONFIG_CFG_PKT gudpuc_ConfigRxPkt;
GCONFIG_MONITOR_PKT *gudpuc_MonitorPkt;

/* LOCAL SUBPROGRAM DECLARATIONS */
void gudpuc_HandleSearchReq(U8_T id);
void gudpuc_HandleSetReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpuc_HandleUpgradeReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpuc_HandleResetReq(U8_T id);
void gudpuc_HandleRebootReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpuc_RebootDevice(void);
void gudpuc_HandleMonitorReq(U8_T id);

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPUC_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPUC_Task(void)
{

} /* End of GUDPUC_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPUC_Init()
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPUC_Init(U16_T localPort)
{
	U8_T	i;

	printd("UDP unicast init ok.\n\r");

	for (i = 0; i < GUDPUC_MAX_CONNS; i++)
		gudpuc_Conns[i].State = GUDPUC_STATE_FREE;

	gudpuc_InterAppId = TCPIP_Bind(GUDPUC_NewConn, GUDPUC_Event, GUDPUC_Receive);
	
	/* unicast packet */
	TCPIP_UdpListen(localPort, gudpuc_InterAppId);
} /* End of GUDPUC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPUC_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GUDPUC_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;

	for (i = 0; i < GUDPUC_MAX_CONNS; i++)
	{
		gudpuc_Conns[i].State = GUDPUC_STATE_CONNECTED;
		gudpuc_Conns[i].UdpSocket = socket;
		return i;
	}
	
	return GUDPUC_NO_NEW_CONN;

} /* End of GUDPUC_NewConn() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPUC_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPUC_Event(U8_T id, U8_T event)
{
	gudpuc_Conns[id].State = event;
} /* End of GUDPUC_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPUC_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPUC_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
	BOOL bValidReq = FALSE;
	GCONFIG_MAC_ADDR macAddr;

	if (length >= 9 && length <= sizeof(gudpuc_ConfigRxPkt)) // 9: search request packet
	{
		memcpy(&gudpuc_ConfigRxPkt, pData, length);

		if (memcmp(&GCONFIG_Gid, pData, sizeof(GCONFIG_GID)) == 0) // Valid data
		{
			macAddr = GCONFIG_GetMacAddress();	

			if (length > GUDPUC_AUTHENTIC_HEADER_LEN)
			{
				if (memcmp(&gudpuc_ConfigRxPkt.MacAddr, &macAddr, sizeof(macAddr)) == 0)
					bValidReq = TRUE;
			}

			switch (gudpuc_ConfigRxPkt.Opcode)
			{
			case GCONFIG_OPCODE_SEARCH_REQ:
            case GCONFIG_OPCODE_SEARCH_REQ1:
				{
					gudpuc_HandleSearchReq(id);
				}
				break;

			case GCONFIG_OPCODE_SET_REQ:
				{
					if (bValidReq && length == sizeof(gudpuc_ConfigRxPkt))
						gudpuc_HandleSetReq(pData, length, id);
				}
				break;

			case GCONFIG_OPCODE_UPGRADE_REQ:
				{
					if (bValidReq)
						gudpuc_HandleUpgradeReq(pData, length, id);
				}
				break;

			case GCONFIG_OPCODE_RESET_REQ:
				{
					if (bValidReq)
						gudpuc_HandleResetReq(id);
				}
				break;
			case GCONFIG_OPCODE_REBOOT_REQ:
				{
					if (bValidReq)
						gudpuc_HandleRebootReq(pData, length, id);
				}
				break;
            case GCONFIG_OPCODE_MONITOR_REQ:
				{
                    gudpuc_HandleMonitorReq(id);
				}
				break;

			default:
				break;	
			}
		}
	}

    TCPIP_UdpClose(gudpuc_Conns[id].UdpSocket);

} /* End of GUDPUC_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpuc_HandleSearchReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpuc_HandleSearchReq(U8_T id) 
{	
	GCONFIG_GetConfigPacket(&gudpuc_ConfigRxPkt);
 	gudpuc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_SEARCH_ACK;
	TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, (U8_T*) &gudpuc_ConfigRxPkt, sizeof(gudpuc_ConfigRxPkt));
} /* End of gudpuc_HandleSearchReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpuc_HandleSetReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpuc_HandleSetReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
	U8_T reboot = (gudpuc_ConfigRxPkt.Option & GCONFIG_OPTION_ENABLE_REBOOT);

	GCONFIG_SetConfigPacket(&gudpuc_ConfigRxPkt);
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_SET_ACK;
	TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, pData, length);

	if (reboot == GCONFIG_OPTION_ENABLE_REBOOT)
	{
		gudpuc_RebootDevice();
	}	
} /* End of gudpuc_HandleSetReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpuc_HandleUpgradeReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpuc_HandleUpgradeReq(U8_T XDATA* pData, U16_T length, U8_T id)
{	
    U8_T *pCfg = pData + GUDPUC_AUTHENTIC_HEADER_LEN;
    U32_T tftpSrvIp;

    /* Get tftp server ip */
    memcpy((char *)&tftpSrvIp, (char *)pCfg, 4);
    GCONFIG_SetTftpServerIp(tftpSrvIp);

    /* point to filename */
    pCfg += 4;
    GCONFIG_SetFilename(pCfg, strlen(pCfg));

	GCONFIG_EnableFirmwareUpgrade();
	GCONFIG_WriteConfigData();
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_UPGRADE_ACK;  	
	TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, pData, length);
	gudpuc_RebootDevice();
} /* End of gudpuc_HandleUpgradeReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpuc_HandleResetReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpuc_HandleResetReq(U8_T id)
{
	U8_T reboot = (gudpuc_ConfigRxPkt.Option & GCONFIG_OPTION_ENABLE_REBOOT);
	
	GCONFIG_ReadDefaultConfigData();
	GCONFIG_WriteConfigData();
	GCONFIG_GetConfigPacket(&gudpuc_ConfigRxPkt);
 	gudpuc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_RESET_ACK;
	TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, (U8_T*) &gudpuc_ConfigRxPkt, sizeof(gudpuc_ConfigRxPkt));

	if (reboot == GCONFIG_OPTION_ENABLE_REBOOT)
	{
		gudpuc_RebootDevice();
	}	
} /* End of gudpuc_HandleResetReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpuc_HandleRebootReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpuc_HandleRebootReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_REBOOT_ACK;  	
	TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, pData, length);
	gudpuc_RebootDevice(); 
}
/* End of gudpuc_HandleRebootReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpuc_RebootDevice
 * Purpose: Delay 10 ms before reboot
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpuc_RebootDevice(void)
{
	U32_T timeStart = SWTIMER_Tick();
	U32_T timeEnd = timeStart;	

	while (timeEnd == timeStart)
	{
		timeEnd = SWTIMER_Tick();
	}

	MCPU_SoftReboot();
} /* End of gudpuc_RebootDevice() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpuc_HandleMonitorReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpuc_HandleMonitorReq(U8_T id)
{
    gudpuc_MonitorPkt = (GCONFIG_MONITOR_PKT *)&gudpuc_ConfigRxPkt;
	
	gudpuc_MonitorPkt->Gid = GCONFIG_Gid;
    gudpuc_MonitorPkt->Opcode = GCONFIG_OPCODE_MONITOR_ACK;
    gudpuc_MonitorPkt->ModemStatus = HSUR2_GetModemStatus(1);
    strcpy((char *)gudpuc_MonitorPkt->VerStr , GCONFIG_VERSION_STRING);
    if (GS2W_CONN_UDP == GS2W_GetConnType())
    {
        gudpuc_MonitorPkt->TxBytes = GUDPDAT_GetTxBytes(0);
        gudpuc_MonitorPkt->RxBytes = GUDPDAT_GetRxBytes(0);
    }
    else
    {
        gudpuc_MonitorPkt->TxBytes = GTCPDAT_GetTxBytes(0);
        gudpuc_MonitorPkt->RxBytes = GTCPDAT_GetRxBytes(0);
    }

	TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, (U8_T *)gudpuc_MonitorPkt, sizeof(GCONFIG_MONITOR_PKT));

} /* End of gudpuc_HandleMonitorReq() */

/* End of gudpuc.c */
