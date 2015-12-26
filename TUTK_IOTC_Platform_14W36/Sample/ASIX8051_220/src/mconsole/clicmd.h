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
 * Module Name: CONSOLe
 * Purpose: The purpose of this package provides the services to CONSOLE
 * Author:
 * Date:
 * Notes:
 * $Log$
*=============================================================================
*/
#ifndef __CLICMD_H__
#define __CLICMD_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define MAX_USER_ACCOUNT 1

/* TYPE DECLARATIONS */

/* GLOBAL VARIABLES */
extern CONSOLE_CmdEntry const FAR CLICMD_userCmdTable[];
extern CONSOLE_Account far CLICMD_userTable[];

/* EXPORTED SUBPROGRAM SPECIFICATIONS */ 
U16_T CLICMD_GetCmdTableSize(void);

#endif /* __CLICMD_H__ */

/* End of clicmd.h */