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
 * Module Name: uart0.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
*=============================================================================
 */
#ifndef __UART_H__
#define __UART_H__

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
#define MAX_TX_UART0_BUF_SIZE	512
#define MAX_TX_UART0_MASK		(MAX_TX_UART0_BUF_SIZE - 1)
#define MAX_RX_UART0_BUF_SIZE	512
#define MAX_RX_UART0_MASK		(MAX_RX_UART0_BUF_SIZE - 1)

/* GLOBAL VARIABLES */
extern U16_T XDATA UART0_RxCount;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	UART_Init(void);
S8_T	putchar(S8_T);
S8_T	_getkey(void);
S8_T	UART0_PutChar(S8_T c);
S8_T	UART0_GetKey(void);
S8_T	UART0_NoBlockGetkey (void);


#endif /* End of __UART_H__ */

/* End of uart.h */