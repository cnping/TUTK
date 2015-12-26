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
 * Module Name:mstimer_cfg.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: mstimer_cfg.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __MSTIMER_CFG_H__
#define __MSTIMER_CFG_H__


/*-------------------------------------------------------------*/
#define SWTIMER_INTERVAL				50	/* sw-timer timeout per 50 ms */

/*-------------------------------------------------------------*/
/* NAMING CONSTANT DECLARATIONS */
#define SWTIMER_NOT_SUPPORT_STOP_FUNC	0	/* driver does not support sw-timer
												stop function. */
#define SWTIMER_SUPPORT_STOP_FUNC		1	/* driver supports stop function. */

#define SWTIMER_STOP_FUNC		(SWTIMER_NOT_SUPPORT_STOP_FUNC)
/*-------------------------------------------------------------*/


#endif /* End of __MSTIMER_CFG_H__ */


/* End of mstimer_cfg.h */