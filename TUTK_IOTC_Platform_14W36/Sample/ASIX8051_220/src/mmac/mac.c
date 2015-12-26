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
 * Module Name: mac.c
 * Purpose:  
 * Author:
 * Date:
 * Notes:
 * $Log: mac.c,v $
 * no message
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "mac.h"
#include "stoe.h"
#include "uart0.h"
#include "delay.h"
#include "at24c02.h"
#include "mpsave.h"
#include "mcpu.h"
#include <intrins.h>
#include "printd.h"

/* GLOBAL VARIABLES DECLARATIONS */
MAC_DEFAULT_AREA code* MAC_Default = 0x6000;
U8_T XDATA MAC_InterruptStatus = 0;
U8_T XDATA MAC_WifiMacAddr[6];
U8_T XDATA MAC_WifiIp[4];
U8_T XDATA MAC_WifiSubMask[4];

/* LOCAL VARIABLES DECLARATIONS */
#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)
  #if (MAC_REMOTE_WAKEUP == MAC_SUPPORT)
  static U8_T XDATA mac_WakeupFlag = 0;
  #endif
#else
  #if (POWER_SAVING)
  static U8_T XDATA mac_WakeupFlag = 0;
  #endif
#endif

/* LOCAL SUBPROGRAM DECLARATIONS */
static void mac_InterruptEnable(void);
static void mac_InterruptDisable(void);
static void mac_StartOperate(void);

/*
 * ----------------------------------------------------------------------------
 * Function Name: mac_ReadReg
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void mac_ReadReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
{
	U8_T	isr;

	isr = EA;
	EA = 0;
	_nop_();
	MCIR = regaddr;
	while (length--)
		pbuf[length] = MDR;
	EA = isr;

} /* End of mac_ReadReg */

/*
 * ----------------------------------------------------------------------------
 * Function Name: mac_WriteReg
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void mac_WriteReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length)
{
	U8_T	isr;

	isr = EA;
	EA = 0;
	while (length--)
		MDR = pbuf[length];
	MCIR = regaddr;
	EA = isr;

} /* End of mac_WriteReg */

#if (MAC_ARBIT_MODE == MAC_ARBIT_WIFI)
/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_ValidateSubsystemId
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_ValidateSubsystemId(void)
{
	U8_T XDATA	temp[6];
	U16_T XDATA	i;

	if (MCPU_NoCfgEeprom())
	{
LOAD_FROM_FLASH:
		if (MAC_Default->HwDefault[0x40] == 'A' &&
			MAC_Default->HwDefault[0x41] == 'S' &&
			MAC_Default->HwDefault[0x42] == 'I' &&
			MAC_Default->HwDefault[0x43] == 'X')
		{
			printd("Load WIFI mac/ip/subnet from FLASH.\n\r");
			MAC_WifiMacAddr[0] = MAC_Default->HwDefault[0x55];
			MAC_WifiMacAddr[1] = MAC_Default->HwDefault[0x54];
			MAC_WifiMacAddr[2] = MAC_Default->HwDefault[0x53];
			MAC_WifiMacAddr[3] = MAC_Default->HwDefault[0x52];
			MAC_WifiMacAddr[4] = MAC_Default->HwDefault[0x51];
			MAC_WifiMacAddr[5] = MAC_Default->HwDefault[0x50];
			MAC_WifiIp[0] = MAC_Default->HwDefault[0x59];
			MAC_WifiIp[1] = MAC_Default->HwDefault[0x58];
			MAC_WifiIp[2] = MAC_Default->HwDefault[0x57];
			MAC_WifiIp[3] = MAC_Default->HwDefault[0x56];
			MAC_WifiSubMask[0] = MAC_Default->HwDefault[0x5D];
			MAC_WifiSubMask[1] = MAC_Default->HwDefault[0x5C];
			MAC_WifiSubMask[2] = MAC_Default->HwDefault[0x5B];
			MAC_WifiSubMask[3] = MAC_Default->HwDefault[0x5A];
		}
		else
		{
			/* Allocate default WIFI mac/ip/subnet */
			printd("Invalid subsystem id in FLASH so that load driver default.\n\r");
			printd("WIFI mac = 00:12:34:56:78:9A\n\r");
			printd("WIFI ip = 192.168.2.250\n\r");
			printd("WIFI subnet = 255.255.255.0\n\r");
			MAC_WifiMacAddr[0] = 0x00;
			MAC_WifiMacAddr[1] = 0x12;
			MAC_WifiMacAddr[2] = 0x34;
			MAC_WifiMacAddr[3] = 0x56;
			MAC_WifiMacAddr[4] = 0x78;
			MAC_WifiMacAddr[5] = 0x9A;
			MAC_WifiIp[0] = 192;
			MAC_WifiIp[1] = 168;
			MAC_WifiIp[2] = 2;
			MAC_WifiIp[3] = 3;
			MAC_WifiSubMask[0] = 255;
			MAC_WifiSubMask[1] = 255;
			MAC_WifiSubMask[2] = 255;
			MAC_WifiSubMask[3] = 0;
			return;
		}
	}
	else
	{
		if(EEPROM_Read(0x40, 4, temp))
		{
			if (temp[0] == 'A' &&
				temp[1] == 'S' &&
				temp[2] == 'I' &&
				temp[3] == 'X')
			{
				if (MAC_Default->HwDefault[0x40] == 'A' &&
					MAC_Default->HwDefault[0x41] == 'S' &&
					MAC_Default->HwDefault[0x42] == 'I' &&
					MAC_Default->HwDefault[0x43] == 'X')
				{
					/* Compare pattern from hardware_type to ip */
					for (i = 0x44; i < 0x5E; i++)
					{
						if (EEPROM_Read(i, 1, temp))
						{
							if (temp[0] != MAC_Default->HwDefault[i])
							{
								printd("Parameters (mac ~ ip) in EEPROM are different with FLASH.\n\rReload from FLASH.\n\r");
								goto LOAD_FROM_FLASH;
							}
						}
						else
						{
							printd("Can not read EEPROM so that reload from FLASH.\n\r");
							goto LOAD_FROM_FLASH;
						}
					}
					
					/* Compare pattern from hardware_type to ip */
					for (i = 0xF4; i < 0x100; i++)
					{
						if (EEPROM_Read(i, 1, temp))
						{
							if (temp[0] != MAC_Default->HwDefault[i])
							{
								printd("Serial num in EEPROM are different with FLASH.\n\rReload from FLASH.\n\r");
								goto LOAD_FROM_FLASH;
							}
						}
						else
						{
							printd("Can not read EEPROM so that reload from FLASH.\n\r");
							goto LOAD_FROM_FLASH;
						}
					}
				}
					
				/* Load WIFI mac from EEPROM */
				if(EEPROM_Read(0x50, 6, temp))
				{
					MAC_WifiMacAddr[0] = temp[5];
					MAC_WifiMacAddr[1] = temp[4];
					MAC_WifiMacAddr[2] = temp[3];
					MAC_WifiMacAddr[3] = temp[2];
					MAC_WifiMacAddr[4] = temp[1];
					MAC_WifiMacAddr[5] = temp[0];
				}
				else
				{
					printd("Can not read EEPROM so that reload from FLASH.\n\r");
					goto LOAD_FROM_FLASH;
				}
				
				/* Load WIFI ip from EEPROM */
				if(EEPROM_Read(0x56, 4, temp))
				{
					MAC_WifiIp[0] = temp[3];
					MAC_WifiIp[1] = temp[2];
					MAC_WifiIp[2] = temp[1];
					MAC_WifiIp[3] = temp[0];
				}
				else
				{
					printd("Can not read EEPROM so that reload from FLASH.\n\r");
					goto LOAD_FROM_FLASH;
				}
				
				/* Load WIFI subnet from EEPROM */
				if(EEPROM_Read(0x5A, 4, temp))
				{
					MAC_WifiSubMask[0] = temp[3];
					MAC_WifiSubMask[1] = temp[2];
					MAC_WifiSubMask[2] = temp[1];
					MAC_WifiSubMask[3] = temp[0];
				}
				else
				{
					printd("Can not read EEPROM so that reload from FLASH.\n\r");
					goto LOAD_FROM_FLASH;
				}
			}
			else
			{
				printd("Invalid subsystem id in EEPROM so that reload from FLASH.\n\r");
				goto LOAD_FROM_FLASH;
			}
		}
		else
		{
			printd("Can not read EEPROM so that reload from FLASH.\n\r");
			goto LOAD_FROM_FLASH;
		}
	}
	
	/* Check out if mac is valid */
	temp[0] = 0;
	temp[1] = 0;
	for (i = 0; i < MAC_ADDRESS_LEN; i++)
	{
		if (MAC_WifiMacAddr[i] == 0x00)
		{
			temp[0] += 1;
		}
		else if (MAC_WifiMacAddr[i] == 0xFF)
		{
			temp[1] += 1;
		}
	}
	
	if (temp[0] == MAC_ADDRESS_LEN ||
		temp[1] == MAC_ADDRESS_LEN)
	{
		printd("Invalid WIFI mac value (all 0x00 or 0xFF) so that load driver default.\n\r");
		printd("WIFI mac = 00:12:34:56:78:9A\n\r");
		MAC_WifiMacAddr[0] = 0x00;
		MAC_WifiMacAddr[1] = 0x12;
		MAC_WifiMacAddr[2] = 0x34;
		MAC_WifiMacAddr[3] = 0x56;
		MAC_WifiMacAddr[4] = 0x78;
		MAC_WifiMacAddr[5] = 0x9A;
	}
	
	/* Check out if ip is valid */
	temp[0] = 0;
	temp[1] = 0;
	for (i = 0; i < 4; i++)
	{
		if (MAC_WifiIp[i] == 0x00)
		{
			temp[0] += 1;
		}
		else if (MAC_WifiIp[i] == 0xFF)
		{
			temp[1] += 1;
		}
	}
	
	if (temp[0] == 4 ||
		temp[1] == 4)
	{
		printd("Invalid WIFI ip value (all 0x00 or 0xFF) so that load driver default.\n\r");
		printd("WIFI ip = 192.168.2.250\n\r");
		printd("WIFI subnet = 255.255.255.0\n\r");
		MAC_WifiIp[0] = 192;
		MAC_WifiIp[1] = 168;
		MAC_WifiIp[2] = 2;
		MAC_WifiIp[3] = 3;
		MAC_WifiSubMask[0] = 255;
		MAC_WifiSubMask[1] = 255;
		MAC_WifiSubMask[2] = 255;
		MAC_WifiSubMask[3] = 0;
	}

} /* MAC_ValidateSubsystemId */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_Init
 * Purpose: initial all registers and variables of MAC.
 * Params: network_type -0: auto- negotiation
 *						-1: fixed 100 full speed.
 *						-2: fixed 100 half speed.
 *						-3: fixed 10 full speed.
 *						-4: fixed 10 half speed.
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_Init(U8_T network_type)
{
	U8_T XDATA	temp[6];
	U8_T		i;

#if (MAC_ARBIT_MODE == MAC_ARBIT_WIFI)
	PNetStation->SecPermStaAddr[0] = MAC_WifiMacAddr[0];
	PNetStation->SecPermStaAddr[1] = MAC_WifiMacAddr[1];
	PNetStation->SecPermStaAddr[2] = MAC_WifiMacAddr[2];
	PNetStation->SecPermStaAddr[3] = MAC_WifiMacAddr[3];
	PNetStation->SecPermStaAddr[4] = MAC_WifiMacAddr[4];
	PNetStation->SecPermStaAddr[5] = MAC_WifiMacAddr[5];
	for (i = 0; i < MAC_ADDRESS_LEN; i++)
		PNetStation->SecCurrStaAddr[i] = PNetStation->SecPermStaAddr[i];

	mac_WriteReg(MAC_ADDR_REG, PNetStation->SecPermStaAddr, MAC_ADDRESS_LEN);
#else /* (MAC_ARBIT_MODE & MAC_ARBIT_ETH) */
	mac_ReadReg(MAC_ADDR_REG, PNetStation->PermStaAddr, MAC_ADDRESS_LEN);

	for (i = 0; i < MAC_ADDRESS_LEN; i++)
		PNetStation->CurrStaAddr[i] = PNetStation->PermStaAddr[i];
#endif

#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)
	/* do not use WiFi, change to Ethernet mode */
	temp[0] = ARBIT_ETH_ONLY;
#else /* MAC_ARBIT_WIFI */
	temp[0] = ARBIT_MII_ONLY;
#endif
	mac_WriteReg(MAC_ARB_CTL_REG, temp, 1);

	PBDP->MacInfo.MediumLinkType = (MEDIUM_ENABLE_RECEIVE | MEDIUM_ENABLE_TX_FLOWCTRL);
	PBDP->MacInfo.FullDuplex = 1;

#if (MAC_ARBIT_MODE == MAC_ARBIT_WIFI)
	/* FIXED_100_HALF in WiFi mode without PHY */
	network_type = FIXED_100_FULL; //FIXED_100_FULL
	PBDP->MacInfo.NetworkType = MAC_LINK_100M_SPEED;
	PBDP->MacInfo.MediumLinkType |= MEDIUM_MII_100M_MODE;
	/* set medium status */
	mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &PBDP->MacInfo.MediumLinkType, 1);
#else  /* (MAC_ARBIT_MODE & MAC_ARBIT_ETH) */
	switch (network_type)
	{
	default:
	case AUTO_NEGOTIATION:
		PBDP->MacInfo.NetworkType = MAC_AUTO_NEGOTIATION;
		PBDP->MacInfo.MediumLinkType|=(MEDIUM_FULL_DUPLEX_MODE | MEDIUM_MII_100M_MODE |
			MEDIUM_ENABLE_RX_FLOWCTRL);
		break;
	case FIXED_100_FULL:
		PBDP->MacInfo.NetworkType = (MAC_LINK_100M_SPEED | MAC_LINK_FULL_DUPLEX);
		PBDP->MacInfo.MediumLinkType|=(MEDIUM_FULL_DUPLEX_MODE | MEDIUM_MII_100M_MODE |
			MEDIUM_ENABLE_RX_FLOWCTRL);
		break;
	case FIXED_100_HALF:
		PBDP->MacInfo.NetworkType = MAC_LINK_100M_SPEED;
		PBDP->MacInfo.MediumLinkType |= MEDIUM_MII_100M_MODE;
		break;
	case FIXED_10_FULL:
		PBDP->MacInfo.NetworkType = (MAC_LINK_10M_SPEED | MAC_LINK_FULL_DUPLEX);
		PBDP->MacInfo.MediumLinkType |= (MEDIUM_FULL_DUPLEX_MODE |
			MEDIUM_ENABLE_RX_FLOWCTRL);
		break;
	case FIXED_10_HALF:
		PBDP->MacInfo.NetworkType = MAC_LINK_10M_SPEED;
		break;
	}
	/* set medium status */
	mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &PBDP->MacInfo.MediumLinkType, 1);
#endif

	/* set IPG */
	temp[0] = 0x2b;
	temp[1] = 0x16;
	temp[2] = 0x95;
	mac_WriteReg(MAC_IPG_CTL_REG, temp, 3);

	/* set RX filter. */
	temp[0] = BIT6;
	mac_WriteReg(MAC_RX_CTL_REG, temp, 1);
	MAC_SetRxFilter(MAC_RCV_BROADCAST);

	PBDP->MacInfo.LinkSpeed = 0; // Ethernet not linkup.

#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
	/* decide interrupt mask */
#if (MAC_GET_INTSTATUS_MODE == MAC_INTERRUPT_MODE) 
	PBDP->MacInfo.InterruptMask = PRIMARY_LINK_CHANGE_ENABLE;
#endif

	/* phy initialize. */
	PHY_Init(network_type);
#endif
} /* End of MAC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_GetMacAddr
 * Purpose:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T* MAC_GetMacAddr(void)
{
#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)	
	return PNetStation->CurrStaAddr;
#else /* MAC_ARBIT_WIFI */
	return PNetStation->SecCurrStaAddr;
#endif
} /* End of MAC_GetMacAddr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_SetMacAddr
 * Purpose:
 * Params:	addr- pointer to MAC address
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_SetMacAddr(U8_T* addr)
{
	U8_T i;

#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH)
		for (i = 0; i < MAC_ADDRESS_LEN; i++)
			PNetStation->CurrStaAddr[i] = addr[i];

		mac_WriteReg(MAC_ADDR_REG, PNetStation->CurrStaAddr, MAC_ADDRESS_LEN);
#else /* MAC_ARBIT_WIFI */
		for (i = 0; i < MAC_ADDRESS_LEN; i++)
			PNetStation->SecCurrStaAddr[i] = addr[i];
			
		mac_WriteReg(MAC_ADDR_REG, PNetStation->SecCurrStaAddr, MAC_ADDRESS_LEN);
#endif
} /* End of MAC_SetMacAddr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_Start
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_Start(void)
{
#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
  #if (MAC_GET_INTSTATUS_MODE == MAC_INTERRUPT_MODE)

	U8_T XDATA	temp;

	/* clear mac interrupt status */
	mac_ReadReg(MAC_WAKEUP_LINK_INT_STATUS_REG, &temp, 1);

	/* enable mac interrupt */
	mac_InterruptEnable();
  #endif
#endif

	/* start mac to receive/transmit packets */
	mac_StartOperate();

} /* End of MAC_Start() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: mac_StartOperate
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void mac_StartOperate(void)
{
	U8_T XDATA	temp;

	mac_ReadReg(MAC_RX_CTL_REG, &temp, 1);

	/* set mac register to start receive/transmit packets. */
	temp |= START_OPERATION;
	mac_WriteReg(MAC_RX_CTL_REG, &temp, 1);

} /* End of mac_StartOperate */

#if 0
/*
 * ----------------------------------------------------------------------------
 * Function Name: mac_StopOperate
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void mac_StopOperate(void)
{
	U8_T XDATA	temp;

	mac_ReadReg(MAC_RX_CTL_REG, &temp, 1);

	temp &= ~START_OPERATION;
	mac_WriteReg(MAC_RX_CTL_REG, &temp, 1);

} /* End of mac_StopOperate */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_SetRxFilter
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_SetRxFilter(U8_T filter)
{
	U8_T XDATA	temp;

	mac_ReadReg(MAC_RX_CTL_REG, &temp, 1);

	/* set rcv filter. */
	temp &= 0xc0;
	if (filter & MAC_RCV_PROMISCUOUS)
		temp |= PACKET_TYPE_PROMISCOUS;
	if (filter & MAC_RCV_ALL_MULTICAST)
		temp |= PACKET_TYPE_ALL_MULTI;
	if (filter & MAC_RCV_BROADCAST)
		temp |= PACKET_TYPE_BROADCAST;
	if (filter & MAC_RCV_MULTICAST)
		temp |= PACKET_TYPE_MULTICAST;

	mac_WriteReg(MAC_RX_CTL_REG, &temp, 1);

} /* End of MAC_SetRxFilter() */

#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
#if (MAC_GET_INTSTATUS_MODE == MAC_INTERRUPT_MODE) 
/*
 * ----------------------------------------------------------------------------
 * Function Name: mac_InterruptEnable
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void mac_InterruptEnable(void)
{
	U8_T XDATA	temp;

	/* set link change interrupt enable */
	temp = PBDP->MacInfo.InterruptMask;
	mac_WriteReg(MAC_LINK_CHANGE_INT_ENABLE_REG, &temp, 1);

} /* End of mac_InterruptEnable */

/*
 * ----------------------------------------------------------------------------
 * Function Name: mac_InterruptDisable
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void mac_InterruptDisable(void)
{
	U8_T XDATA	temp;

	temp = 0;
	mac_WriteReg(MAC_LINK_CHANGE_INT_ENABLE_REG, &temp, 1);

} /* End of mac_InterruptDisable */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_SetInterruptFlag
 * Purpose:
 * Params:
 * Returns:
 * Note: The function only be called by interrupt routine
 * ----------------------------------------------------------------------------
 */
void MAC_SetInterruptFlag(void)
{
	mac_ReadReg(MAC_WAKEUP_LINK_INT_STATUS_REG, &MAC_InterruptStatus, 1);

	if (MCPU_SleepFlag)
		MCPU_SetWakeUpByMacFlag(MAC_InterruptStatus);

	if (!(MAC_InterruptStatus & STATUS_PRIMARY_LINK_CHANGE))
		MAC_InterruptStatus = 0;

} /* End of MAC_SetInterruptFlag() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_GetInterruptFlag
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T MAC_GetInterruptFlag(void)
{
	U8_T flag, isr;

	isr = EA;
	EA = 0;
	flag = MAC_InterruptStatus;
	EA = isr;

	return flag;

} /* End of MAC_GetInterruptFlag() */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: mac_GetMediaType
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T mac_GetMediaType(void)
{
	U16_T	phylinkstatus;

	phylinkstatus = PHY_CheckMediaType();

	PBDP->MacInfo.FullDuplex = TRUE;

	PBDP->MacInfo.MediumLinkType = (MEDIUM_ENABLE_TX_FLOWCTRL | MEDIUM_ENABLE_RECEIVE);

	/* Determine if we're linked to 100 full duplex. */
	if (phylinkstatus & ANLPAR_100TXFD)
	{
		PBDP->MacInfo.LinkSpeed = 100;
		PBDP->MacInfo.MediumLinkType |=(MEDIUM_FULL_DUPLEX_MODE |MEDIUM_MII_100M_MODE);
		printd ("Link to 100 FULL.\n\r");
	}
	/* Determine if we're linked to 100 half duplex. */
	else if (phylinkstatus & ANLPAR_100TX)
	{
		PBDP->MacInfo.LinkSpeed = 100;
		PBDP->MacInfo.MediumLinkType |= MEDIUM_MII_100M_MODE;
		PBDP->MacInfo.FullDuplex = FALSE;
		printd ("Link to 100 HALF.\n\r");
	} 
	/* Determine if we're linked to 10 full duplex. */
	else if (phylinkstatus & ANLPAR_10TFD)
	{
		PBDP->MacInfo.LinkSpeed = 10;
		PBDP->MacInfo.MediumLinkType |= MEDIUM_FULL_DUPLEX_MODE;
		printd ("Link to 10 FULL.\n\r");
	}
	/* we're linked to 10 half duplex. */
	else
	{
		PBDP->MacInfo.LinkSpeed = 10;
		PBDP->MacInfo.FullDuplex = FALSE;
		printd ("Link to 10 HALF.\n\r");
	}

	if (PBDP->MacInfo.FullDuplex)
		PBDP->MacInfo.MediumLinkType |= MEDIUM_ENABLE_RX_FLOWCTRL;
	mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &PBDP->MacInfo.MediumLinkType, 1);

	return PBDP->MacInfo.LinkSpeed;

} /* End of mac_GetMediaType */

#if (MAC_GET_INTSTATUS_MODE == MAC_INTERRUPT_MODE)
/*
 * ----------------------------------------------------------------------------
 * Function Name: mac_LinkSpeedChk
 * Purpose:
 * Params: none
 * Returns:	100: Ethernet is link to 100M
 *			10 : Ethernet is link to 10M
 *			0  : not link
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T mac_LinkSpeedChk(void)
{
	if (MAC_InterruptStatus & STATUS_PRIMARY_IS_LINK_UP)
	{
		STOE_ConnectState |= STOE_ETHERNET_LINK;
		if (PBDP->MacInfo.NetworkType == MAC_AUTO_NEGOTIATION)
			mac_GetMediaType();
	}
	else
	{
		printd ("Ethernet is now unlink.\n\r");
		STOE_ConnectState &= ~STOE_ETHERNET_LINK;
		if (PBDP->MacInfo.LinkSpeed)
		{
			PBDP->MacInfo.LinkSpeed = 0;
		}
	}

	return PBDP->MacInfo.LinkSpeed;

} /* End of mac_LinkSpeedChk */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_ProcessInterrupt
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_ProcessInterrupt(void)
{
	U8_T XDATA			temp;

	if (MAC_InterruptStatus & STATUS_PRIMARY_LINK_CHANGE)
	{
		if (mac_LinkSpeedChk())
		{
			mac_ReadReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
			temp |= MEDIUM_ENABLE_RECEIVE;
			mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
			STOE_DROP_PKT = 0;
		}
		else
		{
			mac_ReadReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
			temp &= ~MEDIUM_ENABLE_RECEIVE;
			mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
			STOE_DROP_PKT = 1;
		}
	}

	MAC_InterruptStatus = 0;

} /* End of MAC_ProcessInterrupt */
#else
/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_LinkSpeedChk
 * Purpose:
 * Params: none
 * Returns:	100: Ethernet is link to 100M
 *			10 : Ethernet is link to 10M
 *			0  : not link
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T MAC_LinkSpeedChk(void)
{
	mac_ReadReg(MAC_WAKEUP_LINK_INT_STATUS_REG, &MAC_InterruptStatus, 1);
	if (MAC_InterruptStatus & STATUS_PRIMARY_IS_LINK_UP)
	{
		STOE_ConnectState |= STOE_ETHERNET_LINK;
		if (PBDP->MacInfo.LinkSpeed == 0)
		{
			if (PBDP->MacInfo.NetworkType == MAC_AUTO_NEGOTIATION)
				mac_GetMediaType();
			else
				PBDP->MacInfo.LinkSpeed = PBDP->MacInfo.NetworkType & 0x7f;
		}
	}
	else
	{
		STOE_ConnectState &= ~STOE_ETHERNET_LINK;
		if (PBDP->MacInfo.LinkSpeed)
		{
			PBDP->MacInfo.LinkSpeed = 0;
		}
	}

	return PBDP->MacInfo.LinkSpeed;

} /* End of MAC_LinkSpeedChk */
#endif

#if (MAC_REMOTE_WAKEUP == MAC_SUPPORT)
/*
* -----------------------------------------------------------------------------
 * Function Name: mac_crc16
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T mac_crc16(U8_T* pbuf, U8_T len)
{
	U16_T	crc, carry;
	U8_T	i, j;
	S8_T	curByte;

	crc = 0xffff;

	for (i = 0; i < len; i++)
	{
		curByte = pbuf[i];
		for (j = 0; j < 8; j++)
		{
			carry = ((crc & 0x8000) ? 1 : 0) ^ (curByte & 0x01);
			crc <<= 1;
			curByte >>= 1;

			if (carry)
			    crc = ((crc ^ 0x8004) | carry);
		}
    }

	return crc;

} /* End of cal_crc16 */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_SetWakeupFrame
 * Purpose: for setting registers about wakeup frame
 * Params:	pWuf : point to MAC_WUF data structure
 * Returns:	0: setting fail.
 *			1: setting success.
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T MAC_SetWakeupFrame(MAC_WUF* pWuf)
{
	U32_T XDATA	mask0, mask1;
	U16_T XDATA crc16;
	U8_T XDATA*	pMask;
	U8_T XDATA	wakeupCmd, offset0, offset1;
	U8_T		mode, i, maskLen, x, y;
	U8_T		frame[64];

	mode = pWuf->Mode;
	offset0 = pWuf->Offset0;
	offset1 = pWuf->Offset1;
	mask0 = pWuf->Mask0;
	mask1 = pWuf->Mask1;

	mac_WriteReg(MAC_WAKEUP_FRAME_OFFSET1, &offset1, 1);
	mac_WriteReg(MAC_WAKEUP_FRAME_OFFSET0, &offset0, 1);
	mac_WriteReg(MAC_WAKEUP_FRAME_MASK1, (U8_T XDATA*)&mask1, 4);
	mac_WriteReg(MAC_WAKEUP_FRAME_MASK0, (U8_T XDATA*)&mask0, 4);

	if (mode < WUF_BOTH_FILTERS) /* single filter */
	{
		if (mode & BIT0)
		{
			wakeupCmd = WAKEUP_FRAME_FILTER1_ENABLE;
			if (mode & BIT1)
				wakeupCmd |= WAKEUP_FRAME_FILTER1_UNI_ENABLE;
		}
		else
		{
			wakeupCmd = WAKEUP_FRAME_FILTER0_ENABLE;
			if (mode & BIT1)
				wakeupCmd |= WAKEUP_FRAME_FILTER0_UNI_ENABLE;
		}
	}
	else if (mode < WUF_CASCADE) /* both filter */
	{
		wakeupCmd = (WAKEUP_FRAME_FILTER1_ENABLE | WAKEUP_FRAME_FILTER0_ENABLE);
		if (mode & BIT1)
			wakeupCmd |= WAKEUP_FRAME_FILTER0_UNI_ENABLE;
	}
	else /* cascade */
	{
		wakeupCmd = (WAKEUP_FRAME_FILTER1_ENABLE | WAKEUP_FRAME_FILTER0_ENABLE |
			WAKEUP_FRAME_CASCADE_ENABLE);
		if (mode & BIT1)
			wakeupCmd |= WAKEUP_FRAME_FILTER0_UNI_ENABLE;
	}

	if ((wakeupCmd & WAKEUP_FRAME_FILTER0_UNI_ENABLE) && (mask0 == 0))
		return 0;

	if ((wakeupCmd & WAKEUP_FRAME_FILTER1_UNI_ENABLE) && (mask1 == 0))
		return 0;

	if ((wakeupCmd & WAKEUP_FRAME_FILTER0_UNI_ENABLE) &&
		!(wakeupCmd & WAKEUP_FRAME_CASCADE_ENABLE))
	{
		maskLen = 0;
		for (i = 0; i < 64; i++) /* clear buffer */
			frame[i] = 0;

		pMask = (U8_T XDATA*)&mask0;
		for (x = 0; x < 4; x++)
		{
			for (y = 0; y < 8; y++)
			{
				if (pMask[3 - x] & (1 << y))
				{
					if ((8*x + y + offset0*2) > pWuf->Length)
						return 0;

					frame[maskLen] = pWuf->Pframe[8*x + y + offset0*2];
					maskLen++;
				}
			}
		}

		crc16 = mac_crc16(frame, maskLen);
		mac_WriteReg(MAC_WAKEUP_FRAME_CRC0, (U8_T XDATA*)&crc16, 2);
		mac_WriteReg(MAC_WAKEUP_FRAME_LAST_BYTE0, &frame[maskLen -1], 1);
	}

	if ((wakeupCmd & WAKEUP_FRAME_FILTER1_UNI_ENABLE) &&
		!(wakeupCmd & WAKEUP_FRAME_CASCADE_ENABLE))
	{
		maskLen = 0;
		for (i = 0; i < 64; i++) /* clear buffer */
			frame[i] = 0;

		pMask = (U8_T XDATA*)&mask1;
		for (x = 0; x < 4; x++)
		{
			for (y = 0; y < 8; y++)
			{
				if (pMask[3 - x] & (1 << y))
				{
					if ((8*x + y + offset1*2) > pWuf->Length)
						return 0;

					frame[maskLen] = pWuf->Pframe[8*x + y + offset1*2];
					maskLen++;
				}
			}
		}

		crc16 = mac_crc16(frame, maskLen);
		mac_WriteReg(MAC_WAKEUP_FRAME_CRC1, (U8_T XDATA*)&crc16, 2);
		mac_WriteReg(MAC_WAKEUP_FRAME_LAST_BYTE1, &frame[maskLen -1], 1);
	}

	if (wakeupCmd & WAKEUP_FRAME_CASCADE_ENABLE)
	{
		maskLen = 0;
		for (i = 0; i < 64; i++) /* clear buffer */
			frame[i] = 0;

		pMask = (U8_T XDATA*)&mask0;
		for (x = 0; x < 4; x++)
		{
			for (y = 0; y < 8; y++)
			{
				if (pMask[3 - x] & (1 << y))
				{
					if ((8*x + y + offset0*2) > pWuf->Length)
						return 0;

					frame[maskLen] = pWuf->Pframe[8*x + y + offset0*2];
					maskLen++;
				}
			}
		}

		pMask = (U8_T XDATA*)&mask1;
		for (x = 0; x < 4; x++)
		{
			for (y = 0; y < 8; y++)
			{
				if (pMask[3 - x] & (1 << y))
				{
					if ((8*x + y + (offset1 + offset0)*2 + 32) > pWuf->Length)
						return 0;

					frame[maskLen] = pWuf->Pframe[8*x + y + (offset1 + offset0)*2 + 32];
					maskLen++;
				}
			}
		}

		crc16 = mac_crc16(frame, maskLen);
		mac_WriteReg(MAC_WAKEUP_FRAME_CRC1, (U8_T XDATA*)&crc16, 2);
		mac_WriteReg(MAC_WAKEUP_FRAME_LAST_BYTE1, &frame[maskLen -1], 1);
	}

	mac_WriteReg(MAC_WAKEUP_FRAME_COMMAND, &wakeupCmd, 1);
	return 1;

} /* End of MAC_SetWakeupFrame() */

/*
* -----------------------------------------------------------------------------
 * Function Name: MAC_SystemSleep
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_SystemSleep(void)
{
	U8_T XDATA	temp;

	/* disable mac receive */
	mac_ReadReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
	temp &= 0xfb;
	mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);

	DELAY_Ms(500);

	PCON &= ~(PMM_ | STOP_ | SWB_ | PMMS_);
	if (MCPU_GetPowerSavingMode() & STOP_)
		PCON |= STOP_;
	else 
		PCON |= (MCPU_GetPowerSavingMode() | PMMS_);

	MCPU_SleepFlag = 1;

	DELAY_Ms(500);

	/* enable mac receive */
	mac_ReadReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
	temp |= 4;
	mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);

} /* End of MAC_SystemSleep() */
#endif
#endif /* #if (MAC_ARBIT_MODE & MAC_ARBIT_ETH) */

#if (MAC_ARBIT_MODE == MAC_ARBIT_ETH) 
  #if (MAC_REMOTE_WAKEUP==MAC_SUPPORT)
/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_WakeupEnable
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_WakeupEnable(U8_T type)
{
	U8_T	temp = 0;

	mac_WakeupFlag |= type;
	if (mac_WakeupFlag & MAC_PRIMARY_PHY_LINK_WAKEUP)
		temp |= WAKEUP_BY_PRIMARY_LINK_UP;
	if (mac_WakeupFlag & MAC_MAGIC_PACKET_WAKEUP)
		temp |= WAKEUP_BY_MAGIC_PACKET;
	if (mac_WakeupFlag & MAC_EXTERNAL_PIN_WAKEUP)
		temp |= WAKEUP_BY_EXTER_PIN_TRIG;
	if (mac_WakeupFlag & MAC_MS_FRAME_WAKEUP)
		temp |= WAKEUP_BY_MICROSOFT_FRAME;

	mac_WriteReg(MAC_WAKEUP_INT_ENABLE_REG, &temp, 1);

	if (mac_WakeupFlag)
	{
		MCPU_SetWakeUpEventFlag(MCPU_WAKEUP_BY_ETHERNET);
	}

} /* End of MAC_WakeupEnable() */
  #endif
#else
  #if (POWER_SAVING)
/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_WakeupEnable
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_WakeupEnable(U8_T type)
{
	U8_T	temp = 0;

	mac_WakeupFlag |= type;
	if (mac_WakeupFlag & MAC_PRIMARY_PHY_LINK_WAKEUP)
		temp |= WAKEUP_BY_PRIMARY_LINK_UP;
	if (mac_WakeupFlag & MAC_MAGIC_PACKET_WAKEUP)
		temp |= WAKEUP_BY_MAGIC_PACKET;
	if (mac_WakeupFlag & MAC_EXTERNAL_PIN_WAKEUP)
		temp |= WAKEUP_BY_EXTER_PIN_TRIG;
	if (mac_WakeupFlag & MAC_MS_FRAME_WAKEUP)
		temp |= WAKEUP_BY_MICROSOFT_FRAME;

	mac_WriteReg(MAC_WAKEUP_INT_ENABLE_REG, &temp, 1);

	if (mac_WakeupFlag)
	{
		MCPU_SetWakeUpEventTable(MCPU_WAKEUP_BY_ETHERNET);
	}

} /* End of MAC_WakeupEnable() */
  #endif
#endif

#if (POWER_SAVING == MAC_SUPPORT)
/*
* -----------------------------------------------------------------------------
 * Function Name: MAC_PowerSavingSleep
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_PowerSavingSleep(void)
{
#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
	U8_T XDATA	temp;

	if ((MCPU_GetPowerSavingMode() & STOP_) || PS_State) /* stop mode or deep sleep */ 
	{
		/* disable mac receive */
		mac_ReadReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
		temp &= 0xfb;
		mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
	}
#endif

	PCON &= ~(PMM_ | STOP_ | SWB_ | PMMS_);
	if (MCPU_GetPowerSavingMode() & STOP_)
		PCON |= STOP_;
	else
		PCON |= (MCPU_GetPowerSavingMode() | PMMS_);

	MCPU_SleepFlag = 1;

#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
	if ((MCPU_GetPowerSavingMode() & STOP_) || PS_State) /* stop mode or deep sleep */
	{
		/* enable mac receive */
		mac_ReadReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
		temp |= 4;
		mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &temp, 1);
	}
#endif

} /* End of MAC_PowerSavingSleep() */
#endif

#if (MAC_MULTICAST_FILTER == MAC_SUPPORT)
/*
* -----------------------------------------------------------------------------
 * Function Name: mac_ComputeCrc32
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T mac_ComputeCrc32(U16_T length, U8_T* pbuf)
{
	U32_T	crc32 = 0xffffffff;
	U8_T	curByte, carry, j;
	
	for (; length; length--)
	{
		curByte = *pbuf++;
		for (j=0; j<8; j++)
		{
			carry = curByte & 1;
			if ( crc32 & 0x80000000 )
				carry ^= 1;
			crc32 <<= 1;
			curByte >>= 1;
			if (carry)
				crc32 ^= 0x04c11db7;
		}
	}
	return crc32;

} /* End of mac_ComputeCrc32() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_MultiFilter
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAC_MultiFilter(U8_T* pbuf, U8_T len)
{
	U8_T	count = len/MAC_ADDRESS_LEN;
	U8_T	index, bitNum;
	U8_T 	filter[8] = {0};

	for (index = 0; index < count; index++)
	{
		if (!(pbuf[0] & 1))
			break;

		bitNum = (U8_T)((mac_ComputeCrc32(MAC_ADDRESS_LEN, pbuf)>>26)&0x3f);
		pbuf += MAC_ADDRESS_LEN;
		filter[7 - (bitNum/8)] |= (1 << (bitNum % 8));
	}

	mac_WriteReg(MAC_MULTICASE_REG, filter, 8);

} /* End of MAC_MultiFilter() */
#endif

#if MAC_DEBUG
/*
 * ----------------------------------------------------------------------------
 * Function Name: MAC_IndirectIO
 * Purpose: mac indirect register read/write, only for debugging.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T MAC_IndirectIO(U8_T rw, U16_T index, U8_T XDATA* pbuf)
{
	U8_T len;

	switch (index)
	{
	case MAC_SRAM_DATA_REG:
	case MAC_MULTICASE_REG:
		len = 8;
		break;
	case MAC_ADDR_REG:
		if (rw == 1)
		{
			MAC_SetMacAddr(pbuf);
		}
		len = 6;
		break;
	case MAC_WAKEUP_FRAME_MASK0:
	case MAC_WAKEUP_FRAME_MASK1:
		len = 4;
		break;
	case MAC_IPG_CTL_REG:
		len = 3;
		break;
	case MAC_SRAM_CMD_REG:
	case MAC_WAKEUP_FRAME_CRC0:
	case MAC_WAKEUP_FRAME_CRC1:
	case MAC_IN_PHY_MSG_REG:
		len = 2;
		break;
	case MAC_RX_CTL_REG:
	case MAC_TEST_REG:
	case MAC_MEDIUM_STATUS_MODE_REG:
	case MAC_ETH_INTERFACE_REG:
	case MAC_WAKEUP_INT_ENABLE_REG:
	case MAC_LINK_CHANGE_INT_ENABLE_REG:
	case MAC_WAKEUP_LINK_INT_STATUS_REG:
	case MAC_WAKEUP_FRAME_COMMAND:
	case MAC_WAKEUP_FRAME_OFFSET0:
	case MAC_WAKEUP_FRAME_LAST_BYTE0:
	case MAC_WAKEUP_FRAME_OFFSET1:
	case MAC_WAKEUP_FRAME_LAST_BYTE1:
	case MAC_ARB_CTL_REG:
	case MAC_IN_PHY_CTL_STATUS_REG:
	case MAC_IN_PHY_INT_ENABLE_REG:
	case MAC_IN_PHY_INT_STATUS_REG:
		len = 1;
		break;
	default:
		return 0xff;
	}

	if (rw == 0) /* read */
		mac_ReadReg (index, pbuf, len);
	else if (rw == 1) /* write */
		mac_WriteReg (index, pbuf, len);
	else
		return 0xff;

	return	len;

} /* End of MAC_IndirectIO() */
#endif

/* End of mac.c */