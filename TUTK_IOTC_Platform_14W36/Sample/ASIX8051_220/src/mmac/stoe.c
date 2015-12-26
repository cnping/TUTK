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
 * Module Name: stoe.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: stoe.c,v $
 * no message
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "stoe.h"
#include "dma.h"
#include "delay.h"
#include "interrupt.h"

#include "adapter.h"
#include "printd.h"
#include <intrins.h>
#include "uart0.h"
#include "at24c02.h"
#include "mcpu.h"

/* NAMING CONSTANT DECLARATIONS */
#define XSTACKSIZE 64

/* GLOBAL VARIABLES DECLARATIONS */
LOCAL_STATION XDATA* PNetStation = {0};
BUF_DESC_PAGE XDATA* PBDP = 0;
void (*STOE_RcvCallback)(U8_T XDATA*, U16_T, U8_T); /* call eth layer rcv process
													function. */
U8_T STOE_ConnectState = 0;
U8_T XDATA STOE_DROP_PKT = 0;
#if STOE_WIFI_QOS_SUPPORT
  U8_T STOE_WiFiQosFlag = 0;
  U8_T STOE_WiFiQos = 0;
#endif

/* LOCAL VARIABLES DECLARATIONS */
static U8_T XDATA stoe_BufferSize[STOE_SOCKET_BUFFER_SIZE] _at_ STOE_BDP_START_ADDR;
static U8_T XDATA stoe_InterruptStatus = 0;
static U8_T stoe_DeferredCnt=0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void stoe_BdpInit(void);
#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)
  static void stoe_InterruptEnable(void);
#endif
static void stoe_StartOperate(void);
static void stoe_RcvHandle(void);
static U8_T stoe_CheckLink(U8_T* pbuf);

/* Reserved xstack for the functions with reenterence in bootloader and runtime code will use those functions */
U8_T XDATA xStackBuf[XSTACKSIZE] _at_ (STOE_BDP_START_ADDR-XSTACKSIZE);

/*
 * ----------------------------------------------------------------------------
 * Function Name: stoe_ReadReg
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_ReadReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
{
	U8_T	isr;

	isr = EA;
	EA = 0;
	_nop_();
	TCIR = regaddr;
	while (length--)
		pbuf[length] = TDR;
	EA = isr;

} /* End of stoe_ReadReg */

/*
 * ----------------------------------------------------------------------------
 * Function Name: stoe_WriteReg
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_WriteReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
{
	U8_T	isr;

	isr = EA;
	EA = 0;
	while (length--)
		TDR = pbuf[length];
	TCIR = regaddr;
	EA = isr;

} /* End of stoe_WriteReg */

/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_Init
 * Purpose: to initial all registers and variables of STOE. 
 * Params: network_type -0: auto- negotiation
 *						-1: fixed 100 full speed.
 *						-2: fixed 100 half speed.
 *						-3: fixed 10 full speed.
 *						-4: fixed 10 half speed.
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_Init(U8_T network_type)
{
	U8_T XDATA	temp[2];
	U8_T XDATA	value[6] = {0,0,0,0,0,0};
#if (STOE_TRANSPARENT == STOE_NON_TRANSPARENT_MODE)
	U8_T		index, count;
#endif

#ifdef RuntimeCodeAt32KH
	MCPU_ExecuteRuntimeFlag |= 0x01;
#else
	MCPU_ExecuteRuntimeFlag = 0;
#endif

	/* set xmit & rcv memory. */
	stoe_BdpInit();

	PNetStation = &PBDP->NetStation;

#if (STOE_TRANSPARENT == STOE_NON_TRANSPARENT_MODE)
	/* clear arp table */
	for (index = 0; index < 128 ; index++)
	{
		count = index * 2;
		stoe_WriteReg(STOE_ARP_ADDR_REG, &count, 1);
		stoe_WriteReg(STOE_ARP_DATA_REG, value, 6);
		count = ARP_CACHE_CMD_GO;
		stoe_WriteReg(STOE_ARP_CMD_REG, &count, 1);
		while (count & ARP_CACHE_CMD_GO)
			stoe_ReadReg(STOE_ARP_CMD_REG, &count, 1);
	}
#endif

#if (MAC_ARBIT_MODE == MAC_ARBIT_WIFI)
	/* Validate Subsystem ID of WCPU */
	MAC_ValidateSubsystemId();

	/* source IP */
	value[0] = MAC_WifiIp[0];
	value[1] = MAC_WifiIp[1];
	value[2] = MAC_WifiIp[2];
	value[3] = MAC_WifiIp[3];
	PNetStation->SecDefaultIP = *(U32_T XDATA*)value;
	PNetStation->SecStationIP = PNetStation->SecDefaultIP;
	
	stoe_WriteReg(STOE_IP_ADDR_REG, (U8_T XDATA*)&PNetStation->SecDefaultIP, 4);
#else /* (MAC_ARBIT_MODE & MAC_ARBIT_ETH) */
	stoe_ReadReg(STOE_IP_ADDR_REG, (U8_T XDATA*)&PNetStation->DefaultIP, 4);
	PNetStation->StationIP = PNetStation->DefaultIP;
#endif

	/* subnet mask */
#if (MAC_ARBIT_MODE == MAC_ARBIT_WIFI)
	value[0] = MAC_WifiSubMask[0];
	value[1] = MAC_WifiSubMask[1];
	value[2] = MAC_WifiSubMask[2];
	value[3] = MAC_WifiSubMask[3];
	PNetStation->SecDefaultMask = *(U32_T XDATA*)value;
	PNetStation->SecSubnetMask = PNetStation->SecDefaultMask;
	
	stoe_WriteReg(STOE_SUBNET_MASK_REG, (U8_T XDATA*)&PNetStation->SecDefaultMask, 4);
#else /* (MAC_ARBIT_MODE & MAC_ARBIT_ETH) */
	stoe_ReadReg(STOE_SUBNET_MASK_REG, (U8_T XDATA*)&PNetStation->DefaultMask, 4);
	PNetStation->SubnetMask = PNetStation->DefaultMask;
#endif

	/* gateway */
#if (MAC_ARBIT_MODE == MAC_ARBIT_WIFI)
	PNetStation->SecDefaultGateway =
		((PNetStation->SecStationIP & PNetStation->SecSubnetMask) |1);
	if (PNetStation->SecStationIP == PNetStation->SecDefaultGateway)
	{
		PNetStation->SecDefaultGateway |= (~PNetStation->SecSubnetMask);
		PNetStation->SecDefaultGateway &= 0xfffffffe; 
	}
	PNetStation->SecGateway = PNetStation->SecDefaultGateway;
	
	stoe_WriteReg(STOE_GATEWAY_IP_REG, (U8_T XDATA*)&PNetStation->SecGateway, 4);
#else /* (MAC_ARBIT_MODE & MAC_ARBIT_ETH) */
	PNetStation->DefaultGateway =((PNetStation->StationIP &PNetStation->SubnetMask)|1);
	if (PNetStation->StationIP == PNetStation->DefaultGateway)
	{
		PNetStation->DefaultGateway |= (~PNetStation->SubnetMask);
		PNetStation->DefaultGateway &= 0xfffffffe; 
	}
	PNetStation->Gateway = PNetStation->DefaultGateway;
		
	stoe_WriteReg(STOE_GATEWAY_IP_REG, (U8_T XDATA*)&PNetStation->Gateway, 4);
#endif

	/* set L2 control register */
#if (STOE_TRANSPARENT == STOE_TRANSPARENT_MODE)
	temp[0] = (RX_TRANSPARENT | TX_TRANSPARENT | RX_VLAN_ENABLE);
#else
	temp[0] = 0;
#endif

	stoe_WriteReg(STOE_L2_CTL_REG, temp, 1);

	/* set ARP table timeout register */
	temp[0] = STOE_ARP_TIMEOUT;
	stoe_WriteReg(STOE_ARP_TIMEOUT_REG, temp, 1);

	/* set L4 control register */
#if (STOE_CHECKSUM_OFFLOAD)
	temp[0] = (DROP_CHKSUMERR_PKT | ENABLE_XMIT_CHKSUM | ENABLE_XMIT_CROSS);
#else
	temp[0] = ENABLE_XMIT_CROSS;
#endif
	stoe_WriteReg(STOE_L4_CTL_REG, temp, 1);

	/* set BDP point */
	temp[0] = (STOE_BDP_START_ADDR >> 16);
	temp[1] = (STOE_BDP_START_ADDR >> 8);
	stoe_WriteReg(STOE_L4_BDP_PNT_REG, temp, 2);

	/* receive callback function initialize to null point. */
	STOE_RcvCallback = 0;

	DMA_Init();
	DELAY_Init();
	MAC_Init(network_type);

#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)
	STOE_SetGateway(PNetStation->DefaultGateway);
#else /* (MAC_ARBIT_MODE == MAC_ARBIT_WIFI) */
	STOE_SetGateway(PNetStation->SecDefaultGateway);
#endif

} /* End of STOE_Init */

/*
* -----------------------------------------------------------------------------
 * Function Name: stoe_BdpInit
 * Purpose: initial Buffer Descriptor Page.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_BdpInit(void)
{
	U16_T	page = (STOE_BDP_START_ADDR >> 8);

	PBDP = STOE_BDP_START_ADDR;

	// set BDP number
	PBDP->BDP_ID = 0;

	// set rcv buffer.
	PBDP->RSPP = page + 1;
	PBDP->REPP = page + PAGES_OF_RCV;
	PBDP->RHPR = PBDP->RSPP;
	PBDP->RTPR = PBDP->RSPP;
	PBDP->RFP = PAGES_OF_RCV;

	// set xmit buffer.
	PBDP->TSPP = page + 1 + PAGES_OF_RCV;
	PBDP->TEPP = page + PAGES_OF_RCV + PAGES_OF_XMIT;
	PBDP->THPR = PBDP->TSPP;
	PBDP->TTPR = PBDP->TSPP;
	PBDP->TFP = PAGES_OF_XMIT;

	// set rcv and xmit start/end buffer address.
	PBDP->RcvStartAddr = STOE_BDP_START_ADDR + 256;
	PBDP->XmtStartAddr = PBDP->RcvStartAddr + STOE_RCV_BUF_SIZE;
	PBDP->RcvEndAddr = PBDP->XmtStartAddr - 1;
	PBDP->XmtEndAddr = PBDP->XmtStartAddr + STOE_XMIT_BUF_SIZE - 1;

} /* End of stoe_BdpInit */

/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_SetIPAddr
 * Purpose:
 * Params:	ip- IP Address
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_SetIPAddr(U32_T ip)
{
#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)	
	if (ip != PNetStation->StationIP)
	{
		PNetStation->StationIP = ip;
		stoe_WriteReg(STOE_IP_ADDR_REG, (U8_T XDATA*)&PNetStation->StationIP, 4);
	}
#else /* (MAC_ARBIT_MODE == MAC_ARBIT_WIFI) */
	if (ip != PNetStation->SecStationIP)
	{
		PNetStation->SecStationIP = ip;
		stoe_WriteReg(STOE_IP_ADDR_REG, (U8_T XDATA*)&PNetStation->SecStationIP, 4);
	}
#endif
} /* End of STOE_SetIPAddr*/

/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_SetSubnetMask
 * Purpose:
 * Params:	subnet- Subnet Mask
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_SetSubnetMask(U32_T subnet)
{
#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)	
	if (subnet != PNetStation->SubnetMask)
	{
		PNetStation->SubnetMask = subnet;
		stoe_WriteReg(STOE_SUBNET_MASK_REG, (U8_T XDATA*)&PNetStation->SubnetMask, 4);
	}
#else /* (MAC_ARBIT_MODE == MAC_ARBIT_WIFI) */
	if (subnet != PNetStation->SecSubnetMask)
	{
		PNetStation->SecSubnetMask = subnet;
		stoe_WriteReg(STOE_SUBNET_MASK_REG, (U8_T XDATA*)&PNetStation->SecSubnetMask, 4);
	}
#endif
} /* End of STOE_SetSubnetMask */

/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_SetGateway
 * Purpose:
 * Params:	gateway- Gateway
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_SetGateway(U32_T gateway)
{
#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)	
	PNetStation->Gateway = gateway;
	if (gateway)
	{
		stoe_WriteReg(STOE_GATEWAY_IP_REG, (U8_T XDATA*)&PNetStation->Gateway, 4);
		ETH_RestartSendArpToGateway();
	}
	else
	{
		gateway = ((PNetStation->StationIP & PNetStation->SubnetMask) | 1);
		if (PNetStation->StationIP == gateway)
		{
			gateway |= (~PNetStation->SubnetMask);
			gateway &= 0xfffffffe;
			PNetStation->Gateway = gateway;
		}

		stoe_WriteReg(STOE_GATEWAY_IP_REG, (U8_T XDATA*)&PNetStation->Gateway, 4);
		}
#else /* (MAC_ARBIT_MODE == MAC_ARBIT_WIFI) */
	PNetStation->SecGateway = gateway;
	if (gateway)
	{
		stoe_WriteReg(STOE_GATEWAY_IP_REG, (U8_T XDATA*)&PNetStation->SecGateway, 4);
		ETH_RestartSendArpToGateway();
	}
	else
	{
		gateway = ((PNetStation->SecStationIP & PNetStation->SecSubnetMask) | 1);
		if (PNetStation->SecStationIP == gateway)
		{
			gateway |= (~PNetStation->SecSubnetMask);
			gateway &= 0xfffffffe;
			PNetStation->SecGateway = gateway;
		}

		stoe_WriteReg(STOE_GATEWAY_IP_REG, (U8_T XDATA*)&PNetStation->SecGateway, 4);
	}

#endif
} /* End of STOE_SetGateway */

/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_Start
 * Purpose: enable all used interrupts and set some register to start
 *			timer, software-dma and processing received packets.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_Start(void)
{
	U8_T XDATA	temp;

	/* clear interrupt status */
	stoe_ReadReg(STOE_INT_STATUS_REG, &temp, 1);

#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)
	/* enable STOE interrupt */
	stoe_InterruptEnable();
#endif

	/* start STOE L2/L3/L4 engines*/
	stoe_StartOperate();

	/* start DMA module */
	DMA_Start();

	/* start MAC module */
	MAC_Start();

	EXTINT4(1);	/* Enable INT4 interrupt for stoe & mac modules. */

} /* End of STOE_Start */

#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)
/*
 * ----------------------------------------------------------------------------
 * Function Name: stoe_InterruptEnable
 * Purpose: enable STOE used interrupt
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_InterruptEnable(void)
{
	U8_T XDATA	temp;

	temp = STOE_DEFAULT_INT_MASK;
	stoe_WriteReg(STOE_INT_MASK_REG, &temp, 1);

} /* End of stoe_InterruptEnable */

/*
 * ----------------------------------------------------------------------------
 * Function Name: stoe_InterruptDisable
 * Purpose: disable STOE used interrupt
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_InterruptDisable(void)
{
	U8_T XDATA	temp;

	temp = 0;
	stoe_WriteReg(STOE_INT_MASK_REG, &temp, 1);

} /* End of stoe_InterruptDisable */

/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_SetInterruptFlag
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_SetInterruptFlag(void)
{
	U8_T XDATA	int_status;

	/* disable stoe interrupt*/
	stoe_InterruptDisable();

	/* read stoe interrupt status */
	stoe_ReadReg(STOE_INT_STATUS_REG, &int_status, 1);

	stoe_InterruptStatus = (int_status & STOE_DEFAULT_INT_MASK);

} /* End of STOE_SetInterruptFlag */

/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_GetInterruptFlag
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T STOE_GetInterruptFlag(void)
{
	return stoe_InterruptStatus;

} /* End of STOE_GetInterruptFlag */

/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_ProcessInterrupt
 * Purpose: When STOE interrupt is trigged, programmer can call this function
 *			to process interrupt event. 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_ProcessInterrupt(void)
{
	U8_T XDATA	temp;
	U8_T		isr = EA;

	while (stoe_InterruptStatus)
	{
		EA = isr;

		if (stoe_InterruptStatus & RCV_PACKET)
		{
			while (PBDP->RFP < PAGES_OF_RCV)
				stoe_RcvHandle();
		}
		if (stoe_InterruptStatus & RCV_BUF_RING_FULL)
		{
			temp = RESUME_PKT_RCV;
			stoe_WriteReg(STOE_L4_CMD_REG, &temp, 1);
		}

		isr = EA;
		EA = 0;

		/* read stoe interrupt status */
		stoe_ReadReg(STOE_INT_STATUS_REG, &stoe_InterruptStatus, 1);
		stoe_InterruptStatus &= STOE_DEFAULT_INT_MASK;
	}

	/* enable stoe interrupt */
	stoe_InterruptEnable();
	EA = isr;

} /* End of STOE_ProcessInterrupt() */
#else
/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_ProcessInterrupt
 * Purpose: programmer can call this function by polling type to find that
 *			interrupt event had happened.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_ProcessInterrupt(void)
{
	U8_T XDATA			temp;
	U8_T				stoe_Page;
	static U8_T			count;

	count = 0;
	
	EMDMR = BIT5;
	stoe_Page = PBDP->RFP;
	EMDMR = 0; 
	while (stoe_Page < PAGES_OF_RCV)
	{
		stoe_RcvHandle();

		if (++count > 3)
		{
			break;
		}

		EMDMR = BIT5;
		stoe_Page = PBDP->RFP;
		EMDMR = 0; 
	}

	/* read stoe interrupt status */
	stoe_ReadReg(STOE_INT_STATUS_REG, &stoe_InterruptStatus, 1);

	if (stoe_InterruptStatus & RCV_BUF_RING_FULL)
	{
		temp = RESUME_PKT_RCV;
		stoe_WriteReg(STOE_L4_CMD_REG, &temp, 1);
	}
}

#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: stoe_StartOperate
 * Purpose: start L2/L3/L4 engine
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_StartOperate(void)
{
	U8_T XDATA	temp;

	stoe_ReadReg(STOE_L2_CTL_REG, &temp, 1);

	temp |= (RX_START_OPERA | TX_START_OPERA);
	stoe_WriteReg(STOE_L2_CTL_REG, &temp, 1);

} /* End of stoe_StartOperate */

#if 0
/*
 * ----------------------------------------------------------------------------
 * Function Name: stoe_StopOperate
 * Purpose: stop L2/L3/L4 engine
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_StopOperate(void)
{
	U8_T XDATA	temp;

	stoe_ReadReg(STOE_L2_CTL_REG, &temp, 1);

	Temp &= ~(RX_START_OPERA | TX_START_OPERA);
	stoe_WriteReg(STOE_L2_CTL_REG, &temp, 1);

} /* End of stoe_StopOperate */
#endif

/*
* -----------------------------------------------------------------------------
 * Function Name: stoe_RcvHandle
 * Purpose: 
 * Params: 
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_RcvHandle(void)
{
	static STOE_FRAME_HEADER XDATA*	pframe;
	U16_T	length;
	U8_T	pages;

	pframe = ((PBDP->RHPR) << 8);
	length = (pframe->Length & 0x7ff);

	if (length > 1536)
	{
		return;
	}

	PBDP->STOE_RxInform.Protocol = pframe->Protocol;
	PBDP->STOE_RxInform.TotalLen = length;
	PBDP->STOE_RxInform.PBuf = &pframe->Packet;

	if (length && STOE_RcvCallback)
		STOE_RcvCallback(&pframe->Packet, length, pframe->Protocol);

	if (PBDP->RHPR > pframe->NPR)
		pages = (PBDP->TSPP - PBDP->RHPR) + (pframe->NPR - PBDP->RSPP);
	else
		pages = pframe->NPR - PBDP->RHPR;

	EMDMR = BIT5;
	PBDP->RHPR = pframe->NPR;
	PBDP->RFP += pages;
	EMDMR = 0;

} /* End of stoe_RcvHandle() */

/*
* -----------------------------------------------------------------------------
 * Function Name: stoe_XmitHandle
 * Purpose: Set send packet bit to tell hardware transmit one packet to ethernet.
 * Params: 
 * Returns:	
 * Note:
 * ----------------------------------------------------------------------------
 */
void stoe_XmitHandle(void)
{
	U8_T XDATA	temp;

    /* Check the tx available */
    stoe_ReadReg(STOE_L4_CMD_REG, &temp, 1);
    if (!(temp & XMIT_PACKET))
    {
        temp = XMIT_PACKET;
        stoe_WriteReg(STOE_L4_CMD_REG, &temp, 1);
    }
    else
        stoe_DeferredCnt++;

} /* stoe_XmitHandle */

/*
* -----------------------------------------------------------------------------
 * Function Name: STOE_DeferredProcess
 * Purpose: Check the packet was deferred and send it if the tx is available.
 * Params: 
 * Returns:	
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_DeferredProcess(void)
{
	U8_T XDATA	temp;

    if (stoe_DeferredCnt)
    {
        /* Check the tx available */
        stoe_ReadReg(STOE_L4_CMD_REG, &temp, 1);
        if (!(temp & XMIT_PACKET))
        {
            temp = XMIT_PACKET;
            stoe_WriteReg(STOE_L4_CMD_REG, &temp, 1);
            stoe_DeferredCnt--;
        }
    }
} /* STOE_DeferredProcess */

/*
* -----------------------------------------------------------------------------
 * Function Name: STOE_Send
 * Purpose:	Call this function to set add STOE header.
 * Params:	pbuf : buffer address, must between TSPP and TEPP.
 *			length : packet length.
 *			protocol : if the packet is ip packet, set the ip layer protocol,
 *						if not a ip packet, set value to 0xff.
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void STOE_Send(U8_T XDATA* pbuf, U16_T length, U8_T protocol)
{
	STOE_FRAME_HEADER XDATA*	pframe = (STOE_FRAME_HEADER XDATA*)pbuf;
	U8_T	occupypages = PBDP->STOE_TxInform.Pages;

	pframe->Length = length;
	pframe->Protocol = protocol;

	EMDMR = BIT5;
	PBDP->TFP -= occupypages;

	PBDP->TTPR += occupypages;
	if (PBDP->TTPR > PBDP->TEPP)
	{
		PBDP->TTPR = PBDP->TSPP + PBDP->TTPR - PBDP->TEPP - 1;
	}
	EMDMR = 0;

	pframe->NPR = PBDP->TTPR;

	stoe_XmitHandle();

} /* End of STOE_Send() */


/*
* -----------------------------------------------------------------------------
 * Function Name: STOE_AssignSendBuf
 * Purpose: Assign a buffer that adress is between TSPP and TEPP, so STOE engine
 *				cans transmit the packet to MAC SRAM.
 * Params: length : Buffer size that the packet occupied.
 * Returns: Assigned transmit buffer address
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T XDATA* STOE_AssignSendBuf(U16_T length)
{
	U8_T XDATA*	pframe;
	U8_T		occupypages;

	occupypages = ((length + PAGE_SIZE - 1)/PAGE_SIZE);
	if (PBDP->TFP <= occupypages)
    {
		return (U8_T XDATA*)0;
    }

	pframe = ((PBDP->TTPR) << 8);
	PBDP->STOE_TxInform.Pages = occupypages;

	return pframe;

} /* End of STOE_AssignSendBuf */

#if STOE_DEBUG
/*
 * ----------------------------------------------------------------------------
 * Function Name: STOE_IndirectIO
 * Purpose: stoe indirect register read/write, only for debugging.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T STOE_IndirectIO(U8_T rw, U16_T index, U8_T XDATA* pbuf)
{
	U8_T len;

	switch (index)
	{
	case STOE_ARP_DATA_REG:
		len = 6;
		break;
	case STOE_IP_ADDR_REG:
		if (rw == 1)
			STOE_SetIPAddr(*(U32_T XDATA*)pbuf);
		len = 4;
		break;
	case STOE_SUBNET_MASK_REG:
		if (rw == 1)
			STOE_SetSubnetMask(*(U32_T XDATA*)pbuf);
		len = 4;
		break;
	case STOE_GATEWAY_IP_REG:
		if (rw == 1)
			STOE_SetGateway(*(U32_T XDATA*)pbuf);
		len = 4;
		break;
	case STOE_RX_VLAN_TAG_REG:
	case STOE_TX_VLAN_TAG_REG:
	case STOE_L4_BDP_PNT_REG:
		len = 2;
		break;
	case STOE_L2_CTL_REG:
	case STOE_BYPASS_REG:
	case STOE_ARP_CMD_REG:
	case STOE_ARP_ADDR_REG:
	case STOE_ARP_TIMEOUT_REG:
	case STOE_CHKSUM_STATUS_REG:
	case STOE_L4_CTL_REG:
	case STOE_L4_CMD_REG:
	case STOE_L4_DMA_BURST_REG:
	case STOE_INT_STATUS_REG:
	case STOE_INT_MASK_REG:
		len = 1;
		break;
	default:
		return 0xff;
	}

	if (rw == 0) /* read */
		stoe_ReadReg (index, pbuf, len);
	else if (rw == 1) /* write */
		stoe_WriteReg (index, pbuf, len);
	else
		return 0xff;

	return	len;

} /* End of STOE_IndirectIO() */
#endif

/* End of stoe.c */