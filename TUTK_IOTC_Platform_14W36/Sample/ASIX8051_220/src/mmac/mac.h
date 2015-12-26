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
 * Module Name: mac.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: mac.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __MAC_H__
#define __MAC_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "stoe_cfg.h"
#include "mcpu_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
#define MAC_DEBUG						1

#define MAC_WIFI_MAC_ADDR				0
#define MAC_ETHERNET_MAC_ADDR			1

#define MAC_RCV_PROMISCUOUS				1
#define MAC_RCV_ALL_MULTICAST			2
#define MAC_RCV_BROADCAST				4
#define MAC_RCV_MULTICAST				8

#define MAC_PRIMARY_PHY_LINK_WAKEUP		1		 
#define MAC_MAGIC_PACKET_WAKEUP			2
#define MAC_EXTERNAL_PIN_WAKEUP			4
#define MAC_MS_FRAME_WAKEUP				8


/* MAC register definitions */
#define MAC_SRAM_CMD_REG				0x00
#define MAC_SRAM_DATA_REG				0x02
#define MAC_RX_CTL_REG					0x0A
#define MAC_IPG_CTL_REG					0x0C
#define MAC_ADDR_REG					0x10
#define MAC_MULTICASE_REG				0x16
#define MAC_TEST_REG					0x1E
#define MAC_MEDIUM_STATUS_MODE_REG		0x20
#define	MAC_ETH_INTERFACE_REG			0x22
#define MAC_WAKEUP_INT_ENABLE_REG		0x24
#define MAC_LINK_CHANGE_INT_ENABLE_REG	0x26
#define MAC_WAKEUP_LINK_INT_STATUS_REG	0x28
#define MAC_WAKEUP_FRAME_COMMAND		0x30
#define MAC_WAKEUP_FRAME_MASK0			0x32
#define MAC_WAKEUP_FRAME_CRC0			0x36
#define MAC_WAKEUP_FRAME_OFFSET0		0x38
#define MAC_WAKEUP_FRAME_LAST_BYTE0		0x3a
#define MAC_WAKEUP_FRAME_MASK1			0x40
#define MAC_WAKEUP_FRAME_CRC1			0x44
#define MAC_WAKEUP_FRAME_OFFSET1		0x46
#define MAC_WAKEUP_FRAME_LAST_BYTE1		0x48

#define	MAC_ARB_CTL_REG					0x49
#define	MAC_IN_PHY_CTL_STATUS_REG		0x50
#define	MAC_IN_PHY_MSG_REG				0x51
#define	MAC_IN_PHY_INT_ENABLE_REG		0x53
#define	MAC_IN_PHY_INT_STATUS_REG		0x54
#define MAC_PACKET_JAM_LIMIT_REG		0x55
#define MAC_CMD_ABORT					0xFF

/* Bit definitions: MAC_RX_CTL_REG */
#define PACKET_TYPE_PROMISCOUS			BIT0
#define PACKET_TYPE_ALL_MULTI			BIT1
#define PACKET_TYPE_RCV_CRC_ERROR		BIT2
#define PACKET_TYPE_BROADCAST			BIT3
#define PACKET_TYPE_MULTICAST			BIT4
#define PACKET_TYPE_UNICAST_MATCH		BIT5
#define START_OPERATION					BIT7

/* Bit definitions: MAC_MEDIUM_STATUS_MODE_REG */
#define MEDIUM_SUPER_MAC				BIT0
#define MEDIUM_STOP_BACKPRESSURE		BIT1
#define MEDIUM_ENABLE_RECEIVE			BIT2
#define MEDIUM_CHECK_ONLY_TYPE_PAUSE	BIT3
#define MEDIUM_ENABLE_TX_FLOWCTRL		BIT4
#define MEDIUM_FULL_DUPLEX_MODE			BIT5
#define MEDIUM_MII_100M_MODE			BIT6
#define MEDIUM_ENABLE_RX_FLOWCTRL		BIT7

/* Bit definitions: MAC_ETH_INTERFACE_REG */
#define EMIR_OPERATE_IN_PHY_MODE		BIT0
#define EMIR_RMII_OR_REVRMII			BIT1
#define EMIR_CSR_DV_ENABLE				BIT2

/* Bit definitions: MAC_WAKEUP_INT_ENABLE_REG */
#define WAKEUP_BY_PRIMARY_LINK_UP		BIT0
#define WAKEUP_BY_SECONDARY_LINK_UP		BIT2
#define WAKEUP_BY_MAGIC_PACKET			BIT4
#define WAKEUP_BY_EXTER_PIN_TRIG		BIT5
#define WAKEUP_BY_MICROSOFT_FRAME		BIT6
#define WAKEUP_BY_REV_MII				BIT7

/* Bit definitions: MAC_LINK_CHANGE_INT_ENABLE_REG */
#define PRIMARY_LINK_CHANGE_ENABLE		BIT0
#define SECOND_LINK_CHANGE_ENABLE		BIT2

/* Bit definitions: MAC_WAKEUP_LINK_INT_STATUS_REG */
#define STATUS_PRIMARY_LINK_CHANGE		BIT0
#define STATUS_PRIMARY_IS_LINK_UP		BIT1
#define STATUS_SECOND_LINK_CHANGE		BIT2
#define STATUS_SECOND_IS_LINK_UP		BIT3
#define STATUS_RCV_MAGIC_PKT			BIT4
#define STATUS_TRIG_EXTER_PIN_TRIG		BIT5
#define STATUS_MICROSOFT_FRAME			BIT6
#define STATUS_TRIG_BY_REV_MII			BIT7

/* Bit definitions: WUF_WAKEUP_FRAME_COMMAND, 0x30 */
#define WAKEUP_FRAME_FILTER0_ENABLE		BIT0
#define WAKEUP_FRAME_FILTER0_UNI_ENABLE	BIT1
#define WAKEUP_FRAME_FILTER1_ENABLE		BIT2
#define WAKEUP_FRAME_FILTER1_UNI_ENABLE	BIT3
#define WAKEUP_FRAME_CASCADE_ENABLE		BIT4

/* wakeup frame Mode */
#define WUF_ONLY_USE_FILTER0			0
#define WUF_ONLY_USE_FILTER1			1
#define WUF_ONLY_FILTER0_MACADDR		2
#define WUF_ONLY_FILTER1_MACADDR		3
#define WUF_BOTH_FILTERS				4
#define WUF_BOTH_FILTERS_MACADDR		6
#define WUF_CASCADE						8
#define WUF_CASCADE_MACADDR				10

/* Bit definitions: MAC_ARB_CTL_REG, 0x49 */
#define ARBIT_BOTH						(BIT0 | BIT4)
#define ARBIT_MII_ONLY					0
#define ARBIT_ETH_ONLY					(BIT1 | BIT5)
#define ARBIT_MII_PRIO_HIGH				BIT2

/* Bit definitions: MAC_ARB_CTL_REG */
#define	TRACR_RCV_AUTO_FORWARD_ENABLE	BIT0
#define	TRACR_RCV_FORWARD_SEL			BIT1
#define	TRACR_RCV_PRIORITY				BIT2
#define	TRACR_XMT_AUTO_FORWARD_ENABLE	BIT4
#define	TRACR_XMT_FORWARD_SEL			BIT5

/* Bit definitions: MAC_IN_PHY_CTL_STATUS_REG */
#define	EMPCSR_PHY_LINK_UP_STATUS		BIT0
#define	EMPCSR_PHY_ISOLATE				BIT1
#define	EMPCSR_PHY_LOOPBACK				BIT2
#define	EMPCSR_PHY_POWER_DOWN			BIT3

/* Bit definitions: MAC_IN_PHY_INT_ENABLE_REG */
#define	EMPIER_ISOLATE_CHANGE_INT_ENABLE		BIT1
#define	EMPIER_LOOPBACK_CHANGE_INT_ENABLE		BIT2
#define	EMPIER_POWERDOWN_CHANGE_INT_ENABLE		BIT3
#define	EMPIER_WRITE_PMMR1_INT_ENABLE			BIT6
#define	EMPIER_READ_PMMR0_INT_ENABLE			BIT7

/* Bit definitions: MAC_IN_PHY_INT_STATUS_REG */
#define	EMPISR_ISOLATE_CHANGE_STATUS			BIT1
#define	EMPISR_LOOPBACK_CHANGE_STATUS			BIT2
#define	EMPISR_POWERDOWN_CHANGE_STATUS			BIT3
#define	EMPISR_WRITE_PMMR1_STATUS				BIT6
#define	EMPISR_READ_PMMR0_STATUS				BIT7


/* TYPE DECLARATIONS */
/*-------------------------------------------------------------*/
typedef struct _MAC_INFORMATION
{
/* definition for NetworkType member. */
#define	MAC_AUTO_NEGOTIATION			0
#define MAC_LINK_100M_SPEED				100
#define MAC_LINK_10M_SPEED				10
#define MAC_LINK_FULL_DUPLEX			0x80

	U8_T	LinkSpeed;
	U8_T	FullDuplex;
	U8_T	MediumLinkType;	/* map to mac register MAC_MEDIUM_STATUS_MODE_REG */
	U8_T	NetworkType;
	U8_T	InterruptMask;

} MAC_INFO;

/*-------------------------------------------------------------*/
typedef struct _MAC_WAKEUP_FRAME
{
	U8_T	Mode;	/* see above wakeup frame Mode */
	U8_T	Offset0;/* filter0 offset */
	U8_T	Offset1;/* filter1 offset */
	U32_T	Mask0;	/* filter0 mask */
	U32_T	Mask1;	/* filter1 mask */
	U8_T*	Pframe;	/* point to the first byte of wakeup frame (destination address) */
	U8_T	Length;

} MAC_WUF;

typedef struct _MAC_DEFAULT_AREA
{
	U8_T	HwDefault[8192]; /* 8K bytes for software default */

} MAC_DEFAULT_AREA;

/* GLOBAL VARIABLES */
extern MAC_DEFAULT_AREA code* MAC_Default;
extern U8_T XDATA MAC_InterruptStatus;
extern U8_T XDATA MAC_WifiMacAddr[6];
extern U8_T XDATA MAC_WifiIp[4];
extern U8_T XDATA MAC_WifiSubMask[4];

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/*-------------------------------------------------------------*/
void MAC_Init(U8_T);
void MAC_Start(void);
void MAC_ValidateSubsystemId(void);

U8_T* MAC_GetMacAddr(void);
void MAC_SetRxFilter(U8_T);

#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
  #if (MAC_GET_INTSTATUS_MODE == MAC_INTERRUPT_MODE)
  void MAC_SetInterruptFlag(void);
  U8_T MAC_GetInterruptFlag(void);
  void MAC_ProcessInterrupt(void);
  #else
  U8_T MAC_LinkSpeedChk(void);
  #endif

  #if (POWER_SAVING || (MAC_REMOTE_WAKEUP == MAC_SUPPORT))
  #define __MAC_WAKEUP_ENABLE__
  void MAC_WakeupEnable(U8_T);
  #endif

  #if (MAC_REMOTE_WAKEUP == MAC_SUPPORT)
  U8_T MAC_SetWakeupFrame(MAC_WUF*);
  void MAC_SystemSleep(void);
  #endif
#endif

#if (POWER_SAVING)
  #ifndef __MAC_WAKEUP_ENABLE__
  void MAC_WakeupEnable(U8_T);
  #endif
  void MAC_PowerSavingSleep(void);
#endif

#if (MAC_MULTICAST_FILTER == MAC_SUPPORT)
  void MAC_MultiFilter(U8_T*, U8_T);
#endif

#if MAC_DEBUG
U8_T MAC_IndirectIO(U8_T, U16_T, U8_T XDATA*);
#endif

void mac_ReadReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length);
void mac_WriteReg(U8_T regaddr, U8_T XDATA* pbuf, U8_T length);

#endif /* __MAC_H__ */


/* End of mac.h */