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
 * Module Name: mwif.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: mwif.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __MWIF_H__
#define __MWIF_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define IO_MCPU					1
#define IO_WCPU					0
#define IO_CPU_TYPE				(IO_MCPU)

#define MWIF_TASK_HOOKUP        1

#define MAX_MWIF_TX_BUFFER_SIZE		128
#define MAX_MWIF_RX_BUFFER_SIZE		128
#define CCTRL_READ_CMD_READY		BIT0
#define CCTRL_READ_CMD_INTR_MASK	BIT1
#define CCTRL_READ_CMD_POLLING		BIT3
#define CCTRL_WRITE_CMD_READY		BIT4
#define CCTRL_WRITE_CMD_INTR_MASK	BIT5
#define CCTRL_WRITE_CMD_POLLING		BIT7
#define CSR_READ_CMD_INTR			BIT1
#define CSR_WRITE_CMD_INTR			BIT5

#define WCPU_DOCD_RESET				BIT6

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void MWIF_Init(void);
void MWIF_Start(void);
void MWIF_Sys(void);
void MWIF_Intr(void);
void MWIF_Send(U8_T, U8_T*, U8_T);

#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
void MWIF_DocdReset(void);
#endif			/***** End of IO_CPU_TYPE macro *****/

#endif /* End of __MWIF_H__ */

/* End of mwif.h */
