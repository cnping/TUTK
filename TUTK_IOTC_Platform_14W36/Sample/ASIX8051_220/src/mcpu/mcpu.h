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
 * Module Name : mcpu.h
 * Purpose     : A header file belongs to mcpu module.
 *               It define all globe parameters in mcpu module.
 * Author      : Robin Lee
 * Date        :
 * Notes       : None.
 * $Log: mcpu.h,v $
 * no message
 *
 *================================================================================
 */
#ifndef MCPU_H
#define MCPU_H

/* INCLUDE FILE DECLARATIONS */
#include "mcpu_cfg.h"
#include "interrupt_cfg.h"
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define		PROG_WTST_0			0
#define		PROG_WTST_1			1
#define		PROG_WTST_2			2
#define		PROG_WTST_3			3
#define		PROG_WTST_4			4
#define		PROG_WTST_5			5
#define		PROG_WTST_6			6
#define		PROG_WTST_7			7
#define		DATA_STRETCH_0		0
#define		DATA_STRETCH_1		1
#define		DATA_STRETCH_2		2
#define		DATA_STRETCH_3		3
#define		DATA_STRETCH_4		4
#define		DATA_STRETCH_5		5
#define		DATA_STRETCH_6		6
#define		DATA_STRETCH_7		7

#define		SHADOW_MEM_ENB		0x10

#define		WD_INTR_ENABLE		1
#define		WD_INTR_DISABLE		0
#define		WD_RESET_ENABLE		1
#define		WD_RESET_DISABLE	0
#define		WD_HIGH_PRIORITY	1
#define		WD_LOW_PRIORITY		0
#define		WD_INTERVAL_131K	0x00
#define		WD_INTERVAL_1M		WD0_
#define		WD_INTERVAL_8M		WD1_
#define		WD_INTERVAL_67M		WD1_|WD0_

/* sfr OCGCR (0x9B) */
#define MCPU_TURN_OFF_40M_STOP_MODE		BIT0
#define MCPU_TURN_OFF_WIFI_40M			BIT1
#define MCPU_TURN_OFF_LBSS_40M			BIT2
#define MCPU_WIFI_IN_PS_MODE			BIT3	/* read only */
#define	MCPU_TURN_OFF_32K_STOP_MODE		BIT4
#define MCPU_TURN_OFF_32K_FULL_PMM_MODE	BIT5
#define MCPU_WIFI_CLOCK_ENABLE			BIT6	/* read only */
#define MCPU_32K_CLOCK_USED				BIT7	/* read only */

/* sfr WakeUp Flag */
#define MCPU_WAKEUP_BY_ETHERNET			BIT0
#define MCPU_WAKEUP_BY_REVMII			BIT1
#define MCPU_WAKEUP_BY_WIFI_PS_CHANGED	BIT2
#define MCPU_WAKEUP_BY_RTC				BIT3
#define MCPU_WAKEUP_BY_UART2			BIT4
#define MCPU_WAKEUP_BY_UART3			BIT5
#define MCPU_WAKEUP_BY_SPI				BIT6
#define MCPU_WAKEUP_BY_LBS				BIT7

/* Power Saving type, for parameter MCPU_PowerSavingMode */
#define MCPU_SYSTEM_PSMODE_STOP			0
#define MCPU_SYSTEM_PSMODE_PMM			1


/* MACRO DECLARATIONS */
/* if flag > 1,  when wcpu enter sleep mode, mcpu does not enter power saving mode */
#define MCPU_SetMcpuDoNotSleepFlag(flag)	{	\
	MCPU_McpuNotPSbyWiFiFlag = flag;			\
	if (MCPU_McpuNotPSbyWiFiFlag)				\
		WKUPSR = 0;								\
	else										\
		WKUPSR = BIT2;						}
/* wakeup events that want to be set to parameter MCPU_WakeUpEventTable */
#define MCPU_SetWakeUpEventTable(flag)		(MCPU_WakeUpEventTable |= flag)
/* wakeup events that want to be clear from parameter MCPU_WakeUpEventTable */
#define MCPU_ClrWakeUpEventTable(flag)		(MCPU_WakeUpEventTable &= ~flag)
/* mac wakeup events that want to be set to parameter MCPU_MacWakeUpEventTable */
#define MCPU_SetMacWakeUpEventTable(flag)	(MCPU_MacWakeUpEventTable = flag)
/* mac wakeup events that want to be clear from parameter MCPU_MacWakeUpEventTable */
#define MCPU_ClrMacWakeUpEventTable(flag)	(MCPU_MacWakeUpEventTable &= ~flag)
/* system be wakeup by MAC wakeup event, call this macro to notice mcpu module */
#define MCPU_SetWakeUpByMacFlag(flag)		(MCPU_WakeUpByMacFlag = flag)
/* Set the system power management mode to STOP or PMM or PMM+SWB */
#define MCPU_SetPowerSaveMode(mode)			(MCPU_PowerSavingMode = mode & (STOP_ | PMM_ | SWB_))
/* to check the power saving is PMM mode or not */
#define MCPU_GetPowerSavingMode()			(MCPU_PowerSavingMode)
/* to check the eeprom is inexistent or not */
#define MCPU_NoCfgEeprom()					(MCPU_NoCfgEeprom)
/* to get system clock */
#define MCPU_GetSysClk()					(MCPU_CpuSysClk)
/* to get system memory is shadow mode or not */
#define MCPU_GetMemSdw()					(MCPU_MemShadow)
/* to get the program wait state */
#define MCPU_GetProgWst()					(MCPU_ProgWtst)
/* to get the data wait state */
#define MCPU_GetDataSth()					(MCPU_DataStretch)
/* software reset to reset CPU core */
#define MCPU_SoftReset()					{	\
	if (EA)										\
		MCPU_GlobeInt = BIT0 | BIT7;			\
	else										\
		MCPU_GlobeInt = BIT7;					\
	EA_GLOBE_DISABLE;							\
	CSREPR |= SW_RST;						}
/* software reboot to restart hardware core of all */
#define MCPU_SoftReboot()					(CSREPR |= SW_RBT)

/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */
extern U8_T MCPU_WcpuReady;
extern U8_T MCPU_SleepFlag;
extern U8_T MCPU_WakeUpEventTable;
extern U8_T MCPU_MacWakeUpEventTable;
extern U8_T	MCPU_WakeUpByMacFlag;
extern U8_T	MCPU_PowerSavingMode;
extern U8_T MCPU_McpuNotSleepFlag;
extern U8_T MCPU_NoCfgEeprom;
extern U8_T MCPU_CpuSysClk;
extern U8_T MCPU_MemShadow;
extern U8_T MCPU_ProgWtst;
extern U8_T MCPU_GlobeInt;

extern U8_T IDATA MCPU_ExecuteRuntimeFlag;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	MCPU_Init(void);
#if AX_WATCHDOG_INT_ENB
void	MCPU_WatchDogSetting(U8_T wdIntrEnb, U8_T wdRstEnb, U8_T wdPrty, U8_T wdTime);
#endif


#endif /* End of MCPU_H */
