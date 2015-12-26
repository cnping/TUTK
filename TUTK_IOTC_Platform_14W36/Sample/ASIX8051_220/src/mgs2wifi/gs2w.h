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
 * Module Name:gs2w.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gs2w.h,v $
 *
 *=============================================================================
 */

#ifndef __GS2W_H__
#define __GS2W_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define GS2W_STATE_IDLE							0 
#define GS2W_STATE_TCP_DATA_PROCESS 			1
#define GS2W_STATE_UDP_DATA_PROCESS 			2

#define GS2W_ENABLE_FLOW_CONTROL				1	// 1:enable 0:disable

// Serial port flow control commands
// Active reply
#define MODEM_STATUS_CHANGE						0x81	
#define IOCTL_SERIAL_GET_DTRRTS					0x01
#define IOCTL_SERIAL_GET_MODEM_CONTROL			0x02
#define IOCTL_SERIAL_GET_MODEM_STATUS			0x03
#define IOCTL_SERIAL_GET_DTRRTS_REPLY			0xA1
#define IOCTL_SERIAL_GET_MODEM_CONTROL_REPLY	0xA2
#define IOCTL_SERIAL_GET_MODEM_STATUS_REPLY		0xA3
#define IOCTL_SERIAL_CLR_DTR					0x11
#define IOCTL_SERIAL_CLR_RTS					0x12
#define IOCTL_SERIAL_SET_DTR					0x13
#define IOCTL_SERIAL_SET_RTS					0x14
#define IOCTL_SERIAL_SET_BAUD_RATE				0x15
#define IOCTL_SERIAL_SET_BREAK_OFF				0x16
#define IOCTL_SERIAL_SET_BREAK_ON				0x17
#define IOCTL_SERIAL_SET_FIFO_CONTROL			0x18
#define IOCTL_SERIAL_SET_LINE_CONTROL			0x19
#define IOCTL_SERIAL_SET_MODEM_CONTROL			0x1A
#define IOCTL_SERIAL_SET_XOFF					0x21
#define IOCTL_SERIAL_SET_XON					0x22
#define IOCTL_SERIAL_XOFF_COUNTER				0x23
#define IOCTL_SERIAL_SET_HANDFLOW               0x24

#define SERIAL_HANDFLOW_NONE      0
#define SERIAL_HANDFLOW_HW        1
#define SERIAL_HANDFLOW_XON_XOFF  2

#define GS2W_CONN_UDP 1
#define GS2W_CONN_TCP 2

/* TYPE DECLARATIONS */

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void GS2W_Task(void);
void GS2W_Init(void);
U8_T GS2W_GetTaskState(void);
void GS2W_SetTaskState(U8_T); 
U8_T GS2W_GetConnType(void);

#endif /* End of __GS2W_H__ */

/* End of gs2w.h */
