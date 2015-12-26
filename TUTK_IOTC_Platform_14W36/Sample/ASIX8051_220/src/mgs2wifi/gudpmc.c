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
 * Module Name: gudpmc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpmc.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "mcpu.h"
#include "adapter.h"
#include "gudpmc.h"
#include "gs2w.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart0.h"
#include "gconfig.h"
#include "printd.h"
#include <string.h>

/* NAMING CONSTANT DECLARATIONS */
#define GUDPMC_MAX_CONNS			1
#define GUDPMC_NO_NEW_CONN			0xFF

#define GUDPMC_STATE_FREE			0
#define	GUDPMC_STATE_WAIT			1
#define	GUDPMC_STATE_CONNECTED		2

/* GLOBAL VARIABLES DECLARATIONS */
extern GCONFIG_CFG_PKT gudpuc_ConfigRxPkt;

/* LOCAL VARIABLES DECLARATIONS */
static GUDPMC_CONN gudpmc_Conns[GUDPMC_MAX_CONNS];
static U8_T gudpmc_InterAppId;
U8_T GUDPMC_Multicast[MAC_ADDRESS_LEN] = {1, 0, 0x5e, 1, 2, 3};

/* LOCAL SUBPROGRAM DECLARATIONS */
void gudpmc_HandleSearchReq(U8_T id);
void gudpmc_HandleSetReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpmc_HandleUpgradeReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpmc_HandleResetReq(U8_T id);
void gudpmc_HandleRebootReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpmc_RebootDevice(void);

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPMC_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPMC_Task(void)
{

} /* End of GUDPMC_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPMC_Init()
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPMC_Init(U16_T localPort)
{
	U8_T	i;

	printd("UDP multicast init ok.\n\r");

	for (i = 0; i < GUDPMC_MAX_CONNS; i++)
		gudpmc_Conns[i].State = GUDPMC_STATE_FREE;

	gudpmc_InterAppId = TCPIP_Bind(GUDPMC_NewConn, GUDPMC_Event, GUDPMC_Receive);
	
	/* set multicase filter */
	MAC_SetRxFilter(MAC_RCV_MULTICAST | MAC_RCV_BROADCAST);
	MAC_MultiFilter(GUDPMC_Multicast, MAC_ADDRESS_LEN);

	/* multicast packets */
	TCPIP_UdpListen(localPort, gudpmc_InterAppId);

} /* End of GUDPMC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPMC_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GUDPMC_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;

	for (i = 0; i < GUDPMC_MAX_CONNS; i++)
	{
		gudpmc_Conns[i].State = GUDPMC_STATE_CONNECTED;
		gudpmc_Conns[i].UdpSocket = socket;

		return i;
	}
	
	return GUDPMC_NO_NEW_CONN;

} /* End of GUDPMC_NewConn() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPMC_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPMC_Event(U8_T id, U8_T event)
{
	gudpmc_Conns[id].State = event;

} /* End of GUDPMC_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPMC_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPMC_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
	BOOL bValidReq = FALSE;
	GCONFIG_MAC_ADDR macAddr;

	if (length == 9 || length == sizeof(gudpuc_ConfigRxPkt)) // 9: search request packet
	{
		if (length == 9)
			memcpy(&gudpuc_ConfigRxPkt, pData, 9);
		else
			memcpy(&gudpuc_ConfigRxPkt, pData, sizeof(gudpuc_ConfigRxPkt));

		if (memcmp(&GCONFIG_Gid, pData, sizeof(GCONFIG_GID)) == 0) // Valid data
		{
			macAddr = GCONFIG_GetMacAddress();	

			if (length == sizeof(gudpuc_ConfigRxPkt))
			{
				if (memcmp(&gudpuc_ConfigRxPkt.MacAddr, &macAddr, sizeof(macAddr)) == 0)
					bValidReq = TRUE;
			}

			switch (gudpuc_ConfigRxPkt.Opcode)
			{
			case GCONFIG_OPCODE_SEARCH_REQ:
			case GCONFIG_OPCODE_SEARCH_REQ1:
				{
					gudpmc_HandleSearchReq(id);
				}
				break;

			case GCONFIG_OPCODE_SET_REQ:
				{					
					if (bValidReq)
						gudpmc_HandleSetReq(pData, length, id);
				}
				break;

			case GCONFIG_OPCODE_UPGRADE_REQ:
				{
					if (bValidReq)
						gudpmc_HandleUpgradeReq(pData, length, id);
				}
				break;

			case GCONFIG_OPCODE_RESET_REQ:
				{
					if (bValidReq)
						gudpmc_HandleResetReq(id);
				}
				break;
			case GCONFIG_OPCODE_REBOOT_REQ:
				{
					if (bValidReq)
						gudpmc_HandleRebootReq(pData, length, id);
				}
				break;

			default:
				break;	
			}
		}
	}

    TCPIP_UdpClose(gudpmc_Conns[id].UdpSocket);
} /* End of GUDPMC_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpmc_HandleSearchReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpmc_HandleSearchReq(U8_T id)
{	
	GCONFIG_GetConfigPacket(&gudpuc_ConfigRxPkt);
 	gudpuc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_SEARCH_ACK;
	TCPIP_UdpSend(gudpmc_Conns[id].UdpSocket, 0, 0, (U8_T*) &gudpuc_ConfigRxPkt, sizeof(gudpuc_ConfigRxPkt));
} /* End of gudpmc_HandleSearchReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpmc_HandleSetReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpmc_HandleSetReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
	U8_T reboot = (gudpuc_ConfigRxPkt.Option & GCONFIG_OPTION_ENABLE_REBOOT);

	GCONFIG_SetConfigPacket(&gudpuc_ConfigRxPkt);
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_SET_ACK;
	TCPIP_UdpSend(gudpmc_Conns[id].UdpSocket, 0, 0, pData, length);

	if (reboot == GCONFIG_OPTION_ENABLE_REBOOT)
	{
		gudpmc_RebootDevice();
	}	
} /* End of gudpmc_HandleSetReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpmc_HandleUpgradeReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpmc_HandleUpgradeReq(U8_T XDATA* pData, U16_T length, U8_T id)
{	
	GCONFIG_EnableFirmwareUpgrade();
	GCONFIG_WriteConfigData();		
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_UPGRADE_ACK;  	
	TCPIP_UdpSend(gudpmc_Conns[id].UdpSocket, 0, 0, pData, length);
	gudpmc_RebootDevice();
} /* End of gudpmc_HandleUpgradeReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpmc_HandleResetReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpmc_HandleResetReq(U8_T id)
{
	U8_T reboot = (gudpuc_ConfigRxPkt.Option & GCONFIG_OPTION_ENABLE_REBOOT);

	GCONFIG_ReadDefaultConfigData();
	GCONFIG_WriteConfigData();
	GCONFIG_GetConfigPacket(&gudpuc_ConfigRxPkt);
 	gudpuc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_RESET_ACK;
	TCPIP_UdpSend(gudpmc_Conns[id].UdpSocket, 0, 0, (U8_T*) &gudpuc_ConfigRxPkt, sizeof(gudpuc_ConfigRxPkt));

	if (reboot == GCONFIG_OPTION_ENABLE_REBOOT)
	{
		gudpmc_RebootDevice();
	}
} /* End of gudpmc_HandleResetReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpmc_HandleRebootReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpmc_HandleRebootReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_REBOOT_ACK;  	
	TCPIP_UdpSend(gudpmc_Conns[id].UdpSocket, 0, 0, pData, length);
	// May store current status/setting here before restart
	gudpmc_RebootDevice();
} /* End of gudpmc_HandleRebootReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpmc_RebootDevice
 * Purpose: Delay 10 ms before reboot
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpmc_RebootDevice(void)
{
	U32_T timeStart = SWTIMER_Tick();
	U32_T timeEnd = timeStart;	

	while ((timeEnd == timeStart))
	{
		timeEnd = SWTIMER_Tick();
	}

	MCPU_SoftReboot();
} /* End of gudpmc_RebootDevice() */

/* End of gudpmc.c */
