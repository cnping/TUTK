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
 * Module Name : delay.c
 * Purpose     : A time delay function.
 *               Depend on the system clock, firmware calculates some cycle of the
 *               instruction to delay with an time interval.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * $Log: delay.c,v $
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "mcpu.h"
#include "delay.h"


/* NAMING CONSTANT DECLARATIONS */
#define CLK_80M		80
#define CLK_40M		40

/* LOCAL VARIABLES DECLARATIONS */
static U16_T	delay_MsLoop = 1;
static U8_T		delay_SysClk = CLK_40M;
static U8_T		delay_ProgWst = 1;

/*
 * ----------------------------------------------------------------------------
 * Function Name: DELAY_Init
 * Purpose: Get system clock and program wait state to decide delay loop count.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DELAY_Init(void)
{
	/* get system clock */
	switch (MCPU_GetSysClk())
	{
		case SCS_80M :
			delay_SysClk = CLK_80M;
			break;
		case SCS_40M :
			delay_SysClk = CLK_40M;
			break;
		default:
			break;
	}

	/* get program wait state */
	delay_ProgWst = MCPU_GetProgWst() + 1;

	/* calculate driver loop count for per ms */
	delay_MsLoop = ((U32_T)delay_SysClk * 1000) / ((U32_T)delay_ProgWst * 40);

} /* End of DELAY_Init */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DELAY_Us
 * Purpose : Delay XXX us.
 * Params  : loop: A time value of us.
 * Returns :
 * Note    :
 * ----------------------------------------------------------------------------
 */
void DELAY_Us(U16_T loop)
{
	U32_T	count;

	count = ((U32_T)delay_SysClk * loop) / ((U32_T)delay_ProgWst * 20);
	if (count > 7)
		count -= 7;
	else
		return;
		
	while (count--);

} /* End of DELAY_Us */

/*
 *--------------------------------------------------------------------------------
 * Function Name: DELAY_Ms
 * Purpose : Delay XXX ms
 * Params  : loop: A time value of ms
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void DELAY_Ms(U16_T loop)
{
	U16_T	x, y;

	for (y = 0; y < loop; y++)
	{
		x = delay_MsLoop;
		while (x--);
	}
} /* End of ms_delay*/


/* End of delay.c */