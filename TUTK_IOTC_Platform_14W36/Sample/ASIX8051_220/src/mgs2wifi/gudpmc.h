/*
 ******************************************************************************
 *     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name:gudpmc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpmc.h,v $
 *
 *=============================================================================
 */

#ifndef __GUDPMC_H__
#define __GUDPMC_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */

/* TYPE DECLARATIONS */
typedef struct _GUDPMC_CONN
{
	U8_T 	State;
	U8_T	UdpSocket;
} GUDPMC_CONN;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void GUDPMC_Task(void);
void GUDPMC_Init(U16_T);
U8_T GUDPMC_NewConn(U32_T XDATA*, U16_T, U8_T);
void GUDPMC_Event(U8_T, U8_T);
void GUDPMC_Receive(U8_T XDATA*, U16_T, U8_T);

#endif /* End of __GUDPMC_H__ */

/* End of gudpmc.h */
