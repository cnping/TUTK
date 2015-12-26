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
 * Module Name : hsuart3.h
 * Purpose     : A header file of UART3 module.
 *               It defines all globe parameters and extend API functions.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 *================================================================================
 */
#ifndef HSUART3_H
#define HSUART3_H

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
#define UR3_RX_TRIGGER_LEVEL		1024
#define UR3_RX_TIMEOUT_GAP			0x06
#define UR3_RX_BUF_START_ADDR		0x8800		/* The low byte of this start address must be 0x00 */
#define	UR3_RX_PAGE_BUF				(256 * 8)	/* The RX ring size must be a multiple of 256bytes */

#define	UR3_TX_PAGE_BUF				512
#define	UR3_TX_PAGE_NUM				6

#define	UR3_SLEEP			0
#define	UR3_HFDPX			BIT6
#define	UR3_SLAVE			BIT7
#define	UR3_MASTER			BIT6|BIT7
#define	UR3_RS485_RECEIVE			BIT6
#define	UR3_RS485_DRIVE				BIT7
#define	UR3_FLOW_CTRL_NO			0
#define	UR3_FLOW_CTRL_HW			1
#define	UR3_FLOW_CTRL_X				2
#define	ASCII_XON					0x11
#define	ASCII_XOFF					0x13

#define	UR3_BUF_EMPTY				0
#define	UR3_BUF_BUSY				1


/* TYPE DECLARATIONS */
typedef	struct	ur3_tx_page_s {
	U8_T			txBusyFlag;
	U16_T 			txDataLen;
	U8_T			txBufArray[UR3_TX_PAGE_BUF];
}	UR3_TX_PAGE_S;


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
BOOL	HSUR3_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen);
BOOL	HSUR3_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen);
void	HSUR3_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl);
BOOL	HSUR3_RxDmaControlInit(U8_T *ptRxRingStart, U8_T *ptRxRingEnd, U16_T rxRingOccupancyTrigLevel, U16_T rxRingTimeoutGap);
void	HSUR3_Start(void);
void	HSUR3_ErrorRecovery(void);
void	HSUR3_IntrEntryFunc(void);
BOOL	HSUR3_RxDmaSwReadPointerUpdated(U16_T lenCount);
U16_T	HSUR3_GetRxDmaAvailCount(void);
BOOL	HSUR3_TxDmaTranmit(U8_T *ptTx, U16_T length);
U8_T	HSUR3_GetLineStatus(U8_T intrSynth);
U8_T	HSUR3_GetModemStatus(U8_T intrSynth);
U8_T	HSUR3_GetLineControl(void);
U8_T	HSUR3_GetFifoControl(void);
U8_T	HSUR3_GetModemControl(void);
void	HSUR3_SetLineControl(U8_T value);
void	HSUR3_SetFifoControl(U8_T value);
void	HSUR3_SetModemControl(U8_T value);
void	HSUR3_SetRTS(void);
void	HSUR3_SetDTR(void);
void	HSUR3_ClearRTS(void);
void	HSUR3_ClearDTR(void);
void	HSUR3_SetupPort(U16_T divBaudRate, U8_T lCtrl);
void	HSUR3_SetBaudRate(U16_T divBaudRate);
void	HSUR3_AutoCtsRtsFlowCtrl(U8_T set);
void	HSUR3_AutoSoftwareFlowCtrl(U8_T setFlow, U8_T modeFlow, U16_T chXon, U16_T chXoff, U16_T chEsc);
void	HSUR3_WakeUpEvent(void);
void	HSUR3_ForceSendCtrlXon(void);
void	HSUR3_ForceSendCtrlXoff(void);
BOOL	HSUR3_XmitOutEnq(U8_T* pAppData, U16_T appLength);
BOOL	HSUR3_XmitOutDeq(void);
void	HSUR3_CopyUartToApp(U8_T* pAppData, U16_T appFreeLen);

#endif /* End of HSUART2_H */
