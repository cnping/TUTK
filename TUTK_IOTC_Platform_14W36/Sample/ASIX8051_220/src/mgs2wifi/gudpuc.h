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
 * Module Name:gudpuc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpuc.h,v $
 *
 *=============================================================================
 */

#ifndef __GUDPUC_H__
#define __GUDPUC_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */

/* TYPE DECLARATIONS */
typedef struct _GUDPUC_CONN
{
	U8_T 	State;
	U8_T	UdpSocket;

} GUDPUC_CONN;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */ 
void GUDPUC_Task(void);
void GUDPUC_Init(U16_T);
U8_T GUDPUC_NewConn(U32_T XDATA*, U16_T, U8_T);
void GUDPUC_Event(U8_T, U8_T);
void GUDPUC_Receive(U8_T XDATA*, U16_T, U8_T);

#endif /* End of __GUDPUC_H__ */

/* End of gudpuc.h */
