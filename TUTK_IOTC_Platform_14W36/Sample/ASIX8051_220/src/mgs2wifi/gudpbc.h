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
 * Module Name:gudpbc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpbc.h,v $
 *
 *=============================================================================
 */

#ifndef __GUDPBC_H__
#define __GUDPBC_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */

/* TYPE DECLARATIONS */
typedef struct _GUDPBC_CONN
{
	U8_T 	State;
	U8_T	UdpSocket;

} GUDPBC_CONN;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */ 
void GUDPBC_Task(void);
void GUDPBC_Init(U16_T);
U8_T GUDPBC_NewConn(U32_T XDATA*, U16_T, U8_T);
void GUDPBC_Event(U8_T, U8_T);
void GUDPBC_Receive(U8_T XDATA*, U16_T, U8_T);

#endif /* End of __GUDPBC_H__ */

/* End of gudpbc.h */
