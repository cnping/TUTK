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
/*=============================================================================
 * Module Name:printd.h
 * Purpose:
 * Author:
 * Date:
 * Notes:	
	printd("justif: \"%-10s\"\n", "left");	=>	justif: "left      "
	printd("justif: \"%10s\"\n", "right");	=>	justif: "     right"
	printd(" 3: %04d zero padded\n", 3u); 	=>	 3: 0003 zero padded
	printd(" 3: %-4d left justif.\n", 3u);	=>	 3: 3    left justif.
	printd(" 3: %4d right justif.\n", 3u);	=>	 3:    3 right justif.
	printd("-3: %04d zero padded\n", -3); 	=>	-3: -003 zero padded
	printd("-3: %-4d left justif.\n", -3);	=>	-3: -3   left justif.
	printd("-3: %4d right justif.\n", -3);	=>	-3:   -3 right justif.
 *
 *=============================================================================
 */
#ifndef PRINTD_H
#define PRINTD_H

/* INCLUDE FILE DECLARATIONS */

/* NAMING CONSTANT DECLARATIONS */
#define	UART0					1
#define	UART1					0
#define DEBUG_PORT				(UART0)

#define PAD_RIGHT				1
#define PAD_ZERO				2
#define PRINT_BUF_LEN			12

#define DRIVER_MSG				0x0001
#define INIT_MSG				0x0002
#define TX_MSG					0x0004
#define RX_MSG					0x0008
#define INT_MSG					0x0010
#define ERROR_MSG				0x0020
#define WARNING_MSG				0x0040
#define DEBUG_MSG				0x0080
#define TCP_MSG					0x0100
#define ALL_MSG					0x01FF
#define NO_MSG					0x0000
#define DEFAULT_MSG				(ALL_MSG) 
#define DEBUG_FLAGS				DEFAULT_MSG


#define DEBUG_DISPLAY_DIAG		1

/* MACRO DECLARATIONS */
#if   ( DEBUG_PORT == UART0 )
#define	DEBUG_PORT_PUTCHR(x)	UART0_PutChar(x)
#else
#define	DEBUG_PORT_PUTCHR(x)	UART1_PutChar(x)
#endif

#if DEBUG_DISPLAY_DIAG
#define PRINTD(flag,x) 			if(flag & DEBUG_FLAGS) {printd x;}
#else
#define PRINTD(flag,x)
#endif

/* GLOBAL VARIABLES */
extern U8_T DEBUG_MsgEnable;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
int printd(const char *format, ...);

#endif /* End of PRINTD_H */

