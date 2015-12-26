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
 * Module Name:phy.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: phy.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __PHY_H__
#define __PHY_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define PHY_DEBUG					1

/* internal phy id address */
#define PHY_ADDRESS					0x15

/* network type */
#define AUTO_NEGOTIATION			0
#define FIXED_100_FULL				1
#define FIXED_100_HALF				2
#define FIXED_10_FULL				3
#define FIXED_10_HALF				4

/* NAMING CONSTANT DECLARATIONS */
/* SFR 0xBE definitions*/
#define PHY_READ_REG				BIT7
#define PHY_ACCESS_GO				BIT6
#define PHY_CMD_ABORT				0xFF

/* MII register definitions */
#define MII_PHY_CONTROL				0x00	// control reg
#define MII_PHY_STATUS				0x01	// status reg
#define MII_PHY_OUI					0x02	// most of the OUI bits
#define MII_PHY_MODEL				0x03	// model/rev bits, and rest of OUI
#define MII_PHY_ANAR				0x04	// AN advertisement reg
#define MII_PHY_ANLPAR				0x05	// AN Link Partner
#define MII_PHY_ANER				0x06	// AN expansion reg

/* Bit definitions: MII Control */
#define CONTROL_RESET				0x8000	// reset bit in control reg
#define CONTROL_LOOPBACK			0x4000	// loopback bit in control reg
#define CONTROL_10MB				0x0000	// 10 Mbit
#define CONTROL_100MB				0x2000	// 100Mbit
#define CONTROL_ENABLE_AUTO			0x1000	// autonegotiate enable
#define CONTROL_POWER_DOWN			0x0800
#define CONTROL_ISOLATE				0x0400	// islolate bit
#define CONTROL_START_AUTO			0x0200	// restart autonegotiate
#define CONTROL_FULL_DUPLEX			0x0100

/* Bit definitions: Auto-Negotiation Advertisement */
#define ANAR_PAUSE					0x0400	// support pause packets
#define ANAR_100T4					0x0200	// support 100BT4
#define ANAR_100TXFD				0x0100	// support 100BTX full duplex
#define ANAR_100TX					0x0080	// support 100BTX half duplex
#define ANAR_10TFD					0x0040	// support 10BT full duplex
#define ANAR_10T					0x0020	// support 10BT half duplex
#define ANAR_SELECTOR_FIELD			0x001F	// selector field.
#define ANAR_SELECTOR_8023			0x0001

/* Bit definitions: Auto-Negotiation Link Partner Ability */
#define ANLPAR_PAUSE				0x0400	// support pause packets
#define ANLPAR_100T4				0x0200	// support 100BT4
#define ANLPAR_100TXFD				0x0100	// support 100BTX full duplex
#define ANLPAR_100TX				0x0080	// support 100BTX half duplex
#define ANLPAR_10TFD				0x0040	// support 10BT full duplex
#define ANLPAR_10T					0x0020	// support 10BT half duplex

/* TYPE DECLARATIONS */
//---------------------------------------------------------------
typedef struct _PHY_INFORMATION
{
	U8_T	PhyAddr;

} PHY_INFO;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
//---------------------------------------------------------------
void PHY_Init(U8_T);
U16_T PHY_CheckMediaType(void);

#if PHY_DEBUG
U8_T PHY_IndirectIO(U8_T, U8_T, U8_T, U16_T XDATA*);
#endif

#endif /* End of __PHY_H__ */


/* End of phy.h */