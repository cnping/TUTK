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
 * Module Name : mstimer.c
 * Purpose     : AX220xx provides a ms timer. This firmware can initial the ms-timer
 *               and get the time tick by applications.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * $Log: mstimer.c,v $
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "types.h"
#include "interrupt.h"
#include "mstimer.h"
#include "dma.h"

/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */
U32_T SWTIMER_Counter = 0;

/* LOCAL VARIABLES DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SWTIMER_Init
 * Purpose: Initiate global values in the MS timer module.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void SWTIMER_Init(void)
{

} /* End of SWTIMER_Init */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SWTIMER_Start
 * Purpose: Enable the interrupt and start software timer.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void SWTIMER_Start(void)
{
	U8_T XDATA	temp[2];

	/* set software timer */
	temp[1] = (U8_T)SWTIMER_INTERVAL;
	temp[0] = (U8_T)(SWTIMER_INTERVAL >> 8) + SWTIMER_START + SWTIMER_INT_ENABLE;
	DMA_WriteReg(SWTIMER_REG, temp, 2);

	EXTINT5_ENABLE;	/* Enable INT5 interrupt for software timer. */

} /* End of SWTIMER_Start */

#if (SWTIMER_STOP_FUNC == SWTIMER_SUPPORT_STOP_FUNC)
/*
 * ----------------------------------------------------------------------------
 * Function Name: SWTIMER_Stop
 * Purpose: Disable the MS Timer.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void SWTIMER_Stop(void)
{
	U8_T XDATA	temp[2];

	/* set software timer */
	temp[1] = (U8_T)SWTIMER_INTERVAL;
	temp[0] = (U8_T)(SWTIMER_INTERVAL >> 8);
	DMA_WriteReg(SWTIMER_REG, temp, 2);

} /* End of SWTIMER_Stop */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: SWTIMER_Tick
 * Purpose: Get the ms-timer current time tick value.
 * Params : None.
 * Returns: timer: A 32bits current tick value.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U32_T SWTIMER_Tick(void)
{
	U32_T	timer;
	BIT		oldEintBit = EA;

	EA_GLOBE_DISABLE;
	timer = SWTIMER_Counter;
	EA = oldEintBit;
	return timer;

} /* End of SWTIMER_Tick */


/* End of mstmier.c */