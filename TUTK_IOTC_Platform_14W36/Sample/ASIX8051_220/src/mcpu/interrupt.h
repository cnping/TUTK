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
 * Module Name : interrupt.h
 * Purpose     : A header file belongs to interrupt module.
 * Author      : Robin Lee
 * Date        :
 * Notes       : None.
 * $Log: interrupt.h,v $
 * no message
 *
 *================================================================================
 */
#ifndef INTERRUPT_H
#define INTERRUPT_H

/* INCLUDE FILE DECLARATIONS */
#include	"types.h"
#include	"interrupt_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
#define		ON		1
#define		OFF		0
#define		EXTINT0(val)	{EX0 = val;}
#define		EXTINT1(val)	{EX1 = val;}
#define		EXTINT2(val)	{EINT2 = val;}
#define		EXTINT3(val)	{EINT3 = val;}
#define		EXTINT4(val)	{EINT4 = val;}
#define		EXTINT5(val)	{EINT5 = val;}
#define		EXTINT6(val)	{EINT6 = val;}

#define		EA_GLOBE_ENABLE		{EA = ON;}
#define		EXTINT0_ENABLE		{EX0 = ON;}
#define		EXTINT1_ENABLE		{EX1 = ON;}
#define		EXTINT2_ENABLE		{EINT2 = ON;}
#define		EXTINT3_ENABLE		{EINT3 = ON;}
#define		EXTINT4_ENABLE		{EINT4 = ON;}
#define		EXTINT5_ENABLE		{EINT5 = ON;}
#define		EXTINT6_ENABLE		{EINT6 = ON;}
#define		EA_GLOBE_DISABLE	{EA = OFF;}
#define		EXTINT0_DISABLE		{EX0 = OFF;}
#define		EXTINT1_DISABLE		{EX1 = OFF;}
#define		EXTINT2_DISABLE		{EINT2 = OFF;}
#define		EXTINT3_DISABLE		{EINT3 = OFF;}
#define		EXTINT4_DISABLE		{EINT4 = OFF;}
#define		EXTINT5_DISABLE		{EINT5 = OFF;}
#define		EXTINT6_DISABLE		{EINT6 = OFF;}


/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void INTR_PmmAutoSet(void);
#if AX_WATCHDOG_INT_ENB
void INTR_SetWatchDogFromWcpu(U8_T mode);
#endif //AX_WATCHDOG_INT_ENB


#endif /* End of INTERRUPT_H */
