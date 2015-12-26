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
 * Module Name : mcpu_cfg.h
 * Purpose     : Configuration setting of mcpu module.
 * Author      : Robin Lee
 * Date        :
 * Notes       : None.
 * $Log: mcpu_cfg.h,v $
 * no message
 *
 *================================================================================
 */
#ifndef MCPU_CFG_H
#define MCPU_CFG_H

/* INCLUDE FILE DECLARATIONS */
#include "stoe_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
  #define	POWER_SAVING				1 // 1:support wifi power saving ; 0:not support
#else
  #define	POWER_SAVING				0 // do not change the value to 1.
#endif

/*-------------------------------------------------------------*/
#define		MCPU_BLOCK_END		0x90
/* The MCPU End block number of code space.
   Bit7=1 : Disable Flash protection mode ;
   Bit7=0 : Enable Flash protection mode.
*/
#define		WCPU_BLOCK_START	0x19
/* The WCPU Start block number of code space.
   One block number indicates 32K bytes size.
   The WCPU start address is 0xC0000, that is
   represented to 0x19 (25th) block number.
*/

/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */


#endif /* End of MCPU_CFG_H */
