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
 * Module Name:delay.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: delay.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __DELAY_H__
#define __DELAY_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void DELAY_Init(void);
void DELAY_Ms(U16_T);
void DELAY_Us(U16_T);


#endif /* End of __DELAY_H__ */


/* End of timer.h */