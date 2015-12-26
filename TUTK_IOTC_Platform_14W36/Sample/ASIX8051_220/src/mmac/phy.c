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
 * Module Name: phy.c
 * Purpose:  
 * Author:
 * Date:
 * Notes:
 * $Log: phy.c,v $
 * no message
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "mcpu.h"
#include "at24c02.h"
#include "phy.h"
#include "stoe.h"
#include "delay.h"
#include <intrins.h>

#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)

/* LOCAL VARIABLES DECLARATIONS */
static U16_T XDATA phy_Control = 0;

/*
* -----------------------------------------------------------------------------
 * Function Name: phy_ReadReg
 * Purpose: Read PHY register.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void phy_ReadReg(U8_T regaddr, U16_T XDATA* pvalue)
{
	U8_T	temp, isr;

	isr = EA;
	EA = 0;
	_nop_();

	/* abort command */
	EPCR = 0xff;

	EPCR = regaddr;
	EPCR = (PBDP->PhyInfo.PhyAddr + PHY_ACCESS_GO + PHY_READ_REG);

	temp = EPCR;
	temp = EPCR;
	while (temp & PHY_ACCESS_GO)
	{
		DELAY_Us(1);
		temp = EPCR;
		temp = EPCR;

	} /* End of while */

	*pvalue = EPDR;
	temp = EPDR;
	EA = isr;
	*pvalue += ((U16_T)temp << 8);

} /* End of phy_ReadReg() */

/*
* -----------------------------------------------------------------------------
 * Function Name: phy_WriteReg
 * Purpose: Set PHY register.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void phy_WriteReg(U8_T regaddr, U16_T value)
{
	U8_T	temp, isr;

	isr = EA;
	EA = 0;
	_nop_();

	/* abort command */
	EPCR = 0xff;

	EPDR = (U8_T)value;
	EPDR = (U8_T)(value >> 8);
	EPCR = regaddr;
	EPCR = (PBDP->PhyInfo.PhyAddr + PHY_ACCESS_GO);

	temp = EPCR;
	temp = EPCR;
	while (temp & PHY_ACCESS_GO)
	{
		DELAY_Us(1);
		temp = EPCR;
		temp = EPCR;

	} /* End of File */
	EA = isr;

} /* End of phy_WriteReg() */

/*
* -----------------------------------------------------------------------------
 * Function Name: PHY_Init
 * Purpose: Initial some registers and variables of PHY.
 * Params: network_type -0: auto- negotiation
 *						-1: fixed 100 full speed.
 *						-2: fixed 100 half speed.
 *						-3: fixed 10 full speed.
 *						-4: fixed 10 half speed.
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PHY_Init(U8_T network_type)
{
	U16_T XDATA	phyctrl, phyanar, tempshort;

	if (MCPU_NoCfgEeprom())
	{
		PBDP->PhyInfo.PhyAddr = MAC_Default->HwDefault[0x20];
	}
	else
	{
		if(!EEPROM_Read(0x20, 1, &PBDP->PhyInfo.PhyAddr))
		{
			PBDP->PhyInfo.PhyAddr = PHY_ADDRESS;
		}
	}

	/* phy power up */
	phy_WriteReg(MII_PHY_CONTROL, 0);
	DELAY_Ms (100);

	/* software reset */
	phy_WriteReg(MII_PHY_CONTROL, CONTROL_RESET);
	DELAY_Ms (50);

	/* read phy register 0 */
	phy_ReadReg(MII_PHY_CONTROL, &phyctrl);
	tempshort = phyctrl;
	phyctrl &= ~(CONTROL_POWER_DOWN | CONTROL_ISOLATE | CONTROL_LOOPBACK |
		CONTROL_RESET | CONTROL_100MB | CONTROL_FULL_DUPLEX);
	if (phyctrl != tempshort)
		phy_WriteReg(MII_PHY_CONTROL, phyctrl);

	switch (network_type)
	{
	default:
	case AUTO_NEGOTIATION:
		phyctrl |= (CONTROL_100MB | CONTROL_FULL_DUPLEX);
		phyanar = ANAR_SELECTOR_8023 + (ANAR_PAUSE | ANAR_100TXFD | ANAR_100TX |
			ANAR_10TFD | ANAR_10T);
		break;
	case FIXED_100_FULL:
		phyctrl |= (CONTROL_100MB | CONTROL_FULL_DUPLEX);
		phyanar = ANAR_SELECTOR_8023 + (ANAR_PAUSE | ANAR_100TXFD);
		break;
	case FIXED_100_HALF:
		phyctrl |= CONTROL_100MB;
		phyanar = ANAR_SELECTOR_8023 + (ANAR_PAUSE | ANAR_100TX);
		break;
	case FIXED_10_FULL:
		phyctrl |= CONTROL_FULL_DUPLEX;
		phyanar = ANAR_SELECTOR_8023 + (ANAR_PAUSE | ANAR_10TFD);
		break;
	case FIXED_10_HALF:
		phyanar = ANAR_SELECTOR_8023 + (ANAR_PAUSE | ANAR_10T);
		break;
	}

	/* set phy register 0 & 4 to start negotitation*/
	phy_WriteReg(MII_PHY_ANAR, phyanar);
	phyctrl |= (CONTROL_ENABLE_AUTO | CONTROL_START_AUTO);
	phy_WriteReg(MII_PHY_CONTROL, phyctrl);
	phy_Control = phyctrl;

} /* End of PHY_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: PHY_CheckMediaType
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T PHY_CheckMediaType(void)
{
	U16_T XDATA	phylinkstatus = 0;

	while (!(phylinkstatus & BIT5))
	{
		DELAY_Ms(10);
		phy_ReadReg(MII_PHY_STATUS, &phylinkstatus);
	}

	phy_ReadReg(MII_PHY_ANLPAR, &phylinkstatus);

	return phylinkstatus;

} /* End of PHY_CheckMediaType() */

#if PHY_DEBUG
/*
 * ----------------------------------------------------------------------------
 * Function Name: PHY_IndirectIO
 * Purpose: phy indirect register read/write, only for debugging.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T PHY_IndirectIO(U8_T rw, U8_T addr, U8_T index, U16_T XDATA* pbuf)
{
	U8_T	phyAddr = PBDP->PhyInfo.PhyAddr;

	PBDP->PhyInfo.PhyAddr = addr;
	if (rw == 0) /* read */
		phy_ReadReg (index, pbuf);
	else /* write */
		phy_WriteReg (index, *pbuf);

	PBDP->PhyInfo.PhyAddr = phyAddr;
	return	2;

} /* End of PHY_IndirectIO() */
#endif /* PHY_DEBUG */

#endif /* #if (MAC_ARBIT_MODE & MAC_ARBIT_ETH) */

/* End of phy.c */