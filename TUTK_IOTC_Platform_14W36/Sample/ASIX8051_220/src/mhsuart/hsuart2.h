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
 * Module Name : hsuart2.h
 * Purpose     : A header file of UART2 module.
 *               It defines all globe parameters and extend API functions.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 *================================================================================
 */
#ifndef HSUART2_H
#define HSUART2_H

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
#define UR2_RX_TRIGGER_LEVEL		1024
#define UR2_RX_TIMEOUT_GAP			0x06
#define UR2_RX_BUF_START_ADDR		0x8000		/* The low byte of this start address must be 0x00 */
#define	UR2_RX_PAGE_BUF				(256 * 8)	/* The RX ring size must be a multiple of 256bytes */

#define	UR2_TX_PAGE_BUF				512
#define	UR2_TX_PAGE_NUM				6

#define	UR2_SLEEP					0
#define	UR2_HFDPX					BIT6
#define	UR2_SLAVE					BIT7
#define	UR2_MASTER					BIT6|BIT7
#define	UR2_RS485_RECEIVE			BIT6
#define	UR2_RS485_DRIVE				BIT7
#define	UR2_FLOW_CTRL_NO			0
#define	UR2_FLOW_CTRL_HW			1
#define	UR2_FLOW_CTRL_X				2
#define	ASCII_XON					0x11
#define	ASCII_XOFF					0x13

#define	UR2_BUF_EMPTY				0
#define	UR2_BUF_BUSY				1


/* TYPE DECLARATIONS */
typedef	struct	ur2_tx_page_s {
	U8_T			txBusyFlag;
	U16_T 			txDataLen;
	U8_T			txBufArray[UR2_TX_PAGE_BUF];
}	UR2_TX_PAGE_S;


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
BOOL	HSUR2_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen);
BOOL	HSUR2_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen);
void	HSUR2_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl);
BOOL	HSUR2_RxDmaControlInit(U8_T *ptRxRingStart, U8_T *ptRxRingEnd, U16_T rxRingOccupancyTrigLevel, U16_T rxRingTimeoutGap);
void	HSUR2_Start(void);
void	HSUR2_ErrorRecovery(void);
void	HSUR2_IntrEntryFunc(void);
BOOL	HSUR2_RxDmaSwReadPointerUpdated(U16_T lenCount);
U16_T	HSUR2_GetRxDmaAvailCount(void);
BOOL	HSUR2_TxDmaTranmit(U8_T *ptTx, U16_T length);
U8_T	HSUR2_GetLineStatus(U8_T intrSynth);
U8_T	HSUR2_GetModemStatus(U8_T intrSynth);
U8_T	HSUR2_GetLineControl(void);
U8_T	HSUR2_GetFifoControl(void);
U8_T	HSUR2_GetModemControl(void);
void	HSUR2_SetLineControl(U8_T value);
void	HSUR2_SetFifoControl(U8_T value);
void	HSUR2_SetModemControl(U8_T value);
void	HSUR2_SetRTS(void);
void	HSUR2_SetDTR(void);
void	HSUR2_ClearRTS(void);
void	HSUR2_ClearDTR(void);
void	HSUR2_SetupPort(U16_T divBaudRate, U8_T lCtrl);
void	HSUR2_SetBaudRate(U16_T divBaudRate);
void	HSUR2_AutoCtsRtsFlowCtrl(U8_T set);
void	HSUR2_AutoSoftwareFlowCtrl(U8_T setFlow, U8_T modeFlow, U16_T chXon, U16_T chXoff, U16_T chEsc);
void	HSUR2_WakeUpEvent(void);
void	HSUR2_ForceSendCtrlXon(void);
void	HSUR2_ForceSendCtrlXoff(void);
BOOL	HSUR2_XmitOutEnq(U8_T* pAppData, U16_T appLength);
BOOL	HSUR2_XmitOutDeq(void);
void	HSUR2_CopyUartToApp(U8_T* pAppData, U16_T appFreeLen);

#endif /* End of HSUART2_H */
