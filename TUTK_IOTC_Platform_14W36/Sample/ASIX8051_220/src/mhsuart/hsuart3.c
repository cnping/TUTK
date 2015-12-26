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
 * Module Name : hsuart3.c
 * Purpose     : AX220xx UART3 module is implemented to support external UART3
 *               peripherals via accessing AX220xx UART3 registers. This module
 *               handles the UART3 data transmission and reception on the serial
 *               bus and supports auto-software and auto-hardware flow control
 *               functions.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"ax22000.h"
#include	"types.h"
#include	"mcpu_cfg.h"
#include	"mcpu.h"
#include	"hsuart.h"
#include	"hsuart3.h"
#include	"dma.h"
#include	"interrupt.h"
#include	"mstimer.h"
#include	"printd.h"

/* NAMING CONSTANT DECLARATIONS */


/* STATIC VARIABLE DECLARATIONS */
static U8_T				hsur3RxBuffer[UR3_RX_PAGE_BUF] _at_ UR3_RX_BUF_START_ADDR;
static U8_T				hsur3FifoCtrl;
static U8_T				hsur3LineStatus;
static U8_T				hsur3ModemCtrl;
static U8_T				hsur3ModemStatus;
static U8_T				hsur3XoffReceived;
static U8_T				hsur3XonReceived;
static U8_T				hsur3TxDmaComplete;
static U16_T			hsur3RxDmaAvailCount;
static U8_T				hsur3FlowCtrlMode;
static U8_T				hsur3SwFlowCtrlGap;
static U8_T				*ptHsur3RxDmaRingStart;
static U8_T				*ptHsur3RxDmaRingEnd;
static U8_T				*ptHsur3RxDmaRingSwRead;
static U8_T				hsur3RxDataAvail;
static U32_T			hsur3ErrTimeStart;
static U32_T			hsur3ErrTimeStop;
static U8_T				hsur3ErrBlocking;
static U8_T				hsur3IntrEnbType;
static U32_T			hsur3RxTxOperationErr;
static U32_T			hsur3RingFull;
static U32_T			hsur3ErrCount;
static U8_T				ur3DsrXCFTL;

static UR3_TX_PAGE_S	hsur3TxPageInfo[UR3_TX_PAGE_NUM] = {0};
static U8_T				hsur3TxEnqPage = 0;
static U8_T				hsur3TxDeqPage = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void		hsur3_IntRegWrite(U8_T addr, U8_T *ptData, U8_T dataLen);
static void		hsur3_IntRegRead(U8_T addr, U8_T *ptData, U8_T dataLen);
static void		hsur3_EnableXmit(void);
static void		hsur3_DisableXmit(void);
static void		hsur3_ReadLsr(void);
static void		hsur3_ReadMsr(void);
static void		hsur3_DmaIntrStatusCheck(void);
static void		hsur3_RxDmaOccupTrigEvent(void);
static void		hsur3_RxDmaCharTimeoutEvent(void);

/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void hsur3_IntRegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Write data bytes into a register in interrupt.
 * Params : addr:An index address to UART3 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be written.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur3_IntRegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	while (dataLen--)
	{
		U3DR = *(ptData + dataLen);
	}
	U3CIR = addr;
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur3_IntRegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Read data bytes from a register in interrupt.
 * Params : addr:An index address to UART3 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be read.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur3_IntRegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	U3CIR = addr;
	while (dataLen--)
	{
		*(ptData + dataLen) = U3DR;
	}
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur3_EnableXmit(void)
 * Purpose: Turn on the interrupt of transmitting FIFO empty in HSIER.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur3_EnableXmit(void)
{
	U8_T	n;
	BIT		oldEintBit = EA;

	EA = 0;

	U3CIR = HSIER;
	n = U3DR;
	n |= HSIER_TFEI_ENB;
	U3DR = n;
	U3CIR = HSIER;

	EA = oldEintBit;
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur3_DisableXmit(void)
 * Purpose: Turn off the interrupt of transmitting FIFO empty in HSIER.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur3_DisableXmit(void)
{
	U8_T	n;
	BIT		oldEintBit = EA;

	EA = 0;

	U3CIR = HSIER;
	n = U3DR;
	n &= ~HSIER_TFEI_ENB;
	U3DR = n;
	U3CIR = HSIER;

	EA = oldEintBit;
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur_ReadLsr(void)
 * Purpose: Read the Line Status Register to record the error events in a counter.
 *          And begin the error recovering if happening many errors.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur3_ReadLsr(void)
{
	U8_T	lineStatus;
	U8_T	fifoCtrl;
	U8_T	intrEnb;
	U16_T	reg16b;
	U32_T	reg32b;

	hsur3_IntRegRead(HSLSR, &lineStatus, 1);
	hsur3LineStatus = lineStatus;
	if (lineStatus & HSLSR_OE_OVER)
	{
		hsur3ErrCount ++;
		// Overrun Error
	}
	else if (lineStatus & HSLSR_PE_ERROR)
	{
		hsur3ErrCount ++;
		// Parity Error
	}
	else if (lineStatus & HSLSR_FE_ERROR)
	{
		hsur3ErrCount ++;
		// Framing Error
	}
	else if (lineStatus & HSLSR_BI_INT)
	{
		hsur3ErrCount ++;
		// Break Interrupt Occured
	}
	else if (lineStatus & HSLSR_FERR_ERROR)
	{
		hsur3ErrCount ++;
		// Mixing Error
}

	/* For Error handling before data synchrony */
	if (hsur3ErrCount)
{
		/* Enable the Receiver FIFO STOP (RSTOP) bit to stop data characters transfer into RX FIFO.
		   And Reset Rx FIFO and TX FIFO */
		fifoCtrl = (hsur3FifoCtrl | HSFCR_RSTOP | HSFCR_RFR | HSFCR_TFR);
		hsur3_IntRegWrite(HSFCR, &fifoCtrl, 1);
		/* Read the error data pointer */
		hsur3_IntRegRead(HSRBDEP, (U8_T *)&reg16b, 2);
		/* Flush all characters in the RX buffer ring by HSRBRP = HSRBWP when UART2 is in DMA mode */
		reg16b = 0;
		hsur3_IntRegRead(HSRBWP, (U8_T *)&reg16b, 2);
		hsur3_IntRegWrite(HSRBRP, (U8_T *)&reg16b, 2);
		reg32b = (U32_T)ptHsur3RxDmaRingStart & 0xFF0000;
		reg32b += (U32_T)reg16b;
		ptHsur3RxDmaRingSwRead = reg32b;

		/* Disable the Receiver FIFO STOP (RSTOP) bit to re-start RX FIFO */
		fifoCtrl = (hsur3FifoCtrl | HSFCR_RFR | HSFCR_TFR);
		fifoCtrl &= ~HSFCR_RSTOP;
		hsur3_IntRegWrite(HSFCR, &fifoCtrl, 1);

		if (hsur3ErrCount >= 100)
	{
			hsur3ErrBlocking = 1;
			/* Disable the UART3 interrupt */
			intrEnb = 0;
			hsur3_IntRegRead(HSIER, &intrEnb, 1);
		}
	}
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur3_ReadMsr(void)
 * Purpose: Read the Modem Status Register to record in a corresponding interrupt.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur3_ReadMsr(void)
{
	U8_T	modemStatus = 0;

	hsur3_IntRegRead(HSMSR, &modemStatus, 1);
	hsur3ModemStatus = modemStatus;
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur3_DmaIntrStatusCheck(void)
 * Purpose: Read the DMA Status Register to record in a corresponding interrupt.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur3_DmaIntrStatusCheck(void)
{
	U8_T	ur3DmaIntrStatus = 0;
	U8_T	regDcr = 0;
	
	hsur3_IntRegRead(HSDSR, &ur3DmaIntrStatus, 1);
	hsur3_IntRegRead(HSSFCGR, &hsur3SwFlowCtrlGap, 1);
	
	if (ur3DmaIntrStatus & HSDSR_TDC_CPL)
	{
		hsur3TxDmaComplete = 1; // Clear the  Tx DMA busy flag
	}
	if (ur3DmaIntrStatus & HSDSR_XCFTL)
	{
		ur3DsrXCFTL = 1;
	}
	if (ur3DmaIntrStatus & HSDSR_RBTDE)
	{
		hsur3RxTxOperationErr ++;
	}
	if (ur3DmaIntrStatus & HSDSR_RBRF)
		{
		hsur3RingFull ++;
		}
	if (ur3DmaIntrStatus & HSDSR_ECRRT)
		{
		if (hsur3SwFlowCtrlGap == 0)
		{
			U3CIR = HSDCR;
			regDcr = U3DR;
			regDcr |= HSDCR_PRERF;
			U3DR = regDcr;
			U3CIR = HSDCR;
		}
	}
	if (ur3DmaIntrStatus & HSDSR_FCCR)
	{
		if (ur3DmaIntrStatus & HSDSR_FCCRS) // Receuved Xoff
		{
			hsur3XoffReceived = 1;
		}
		else // Received Xon
		{
			hsur3XonReceived = 1;
		}
	}
	if (ur3DmaIntrStatus & HSDSR_WAKE_UP)
	{
		HSUR3_WakeUpEvent();
	}
}

/*
 *--------------------------------------------------------------------------------
 * void hsur3_RxDmaOccupTrigEvent(void)
 * Purpose: When the RX buffer ring has many valid datas and reachs occupying
 *          trigger level, asserting a flag to notice applications.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void hsur3_RxDmaOccupTrigEvent(void)
{
	U8_T	reg8b;
	BIT		oldEintBit = EA;

	EA = 0;
	/* turn off RDI interrupt to avoid ISR triggered without end */
	hsur3_IntRegRead(HSIER, &reg8b, 1);
	reg8b &= ~HSIER_RDI_ENB;
	hsur3_IntRegWrite(HSIER, &reg8b, 1);

	hsur3RxDataAvail = 1;
	
	EA = oldEintBit;
}

/*
 *--------------------------------------------------------------------------------
 * void hsur3_RxDmaCharTimeoutEvent(void)
 * Purpose: When the RX buffer ring has one byte valid data at least,
 *          asserting a flag to notice applications.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void hsur3_RxDmaCharTimeoutEvent(void)
{
	U8_T	reg8b;
	BIT		oldEintBit = EA;

	EA = 0;
	/* turn off RDI interrupt to avoid ISR triggered without end */
	hsur3_IntRegRead(HSIER, &reg8b, 1);
	reg8b &= ~HSIER_RDI_ENB;
	hsur3_IntRegWrite(HSIER, &reg8b, 1);
	
	hsur3RxDataAvail = 1;

	EA = oldEintBit;
}


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR3_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Write data bytes into a register with interrupt locked.
 * Params : addr:An index address to UART3 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be written.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR3_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	BIT		oldEintBit = EA;

	EA = 0;
	while (dataLen--)
	{
		U3DR = *(ptData + dataLen);
	}
	U3CIR = addr;
	EA = oldEintBit;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR3_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Read data bytes from a register with interrupt locked.
 * Params : addr:An index address to UART3 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be read.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR3_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	BIT		oldEintBit = EA;

	EA = 0;
	U3CIR = addr;
	while (dataLen--)
	{
		*(ptData + dataLen) = U3DR;
	}
	EA = oldEintBit;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
 * Purpose: Use this function can initialize the UART3 basic mode. It will configure
 *          the baudrate, data bus type, interrupt source, FIFO setup and modem status.
 * Params : divisor : A 16-bit Divisor Latch value to calaulate the baudrate.
 *          lCtrl : A value to Line Control Register.
 *          intEnb : The interrupt source type of Interrupt Enable Register.
 *          fCtrl : A value to FIFO Control Register.
 *          mCtrl : A value to Modem Control Register.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
{
	U8_T	lineCtrl=0, dll=0, dlh=0;
	U16_T	i, j;

	/* Set UART3 Baudrate */
	lineCtrl = HSLCR_DLAB_ENB;
	HSUR3_RegWrite(HSLCR, &lineCtrl, 1);
	dll = (U8_T)(divisor & 0x00FF);
	dlh = (U8_T)((divisor & 0xFF00) >> 8);
	HSUR3_RegWrite(HSDLLR, &dll, 1);
	HSUR3_RegWrite(HSDLHR, &dlh, 1);
	lineCtrl &= ~HSLCR_DLAB_ENB;
	HSUR3_RegWrite(HSLCR, &lineCtrl, 1);
	/* Set Line Control Register */
	lCtrl &= ~HSLCR_DLAB_ENB;
	HSUR3_RegWrite(HSLCR, &lCtrl, 1);
	/* Set FIFO Control Register */
	hsur3FifoCtrl = fCtrl | HSFCR_RSTOP; // Enable the RSTOP bit to stop the data characters into RX FIFO.
	hsur3FifoCtrl &= ~(HSFCR_RFR | HSFCR_TFR);
	HSUR3_RegWrite(HSFCR, &fCtrl, 1);
	/* Set Modem Control Register */
	HSUR3_RegWrite(HSMCR, &mCtrl, 1);
	/* Disable DMA Mode */
	dll = 0;
	HSUR3_RegWrite(HSDCR, &dll, 1);
	/* Initial Variables */
	for (i=0 ; i<UR3_TX_PAGE_NUM ; i++)
	{
        hsur3TxPageInfo[i].txBusyFlag = UR3_BUF_EMPTY;
        hsur3TxPageInfo[i].txDataLen = 0;
		for (j=0 ; j<UR3_TX_PAGE_BUF ; j++)
			hsur3TxPageInfo[i].txBufArray[j] = 0;
	}
	for (i=0 ; i<UR3_RX_PAGE_BUF ; i++)
	{
		hsur3RxBuffer[i] = 0;
	}
    HSUR3_RxDmaControlInit(hsur3RxBuffer, (hsur3RxBuffer+UR3_RX_PAGE_BUF-1), UR3_RX_TRIGGER_LEVEL, UR3_RX_TIMEOUT_GAP);
	/*	Set FIFO Control Register with users' trig level.
		Enable the RSTOP bit to stop the data characters into RX FIFO. */
	hsur3FifoCtrl = (fCtrl | HSFCR_HSUART_ENB | HSFCR_FIFOE | HSFCR_RFR | HSFCR_TFR);
	hsur3FifoCtrl &= ~HSFCR_RSTOP;
	HSUR3_RegWrite(HSFCR, &fCtrl, 1);

	hsur3ModemCtrl = mCtrl;
	hsur3LineStatus = 0;
	hsur3ErrCount = 0;
	hsur3IntrEnbType = intEnb;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR3_RxDmaControlInit(U8_T *ptRxRingStart, U8_T *ptRxRingEnd,
 *      U16_T rxRingOccupancyTrigLevel, U16_T rxRingTimeoutGap)
 * Purpose: Initialize the UART3 RX buffer ring in DMA mode. It will configure the
 *          ring start/end address, receiving data trig level and receiving timeout gap.
 * Params : ptRxRingStart : A pointer to indicate the RX buffer ring start address.
 *                          The low byte of this must be 0x00.
 *          ptRxRingEnd : A pointer to indicate the RX buffer ring end address.
 *                        The low byte of this must be 0xFF.
 *          rxRingOccupancyTrigLevel : RX ring trigger level to notice driver with numbers of RX data.
 *          rxRingTimeoutGap : To tell hardware how many character times to wait before reporting timeout in RX buffer ring.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR3_RxDmaControlInit(U8_T *ptRxRingStart, U8_T *ptRxRingEnd,
	 U16_T rxRingOccupancyTrigLevel, U16_T rxRingTimeoutGap)
		{
	U32_T	temp32b = 0;
	U16_T	temp16b = 0;
	U8_T	temp8b = 0;

	if (((U8_T)ptRxRingStart != 0) || ((U8_T)ptRxRingEnd != 0xFF))
		return FALSE;

	ptHsur3RxDmaRingStart = ptRxRingStart;
	temp32b = ((U32_T)ptRxRingStart - 0x010000);
	temp32b <<= 8;
	HSUR3_RegWrite(HSRBSP, (U8_T *)&temp32b, 2);

	ptHsur3RxDmaRingEnd = ptRxRingEnd;
	temp32b = ((U32_T)ptRxRingEnd - 0x010000);
	temp32b <<= 8;
	HSUR3_RegWrite(HSRBEP, (U8_T *)&temp32b, 2);

	ptHsur3RxDmaRingSwRead = ptRxRingStart;
	temp32b = ((U32_T)ptRxRingStart - 0x010000);
	temp32b <<= 16;
	temp32b &= 0xFF000000;
	HSUR3_RegWrite(HSRBRP, (U8_T *)&temp32b, 2);

	HSUR3_RegWrite(HSRBOTL, (U8_T *)&rxRingOccupancyTrigLevel, 2);

	HSUR3_RegWrite(HSRBRTG, (U8_T *)&rxRingTimeoutGap, 2);
	
	/* Enable Receive Buffer Ring */
	HSUR3_RegRead(HSDCR, &temp8b, 1);
	temp8b |= HSDCR_RBRE_ENB;
	HSUR3_RegWrite(HSDCR, &temp8b, 1);
	
	return TRUE;
		}

/*
 *--------------------------------------------------------------------------------
 * void HSUR3_Start(void)
 * Purpose: Start function will enable the interrupt sources that be configured
 *          in the setup function. This function should be closely called before
 *          running application tasks.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_Start(void)
{
	/* Enable the UART3 interrupt */
	HSUR3_RegWrite(HSIER, &hsur3IntrEnbType, 1);
}		

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_ErrorRecovery(void)
 * Purpose: Checking the UART3 Line Status Register value of errors and
 *          re-enable interrupts of receiving and line status.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_ErrorRecovery(void)
{
	U8_T	valLsr;
	U8_T	valFcr;
	U16_T	reg16b;
	U32_T	reg32b;

	while (hsur3ErrBlocking)
	{
		printd("hsur3ErrBlocking = 1\n\r");
		HSUR3_RegRead(HSLSR, &valLsr, 1);
		if (valLsr & HSLSR_DR_DATA)
		{
			/* Enable the Receiver FIFO STOP (RSTOP) bit to stop data characters transfer into RX FIFO.
			   And Reset Rx FIFO and TX FIFO */
			valFcr = (hsur3FifoCtrl | HSFCR_RSTOP | HSFCR_RFR | HSFCR_TFR);
			hsur3_IntRegWrite(HSFCR, &valFcr, 1);
			/* Read the error data pointer */
			hsur3_IntRegRead(HSRBDEP, (U8_T *)&reg16b, 2);
			/* Flush all characters in the RX buffer ring by HSRBRP = HSRBWP when UART2 is in DMA mode */
			reg16b = 0;
			hsur3_IntRegRead(HSRBWP, (U8_T *)&reg16b, 2);
			hsur3_IntRegWrite(HSRBRP, (U8_T *)&reg16b, 2);
			reg32b = (U32_T)ptHsur3RxDmaRingStart & 0xFF0000;
			reg32b += (U32_T)reg16b;
			ptHsur3RxDmaRingSwRead = reg32b;
			/* Disable the Receiver FIFO STOP (RSTOP) bit to re-start RX FIFO */
			valFcr = (hsur3FifoCtrl | HSFCR_RFR | HSFCR_TFR);
			valFcr &= ~HSFCR_RSTOP;
			hsur3_IntRegWrite(HSFCR, &valFcr, 1);
	}
	else
	{
			printd("clear hsur3ErrBlocking = 0\n\r");
			EA = 0;
			hsur3ErrBlocking = 0;
			hsur3ErrCount = 0;
			/* Enable the UART3 interrupt again */
			HSUR3_RegWrite(HSIER, &hsur3IntrEnbType, 1);
			EA = 1;
	}
	}
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR3_IntrEntryFunc(void)
 * Purpose: An interrupt service rounine that should be included in MCPU peripheral
 *          interface interrupt service routine within the AX220xx MCPU module.
 *          It will be trigger by interrupt events that be incicated in the HSIIR register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_IntrEntryFunc(void)
{
	U8_T	intrStatus = 0;

	hsur3_IntRegRead(HSIIR, &intrStatus, 1);
	intrStatus &= 0x1F;

	if (intrStatus == HSIIR_RLS_INTR)
	{
		hsur3_ReadLsr();
	}
	else if (intrStatus == HSIIR_DMAS_INTR)
	{
		hsur3_DmaIntrStatusCheck();
	}
	else if (intrStatus == HSIIR_RD_TRIG_INTR)
	{
		hsur3_RxDmaOccupTrigEvent();
		}
	else if (intrStatus == HSIIR_RD_TI_INTR)
	{
		hsur3_RxDmaCharTimeoutEvent();
	}
	else if (intrStatus == HSIIR_TFE_INTR)
	{
		/* This status will not be occurred in Tx DMA mode */
	}
	else if (intrStatus == HSIIR_MS_INTR)
	{
		hsur3_ReadMsr();
	}
	else
	{

	}
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR3_RxDmaSwReadPointerUpdated(U16_T lenCount)
 * Purpose: release the RX buffer ring memory that have been retrieved.
 * Params : lenCount : A length that indicates how many valid data can be retrieved.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR3_RxDmaSwReadPointerUpdated(U16_T lenCount)
{
	U32_T	temp32b = 0;
	U8_T	temp8b = 0;

	ptHsur3RxDmaRingSwRead = ptHsur3RxDmaRingSwRead + lenCount;

	if (ptHsur3RxDmaRingSwRead > ptHsur3RxDmaRingEnd)
		ptHsur3RxDmaRingSwRead = ptHsur3RxDmaRingStart + (ptHsur3RxDmaRingSwRead - ptHsur3RxDmaRingEnd - 1);
	
	temp32b = ((U32_T)ptHsur3RxDmaRingSwRead - 0x010000);
	temp32b <<= 16;

	HSUR3_RegWrite(HSRBRP, (U8_T *)&temp32b, 2);

	/* Re-enable the RxDMA interrupt */
	HSUR3_RegRead(HSIER, &temp8b, 1);
	temp8b |= HSIER_RDI_ENB;
	HSUR3_RegWrite(HSIER, &temp8b, 1);

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * U16_T HSUR3_GetRxDmaAvailCount(void)
 * Purpose: The number of valid data bytes in Receive Buffer Ring.
 *          Check this to determine how many bytes have been received.
 *          This function will directly read a register HSRBABC (0x20) to return.
 * Params : None.
 * Returns: hsur3RxDmaAvailCount: A veriable of HSRBABC (0x20).
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
U16_T HSUR3_GetRxDmaAvailCount(void)
{
	U16_T	reg16b;

	if (hsur3RxDataAvail+hsur3RingFull == 0)
	{
		hsur3RxDmaAvailCount = 0;
		return hsur3RxDmaAvailCount;
	}

	/* update the HSRBABC to get the available count of Rx ring*/
	HSUR3_RegRead(HSRBABC, (U8_T *)&reg16b, 2);
	hsur3RxDmaAvailCount = reg16b;
	
	return hsur3RxDmaAvailCount;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR3_TxDmaTranmit(U8_T *ptTx, U16_T length)
 * Purpose: This function will directly transmit characters in TX buffer memory.
 *          Give a start address of TX buffer with a length to the hardware
 *          for DMA copying and get a status to check.
 * Params : ptTx: A point to indicate the TX buffer on xDATA momory.
 *          length: The length that will be transmitted by TX DMA.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR3_TxDmaTranmit(U8_T *ptTx, U16_T length)
{
	U32_T	temp32b;
	U8_T	temp8b;

	/* if TxDMA still busy, give up */
    do
    {
        HSUR3_RegRead(HSDCR, &temp8b, 1);
    } while (temp8b & HSDCR_STD);

	temp32b = ((U32_T)ptTx - 0x010000); // for Keil C pointer convert
	temp32b <<= 8;
	HSUR3_RegWrite(HSTDAR, (U8_T *)&temp32b, 3);
	
	HSUR3_RegWrite(HSTDBCR, (U8_T *)&length, 2);

	temp8b |= HSDCR_STD;
	HSUR3_RegWrite(HSDCR, &temp8b, 1);

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * U8_T HSUR3_GetLineStatus(U8_T intrSynth)
 * Purpose: This function will return a variable of the UART3 Line Status Register.
 * Params : intrSynth: Set 1 to get the Line Status in the interrupt recording.
 *                     Set 0 to directly read the Line Status Register (HSLSR).
 * Returns: valLsr: A variable of the UART3 Line Status Register.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
U8_T HSUR3_GetLineStatus(U8_T intrSynth)
{
	U8_T valLsr;

	if (intrSynth)
		valLsr = hsur3LineStatus & 0x9E;
	else
		HSUR3_RegRead(HSLSR, &valLsr, 1);

	return valLsr;
}

/*
 * ----------------------------------------------------------------------------
 * U8_T HSUR3_GetModemStatus(U8_T intrSynth)
 * Purpose: Getting the UART3 Modem Status .
 * Params : intrSynth: 1: Get MSR according to an interrupt trigger.
 *                     0: Get MSR by directly reading the register.
 * Returns: varModemStatus: Modem status register variable.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T HSUR3_GetModemStatus(U8_T intrSynth)
{
	U8_T	varModemStatus = 0;

	if (intrSynth)
		varModemStatus = hsur3ModemStatus;
	else
		HSUR3_RegRead(HSMSR, &varModemStatus, 1);

	return varModemStatus;
}

/*
 * ----------------------------------------------------------------------------
 * U8_T HSUR3_GetLineControl(void)
 * Purpose: Getting the UART3 Line Control register variables.
 * Params : None.
 * Returns: varLineCtrl: A variable of the UART3 Line Control Register.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T HSUR3_GetLineControl(void)
{
	U8_T	varLineCtrl = 0;

	HSUR3_RegRead(HSLCR, &varLineCtrl, 1);

	return varLineCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * U8_T HSUR3_GetFifoControl(void)
 * Purpose: Getting the UART3 FIFO Control register variables.
 * Params : None.
 * Returns: hsur3FifoCtrl: A variable of the UART3 FIFO Control Register in initialization.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T HSUR3_GetFifoControl(void)
{
	return hsur3FifoCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * U8_T HSUR3_GetModemControl(void)
 * Purpose: Getting the UART3 Modem Control register variables.
 * Params : None.
 * Returns: hsur3ModemCtrl:A variable of the UART3 Modem Control Register in initialization.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T HSUR3_GetModemControl(void)
{
	return hsur3ModemCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_SetLineControl(U8_T value)
 * Purpose: Setting the UART3 Line control register with a new value.
 * Params : value: variable to be set.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_SetLineControl(U8_T value)
{
	U8_T	lCtrl = value;

	HSUR3_RegWrite(HSLCR, &lCtrl, 1);
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_SetFifoControl(U8_T value)
 * Purpose: Setting the UART3 FIFO control register with a new value.
 * Params : value: variable to be set.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_SetFifoControl(U8_T value)
{
	U8_T	fCtrl = value;

	HSUR3_RegWrite(HSFCR, &fCtrl, 1);

	hsur3FifoCtrl = fCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_SetModemControl(U8_T value)
 * Purpose: Setting the UART3 Modem control register with a new value.
 * Params : value: variable to be set.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_SetModemControl(U8_T value)
{
	U8_T	mCtrl = value;
	
	HSUR3_RegWrite(HSMCR, &mCtrl, 1);

	hsur3ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_SetRTS(void)
 * Purpose: Only setting the UART3 RTS bit of Modem control register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_SetRTS(void)
{
	U8_T	mCtrl;

	HSUR3_RegRead(HSMCR, &mCtrl, 1);
	mCtrl |= HSMCR_RTS;
	HSUR3_RegWrite(HSMCR, &mCtrl, 1);

	hsur3ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_SetDTR(void)
 * Purpose: Only setting the UART3 DTR bit of Modem control register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_SetDTR(void)
{
	U8_T	mCtrl;

	HSUR3_RegRead(HSMCR, &mCtrl, 1);
	mCtrl |= HSMCR_DTR;
	HSUR3_RegWrite(HSMCR, &mCtrl, 1);
	
	hsur3ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_ClearRTS(void)
 * Purpose: Only clear the UART3 RTS bit of Modem control register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_ClearRTS(void)
{
	U8_T	mCtrl;

	HSUR3_RegRead(HSMCR, &mCtrl, 1);
	mCtrl &= ~HSMCR_RTS;
	HSUR3_RegWrite(HSMCR, &mCtrl, 1);
	
	hsur3ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_ClearDTR(void)
 * Purpose: Only clear the UART3 DTR bit of Modem control register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_ClearDTR(void)
{
	U8_T	mCtrl;

	HSUR3_RegRead(HSMCR, &mCtrl, 1);
	mCtrl &= ~HSMCR_DTR;
	HSUR3_RegWrite(HSMCR, &mCtrl, 1);

	hsur3ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_SetupPort(U16_T divBaudRate, U8_T lCtrl)
 * Purpose: Setting the UART3 baudrate.
 * Params : divBaudRate: The divisor of the baudrate.
 *          lCtrl: A Line Control Register value.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_SetupPort(U16_T divBaudRate, U8_T lCtrl)
{
	U8_T	dll, dlh, lineCtrl;

	/* Set UART3 Baudrate */
	lineCtrl = HSLCR_DLAB_ENB;
	HSUR3_RegWrite(HSLCR, &lineCtrl, 1);
	dll = (U8_T)(divBaudRate & 0x00FF);
	dlh = (U8_T)((divBaudRate & 0xFF00) >> 8);
	HSUR3_RegWrite(HSDLLR, &dll, 1);
	HSUR3_RegWrite(HSDLHR , &dlh, 1);
	lineCtrl &= ~HSLCR_DLAB_ENB;
	HSUR3_RegWrite(HSLCR, &lineCtrl, 1);
	/* Set Line Control Register */
	lCtrl &= ~HSLCR_DLAB_ENB;
	HSUR3_RegWrite(HSLCR, &lCtrl, 1);
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR3_SetBaudRate(U16_T divBaudRate)
 * Purpose: Setting the UART3 baudrate.
 * Params : divBaudRate: The divisor of the baudrate.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR3_SetBaudRate(U16_T divBaudRate)
{
	U8_T	dll, dlh, lineCtrl;

	/* Set UART3 Baudrate */
	lineCtrl = HSLCR_DLAB_ENB;
	HSUR3_RegWrite(HSLCR, &lineCtrl, 1);
	dll = (U8_T)(divBaudRate & 0x00FF);
	dlh = (U8_T)((divBaudRate & 0xFF00) >> 8);
	HSUR3_RegWrite(HSDLLR, &dll, 1);
	HSUR3_RegWrite(HSDLHR , &dlh, 1);
	lineCtrl &= ~HSLCR_DLAB_ENB;
	HSUR3_RegWrite(HSLCR, &lineCtrl, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR3_AutoCtsRtsFlowCtrl(U8_T set)
 * Purpose: To enable/disable the automatic CTS/RTS flow control executed by hardware.
 * Params : set: A value 1 to enable the auto-CTS/RTS flow control function.
 *               A value 0 to disable.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_AutoCtsRtsFlowCtrl(U8_T set)
{
	U8_T	reg8b;
	
	/* If turn HW CTS/RTS, the software flow control need not be enabled */
	HSUR3_RegRead(HSAFCSR, &reg8b, 1);
	if (set)
		reg8b = HSAFCSR_ACTSE_ENB | HSAFCSR_ARTSE_ENB; // only use HW CTS/RTS */
	else
		reg8b &= ~(HSAFCSR_ACTSE_ENB | HSAFCSR_ARTSE_ENB);
	HSUR3_RegWrite(HSAFCSR, &reg8b, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR3_AutoSoftwareFlowCtrl(U8_T setFlow, U8_T modeFlow, U16_T chXon, U16_T chXoff, U16_T chEsc)
 * Purpose: To enable / disable the automatic Software Xon/Xoff flow control
 *          function with Escape or Non-Eacape that is executed by hardware.
 * Params : setFlow: A value 1 to enable the auto-Xon/Xoff flow control function.
 *                   A value 0 to disable.
 *          modeFlow: A variable of  HSAFCSR(0x08).
 *                    There are 4 bit can be set for auto-software flow control mode.
 *          chXon: The Xon characters. Only low byte is valid if setting one byte flow control.
 *          chXoff: The Xoff characters. Only low byte is valid if setting one byte flow control.
 *          chEsc: The ESC characters. Only low byte is valid if setting one byte attached.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_AutoSoftwareFlowCtrl(U8_T setFlow, U8_T modeFlow, U16_T chXon, U16_T chXoff, U16_T chEsc)
{
	U8_T	reg8b;
	
	/* If turn HW CTS/RTS, the software flow control need not be enabled */
	HSUR3_RegRead(HSAFCSR, &reg8b, 1);
	if (setFlow)
	{
		reg8b = HSAFCSR_ASFCE_ENB; // only use Automatic Software flow control Enable
		
		HSUR3_RegWrite(HSXONR, (U8_T *)&chXon, 2);

		HSUR3_RegWrite(HSXOFFR, (U8_T *)&chXoff, 2);
		
		HSUR3_RegWrite(HSECR, (U8_T *)&chEsc, 2);
			
		if (modeFlow & HSAFCSR_DCSE_ENB) //Double Xon/Xoff Characters Select Enable
			reg8b |= HSAFCSR_DCSE_ENB;
		if (modeFlow & HSAFCSR_AECE_ENB) //Auto Escape Character Enable
			reg8b |= HSAFCSR_AECE_ENB;
		if (modeFlow & HSAFCSR_DECE_ENB) //Double Escape Character Enable
			reg8b |= HSAFCSR_DECE_ENB;
	}
	else
	{
		reg8b &= ~(HSAFCSR_ASFCE_ENB | HSAFCSR_DCSE_ENB | HSAFCSR_AECE_ENB | HSAFCSR_DECE_ENB | HSAFCSR_SXON | HSAFCSR_SXOFF);
	}
	HSUR3_RegWrite(HSAFCSR, &reg8b, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR3_WakeUpEvent(void)
 * Purpose: An interrupt service rounine that should be included in MCPU
 *          Wake-Up interrupt service routine within the AX220xx MCPU module.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_WakeUpEvent(void)
{
	U8_T	reg8b;

	hsur3_IntRegRead(HSDSR, &reg8b, 1);
	// Add wakeup process from here...
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR3_ForceSendCtrlXon(void)
 * Purpose: Force AX220xx UART3 to send the Xon character with single or double.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_ForceSendCtrlXon(void)
{
	U8_T	reg8b;
	
	HSUR3_RegRead(HSAFCSR,&reg8b, 1);
	reg8b |= HSAFCSR_SXON;
	HSUR3_RegWrite(HSAFCSR,&reg8b, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR3_ForceSendCtrlXoff(void)
 * Purpose: Force AX220xx UART3 to send the Xoff character with single or double.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_ForceSendCtrlXoff(void)
{
	U8_T	reg8b;
	
	HSUR3_RegRead(HSAFCSR,&reg8b, 1);
	reg8b |= HSAFCSR_SXOFF;
	HSUR3_RegWrite(HSAFCSR,&reg8b, 1);
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR3_XmitOutEnq(U8_T* pAppData, U16_T appLength)
 * Purpose: A transmitting en-queue function is used by an application up-layer.
 *          When the up-layer software wants to send data on to the serial bus,
 *          it can call this function to put data into a TX buffer. The en-queue
 *          function will keep the data until the transmitting task is processed.
 * Params : pAppData: A point to indicate the application data stream that will
 *                    be transmitted by TX DMA. This pointer will be transformed
 *                    to a real start address on xDATA momory.
 *          appLength: The length of application transmitting data.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR3_XmitOutEnq(U8_T* pAppData, U16_T appLength)
{
    U16_T txUr3Len;
    
    while (appLength)
    {
	    /* Check the transmitting buffer for free to use */
	    if (hsur3TxPageInfo[hsur3TxEnqPage].txBusyFlag & UR3_BUF_BUSY)
		    return FALSE;

        txUr3Len = appLength;
        if (txUr3Len > UR3_TX_PAGE_BUF)
        {
            txUr3Len = UR3_TX_PAGE_BUF;
        }
        appLength -= txUr3Len;

        hsur3TxPageInfo[hsur3TxEnqPage].txBusyFlag |= UR3_BUF_BUSY;
        hsur3TxPageInfo[hsur3TxEnqPage].txDataLen = txUr3Len;
        if (txUr3Len)
            DMA_Grant(hsur3TxPageInfo[hsur3TxEnqPage].txBufArray, pAppData, txUr3Len);

 	    pAppData += txUr3Len;
        hsur3TxEnqPage ++;
        if (hsur3TxEnqPage >= UR3_TX_PAGE_NUM)
            hsur3TxEnqPage = 0;
    }
    return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR3_XmitOutDeq(void)
 * Purpose: A transmitting de-queue function is used by an application up-layer.
 *          The up-layer software can call this function to transmit data from the
 *          TX buffer to FIFO. This function will use the function, HSUR3_TxDmaTranmit( ),
 *          to send data stream out. An application must include this de-queue
 *          function into a transmitting task that should have a responsibility
 *          for sending data to serial bus.
 * Params : None.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR3_XmitOutDeq(void)
{
	U8_T	*pTxOut;
	U16_T	txLength;
		
	/* Check the transmitting buffer for valid to send */
	if (hsur3TxPageInfo[hsur3TxDeqPage].txBusyFlag == UR3_BUF_EMPTY)
		return FALSE;

	pTxOut = hsur3TxPageInfo[hsur3TxDeqPage].txBufArray;
	txLength = hsur3TxPageInfo[hsur3TxDeqPage].txDataLen;
	HSUR3_TxDmaTranmit(pTxOut, txLength);
	hsur3TxPageInfo[hsur3TxDeqPage].txBusyFlag = UR3_BUF_EMPTY;

	hsur3TxDeqPage ++;
	if (hsur3TxDeqPage >= UR3_TX_PAGE_NUM)
		hsur3TxDeqPage = 0;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR3_CopyUartToApp(U8_T* pAppData, U16_T appGetLen)
 * Purpose: If the up-layer application has a memory resource as a RX buffer,
 *          this function can help to copy the received data block from the firmware
 *          receiving buffer ring to up-layer buffer by a SW DMA function. After
 *          completing the DMA copying, the function, HSUR3_RxDmaSwReadPointerUpdated,
 *          will be called to release the RX buffer ring. This function alwaye need
 *          a real memory resource at the up-layer application. But the application
 *          can also use other API functions to get the RX buffer ring status
 *          and can release the RX buffer ring.
 * Params : pAppData : A point to indicate the up-layer application data buffer that
 *                     storing received characters.
 *          appFreeLen : The maximum length of the up-layer application data buffer.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR3_CopyUartToApp(U8_T* pAppData, U16_T appGetLen)
{
	U8_T	*pRxStart, *pRxCur, *pRxEnd;
	U16_T	rxCount, tempCount1, tempCount2;
    BIT		oldEintBit;

    if (!appGetLen)
        return;

	rxCount = appGetLen;
	pRxStart = ptHsur3RxDmaRingStart;
	pRxCur = ptHsur3RxDmaRingSwRead;
	pRxEnd = ptHsur3RxDmaRingEnd;

	if ((pRxCur + rxCount) > pRxEnd)
	{
		/* copy data to application from pRxCur to pRxEnd */
		tempCount1 = (pRxEnd - pRxCur + 1);
		if (tempCount1)
            DMA_Grant(pAppData, pRxCur, tempCount1);
		/* copy data to application for last length from pRxStart */
		tempCount2 = rxCount - tempCount1;
        if (tempCount2)
            DMA_Grant((pAppData + tempCount1), pRxStart, tempCount2);
 	}
	else
	{
        DMA_Grant(pAppData, pRxCur, rxCount);
 	}

	HSUR3_RxDmaSwReadPointerUpdated(rxCount);
	oldEintBit = EA;
	EA = 0;
    hsur3RingFull=0;
	hsur3RxDataAvail = 0;
	EA = oldEintBit;
}

/* End of hsuart3.c */
