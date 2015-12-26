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
 * Module Name : mcpu.c
 * Purpose     : Initialization and globe values setting.
 * Author      : Robin Lee
 * Date        :
 * Notes       : In the initial function, all external interrupt are disable in
 *               default. User must enable a specific interrupt in its module.
 *               Several system values are initial in beginning
 * $Log: mcpu.c,v $
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"ax22000.h"
#include	"types.h"
#include	"mcpu_cfg.h"
#include	"mcpu.h"
#include	"interrupt.h"
#include	"mac.h"
#include	"mpsave.h"

/* GLOBAL VARIABLES DECLARATIONS */
U8_T	MCPU_WcpuReady = 0;
U8_T	MCPU_SleepFlag = 0;
U8_T	MCPU_WakeUpEventTable = 0;
U8_T	MCPU_MacWakeUpEventTable = 0;
U8_T	MCPU_WakeUpByMacFlag = 0;
U8_T	MCPU_PowerSavingMode = 0;
U8_T	MCPU_McpuNotSleepFlag = 0;
U8_T	MCPU_NoCfgEeprom = 0;
U8_T	MCPU_CpuSysClk = 0;
U8_T	MCPU_MemShadow = 0;
U8_T	MCPU_ProgWtst = 0;
U8_T	MCPU_DataStretch = 0;
U8_T	MCPU_GlobeInt = 0;

U8_T IDATA	MCPU_ExecuteRuntimeFlag _at_ 0x30;


/* STATIC VARIABLE DECLARATIONS */
static U8_T data MCPU_ReservedForStackPoint[2] _at_ 0x0d;

/* LOCAL SUBPROGRAM DECLARATIONS */


/* LOCAL SUBPROGRAM BODIES */


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void MCPU_Init(void)
 * Purpose : This function initializes AX220xx CPU Core for the whole system operation.
             AX220xx supports 2 clock rates of 40MHz and 80MHz.
			 The Program Wait State and Data Stretch Cycle are configured via
			 different clock rates and the Memory Shadow Mode as below table,
			 the range of both the Program Wait State and the Data Stretch Cycle are 0 ~7.
			 All AX220xx external interrupts are disabled during AX220xx system initialization.
 * Params  : None
 * Returns : None
 * Note    : None
 *--------------------------------------------------------------------------------
 */
void MCPU_Init(void)
{
	EA = 0; // turn off globe interrupt

	P0 = 0xFF;
	P1 = 0xFF;
	P2 = 0xFF;
	P3 = 0xFF;

	MCPU_NoCfgEeprom = (CSREPR & ICD) ? 1 : 0;

	/* Check program wait-state and data memory wait-state */
	MCPU_MemShadow = (CSREPR & PMS) ? 1 : 0;

	switch (CSREPR & BIT7)
	{
		case SCS_80M :
			MCPU_CpuSysClk = SCS_80M;
			WTST = 0x05;
			CKCON = 0x01;
			break;
		case SCS_40M :
			MCPU_CpuSysClk = SCS_40M;
			WTST = 0x02;
			CKCON = 0x01;
			break;
		default :
			MCPU_CpuSysClk = SCS_40M;
			WTST = 0x02;
			CKCON = 0x01;
			break;
	}

	MCPU_ProgWtst = WTST & 0x07;
	MCPU_DataStretch = CKCON & 0x07;

	PCON	&= ~SWB_;	// Turn off switchback function.

	/* Clear the INT5F & INT6F of Interrupt5 & interrupt6 (write 1 to clear) */
	EIF		= INT5F;	// Clear the flag of interrupt 5.
	EIF		= INT6F;	// Clear the flag of interrupt 6.
	
	if (WTRF)
	{
		TA = 0xAA;
		TA = 0x55;
		WTRF = 0;	// Clear Watch Dog Time Out Reset Flag.
		CSREPR |= SW_RBT;
	}

	/* Disable all external interrupts */
	EXTINT0(OFF);		// EINT0 interrupt.
	EXTINT1(OFF);		// EINT1 interrupt.
	EXTINT2(OFF);		// EINT2 interrupt for commands between MCPU & WCPU
	EXTINT3(OFF);		// EINT3 interrupt for PCA.
	EXTINT4(OFF);		// EINT4 interrupt for peripheral device.
	EXTINT5(OFF);		// EINT5 interrupt for Flash, RTC, software DMA and software Timer.
	EXTINT6(OFF);		// EINT6 interrupt for wake-up of power management mode and stop mode.

	if (MCPU_GlobeInt & BIT7)
		EA = MCPU_GlobeInt & BIT0;
	else
		EA = 1;			// Enable the globe interrupt.

} /* End of MCPU_Init() */

#if AX_WATCHDOG_INT_ENB
/*
 *--------------------------------------------------------------------------------
 * void MCPU_WatchDogSetting(U8_T wdIntrEnb, U8_T wdRstEnb, U8_T wdPrty, U8_T wdTime)
 * Purpose : Configure the Watchdog Timer function of AX220xx MCPU sub-system.
 * Params  : wdIntrEnb : Enable / Disable a Watchdog Timer.
			 wdRstEnb : Enable / Disable a Watchdog Timer counter reset.
			            Enable this will reset the MCPU core when a watchdog timeout occurred.
			 wdPrty : Enable / Disable the Watchdog Priority.
			 wdTime : Set the Watchdog Timer Timeout value.
 * Returns : None
 * Note    : None
 *--------------------------------------------------------------------------------
 */
void MCPU_WatchDogSetting(U8_T wdIntrEnb, U8_T wdRstEnb, U8_T wdPrty, U8_T wdTime)
{
	CKCON |= wdTime;

	if (wdIntrEnb & WD_INTR_ENABLE)
	{
		EWDI = 1;
	}
	else
	{
		EWDI = 0;
	}

	if (wdRstEnb & WD_RESET_ENABLE)
	{
		TA = 0xAA;
		TA = 0x55;
		EWT = 1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		EWT = 0;
	}

	if (wdPrty & WD_HIGH_PRIORITY)
	{
		PWDI = 1;
	}
	else
	{
		PWDI = 0;
	}
	
	TA = 0xAA;
	TA = 0x55;
	RWT = 1;
	WTRF = 0;
	WDIF = 0;
}
#endif //AX_WATCHDOG_INT_ENB


/* End of mcpu.c */
