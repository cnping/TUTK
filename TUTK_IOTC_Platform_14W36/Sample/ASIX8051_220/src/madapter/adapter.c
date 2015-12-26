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
 * Module Name: adapter.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: adapter.c,v $
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "adapter.h"
#include "stoe.h"
#include "dma.h"
#include "tcpip.h"
#include "pppoe.h"
#include "mstimer.h"

/* NAMING CONSTANT DECLARATIONS */
#define ETH_HEADER_LEN		14
#define ETH_NON_TRANSPARENT	0
#define ETH_TRANSPARENT		1
#define ETH_ARP_SEND_INTERVAL_TIME	180

/* GLOBAL VARIABLES DECLARATIONS */
U8_T ETH_IsPppoe = 0;
U8_T ETH_DoDhcp = 0;
U8_T ETH_PppoeHeaderLen = 0;
U8_T ETH_DhcpArpReply = TRUE;


/* LOCAL VARIABLES DECLARATIONS */
static U8_T eth_EthHeadLen = 0;
static U16_T eth_PayLoadOffset = 0;
#if (!STOE_TRANSPARENT)
static BOOL eth_GatewayArpStaticFlag = FALSE;
#endif
static U32_T eth_ArpSendIntervalTime = 3;
static U32_T eth_ArpSendTime = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void eth_RcvHandle(U8_T XDATA*, U16_T, U8_T);
#if (!STOE_TRANSPARENT)
static BOOL eth_CheckGatewayIp(U8_T XDATA* pBuf);
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: ETH_Init
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void ETH_Init(void)
{
	U8_T*		point;

	/* ethernet initiation */
	STOE_Init(ETH_PHY_SENSE_TYPE);
	STOE_RcvCallback = eth_RcvHandle;

#if (STOE_TRANSPARENT)
	/* Initialise TCP/UDP up-layer interfaces */
	TCPIP_Init(ETH_TRANSPARENT);
	eth_EthHeadLen = ETH_HEADER_LEN;
#else
	TCPIP_Init(ETH_NON_TRANSPARENT);
	eth_EthHeadLen = 0;
#endif
	/* set IP address to tcpip module */
	TCPIP_SetIPAddr(STOE_GetIPAddr());

	/* set mac address to tcpip module */
	point = MAC_GetMacAddr();

	TCPIP_SetMacAddr(point);

	TCPIP_AssignLowlayerXmitFunc(ETH_Send);

	/* PPPoE initiation */
	PPPOE_Init();

} /* End of ETH_Init */

/*
 * ----------------------------------------------------------------------------
 * Function Name: ETH_Start
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void ETH_Start(void)
{
	STOE_Start();

} /* End of ETH_Start */

/*
 * ----------------------------------------------------------------------------
 * Function Name: eth_RcvHandle
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void eth_RcvHandle(U8_T XDATA* pbuf, U16_T length, U8_T protocol)
{
	/* copy packet into uIP buffer */
	DMA_GrantXdata(TCPIP_GetRcvBuffer(), pbuf, length);

	TCPIP_SetRcvLength(length);

#if (STOE_TRANSPARENT == STOE_NON_TRANSPARENT_MODE)
	if (protocol == 0xff)
	{
		if ((*pbuf > 0x44) && (*pbuf < 0x50))
		{
			protocol = *(pbuf + 9);
			PBDP->STOE_RxInform.Protocol = protocol;
		}
	}
#endif

	switch (protocol)
	{
	default:
		break;
	case 1:
	case 2:
	case 6:
	case 17:
#if STOE_TRANSPARENT
		if (ETH_IsPppoe)
		{
			if (!PPPOE_Receive(TCPIP_GetRcvBuffer(), length))
				return;
		}
		else if (ETH_DoDhcp == 0)
			uip_arp_ipin();
#endif
		TCPIP_Receive();
		if(TCPIP_GetXmitLength() > 0)
		{
#if STOE_TRANSPARENT
			if (!ETH_IsPppoe)
				uip_arp_out();
#endif
			ETH_Send(0);
		}
		break;
	case 0xff:
#if STOE_TRANSPARENT
		if (ETH_IsPppoe)
		{
			if (!PPPOE_Receive(TCPIP_GetRcvBuffer(), length))
				return;
		}
		else
		{
			if(pbuf[12] == 0x08 && pbuf[13] == 0x06)
			{
				uip_arp_arpin();
				if(TCPIP_GetXmitLength() > 0)
					ETH_Send(TCPIP_GetXmitLength());

				if (TCPIP_GetGwMacAddrFlag())
					eth_ArpSendIntervalTime = ETH_ARP_SEND_INTERVAL_TIME;
			}
		}
#else
		// receive a arp reply packet, check the ip.
		// If this ip is the gateway's ip, update the hardware arp table,
		// and set it static
		if(pbuf[12] == 0x08 && pbuf[13] == 0x06 && pbuf[21] == 0x02)
		{
			if (eth_CheckGatewayIp(pbuf))
			{
				eth_GatewayArpStaticFlag = TRUE;
				eth_ArpSendIntervalTime = ETH_ARP_SEND_INTERVAL_TIME;
			}
		}
#endif
		break;
	}  /* End of switch */

} /* End of eth_RcvHandle */

/*
* -----------------------------------------------------------------------------
 * Function Name: ETH_Send
 * Purpose: Copy Data to Mac ram and send to Ethernet.
 * Params: 
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void ETH_Send(U16_T	payloadOffset)
{
	U16_T	length = TCPIP_GetXmitLength();

	if (!length)
		return;

	eth_PayLoadOffset = payloadOffset;

#if STOE_TRANSPARENT
	if (ETH_IsPppoe)
	{
	 	length += (eth_EthHeadLen + ETH_PppoeHeaderLen);
		if (!PPPOE_GetState())
			return;

		PPPOE_Send (TCPIP_GetXmitBuffer(), length);
	}
#endif

	ETH_SendPkt(TCPIP_GetXmitBuffer(), length);
		
} /* End of ETH_Send() */

/*
* -----------------------------------------------------------------------------
 * Function Name: ETH_SendPkt
 * Purpose: Copy Data to Mac ram and send to Ethernet.
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void ETH_SendPkt(U8_T XDATA* psour, U16_T length)
{
	U8_T XDATA* pBuf;
	U8_T XDATA* point;
	U16_T		len;
	U16_T		addr;
	U8_T		isPppoeIp = 0;
	U16_T		totalLen = length;

#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)
	if (!(STOE_ConnectState & STOE_ETHERNET_LINK))
		return;
#else /* MAC_ARBIT_MODE == MAC_ARBIT_WIFI */
	if (!(STOE_ConnectState & STOE_WIRELESS_LINK))
		return;
#endif

	pBuf = STOE_AssignSendBuf(length + MAC_TX_RX_HEADER);
	if (!pBuf)
		return;

	point = pBuf + MAC_TX_RX_HEADER;

#if STOE_TRANSPARENT
	if (ETH_IsPppoe)
	{
		if ((*(psour + 12) == 0x88) && (*(psour + 13) == 0x64) && (*(psour + 20) == 0) && (*(psour + 21) == 0x21))
		{
			isPppoeIp = 1;
			PBDP->STOE_TxInform.Protocol = *(psour + 31);
		}
		else
		{
			eth_PayLoadOffset = length;
			PBDP->STOE_TxInform.Protocol = 0xff;
		}
	}
	else
#endif
	{
#if STOE_TRANSPARENT
		if ((*(psour + 12) == 8) && (*(psour + 13) == 0))
			PBDP->STOE_TxInform.Protocol = *(psour + 23);
		else
			PBDP->STOE_TxInform.Protocol = 0xff;
#else
		if (*psour == 0x45)
			PBDP->STOE_TxInform.Protocol = *(psour + 9);
		else
			PBDP->STOE_TxInform.Protocol = 0xff;
#endif
	}

	if (pBuf)
	{
		if (eth_PayLoadOffset)
			len = eth_PayLoadOffset;
		else
		{
#if STOE_TRANSPARENT
			if (ETH_IsPppoe)
				len = 48 + eth_EthHeadLen;
			else
#endif
				len = 40 + eth_EthHeadLen;
		}

		if (length > len)
		{
			/* Do software DMA */
			DMA_GrantXdata(point, TCPIP_GetXmitBuffer(), len);

			point += len;
			length -= len;

			addr = (U32_T)(TCPIP_GetPayloadBuffer()) >> 8;

			if(addr > 0x8000)	/*if(flag_dataincode)*/
				DMA_ProgramToData((U32_T)TCPIP_GetPayloadBuffer()- 0x810000, (U32_T)point, length);
#if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K)
			else if (addr >= 0x100)
				DMA_Grant(point, TCPIP_GetPayloadBuffer(), length);
#endif
			else
				DMA_GrantXdata(point, TCPIP_GetPayloadBuffer(), length);
		}
		else
		{
			/* Do software DMA */
			DMA_GrantXdata(point, TCPIP_GetXmitBuffer(), len);
		} /* End of if (length > len) */

#if STOE_TRANSPARENT
		if (isPppoeIp)
			pBuf[5] = 0x80;
		else
#endif
			pBuf[5] = 0;
		STOE_Send(pBuf, totalLen, PBDP->STOE_TxInform.Protocol);
	}

	eth_PayLoadOffset = 0; /* clear offset flag */

} /* End of ETH_SendPkt() */

/*
* -----------------------------------------------------------------------------
 * Function Name: eth_CheckGatewayIp
 * Purpose: Copy Data to Mac ram and send to Ethernet.
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
BOOL eth_CheckGatewayIp(U8_T XDATA* pBuf)
{
	U32_T	gatewayIp, receiveIp;

	gatewayIp = STOE_GetGateway();
	if (!gatewayIp)
	{
		return FALSE;
	}

	receiveIp = *(U32_T *)&pBuf[28];
	if (gatewayIp != receiveIp)
	{
		return FALSE;
	}

	return TRUE;
}

/*
* -----------------------------------------------------------------------------
 * Function Name: ETH_SendArpToGateway
 * Purpose: send arp request to gateway
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void ETH_SendArpToGateway(BOOL StopSendAfterRcvReply)
{
	U32_T	gateway, ip, TempTime;
	U8_T	*buf = uip_buf;
	U8_T*	point;

	if (StopSendAfterRcvReply)
	{
#if (STOE_TRANSPARENT)
		if (TCPIP_GetGwMacAddrFlag())
			return;
#else
		if (eth_GatewayArpStaticFlag)
			return;
#endif
	}

	// first check stoe gateway setting
	gateway = STOE_GetGateway();
	if (!gateway)
		return;

	// check stoe source ip setting
	ip = STOE_GetIPAddr();
	if (!ip)
		return;

	// check is it time to send arp request
	TempTime = SWTIMER_Tick();
	if ((TempTime - eth_ArpSendTime) < (eth_ArpSendIntervalTime * SWTIMER_COUNT_SECOND))
		return;

	// update time
	eth_ArpSendTime = TempTime;

#if (STOE_TRANSPARENT)
	if (TCPIP_GetGwMacAddrFlag())
#else
	if (eth_GatewayArpStaticFlag)
#endif
	{
		eth_ArpSendIntervalTime = ETH_ARP_SEND_INTERVAL_TIME;
	}
	else
	{
		eth_ArpSendIntervalTime = 10;
	}

	// send arp request to gateway,
	// in order to get gateway MAC address.
	point = MAC_GetMacAddr();
	uip_len = 42;

	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	buf[4] = 0xFF;
	buf[5] = 0xFF;
	buf[6] = *(point);
	buf[7] = *(point + 1);
	buf[8] = *(point + 2);
	buf[9] = *(point + 3);
	buf[10] = *(point + 4);
	buf[11] = *(point + 5);
	buf[12] = 0x08; 
	buf[13] = 0x06;

	buf[14] = 0x00;
	buf[15] = 0x01;
	buf[16] = 0x08;
	buf[17] = 0x00;
	buf[18] = 0x06;
	buf[19] = 0x04;
	buf[20] = 0x00;
	buf[21] = 0x01;

	buf[22] = *(point);
	buf[23] = *(point + 1);
	buf[24] = *(point + 2);
	buf[25] = *(point + 3);
	buf[26] = *(point + 4);
	buf[27] = *(point + 5);
	*(U32_T *)&buf[28] = STOE_GetIPAddr();
	
	buf[32] = 0x00;
	buf[33] = 0x00;
	buf[34] = 0x00;
	buf[35] = 0x00;
	buf[36] = 0x00;
	buf[37] = 0x00;
	*(U32_T *)&buf[38] = gateway;

	ETH_Send(uip_len);

	return;

} /* End of ETH_SendArpToGateway() */

/*
* -----------------------------------------------------------------------------
 * Function Name: ETH_SendArpRequest
 * Purpose: send arp request to dstIp
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void ETH_SendArpRequest(U32_T dstIp)
{
	U8_T *buf = uip_buf;
	U8_T *point;

    // in order to get gateway MAC address.
	point = MAC_GetMacAddr();

	uip_len = 42;

	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	buf[4] = 0xFF;
	buf[5] = 0xFF;
	buf[6] = *(point);
	buf[7] = *(point + 1);
	buf[8] = *(point + 2);
	buf[9] = *(point + 3);
	buf[10] = *(point + 4);
	buf[11] = *(point + 5);
	buf[12] = 0x08; 
	buf[13] = 0x06;

	buf[14] = 0x00;
	buf[15] = 0x01;
	buf[16] = 0x08;
	buf[17] = 0x00;
	buf[18] = 0x06;
	buf[19] = 0x04;
	buf[20] = 0x00;
	buf[21] = 0x01;

	buf[22] = *(point);
	buf[23] = *(point + 1);
	buf[24] = *(point + 2);
	buf[25] = *(point + 3);
	buf[26] = *(point + 4);
	buf[27] = *(point + 5);
	*(U32_T *)&buf[28] = STOE_GetIPAddr();
	
	buf[32] = 0x00;
	buf[33] = 0x00;
	buf[34] = 0x00;
	buf[35] = 0x00;
	buf[36] = 0x00;
	buf[37] = 0x00;
	*(U32_T *)&buf[38] = dstIp;

	ETH_Send(0);

	return;
}

/*
* -----------------------------------------------------------------------------
 * Function Name: ETH_RestartSendArpToGateway
 * Purpose: Restart sending arp request to gateway
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void ETH_RestartSendArpToGateway(void)
{
#if (STOE_TRANSPARENT)
	TCPIP_SetGwMacAddrFlag(FALSE);
#else
	eth_GatewayArpStaticFlag = FALSE;
#endif
	eth_ArpSendIntervalTime = 3;
}

/*
* -----------------------------------------------------------------------------
 * Function Name: ETH_SendDhcpArpRequest
 * Purpose: send arp request to gateway
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void ETH_SendDhcpArpRequest(void)
{
	U8_T	*buf = uip_buf;
	U8_T*	point;

	eth_ArpSendIntervalTime = 5;

	point = MAC_GetMacAddr();

	uip_len = 42;

	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	buf[4] = 0xFF;
	buf[5] = 0xFF;
	buf[6] = *(point);
	buf[7] = *(point + 1);
	buf[8] = *(point + 2);
	buf[9] = *(point + 3);
	buf[10] = *(point + 4);
	buf[11] = *(point + 5);
	buf[12] = 0x08; 
	buf[13] = 0x06;

	buf[14] = 0x00;
	buf[15] = 0x01;
	buf[16] = 0x08;
	buf[17] = 0x00;
	buf[18] = 0x06;
	buf[19] = 0x04;
	buf[20] = 0x00;
	buf[21] = 0x01;

	buf[22] = *(point);
	buf[23] = *(point + 1);
	buf[24] = *(point + 2);
	buf[25] = *(point + 3);
	buf[26] = *(point + 4);
	buf[27] = *(point + 5);
	*(U32_T *)&buf[28] = STOE_GetIPAddr(); // Source IP address
	
	buf[32] = 0x00;
	buf[33] = 0x00;
	buf[34] = 0x00;
	buf[35] = 0x00;
	buf[36] = 0x00;
	buf[37] = 0x00;
	*(U32_T *)&buf[38] = STOE_GetIPAddr(); // Destination IP address

	ETH_DhcpArpReply = FALSE;

	ETH_Send(0);

	return;
}

/* End of adapter.c */