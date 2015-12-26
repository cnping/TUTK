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
 * Module Name:gevent.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gevent.h,v $
 *
 *=============================================================================
 */

#ifndef __GEVENT_H__
#define __GEVENT_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "main.h"

#if (INCLUDE_EVENT_DETECT)
/* NAMING CONSTANT DECLARATIONS */
#define GEVENT_ENABLE_DEBUG_MSG	0
/* TYPE DECLARATIONS */
typedef struct _GEVENT_FLAGS
{
	U16_T AutoWarning;

	U8_T PSWtemp[2][16];
	U8_T USNtemp[2][16];
	U8_T LVLtemp[2];	

	U32_T IPtemp[2];	
	
	U8_T WaitTimer;
} GEVENT_FLAGS;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void GEVENT_Task(void);
void GEVENT_Init(void);

void GEVENT_SetColdStartEvent(U8_T);
U8_T GEVENT_GetColdStartEvent(void);

void GEVENT_SetAuthFailEvent(U8_T);
U8_T GEVENT_GetAuthFailEvent(void);

void GEVENT_SetIPchgEvent(U8_T);
U8_T GEVENT_GetIPchgEvent(void);
void GEVENT_DetectIPchgEvent(void);

void GEVENT_SetPSWchgEvent(U8_T);
U8_T GEVENT_GetPSWchgEvent(void);
void GEVENT_DetectPSWchgEvent(void);

#endif /* (INCLUDE_EVENT_DETECT) */

#endif /* End of __GEVENT_H__ */

/* End of gconfig.h */
