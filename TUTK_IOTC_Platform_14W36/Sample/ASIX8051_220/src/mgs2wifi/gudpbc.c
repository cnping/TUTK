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
 * Module Name: gudpbc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpbc.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "mcpu.h"
#include "adapter.h"
#include "gudpbc.h"
#include "gs2w.h"
#include "tcpip.h"
#include "uip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart0.h"
#include "gconfig.h"
#include "ax22000.h"
#include "mac.h"
#include "printd.h"
#include <string.h>

/* NAMING CONSTANT DECLARATIONS */
#define GUDPBC_MAX_CONNS			1
#define GUDPBC_NO_NEW_CONN			0xFF

#define GUDPBC_STATE_FREE			0
#define	GUDPBC_STATE_WAIT			1
#define	GUDPBC_STATE_CONNECTED		2

/* GLOBAL VARIABLES DECLARATIONS */
extern GCONFIG_CFG_PKT gudpuc_ConfigRxPkt;

/* LOCAL VARIABLES DECLARATIONS */
static GUDPBC_CONN gudpbc_Conns[GUDPBC_MAX_CONNS];
static U8_T gudpbc_InterAppId;

/* LOCAL SUBPROGRAM DECLARATIONS */
void gudpbc_HandleSearchReq(U8_T XDATA* pData, U8_T id);
void gudpbc_HandleSetReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpbc_HandleUpgradeReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpbc_HandleResetReq(U8_T id);
void gudpbc_HandleRebootReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpbc_RebootDevice(void);

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPBC_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPBC_Task(void)
{

} /* End of GUDPBC_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPBC_Init()
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPBC_Init(U16_T localPort)
{
	U8_T	i;

	printd("UDP broadcast init ok.\n\r");

	for (i = 0; i < GUDPBC_MAX_CONNS; i++)
		gudpbc_Conns[i].State = GUDPBC_STATE_FREE;

	gudpbc_InterAppId = TCPIP_Bind(GUDPBC_NewConn, GUDPBC_Event, GUDPBC_Receive);
	
	/* unicast packet */
	TCPIP_UdpListen(localPort, gudpbc_InterAppId);
} /* End of GUDPBC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPBC_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GUDPBC_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;

	pip = pip;
	remotePort = remotePort;


	for (i = 0; i < GUDPBC_MAX_CONNS; i++)
	{
		gudpbc_Conns[i].State = GUDPBC_STATE_CONNECTED;
		gudpbc_Conns[i].UdpSocket = socket;

		return i;
	}
	
	return GUDPBC_NO_NEW_CONN;

} /* End of GUDPBC_NewConn() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPBC_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPBC_Event(U8_T id, U8_T event)
{
	gudpbc_Conns[id].State = event;

} /* End of GUDPBC_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPBC_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPBC_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
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
					gudpbc_HandleSearchReq(pData, id);
				}
				break;

			case GCONFIG_OPCODE_SET_REQ:
				{					
					if (bValidReq)
						gudpbc_HandleSetReq(pData, length, id);
				}
				break;

			case GCONFIG_OPCODE_UPGRADE_REQ:
				{
					if (bValidReq)
						gudpbc_HandleUpgradeReq(pData, length, id);
				}
				break;

			case GCONFIG_OPCODE_RESET_REQ:
				{
					if (bValidReq)
						gudpbc_HandleResetReq(id);
				}
				break;
			case GCONFIG_OPCODE_REBOOT_REQ:
				{
					if (bValidReq)
						gudpbc_HandleRebootReq(pData, length, id);
				}
				break;

			default:
				break;	
			}
		}
	}

    TCPIP_UdpClose(gudpbc_Conns[id].UdpSocket);
} /* End of GUDPBC_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleSearchReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleSearchReq(U8_T XDATA* pData, U8_T id)
{	
	U16_T		length, headerLen, tmpShort;
	U8_T XDATA*	pRcvBuf;
#if STOE_TRANSPARENT
	U8_T XDATA*	pMacAddr;
#endif
	uip_udpip_hdr XDATA*	pUdpHeader;

	GCONFIG_GetConfigPacket(&gudpuc_ConfigRxPkt);
 	gudpuc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_SEARCH_ACK;
	id = id;

	pRcvBuf = TCPIP_GetRcvBuffer();
	headerLen = (U16_T)pData - (U16_T)pRcvBuf;
	length = sizeof(gudpuc_ConfigRxPkt) + headerLen;

#if STOE_TRANSPARENT
	/* ethernet header */
	pMacAddr = MAC_GetMacAddr();
	pRcvBuf[0] = 0xFF;
	pRcvBuf[1] = 0xFF;
	pRcvBuf[2] = 0xFF;
	pRcvBuf[3] = 0xFF;
	pRcvBuf[4] = 0xFF;
	pRcvBuf[5] = 0xFF;
	pRcvBuf[MAC_ADDRESS_LEN] = pMacAddr[0];
	pRcvBuf[MAC_ADDRESS_LEN +1] = pMacAddr[1];
	pRcvBuf[MAC_ADDRESS_LEN +2] = pMacAddr[2];
	pRcvBuf[MAC_ADDRESS_LEN +3] = pMacAddr[3];
	pRcvBuf[MAC_ADDRESS_LEN +4] = pMacAddr[4];
	pRcvBuf[MAC_ADDRESS_LEN +5] = pMacAddr[5];

	/* ip header */
	pUdpHeader = (uip_udpip_hdr XDATA*)(pRcvBuf + ETH_HEADER_LEN);
	tmpShort = length - ETH_HEADER_LEN;
#else
	/* ip header */
	pUdpHeader = (uip_udpip_hdr XDATA*)pRcvBuf;
	tmpShort = length;
#endif

	pUdpHeader->len[0] = (U8_T)(tmpShort >> 8);
	pUdpHeader->len[1] = (U8_T)tmpShort;
	pUdpHeader->ipchksum = 0;
	pUdpHeader->destipaddr[0] = 0xFFFF;
 	pUdpHeader->destipaddr[1] = 0xFFFF;

	uip_gethostaddr(pUdpHeader->srcipaddr);

	/* udp header */
	pUdpHeader = (uip_udpip_hdr XDATA*)(pData - 28);
	tmpShort = pUdpHeader->srcport;
	pUdpHeader->srcport = pUdpHeader->destport;
	pUdpHeader->destport = tmpShort;
	pUdpHeader->udplen = sizeof(gudpuc_ConfigRxPkt) + 8;
	pUdpHeader->udpchksum = 0;

	TCPIP_SetXmitLength(length);

	uip_appdata = (U8_T*)&gudpuc_ConfigRxPkt;

	ETH_Send(headerLen);
	uip_slen = 0;

} /* End of gudpbc_HandleSearchReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleSetReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleSetReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
	U8_T reboot = (gudpuc_ConfigRxPkt.Option & GCONFIG_OPTION_ENABLE_REBOOT);

	GCONFIG_SetConfigPacket(&gudpuc_ConfigRxPkt);
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_SET_ACK;
	TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);

	if (reboot == GCONFIG_OPTION_ENABLE_REBOOT)
	{
		gudpbc_RebootDevice();
	}
} /* End of gudpbc_HandleSetReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleUpgradeReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleUpgradeReq(U8_T XDATA* pData, U16_T length, U8_T id)
{	
	GCONFIG_EnableFirmwareUpgrade();
	GCONFIG_WriteConfigData();		
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_UPGRADE_ACK;  	
	TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
	gudpbc_RebootDevice();
} /* End of gudpbc_HandleUpgradeReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleResetReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleResetReq(U8_T id)
{
	U8_T reboot = (gudpuc_ConfigRxPkt.Option & GCONFIG_OPTION_ENABLE_REBOOT);
	
	GCONFIG_ReadDefaultConfigData();
	GCONFIG_WriteConfigData();
	GCONFIG_GetConfigPacket(&gudpuc_ConfigRxPkt);
 	gudpuc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_RESET_ACK;
	TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, (U8_T*) &gudpuc_ConfigRxPkt, sizeof(gudpuc_ConfigRxPkt));

	if (reboot == GCONFIG_OPTION_ENABLE_REBOOT)
	{
		gudpbc_RebootDevice();
	}	
} /* End of gudpbc_HandleResetReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleRebootReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleRebootReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_REBOOT_ACK;  	
	TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
	gudpbc_RebootDevice();
} /* End of gudpbc_HandleRebootReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_RebootDevice
 * Purpose: Delay 10 ms before reboot
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_RebootDevice(void)
{
	U32_T timeStart = SWTIMER_Tick();
	U32_T timeEnd = timeStart;	

	while ((timeEnd == timeStart))
	{
		timeEnd = SWTIMER_Tick();
	}

	MCPU_SoftReboot();
} /* End of gudpbc_RebootDevice() */

/* End of gudpbc.c */
