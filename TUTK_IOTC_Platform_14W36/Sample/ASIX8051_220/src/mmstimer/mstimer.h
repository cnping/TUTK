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
 * Module Name:mstimer.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: mstimer.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __MSTIMER_H__
#define __MSTIMER_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "mstimer_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
#define SWTIMER_COUNT_SECOND	(1000 / SWTIMER_INTERVAL) /* timer expired count
														per second*/

/* SFR 0x94 definitions*/
#define SWTIMER_EXPIRED			BIT1

/* TIMER register definitions */
#define SWTIMER_REG				0x0C

/* Bit definitions: SWTIMER_REG */
#define SWTIMER_START			BIT4
#define SWTIMER_RESET			BIT5
#define SWTIMER_INT_ENABLE		BIT7

/* GLOBAL VARIABLES */
extern U32_T SWTIMER_Counter;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
//---------------------------------------------------------------
void SWTIMER_Init(void);
void SWTIMER_Start(void);
U32_T SWTIMER_Tick(void);
#if (SWTIMER_STOP_FUNC == SWTIMER_SUPPORT_STOP_FUNC)
void SWTIMER_Stop(void);
#endif

				  
#endif /* End of __MSTIMER_H__ */


/* End of mstimer.h */