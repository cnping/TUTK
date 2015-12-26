/*
 *********************************************************************************
 *     Copyright (c) 2010   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : dhcps.c
 * Purpose     : DHCP Server module implement DHCP server function over uIP stack.
 *               module.
 * Author      :
 * Date        :
 * Notes       :
 * $Log: dhcps.c,v $
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <string.h>
#include "main.h"
#include "dhcps.h"
#include "adapter.h"
#include "tcpip.h"
#include "uip.h"
#include "mstimer.h"
#include "dma.h"
#include "stoe.h"

#if (INCLUDE_DHCP_SERVER)
/* NAMING CONSTANT DECLARATIONS */
/* GLOBAL VARIABLES DECLARATIONS */
U8_T DHCPS_EnableFlag = 0xFF;
U32_T DHCPS_IpPoolStartAddr = 0;
U32_T DHCPS_IpPoolEndAddr = 0;
U32_T DHCPS_SubnetMask = 0;
U32_T DHCPS_Router = 0;
U16_T DHCPS_LeaseTime = 0;

/* STATIC VARIABLE DECLARATIONS */
static DHCPS_CONN     dhcps_Conn;
static DHCPS_CONNINFO dhcps_ConnsInfo[DHCPS_MAX_STA_CONNS];
static DHCPS_POOL	  dhcps_IpPoolTable[DHCPS_MAX_IP_POOL_NUM];
static DHCPS_HEADER	  dhcps_Buffer;
static U8_T	          dhcps_AppIfId;
static U8_T	          dhcps_CurrConnId;
static U16_T          dhcps_RcvPktLength;
static U16_T          dhcps_RcvPktLeftLength;
static DHCPS_HEADER   *dhcps_PktHeaderPtr;
static U8_T *dhcps_pOpt;
static U8_T dhcps_StartIpOffset=0;
static U16_T dhcps_MaxIpPool=0;
static U32_T dhcps_LastTimeTick = 0;
static U8_T dhcps_Init=0;
static U32_T dhcps_DnsServer = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void dhcps_SendPkt(U8_T pktType);
static void dhcps_ReceiveMessage(void);
static U8_T dhcps_FindConnInfoIndex(U8_T* pMacAddr);
static U8_T *dhcps_GetOption(U8_T *optMsg, U8_T msgCode, U8_T *vLen, U16_T);
static U8_T dhcps_FindIpPoolByIpAddr(U32_T ipAddr);
static U8_T dhcps_FindIpPoolByMacAddr(U8_T* pMacAddr);
static U8_T dhcps_CreateNewElementToIpPool(U32_T ipAddr, U8_T* pMacAddr);

/*
 *--------------------------------------------------------------------------------
 * Function Name: DHCPS_Init
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL DHCPS_Init(U32_T startIp, U32_T endIp, U32_T netmask, U32_T gateway, U16_T lease, U8_T status)
{
	U8_T index;

    if (dhcps_Init)
        return TRUE;

    dhcps_Init = 1;
    dhcps_Conn.State = DHCPS_STATE_IDLE;
    for (index = 0; index < DHCPS_MAX_STA_CONNS; index++)
        dhcps_ConnsInfo[index].State = DHCPS_STATE_IDLE;

    /* Initialize DHCP Server IP Pool Table */
    for (index = 0; index < DHCPS_MAX_IP_POOL_NUM; index++) 
    {
        dhcps_IpPoolTable[index].Occupy = 0;
    }

    dhcps_AppIfId = TCPIP_Bind(DHCPS_NewConnHandle, DHCPS_EventHandle, DHCPS_ReceiveHandle);
    TCPIP_UdpListen(DHCPS_SERVER_PORT, dhcps_AppIfId); /* Enable DHCP Server Listen Port */

    DHCPS_IpPoolStartAddr = startIp;
    DHCPS_IpPoolEndAddr = endIp;
    DHCPS_SubnetMask = netmask;
    DHCPS_Router = gateway;
    DHCPS_LeaseTime = lease;

    dhcps_MaxIpPool = DHCPS_IpPoolEndAddr - DHCPS_IpPoolStartAddr + 1;
    if (dhcps_MaxIpPool > DHCPS_MAX_IP_POOL_NUM)
        dhcps_MaxIpPool = DHCPS_MAX_IP_POOL_NUM;
        
    DHCPS_EnableFlag = status;        

	return TRUE;

} /* End of DHCPS_Init() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: DHCPS_GetServerSettings
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void DHCPS_GetServerSettings(U32_T *pStartIp, U32_T *pEndIp, U32_T *pNetmask, U32_T *pGateway, U16_T *pLease)
{
    if (!pStartIp || !pEndIp || !pNetmask || !pGateway || !pLease)
        return;

    *pStartIp = DHCPS_IpPoolStartAddr;
    *pEndIp = DHCPS_IpPoolEndAddr;
    *pNetmask = DHCPS_SubnetMask;
    *pGateway = DHCPS_Router;
    *pLease = DHCPS_LeaseTime;
    
} /* End of DHCPS_GetServerSettings() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: DHCPS_SetServerSettings
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void DHCPS_SetServerSettings(U32_T startIp, U32_T endIp, U32_T netmask, U32_T gateway, U16_T lease)
{
    if (!dhcps_Init)
        return;

    DHCPS_IpPoolStartAddr = startIp;
    DHCPS_IpPoolEndAddr = endIp; 
    DHCPS_SubnetMask = netmask;
    DHCPS_Router = gateway;
    DHCPS_LeaseTime = lease;

    dhcps_MaxIpPool = DHCPS_IpPoolEndAddr - DHCPS_IpPoolStartAddr + 1;
    if (dhcps_MaxIpPool > DHCPS_MAX_IP_POOL_NUM)
        dhcps_MaxIpPool = DHCPS_MAX_IP_POOL_NUM;
    
} /* End of DHCPS_SetServerSettings() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: DHCPS_SetServerStatus
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void DHCPS_SetServerStatus(U8_T status)
{
    DHCPS_EnableFlag = status;
} /* End of DHCPS_SetServerStatus() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: DHCPS_GetServerStatus
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T DHCPS_GetServerStatus(void)
{
    return DHCPS_EnableFlag ;
} /* End of DHCPS_GetServerStatus() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: DHCPS_SetDnsServer
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void DHCPS_SetDnsServer(U32_T dnsSrv)
{
    dhcps_DnsServer = dnsSrv;
} /* End of DHCPS_SetDnsServer() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DHCPS_NewConnHandle
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T DHCPS_NewConnHandle(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	if (!DHCPS_EnableFlag || DHCPS_CLIENT_PORT != remotePort) /* frame not from DHCP client. */
		return TCPIP_NO_NEW_CONN;

	if (dhcps_Conn.State == DHCPS_STATE_IDLE)
	{
		dhcps_Conn.State = DHCPS_STATE_ACTIVE;
		dhcps_Conn.Ip = *pip;
		dhcps_Conn.SocketId = socket;

		return 0;
	}

	return TCPIP_NO_NEW_CONN;
} /* End of DHCPS_NewConnHandle() */

/*
 * ----------------------------------------------------------------------------
 * void DHCPS_EventHandle(U8_T connId, U8_T event)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DHCPS_EventHandle(U8_T connId, U8_T event)
{
	if (!DHCPS_EnableFlag || connId != 0)
		return;

	if (event == TCPIP_CONNECT_CANCEL)
	{
		dhcps_Conn.State = DHCPS_STATE_IDLE;
	}
} /* End of DHCPS_EventHandle() */

/*
 * ----------------------------------------------------------------------------
 * void DHCPS_ReceiveHandle(U8_T XDATA* pData, U16_T length, U8_T connId)
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DHCPS_ReceiveHandle(U8_T XDATA *pData, U16_T length, U8_T connId)
{
    if (!DHCPS_EnableFlag)
        return;

	if ((connId != 0) || (dhcps_Conn.State == DHCPS_STATE_IDLE)) /* connection has not be created */
	{
		TCPIP_UdpClose(dhcps_Conn.SocketId);
		return;
	}

	dhcps_RcvPktLength = length;
	dhcps_RcvPktLeftLength = length;
	dhcps_PktHeaderPtr = (DHCPS_HEADER*)pData;

	dhcps_ReceiveMessage();
    TCPIP_UdpClose(dhcps_Conn.SocketId);
    dhcps_Conn.State = DHCPS_STATE_IDLE;

} /* End of DHCPS_ReceiveHandle() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: dhcps_ReceiveMessage
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void dhcps_ReceiveMessage(void)
{
	DHCPS_HEADER *pDhcpMsg;
	U8_T         *optMsg, ipPoolIndex, dhcpMsgType = 0;
	U8_T         index, rcvChAddr[6], foundInIpPool = 0;
	U32_T        requestIp = 0;

	pDhcpMsg = dhcps_PktHeaderPtr; /* DHCP payload pointer */

	/* calculate the options item length */
	dhcps_RcvPktLeftLength -= DHCPS_OFFSET_OPTIONS;
	dhcps_pOpt = pDhcpMsg->Options;

	if (pDhcpMsg->Op == DHCP_C_TO_S)
	{
		for (index = 0 ; index < DHCPS_MAC_ADDRESS_LEN; index++)
			rcvChAddr[index] = pDhcpMsg->ChAddr[index];

		/* check the received MAC address within ChAddr, except 00-00-00-00-00-00 & FF-FF-FF-FF-FF-FF */
		if ((rcvChAddr[0]==0 && rcvChAddr[1]==0 && rcvChAddr[2]==0 && rcvChAddr[3]==0 &&
			rcvChAddr[4]==0 && rcvChAddr[5]==0)||(rcvChAddr[0]==0xFF && rcvChAddr[1]==0xFF &&
			rcvChAddr[2]==0xFF && rcvChAddr[3]==0xFF && rcvChAddr[4]==0xFF && rcvChAddr[5]==0xFF))
		{
			return;
		}

		dhcps_CurrConnId = dhcps_FindConnInfoIndex(rcvChAddr);
		if (dhcps_CurrConnId == 0xFF)
        {
			return;
        }

		dhcps_ConnsInfo[dhcps_CurrConnId].Timer = DHCPS_CONNECTION_TIMEOUT;
		if (dhcps_ConnsInfo[dhcps_CurrConnId].State == DHCPS_STATE_ACTIVE)
		{
			dhcps_ConnsInfo[dhcps_CurrConnId].Xid = pDhcpMsg->Xid;
		}
		else /* DHCPS_OFFER_STATE */
		{
			if (dhcps_ConnsInfo[dhcps_CurrConnId].Xid != pDhcpMsg->Xid)
			{
				dhcps_ConnsInfo[dhcps_CurrConnId].Xid = pDhcpMsg->Xid;
				dhcps_ConnsInfo[dhcps_CurrConnId].State = DHCPS_STATE_ACTIVE;
			}
		}

		if ((*(dhcps_pOpt + 0) == 0x63) && (*(dhcps_pOpt + 1) == 0x82) &&
			(*(dhcps_pOpt + 2) == 0x53) && (*(dhcps_pOpt + 3) == 0x63))
		{
			dhcps_pOpt += 4;
			dhcps_RcvPktLeftLength -= 4;

			index = 0;
			optMsg = dhcps_GetOption(dhcps_pOpt, DHCPS_OPTION_MSG_TYPE, &index, dhcps_RcvPktLeftLength);
			if (optMsg && index == 1)
				dhcpMsgType = *optMsg;

			if ((dhcps_ConnsInfo[dhcps_CurrConnId].State == DHCPS_STATE_ACTIVE) && (dhcpMsgType != DHCPS_DISCOVER))
			{
				if (dhcpMsgType == DHCPS_REQUEST)
				{
                    dhcps_ConnsInfo[dhcps_CurrConnId].State = DHCPS_STATE_OFFER;
				}
				else
				{
					return;
				}
			}
            else if ((dhcps_ConnsInfo[dhcps_CurrConnId].State == DHCPS_STATE_OFFER) && (dhcpMsgType != DHCPS_REQUEST))
			{
				if (dhcpMsgType == DHCPS_DISCOVER)
				{
					dhcps_ConnsInfo[dhcps_CurrConnId].State = DHCPS_STATE_ACTIVE;
				}
				else if (dhcpMsgType == DHCPS_DECLINE)
				{
					ipPoolIndex = dhcps_FindIpPoolByMacAddr(rcvChAddr);
					if (ipPoolIndex < dhcps_MaxIpPool)
					{
						dhcps_IpPoolTable[ipPoolIndex].IpAddr = 0xFFFFFFFF;
						dhcps_IpPoolTable[ipPoolIndex].MacAddr[0] = 0xFF;
                        dhcps_IpPoolTable[ipPoolIndex].Lease = 0;
                        dhcps_IpPoolTable[ipPoolIndex].Occupy = 0;
					}
					return;
				}
				else
                {
					return;
                }
			}

			index = 0;
			optMsg = dhcps_GetOption(dhcps_pOpt, DHCPS_OPTION_REQ_IPADDR, &index, dhcps_RcvPktLeftLength);
			if ((pDhcpMsg->CiAddr) || (optMsg && index == DHCPS_IP_ADDRESS_LEN))
			{
				/* check the ip address requested by dhcp client */
                if (pDhcpMsg->CiAddr)
                    requestIp = pDhcpMsg->CiAddr;
                else
				    requestIp = *(U32_T*)optMsg;

				if (((requestIp & DHCPS_SubnetMask) == (DHCPS_IpPoolStartAddr & DHCPS_SubnetMask)) &&
					(requestIp >= DHCPS_IpPoolStartAddr) && (requestIp <= DHCPS_IpPoolEndAddr))
				{
                     /* find index of ip pool by ip address */
					ipPoolIndex = dhcps_FindIpPoolByIpAddr(requestIp);
					if (ipPoolIndex < dhcps_MaxIpPool) /* had found in ip pool */
					{
                        if (!memcmp(rcvChAddr, dhcps_IpPoolTable[ipPoolIndex].MacAddr, 6))
						{
							foundInIpPool = 1;
						}
						else
						{
                            /* request the ip address but different host */
							requestIp = 0;
							ipPoolIndex = dhcps_FindIpPoolByMacAddr(rcvChAddr);
							if (ipPoolIndex < dhcps_MaxIpPool)
								foundInIpPool = 1;
						}
					}
					else
					{
						ipPoolIndex = dhcps_FindIpPoolByMacAddr(rcvChAddr);
						if (ipPoolIndex < dhcps_MaxIpPool)
						{
							requestIp = 0;
							foundInIpPool = 1;
						}
					}
				}
				else
				{
                    if (dhcpMsgType == DHCPS_REQUEST)
                    {
                        dhcps_SendPkt(DHCPS_NAK);
                        return;
                    }
					requestIp = 0;
                    /* Check mac address existed in IP Pool table */
                    ipPoolIndex = dhcps_FindIpPoolByMacAddr(rcvChAddr);
                    if (ipPoolIndex < dhcps_MaxIpPool)
                        foundInIpPool = 1;
				}
			}
			else
			{
				ipPoolIndex = dhcps_FindIpPoolByMacAddr(rcvChAddr);
				if (ipPoolIndex < dhcps_MaxIpPool)
					foundInIpPool = 1;
			}

			if (!foundInIpPool)
			{
				ipPoolIndex = dhcps_CreateNewElementToIpPool(requestIp, rcvChAddr); /* add a new element to IP pool*/
				if (ipPoolIndex >= dhcps_MaxIpPool) /* new item created failed, no resource */
                {
					return;
                }
			}
            else
            {
                dhcps_IpPoolTable[ipPoolIndex].Lease = DHCPS_LeaseTime * 60 * SWTIMER_COUNT_SECOND;
            }

			dhcps_ConnsInfo[dhcps_CurrConnId].IpPoolIndex = ipPoolIndex;

			if (dhcps_ConnsInfo[dhcps_CurrConnId].State == DHCPS_STATE_ACTIVE)
			{
				dhcps_SendPkt(DHCPS_OFFER);
				dhcps_ConnsInfo[dhcps_CurrConnId].State = DHCPS_STATE_OFFER;
			}
			else /* DHCPS_OFFER_STATE */
			{
				dhcps_SendPkt(DHCPS_ACK);
			}
		}
	}
} /* End of dhcps_ReceiveMessage() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: dhcps_FindConnInfoIndex
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T dhcps_FindConnInfoIndex(U8_T* pMacAddr)
{
	U8_T index;

	for (index = 0; index < DHCPS_MAX_STA_CONNS; index++)
	{
		if (dhcps_ConnsInfo[index].State == DHCPS_STATE_IDLE)
			continue;

        if (!memcmp((char *)dhcps_ConnsInfo[index].ChMacAddr, (char *)pMacAddr, DHCPS_MAC_ADDRESS_LEN))
            return index;
	}

	/* not found in table */
	for (index = 0; index < DHCPS_MAX_STA_CONNS; index++)
	{
		if (dhcps_ConnsInfo[index].State == DHCPS_STATE_IDLE)
		{
			dhcps_ConnsInfo[index].State = DHCPS_STATE_ACTIVE;
            memcpy((char *)dhcps_ConnsInfo[index].ChMacAddr, (char *)pMacAddr, DHCPS_MAC_ADDRESS_LEN);
			return index;
		}
	}

	return 0xFF;
} /* End of dhcps_FindConnInfoIndex() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: U8_T* dhcps_GetOption(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T *dhcps_GetOption(U8_T *optMsg, U8_T msgCode, U8_T *vLen, U16_T maxLen)
{
	U8_T	len, *optMsgEnd;
    
	optMsgEnd = optMsg + maxLen;
	while(*optMsg != DHCPS_OPTION_END && optMsg <= optMsgEnd) /* first to parse the msg type */
	{
		if (*optMsg == msgCode)
		{
			*vLen = *(optMsg + 1);
			optMsg += 2;
			return optMsg;
		}
		else
		{
			len = *(optMsg + 1);
			optMsg += (len + 2);
		}
	}

	return 0;
} /* End of dhcps_GetOption() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: dhcps_SendPkt
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void dhcps_SendPkt(U8_T pktType)
{
	DHCPS_HEADER*	pDhcpMsg;
	U8_T XDATA*		pMacAddr = MAC_GetMacAddr();
	U8_T XDATA*		pRcvBuf = TCPIP_GetRcvBuffer();
	U16_T			length = DHCPS_OFFSET_OPTIONS;
	U16_T			headerLen, tmpShort, totalLen;

	uip_udpip_hdr XDATA*	pUdpHeader;

	pDhcpMsg = &dhcps_Buffer;
	DMA_Grant((U8_T*)&dhcps_Buffer, (U8_T*)dhcps_PktHeaderPtr, dhcps_RcvPktLength);

	pDhcpMsg->Op = DHCP_S_TO_C;
	pDhcpMsg->CiAddr = 0;
    if (DHCPS_NAK != pktType)
        pDhcpMsg->YiAddr = dhcps_IpPoolTable[dhcps_ConnsInfo[dhcps_CurrConnId].IpPoolIndex].IpAddr;

	/* Add magic cookie bytes */
	dhcps_pOpt = &pDhcpMsg->Options;
	*(dhcps_pOpt + 0) = 0x63;
	*(dhcps_pOpt + 1) = 0x82;
	*(dhcps_pOpt + 2) = 0x53;
	*(dhcps_pOpt + 3) = 0x63;
	dhcps_pOpt += 4;
	length += 4;

	/* Add DHCPS_OPTION_MSG_TYPE options */
	*(dhcps_pOpt + 0) = DHCPS_OPTION_MSG_TYPE;
	*(dhcps_pOpt + 1) = 1; /* length */
	*(dhcps_pOpt + 2) = pktType;
	dhcps_pOpt += 3;
	length += 3;

	/* Add DHCPS_OPTION_SERVER_ID option */
	*(dhcps_pOpt + 0) = DHCPS_OPTION_SERVER_ID;
	*(dhcps_pOpt + 1) = 4; /* length */
	*((U32_T*)(dhcps_pOpt + 2)) = STOE_GetIPAddr();
	dhcps_pOpt += 6;
	length += 6;

    if (DHCPS_NAK != pktType)
    {
        /* Add DHCPS_OPTION_LEASE_TIME options */
	    *(dhcps_pOpt + 0) = DHCPS_OPTION_LEASE_TIME;
	    *(dhcps_pOpt + 1) = 4; /* length */
	    *((U32_T*)(dhcps_pOpt + 2)) = (DHCPS_LeaseTime * 60); /* 1 day */
	    dhcps_pOpt += 6;
	    length += 6;

        /* Add DHCPS_OPTION_SUBNET_MASK option */
        *(dhcps_pOpt + 0) = DHCPS_OPTION_SUBNET_MASK;
        *(dhcps_pOpt + 1) = 4; /* length */
        *((U32_T*)(dhcps_pOpt + 2)) = DHCPS_SubnetMask;
        dhcps_pOpt += 6;
        length += 6;

        /* Add DHCPS_OPTION_ROUTER option */
        *(dhcps_pOpt + 0) = DHCPS_OPTION_ROUTER;
        *(dhcps_pOpt + 1) = 4; /* length */
        *((U32_T*)(dhcps_pOpt + 2)) = DHCPS_Router;
        dhcps_pOpt += 6;
        length += 6;

        /* Add DHCPS_OPTION_DNS_SERVER option */
        *(dhcps_pOpt + 0) = DHCPS_OPTION_DNS_SERVER;
        *(dhcps_pOpt + 1) = 4; /* length */
        *((U32_T*)(dhcps_pOpt + 2)) = dhcps_DnsServer;
        dhcps_pOpt += 6;
        length += 6;
    }

	/* Add DHCPS_OPTION_END option */
	*(dhcps_pOpt + 0) = DHCPS_OPTION_END;
	length++;

	headerLen = (U16_T)dhcps_PktHeaderPtr - (U16_T)pRcvBuf;
	totalLen = length + headerLen;

#if STOE_TRANSPARENT
	pRcvBuf[0] = 0xFF;
	pRcvBuf[1] = 0xFF;
	pRcvBuf[2] = 0xFF;
	pRcvBuf[3] = 0xFF;
	pRcvBuf[4] = 0xFF;
	pRcvBuf[5] = 0xFF;
	pRcvBuf[DHCPS_MAC_ADDRESS_LEN] = pMacAddr[0];
	pRcvBuf[DHCPS_MAC_ADDRESS_LEN +1] = pMacAddr[1];
	pRcvBuf[DHCPS_MAC_ADDRESS_LEN +2] = pMacAddr[2];
	pRcvBuf[DHCPS_MAC_ADDRESS_LEN +3] = pMacAddr[3];
	pRcvBuf[DHCPS_MAC_ADDRESS_LEN +4] = pMacAddr[4];
	pRcvBuf[DHCPS_MAC_ADDRESS_LEN +5] = pMacAddr[5];

	/* ip header */
	pUdpHeader = (uip_udpip_hdr XDATA*)(pRcvBuf + ETH_HEADER_LEN);
	tmpShort = totalLen - ETH_HEADER_LEN;
#else
	/* ip header */
	pUdpHeader = (uip_udpip_hdr XDATA*)pRcvBuf;
	tmpShort = totalLen;
#endif

	pUdpHeader->len[0] = (U8_T)(tmpShort >> 8);
	pUdpHeader->len[1] = (U8_T)tmpShort;
	pUdpHeader->ipchksum = 0;
	pUdpHeader->destipaddr[0] = 0xFFFF;
	pUdpHeader->destipaddr[1] = 0xFFFF;

	uip_gethostaddr(pUdpHeader->srcipaddr);

	/* udp header */
	tmpShort = pUdpHeader->srcport;
	pUdpHeader->srcport = pUdpHeader->destport;
	pUdpHeader->destport = tmpShort;
	pUdpHeader->udplen = length + 8;
	pUdpHeader->udpchksum = 0;

	TCPIP_SetXmitLength(totalLen);

	uip_appdata = (U8_T*)&dhcps_Buffer;

	ETH_Send(headerLen);
	uip_slen = 0;

} /* End of dhcps_SendPkt() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: dhcps_FindIpPoolByIpAddr
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T dhcps_FindIpPoolByIpAddr(U32_T ipAddr)
{
	U8_T	index;

	for (index = 0; index < dhcps_MaxIpPool; index++)
	{
		if (dhcps_IpPoolTable[index].Occupy != 0 && (dhcps_IpPoolTable[index].IpAddr == ipAddr))
			return index;
	}

	return 0xFF;
} /* End of dhcps_FindIpPoolByIpAddr() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: dhcps_FindIpPoolByMacAddr
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T dhcps_FindIpPoolByMacAddr(U8_T* pMacAddr)
{
	U8_T index;

	for (index = 0; index < dhcps_MaxIpPool; index++)
	{
		if (dhcps_IpPoolTable[index].Occupy != 0 &&
            !memcmp((char *)dhcps_IpPoolTable[index].MacAddr, (char *)pMacAddr, DHCPS_MAC_ADDRESS_LEN))
            return index;
	}

	return 0xFF;
} /* End of dhcps_FindIpPoolByMacAddr() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: dhcps_CreateNewElementToIpPool
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T dhcps_CreateNewElementToIpPool(U32_T ipAddr, U8_T* pMacAddr)
{
    U8_T index, wrapCnt;

    wrapCnt = 0;
	for (index = 0; index < dhcps_MaxIpPool; index++)
	{
		if (dhcps_IpPoolTable[index].Occupy == 0)
		{
			if (ipAddr != 0)
			{
				dhcps_IpPoolTable[index].IpAddr = ipAddr;
			}
			else
			{
                do 
                {
				    dhcps_IpPoolTable[index].IpAddr = DHCPS_IpPoolStartAddr + dhcps_StartIpOffset++;
                    /* skip xxx.xxx.xxx.0 and xxx.xxx.xxx.255 */
                    if (!(dhcps_IpPoolTable[index].IpAddr & 0x000000FF))
                    {
                        dhcps_IpPoolTable[index].IpAddr = DHCPS_IpPoolStartAddr + dhcps_StartIpOffset++;
                    }
                    else if ((dhcps_IpPoolTable[index].IpAddr & 0x000000FF) == 0x000000FF)
                    {
                       dhcps_StartIpOffset += 2;
                       dhcps_IpPoolTable[index].IpAddr = DHCPS_IpPoolStartAddr + dhcps_StartIpOffset;
                    }
                    /* Wrapped */
                    if (dhcps_StartIpOffset >= dhcps_MaxIpPool)
                    {
                        wrapCnt++;
                        dhcps_StartIpOffset = 0;
                    }

                } while (wrapCnt < 2 && dhcps_FindIpPoolByIpAddr(dhcps_IpPoolTable[index].IpAddr) != 0xFF);
 			}

            dhcps_IpPoolTable[index].Lease = DHCPS_LeaseTime * 60 * SWTIMER_COUNT_SECOND;
			dhcps_IpPoolTable[index].Occupy = 1;
            memcpy(dhcps_IpPoolTable[index].MacAddr, pMacAddr, DHCPS_MAC_ADDRESS_LEN);

			return index;
		}
	}

	return 0xFF;
} /* End of dhcps_CreateNewElementToIpPool() */

/*
 *--------------------------------------------------------------------------------
 * Function Name: DHCPS_StateCheck
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void DHCPS_ConnsCheck(void)
{
	U8_T	index;
	U32_T	elapse, timeoutTick;

	if (!dhcps_Init || !DHCPS_EnableFlag)
    {
        dhcps_LastTimeTick = SWTIMER_Tick();
		return;
    }

    timeoutTick = SWTIMER_Tick();
    if (timeoutTick >= dhcps_LastTimeTick)
        elapse = timeoutTick - dhcps_LastTimeTick;
    else
        elapse = (0xFFFFFFFF - dhcps_LastTimeTick) + timeoutTick;

    dhcps_LastTimeTick = timeoutTick;

	for (index = 0; index < DHCPS_MAX_STA_CONNS; index++)
	{
		if (dhcps_ConnsInfo[index].State == DHCPS_STATE_IDLE)
			continue;

		if (elapse > dhcps_ConnsInfo[index].Timer)
        {
            dhcps_ConnsInfo[index].Timer = 0;
			dhcps_ConnsInfo[index].State = DHCPS_STATE_IDLE;
        }
        else
        {
            dhcps_ConnsInfo[index].Timer -= elapse;
        }
	}

    /* mantain the allocated IP table */
	for (index = 0; index < dhcps_MaxIpPool; index++)
	{
		if (dhcps_IpPoolTable[index].Occupy != 0)
		{
            if (elapse > dhcps_IpPoolTable[index].Lease)
            {
                dhcps_IpPoolTable[index].Lease = 0;
			    dhcps_IpPoolTable[index].Occupy = 0;
            }
            else
                dhcps_IpPoolTable[index].Lease -= elapse;
 		}
	}

} /* End of DHCPS_StateCheck() */

#endif /* INCLUDE_DHCP_SERVER */

/* End of dhcpc.c */
