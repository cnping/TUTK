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
 * Module Name: mpsave.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: mpsave.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __MPSAVE_H__
#define __MPSAVE_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "ax22000.H"
#include "mcpu_cfg.h"

#if POWER_SAVING

/* NAMING CONSTANT DECLARATIONS */
/* MCISR sfr 0xff */
#define PS_WCPU_CMD_WRITE		BIT2
#define PS_WCPU_CMD_READ		BIT6
#define PS_INTR_STATUS_DEFAULT	(PS_WCPU_CMD_WRITE)

/* MCCR sfr 0xfe */
#define PS_WCPU_CMD_WRITE_MASK	BIT2
#define PS_WCPU_CMD_READ_MASK	BIT6
#define PS_INTR_MASK_DEFAULT	(PS_WCPU_CMD_WRITE_MASK)

/* MPCSR sfr 0xf6 */
#define PS_WCPU_CMD_WRITE_POLL	BIT2
#define PS_WCPU_CMD_READ_POLL	BIT6

/* OCGCR sfr 0x9b */
#define PS_OFF_40M_AT_STOP_MODE			BIT0
#define PS_OFF_40M_AT_FULL_PMM_MODE		BIT1
#define PS_OFF_40M_LBSS					BIT2
#define PS_WIFI_IN_PSAVE_MODE			BIT3
#define PS_OFF_32768_AT_STOP_MODE		BIT4
#define PS_OFF_32768_AT_FULL_PMM_MODE	BIT5
#define PS_WIFI_CLOCK_ENABLE			BIT6
#define PS_32768_CLOCK_PRESENT			BIT7

/* MCPU Software Power Saving Command */
#define PS_DEEP_SLEEP_CMD			0x13
#define PS_DEEP_SLEEP_ACK_CMD		0x93
#define PS_EXIT_DEEP_SLEEP_CMD		0x14
#define PS_EXIT_DEEP_SLEEP_ACK_CMD	0x94

#define PS_SET_MODE_NORMAL			0x20
#define PS_SET_MODE_TYPICAL			0x21
#define PS_SET_MODE_FAST			0x22
#define PS_SET_MODE_MAXIMUM			0x23

#define PS_SET_STOP_MODE			0x30
#define PS_SET_PMM_MODE				0x31

/* for debug */
#define PS_DBG_DTIM_0				0xE0
#define PS_DBG_DTIM_1				0xE1
#define PS_DBG_DTIM_2				0xE2
#define PS_DBG_DTIM_3				0xE3
#define PS_DBG_DTIM_4				0xE4
#define PS_DBG_DTIM_5				0xE5
#define PS_DBG_DTIM_6				0xE6
#define PS_DBG_DTIM_7				0xE7
#define PS_DBG_DTIM_8				0xE8
#define PS_DBG_DTIM_9				0xE9
#define PS_DBG_DTIM_10				0xEA
#define PS_DBG_DTIM_11				0xEB
#define PS_DBG_DTIM_12				0xEC
#define PS_DBG_DTIM_13				0xED
#define PS_DBG_DTIM_14				0xEE
#define PS_DBG_DTIM_15				0xEF
/* for debug */

#define PS_UNKNOW_CMD				0xff

/* MCPU Software Power Saving State */
#define PS_STATE_DISABLE					0x00
#define PS_STATE_WAIT_TO_NEXT_DEEP_SLEEP	0xAE
#define PS_STATE_WAIT_TO_DEEP_SLEEP			0xAF
#define PS_STATE_DEEP_SLEEP_REQ				0xB0
#define PS_STATE_WAIT_DEEP_SLEEP_ACK		0xB1
#define PS_STATE_DEEP_SLEEP_READY			0xB2
#define PS_STATE_IN_DEEP_SLEEP				0xB3
#define PS_STATE_EXIT_DEEP_SLEEP_REQ		0xB7
#define PS_STATE_WAIT_EXIT_DEEP_SLEEP_ACK	0xB8


/* GLOBAL VARIABLES */
extern U8_T PS_State;
extern U8_T PS_WaitToDisableFlag;
extern U8_T PS_StopDeepSleepTest;
extern U16_T PS_DeepSleepTestCounter;
extern U8_T PS_PingCounter;
extern U8_T PS_Mode;
extern U16_T PS_SleepTime;
extern U16_T PS_SleepTimeOut;

extern U8_T PS_DbgState;
extern U8_T PS_DbgValue;
extern U8_T PS_DbgErrorValue;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void PS_Init(void);
void PS_Start(void);
void PS_Intr(U8_T);
void PS_NoticeWcpu(U8_T);
void PS_CheckState(void);

#endif /* End of #if POWER_SAVING*/


#endif /* End of __MPSAVE_H__ */

/* End of mpsave.h */
