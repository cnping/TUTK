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
 * Module Name: mpsave.c
 * Purpose: MCPU Power Saving module
 * Author:
 * Date:
 * Notes:
 * $Log: mpsave.c,v $
 * no message
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "mpsave.h"
#include "ax22000.h"
#include "mwif.h"
#include "uart0.h"
#include "mcpu.h"
#include "stoe.h"
#include "delay.h"
#include "mstimer.h"
#include "interrupt.h"
#include "ping.h"
#include "printd.h"

#if (POWER_SAVING && (MAC_ARBIT_MODE & MAC_ARBIT_WIFI))

/* STATIC VARIABLE DECLARATIONS */

/* GLOBAL VARIABLE DECLARATIONS */
U8_T PS_State;
U8_T PS_WaitToDisableFlag = 0;
U8_T PS_StopDeepSleepTest = 0;
U16_T PS_DeepSleepTestCounter = 0;
U8_T PS_PingCounter = 0xFF;
U8_T PS_Mode = 0; /* 0:disable, 1:fast 2:maximum */
U16_T PS_SleepTime = 100; /* 100ms, for turn off 32K clock in normal/pmm mode */
U16_T PS_SleepTimeOut = 0;


/* STATIC VARIABLE DECLARATIONS */
U16_T ps_WaitDeepSleepTimer;

/* LOCAL SUBPROGRAM DECLARATIONS */


/* GLOBAL SUBPROGRAM BODIES */
/*
 * ----------------------------------------------------------------------------
 * Function Name: PS_Init
 * Purpose: To initiate MCPU Power Saving module
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void PS_Init(void)
{
	PS_State = PS_STATE_DISABLE;

	OCGCR = PS_OFF_40M_AT_STOP_MODE;

} /* End of PS_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: PS_Start
 * Purpose:
 * Params:
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void PS_Start(void)
{
	CCTRL |= PS_INTR_MASK_DEFAULT;
	WKUPSR = BIT2;

	EXTINT6(ON);

}	/* End of PS_Start() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: PS_Intr
 * Purpose:
 * Params:
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void PS_Intr(U8_T flag)
{
	U8_T	tmp;

	if (flag == PS_WCPU_CMD_WRITE)
	{
		/* check the data register */
		tmp = MPCDR;
		switch (tmp)
		{
		case PS_DEEP_SLEEP_ACK_CMD:
			PS_State = PS_STATE_DEEP_SLEEP_READY;
			break;
		case PS_EXIT_DEEP_SLEEP_ACK_CMD:
			if (PS_StopDeepSleepTest == 1)
			{
				PS_State = PS_STATE_DISABLE;
				PS_StopDeepSleepTest = 0;
				OCGCR = 0x81;
			}
			else
			{
				PS_State = PS_STATE_WAIT_TO_NEXT_DEEP_SLEEP;
				ps_WaitDeepSleepTimer = (U16_T)SWTIMER_Tick();
				PS_DeepSleepTestCounter++;
			}
			break;
		case PS_UNKNOW_CMD:
			break;
		default:
			break;
		}
	}

}	/* End of PS_Intr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: PS_NoticeWcpu
 * Purpose:
 * Params:
 * Returns: 0: success
 *			1: fail
 * Note: none
 * ----------------------------------------------------------------------------
 */
void PS_NoticeWcpu(U8_T cmd)
{
	U8_T	tmp = 0;

	MPCDR = cmd;

	while (!(tmp & PS_WCPU_CMD_READ_POLL))
		tmp = MPCCR;

	return;

} /* End of PS_NoticeWcpu() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: PS_CheckState
 * Purpose:
 * Params:
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void PS_CheckState(void)
{
	U16_T	tmptimer;
	U8_T	printFlag = 0;
	U8_T	isr;

	isr = EA;
	EA = 0;
	switch (PS_State)
	{
	case PS_STATE_WAIT_TO_NEXT_DEEP_SLEEP:
		if (PS_PingCounter != 0xFF)
			PS_PingCounter = 0;
		PS_State = PS_STATE_WAIT_TO_DEEP_SLEEP;
		EA = isr;
		printd ("Had completed %d timers deep sleep test.\n\r", PS_DeepSleepTestCounter);
		break;
	case PS_STATE_WAIT_TO_DEEP_SLEEP:
		tmptimer = (U16_T)SWTIMER_Tick();
		if (!(STOE_ConnectState & STOE_WIRELESS_LINK))
		{
			EA = isr;
			break;
		}
		if ((tmptimer - ps_WaitDeepSleepTimer) >= 300) /* 15 second */
		{
			MAC_WakeupEnable(MAC_EXTERNAL_PIN_WAKEUP);
			EXTINT6(ON);
			if ((PS_PingCounter != 0xFF) && (PS_PingCounter == 0))
			{
				PS_State = 0xFF; /* for debug */
				EA = isr;
				PingFlag = 0; /*stop ping*/
				printd ("Do not receive ping response packet.\n\r");
				break;
			}
			PS_State = PS_STATE_DEEP_SLEEP_REQ;
			PS_NoticeWcpu(PS_DEEP_SLEEP_CMD);
			PS_State = PS_STATE_WAIT_DEEP_SLEEP_ACK;
		}

		EA = isr;
		break;
	case PS_STATE_DEEP_SLEEP_READY:
		PS_State = PS_STATE_IN_DEEP_SLEEP;
		EA = isr;
		MAC_PowerSavingSleep();
		break;
	case PS_STATE_EXIT_DEEP_SLEEP_REQ:
		PS_State = PS_STATE_WAIT_EXIT_DEEP_SLEEP_ACK;
		PS_NoticeWcpu(PS_EXIT_DEEP_SLEEP_CMD);
		EA = isr;
		break;
	default:
		EA = isr;
		break;		
	}

} /* End of PS_CheckState() */

#endif /* End of #if (POWER_SAVING && (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)) */

/* End of mpsave.c */