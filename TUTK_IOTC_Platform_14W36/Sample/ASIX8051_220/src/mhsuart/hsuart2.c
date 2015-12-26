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
 * Module Name : hsuart2.c
 * Purpose     : AX220xx UART2 module is implemented to support external UART2
 *               peripherals via accessing AX220xx UART2 registers. This module
 *               handles the UART2 data transmission and reception on the serial
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
#include	"hsuart2.h"
#include	"dma.h"
#include	"interrupt.h"
#include	"mstimer.h"
#include	"printd.h"

/* NAMING CONSTANT DECLARATIONS */


/* STATIC VARIABLE DECLARATIONS */
static U8_T				hsur2RxBuffer[UR2_RX_PAGE_BUF] _at_ UR2_RX_BUF_START_ADDR;
static U8_T				hsur2FifoCtrl;
static U8_T				hsur2LineStatus;
static U8_T				hsur2ModemCtrl;
static U8_T				hsur2ModemStatus;
static U8_T				hsur2XoffReceived;
static U8_T				hsur2XonReceived;
static U8_T				hsur2TxDmaComplete;
static U8_T				hsur2FlowCtrlMode;
static U8_T				hsur2SwFlowCtrlGap;
static U8_T				*ptHsur2RxDmaRingStart;
static U8_T				*ptHsur2RxDmaRingEnd;
static U8_T				*ptHsur2RxDmaRingSwRead;
static U8_T				hsur2RxDataAvail=0;
static U32_T			hsur2ErrTimeStart;
static U32_T			hsur2ErrTimeStop;
static U8_T				hsur2ErrBlocking;
static U8_T				hsur2IntrEnbType;
static U32_T			hsur2RxTxOperationErr;
static U32_T			hsur2RingFull;
static U32_T			hsur2ErrCount;
static U8_T				ur2DsrXCFTL;

static UR2_TX_PAGE_S	hsur2TxPageInfo[UR2_TX_PAGE_NUM] = {0};
static U8_T				hsur2TxEnqPage = 0;
static U8_T				hsur2TxDeqPage = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void		hsur2_IntRegWrite(U8_T addr, U8_T *ptData, U8_T dataLen);
static void		hsur2_IntRegRead(U8_T addr, U8_T *ptData, U8_T dataLen);
static void		hsur2_EnableXmit(void);
static void		hsur2_DisableXmit(void);
static void		hsur2_ReadLsr(void);
static void		hsur2_ReadMsr(void);
static void		hsur2_DmaIntrStatusCheck(void);
static void		hsur2_RxDmaOccupTrigEvent(void);
static void		hsur2_RxDmaCharTimeoutEvent(void);

/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void hsur2_IntRegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Write data bytes into a register in interrupt.
 * Params : addr:An index address to UART2 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be written.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur2_IntRegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	while (dataLen--)
	{
		U2DR = *(ptData + dataLen);
	}
	U2CIR = addr;
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur2_IntRegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Read data bytes from a register in interrupt.
 * Params : addr:An index address to UART2 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be read.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur2_IntRegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	U2CIR = addr;
	while (dataLen--)
	{
		*(ptData + dataLen) = U2DR;
	}
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur2_EnableXmit(void)
 * Purpose: Turn on the interrupt of transmitting FIFO empty in HSIER.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur2_EnableXmit(void)
{
	U8_T	n;
	BIT		oldEintBit = EA;

	EA = 0;

	U2CIR = HSIER;
	n = U2DR;
	n |= HSIER_TFEI_ENB;
	U2DR = n;
	U2CIR = HSIER;

	EA = oldEintBit;
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur2_DisableXmit(void)
 * Purpose: Turn off the interrupt of transmitting FIFO empty in HSIER.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur2_DisableXmit(void)
{
	U8_T	n;
	BIT		oldEintBit = EA;

	EA = 0;

	U2CIR = HSIER;
	n = U2DR;
	n &= ~HSIER_TFEI_ENB;
	U2DR = n;
	U2CIR = HSIER;

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
static void hsur2_ReadLsr(void)
{
	U8_T	lineStatus;
	U8_T	fifoCtrl;
	U8_T	intrEnb;
	U16_T	reg16b;
        U32_T   reg32b;

	hsur2_IntRegRead(HSLSR, &lineStatus, 1);
	hsur2LineStatus = lineStatus;
	if (lineStatus & HSLSR_OE_OVER)
	{
		hsur2ErrCount ++;
		// Overrun Error
	}
	else if (lineStatus & HSLSR_PE_ERROR)
	{
		hsur2ErrCount ++;
		// Parity Error
	}
	else if (lineStatus & HSLSR_FE_ERROR)
	{
		hsur2ErrCount ++;
		// Framing Error
	}
	else if (lineStatus & HSLSR_BI_INT)
	{
		hsur2ErrCount ++;
		// Break Interrupt Occured
	}
	else if (lineStatus & HSLSR_FERR_ERROR)
	{
		hsur2ErrCount ++;
		// Mixing Error
	}

	/* For Error handling before data synchrony */
	if (hsur2ErrCount)
	{
		/* Enable the Receiver FIFO STOP (RSTOP) bit to stop data characters transfer into RX FIFO.
		   And Reset Rx FIFO and TX FIFO */
		fifoCtrl = (hsur2FifoCtrl | HSFCR_RSTOP | HSFCR_RFR | HSFCR_TFR);
		hsur2_IntRegWrite(HSFCR, &fifoCtrl, 1);
		/* Read the error data pointer */
		hsur2_IntRegRead(HSRBDEP, (U8_T *)&reg16b, 2);
		/* Flush all characters in the RX buffer ring by HSRBRP = HSRBWP when UART2 is in DMA mode */
		reg16b = 0;
		hsur2_IntRegRead(HSRBWP, (U8_T *)&reg16b, 2);
		hsur2_IntRegWrite(HSRBRP, (U8_T *)&reg16b, 2);
		reg32b = (U32_T)ptHsur2RxDmaRingStart & 0xFF0000;
		reg32b += (U32_T)reg16b;
		ptHsur2RxDmaRingSwRead = reg32b;

		/* Disable the Receiver FIFO STOP (RSTOP) bit to re-start RX FIFO */
		fifoCtrl = (hsur2FifoCtrl | HSFCR_RFR | HSFCR_TFR);
		fifoCtrl &= ~HSFCR_RSTOP;
		hsur2_IntRegWrite(HSFCR, &fifoCtrl, 1);

		if (hsur2ErrCount >= 100)
			{
			hsur2ErrBlocking = 1;
			/* Disable the UART2 interrupt */
			intrEnb = 0;
			hsur2_IntRegRead(HSIER, &intrEnb, 1);
		}
	}
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur2_ReadMsr(void)
 * Purpose: Read the Modem Status Register to record in a corresponding interrupt.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur2_ReadMsr(void)
{
	U8_T	modemStatus = 0;

	hsur2_IntRegRead(HSMSR, &modemStatus, 1);
	hsur2ModemStatus = modemStatus;
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur2_DmaIntrStatusCheck(void)
 * Purpose: Read the DMA Status Register to record in a corresponding interrupt.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
static void hsur2_DmaIntrStatusCheck(void)
{
	U8_T	ur2DmaIntrStatus = 0;
	U8_T	regDcr = 0;
	
	hsur2_IntRegRead(HSDSR, &ur2DmaIntrStatus, 1);
	hsur2_IntRegRead(HSSFCGR, &hsur2SwFlowCtrlGap, 1);
	
	if (ur2DmaIntrStatus & HSDSR_TDC_CPL)
	{
		hsur2TxDmaComplete = 1; // Clear the  Tx DMA busy flag
	}
	if (ur2DmaIntrStatus & HSDSR_XCFTL)
	{
		ur2DsrXCFTL = 1;
	}
	if (ur2DmaIntrStatus & HSDSR_RBTDE)
	{
		hsur2RxTxOperationErr ++;
	}
	if (ur2DmaIntrStatus & HSDSR_RBRF)
	{
		hsur2RingFull ++;
	}
	if (ur2DmaIntrStatus & HSDSR_ECRRT)
	{
		if (hsur2SwFlowCtrlGap == 0)
		{
			U2CIR = HSDCR;
			regDcr = U2DR;
			regDcr |= HSDCR_PRERF;
			U2DR = regDcr;
			U2CIR = HSDCR;
		}
	}
	if (ur2DmaIntrStatus & HSDSR_FCCR)
	{
		if (ur2DmaIntrStatus & HSDSR_FCCRS) // Receuved Xoff
		{
			hsur2XoffReceived = 1;
		}
		else // Received Xon
		{
			hsur2XonReceived = 1;
		}
	}
	if (ur2DmaIntrStatus & HSDSR_WAKE_UP)
	{
		HSUR2_WakeUpEvent();
	}
}

/*
 *--------------------------------------------------------------------------------
 * void hsur2_RxDmaOccupTrigEvent(void)
 * Purpose: When the RX buffer ring has many valid datas and reachs occupying
 *          trigger level, asserting a flag to notice applications.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void hsur2_RxDmaOccupTrigEvent(void)
{
	U8_T	reg8b;
	BIT		oldEintBit = EA;

	EA = 0;
	/* turn off RDI interrupt to avoid ISR triggered without end */
	hsur2_IntRegRead(HSIER, &reg8b, 1);
	reg8b &= ~HSIER_RDI_ENB;
	hsur2_IntRegWrite(HSIER, &reg8b, 1);

	hsur2RxDataAvail = 1;
	
	EA = oldEintBit;
}

/*
 *--------------------------------------------------------------------------------
 * void hsur2_RxDmaCharTimeoutEvent(void)
 * Purpose: When the RX buffer ring has one byte valid data at least,
 *          asserting a flag to notice applications.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void hsur2_RxDmaCharTimeoutEvent(void)
{
	U8_T	reg8b;
	BIT		oldEintBit = EA;

	EA = 0;
	/* turn off RDI interrupt to avoid ISR triggered without end */
	hsur2_IntRegRead(HSIER, &reg8b, 1);
	reg8b &= ~HSIER_RDI_ENB;
	hsur2_IntRegWrite(HSIER, &reg8b, 1);
	
	hsur2RxDataAvail = 1;
	
	EA = oldEintBit;
}


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Write data bytes into a register with interrupt locked.
 * Params : addr:An index address to UART2 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be written.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR2_RegWrite(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	BIT		oldEintBit = EA;

	EA = 0;
	while (dataLen--)
	{
		U2DR = *(ptData + dataLen);
	}
	U2CIR = addr;
	EA = oldEintBit;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
 * Purpose: Read data bytes from a register with interrupt locked.
 * Params : addr:An index address to UART2 register.
 *          ptData:A pointer to indicate the register data.
 *          dataLen:A number of bytes to indicate how many bytes will be read.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR2_RegRead(U8_T addr, U8_T *ptData, U8_T dataLen)
{
	BIT		oldEintBit = EA;

	EA = 0;
	U2CIR = addr;
	while (dataLen--)
	{
		*(ptData + dataLen) = U2DR;
	}
	EA = oldEintBit;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
 * Purpose: Use this function can initialize the UART2 basic mode. It will configure
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
void HSUR2_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
{
	U8_T	lineCtrl=0, dll=0, dlh=0;
	U16_T	i, j;

	/* Set UART2 Baudrate */
	lineCtrl = HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lineCtrl, 1);
	dll = (U8_T)(divisor & 0x00FF);
	dlh = (U8_T)((divisor & 0xFF00) >> 8);
	HSUR2_RegWrite(HSDLLR, &dll, 1);
	HSUR2_RegWrite(HSDLHR , &dlh, 1);
	lineCtrl &= ~HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lineCtrl, 1);
	/* Set Line Control Register */
	lCtrl &= ~HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lCtrl, 1);
	/* Set FIFO Control Register */
	hsur2FifoCtrl = fCtrl | HSFCR_RSTOP; // Enable the RSTOP bit to stop the data characters into RX FIFO.
	hsur2FifoCtrl &= ~(HSFCR_RFR | HSFCR_TFR);
	HSUR2_RegWrite(HSFCR, &fCtrl, 1);
	/* Set Modem Control Register */
	HSUR2_RegWrite(HSMCR, &mCtrl, 1);
	/* Disable DMA Mode */
	dll = 0;
	HSUR2_RegWrite(HSDCR, &dll, 1);
	/* Initial Variables */
	for (i=0 ; i < UR2_TX_PAGE_NUM ; i++)
	{
        hsur2TxPageInfo[i].txBusyFlag = UR2_BUF_EMPTY;
        hsur2TxPageInfo[i].txDataLen = 0;
		for (j=0 ; j<UR2_TX_PAGE_BUF ; j++)
			hsur2TxPageInfo[i].txBufArray[j] = 0;
	}
	for (i=0 ; i<UR2_RX_PAGE_BUF ; i++)
	{
		hsur2RxBuffer[i] = 0;
	}
    HSUR2_RxDmaControlInit(hsur2RxBuffer, (hsur2RxBuffer+UR2_RX_PAGE_BUF-1), UR2_RX_TRIGGER_LEVEL, UR2_RX_TIMEOUT_GAP);
	/*	Set FIFO Control Register with users' trig level.
		Enable the RSTOP bit to stop the data characters into RX FIFO. */
	hsur2FifoCtrl = (fCtrl | HSFCR_HSUART_ENB | HSFCR_FIFOE | HSFCR_RFR | HSFCR_TFR);
	hsur2FifoCtrl &= ~HSFCR_RSTOP;
	HSUR2_RegWrite(HSFCR, &fCtrl, 1);

	hsur2ModemCtrl = mCtrl;
	hsur2LineStatus = 0;
	hsur2ErrCount = 0;
	hsur2IntrEnbType = intEnb;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_RxDmaControlInit(U8_T *ptRxRingStart, U8_T *ptRxRingEnd,
 *      U16_T rxRingOccupancyTrigLevel, U16_T rxRingTimeoutGap)
 * Purpose: Initialize the UART2 RX buffer ring in DMA mode. It will configure the
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
BOOL HSUR2_RxDmaControlInit(U8_T *ptRxRingStart, U8_T *ptRxRingEnd,
	 U16_T rxRingOccupancyTrigLevel, U16_T rxRingTimeoutGap)
{
	U32_T	temp32b = 0;
	U16_T	temp16b = 0;
	U8_T	temp8b = 0;

	if (((U8_T)ptRxRingStart != 0) || ((U8_T)ptRxRingEnd != 0xFF))
		return FALSE;

	ptHsur2RxDmaRingStart = ptRxRingStart;
	temp32b = ((U32_T)ptRxRingStart - 0x010000);
	temp32b <<= 8;
	HSUR2_RegWrite(HSRBSP, (U8_T *)&temp32b, 2);

	ptHsur2RxDmaRingEnd = ptRxRingEnd;
	temp32b = ((U32_T)ptRxRingEnd - 0x010000);
	temp32b <<= 8;
	HSUR2_RegWrite(HSRBEP, (U8_T *)&temp32b, 2);

	ptHsur2RxDmaRingSwRead = ptRxRingStart;
	temp32b = ((U32_T)ptRxRingStart - 0x010000);
	temp32b <<= 16;
	temp32b &= 0xFF000000;
	HSUR2_RegWrite(HSRBRP, (U8_T *)&temp32b, 2);

	HSUR2_RegWrite(HSRBOTL, (U8_T *)&rxRingOccupancyTrigLevel, 2);

	HSUR2_RegWrite(HSRBRTG, (U8_T *)&rxRingTimeoutGap, 2);
	
	/* Enable Receive Buffer Ring */
	HSUR2_RegRead(HSDCR, &temp8b, 1);
	temp8b |= HSDCR_RBRE_ENB;
	HSUR2_RegWrite(HSDCR, &temp8b, 1);
	
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_Start(void)
 * Purpose: Start function will enable the interrupt sources that be configured
 *          in the setup function. This function should be closely called before
 *          running application tasks.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_Start(void)
{
	/* Enable the UART2 interrupt */
	HSUR2_RegWrite(HSIER, &hsur2IntrEnbType, 1);
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_ErrorRecovery(void)
 * Purpose: Checking the UART2 Line Status Register value of errors and
 *          re-enable interrupts of receiving and line status.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_ErrorRecovery(void)
{
	U8_T	valLsr;
	U8_T	valFcr;
	U16_T	reg16b;
    U32_T   reg32b;

	while (hsur2ErrBlocking)
	{
		printd("hsur2ErrBlocking = 1\n\r");
		HSUR2_RegRead(HSLSR, &valLsr, 1);
		if (valLsr & HSLSR_DR_DATA)
		{
			/* Enable the Receiver FIFO STOP (RSTOP) bit to stop data characters transfer into RX FIFO.
			   And Reset Rx FIFO and TX FIFO */
			valFcr = (hsur2FifoCtrl | HSFCR_RSTOP | HSFCR_RFR | HSFCR_TFR);
			hsur2_IntRegWrite(HSFCR, &valFcr, 1);
			/* Read the error data pointer */
			hsur2_IntRegRead(HSRBDEP, (U8_T *)&reg16b, 2);
			/* Flush all characters in the RX buffer ring by HSRBRP = HSRBWP when UART2 is in DMA mode */
			reg16b = 0;
			hsur2_IntRegRead(HSRBWP, (U8_T *)&reg16b, 2);
			hsur2_IntRegWrite(HSRBRP, (U8_T *)&reg16b, 2);
			reg32b = (U32_T)ptHsur2RxDmaRingStart & 0xFF0000;
		    reg32b += (U32_T)reg16b;
		    ptHsur2RxDmaRingSwRead = reg32b;
			/* Disable the Receiver FIFO STOP (RSTOP) bit to re-start RX FIFO */
			valFcr = (hsur2FifoCtrl | HSFCR_RFR | HSFCR_TFR);
			valFcr &= ~HSFCR_RSTOP;
			hsur2_IntRegWrite(HSFCR, &valFcr, 1);
		}
		else
		{
			printd("clear hsur2ErrBlocking = 0\n\r");
			EA = 0;
			hsur2ErrBlocking = 0;
			hsur2ErrCount = 0;
			/* Enable the UART2 interrupt again */
			HSUR2_RegWrite(HSIER, &hsur2IntrEnbType, 1);
			EA = 1;
		}
	}
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_IntrEntryFunc(void)
 * Purpose: An interrupt service rounine that should be included in MCPU peripheral
 *          interface interrupt service routine within the AX220xx MCPU module.
 *          It will be trigger by interrupt events that be incicated in the HSIIR register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_IntrEntryFunc(void)
{
	U8_T	intrStatus = 0;

	hsur2_IntRegRead(HSIIR, &intrStatus, 1);
	intrStatus &= 0x1F;

	if (intrStatus == HSIIR_RLS_INTR)
	{
		hsur2_ReadLsr();
	}
	else if (intrStatus == HSIIR_DMAS_INTR)
	{
		hsur2_DmaIntrStatusCheck();
	}
	else if (intrStatus == HSIIR_RD_TRIG_INTR)
	{
		hsur2_RxDmaOccupTrigEvent();
	}
	else if (intrStatus == HSIIR_RD_TI_INTR)
	{
		hsur2_RxDmaCharTimeoutEvent();
	}
	else if (intrStatus == HSIIR_TFE_INTR)
	{
		/* This status will not be occurred in Tx DMA mode */
	}
	else if (intrStatus == HSIIR_MS_INTR)
	{
		hsur2_ReadMsr();
	}
	else
	{

	}
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_RxDmaSwReadPointerUpdated(U16_T lenCount)
 * Purpose: release the RX buffer ring memory that have been retrieved.
 * Params : lenCount : A length that indicates how many valid data can be retrieved.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR2_RxDmaSwReadPointerUpdated(U16_T lenCount)
{
	U32_T	temp32b = 0;
	U8_T	temp8b = 0;

	ptHsur2RxDmaRingSwRead = ptHsur2RxDmaRingSwRead + lenCount;

	if (ptHsur2RxDmaRingSwRead > ptHsur2RxDmaRingEnd)
		ptHsur2RxDmaRingSwRead = ptHsur2RxDmaRingStart + (ptHsur2RxDmaRingSwRead - ptHsur2RxDmaRingEnd - 1);

	temp32b = ((U32_T)ptHsur2RxDmaRingSwRead - 0x010000);
	temp32b <<= 16;

	HSUR2_RegWrite(HSRBRP, (U8_T *)&temp32b, 2);
	
	/* Re-enable the RxDMA interrupt */
	HSUR2_RegRead(HSIER, &temp8b, 1);
	temp8b |= HSIER_RDI_ENB;
	HSUR2_RegWrite(HSIER, &temp8b, 1);

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * U16_T HSUR2_GetRxDmaAvailCount(void)
 * Purpose: The number of valid data bytes in Receive Buffer Ring.
 *          Check this to determine how many bytes have been received.
 *          This function will directly read a register HSRBABC (0x20) to return.
 * Params : None.
 * Returns: available bytes, HSRBABC (0x20).
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
U16_T HSUR2_GetRxDmaAvailCount(void)
{
	U16_T	reg16b;

	if ((hsur2RxDataAvail+hsur2RingFull) == 0)
	{
		return 0;
	}

	/* update the HSRBABC to get the available count of Rx ring*/
	HSUR2_RegRead(HSRBABC, (U8_T *)&reg16b, 2);

	return reg16b;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_TxDmaTranmit(U8_T *ptTx, U16_T length)
 * Purpose: This function will directly transmit characters in TX buffer memory.
 *          Give a start address of TX buffer with a length to the hardware
 *          for DMA copying and get a status to check.
 * Params : ptTx: A point to indicate the TX buffer on xDATA momory.
 *          length: The length that will be transmitted by TX DMA.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR2_TxDmaTranmit(U8_T *ptTx, U16_T length)
{
	U32_T	temp32b;
	U8_T	temp8b;

	/* if TxDMA still busy, give up */
    do
    {
        HSUR2_RegRead(HSDCR, &temp8b, 1);
    } while (temp8b & HSDCR_STD);

	temp32b = ((U32_T)ptTx - 0x010000); // for Keil C pointer convert
	temp32b <<= 8;
	HSUR2_RegWrite(HSTDAR, (U8_T *)&temp32b, 3);
	
	HSUR2_RegWrite(HSTDBCR, (U8_T *)&length, 2);

	temp8b |= HSDCR_STD;
	HSUR2_RegWrite(HSDCR, &temp8b, 1);

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * U8_T HSUR2_GetLineStatus(U8_T intrSynth)
 * Purpose: This function will return a variable of the UART2 Line Status Register.
 * Params : intrSynth: Set 1 to get the Line Status in the interrupt recording.
 *                     Set 0 to directly read the Line Status Register (HSLSR).
 * Returns: valLsr: A variable of the UART2 Line Status Register.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
U8_T HSUR2_GetLineStatus(U8_T intrSynth)
{
	U8_T valLsr;

	if (intrSynth)
		valLsr = hsur2LineStatus & 0x9E;
	else
		HSUR2_RegRead(HSLSR, &valLsr, 1);

	return valLsr;
}

/*
 * ----------------------------------------------------------------------------
 * U8_T HSUR2_GetModemStatus(U8_T intrSynth)
 * Purpose: Getting the UART2 Modem Status .
 * Params : intrSynth: 1: Get MSR according to an interrupt trigger.
 *                       0: Get MSR by directly reading the register.
 * Returns: varModemStatus: Modem status register variable.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T HSUR2_GetModemStatus(U8_T intrSynth)
{
	U8_T	varModemStatus = 0;

	if (intrSynth)
		varModemStatus = hsur2ModemStatus;
	else
		HSUR2_RegRead(HSMSR, &varModemStatus, 1);

	return varModemStatus;
}

/*
 * ----------------------------------------------------------------------------
 * U8_T HSUR2_GetLineControl(void)
 * Purpose: Getting the UART2 Line Control register variables.
 * Params : None.
 * Returns: varLineCtrl: A variable of the UART2 Line Control Register.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T HSUR2_GetLineControl(void)
{
	U8_T	varLineCtrl = 0;

	HSUR2_RegRead(HSLCR, &varLineCtrl, 1);

	return varLineCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * U8_T HSUR2_GetFifoControl(void)
 * Purpose: Getting the UART2 FIFO Control register variables.
 * Params : None.
 * Returns: hsur2FifoCtrl: A variable of the UART2 FIFO Control Register in initialization.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T HSUR2_GetFifoControl(void)
{
	return hsur2FifoCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * U8_T HSUR2_GetModemControl(void)
 * Purpose: Getting the UART2 Modem Control register variables.
 * Params : None.
 * Returns: hsur2ModemCtrl:A variable of the UART2 Modem Control Register in initialization.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
U8_T HSUR2_GetModemControl(void)
{
	return hsur2ModemCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_SetLineControl(U8_T value)
 * Purpose: Setting the UART2 Line control register with a new value.
 * Params : value: variable to be set.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_SetLineControl(U8_T value)
{
	U8_T	lCtrl = value;
	
	HSUR2_RegWrite(HSLCR, &lCtrl, 1);
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_SetFifoControl(U8_T value)
 * Purpose: Setting the UART2 FIFO control register with a new value.
 * Params : value: variable to be set.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_SetFifoControl(U8_T value)
{
	U8_T	fCtrl = value;
	
	HSUR2_RegWrite(HSFCR, &fCtrl, 1);
	
	hsur2FifoCtrl = fCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_SetModemControl(U8_T value)
 * Purpose: Setting the UART2 Modem control register with a new value.
 * Params : value: variable to be set.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_SetModemControl(U8_T value)
{
	U8_T	mCtrl = value;
	
	HSUR2_RegWrite(HSMCR, &mCtrl, 1);
	
	hsur2ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_SetRTS(void)
 * Purpose: Only setting the UART2 RTS bit of Modem control register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_SetRTS(void)
{
	U8_T	mCtrl;

	HSUR2_RegRead(HSMCR, &mCtrl, 1);
	mCtrl |= HSMCR_RTS;
	HSUR2_RegWrite(HSMCR, &mCtrl, 1);
	
	hsur2ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_SetDTR(void)
 * Purpose: Only setting the UART2 DTR bit of Modem control register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_SetDTR(void)
{
	U8_T	mCtrl;

	HSUR2_RegRead(HSMCR, &mCtrl, 1);
	mCtrl |= HSMCR_DTR;
	HSUR2_RegWrite(HSMCR, &mCtrl, 1);
	
	hsur2ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_ClearRTS(void)
 * Purpose: Only clear the UART2 RTS bit of Modem control register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_ClearRTS(void)
{
	U8_T	mCtrl;

	HSUR2_RegRead(HSMCR, &mCtrl, 1);
	mCtrl &= ~HSMCR_RTS;
	HSUR2_RegWrite(HSMCR, &mCtrl, 1);
	
	hsur2ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_ClearDTR(void)
 * Purpose: Only clear the UART2 DTR bit of Modem control register.
 * Params : None.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_ClearDTR(void)
{
	U8_T	mCtrl;

	HSUR2_RegRead(HSMCR, &mCtrl, 1);
	mCtrl &= ~HSMCR_DTR;
	HSUR2_RegWrite(HSMCR, &mCtrl, 1);
	
	hsur2ModemCtrl = mCtrl;
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_SetupPort(U16_T divBaudRate, U8_T lCtrl)
 * Purpose: Setting the UART2 baudrate.
 * Params : divBaudRate: The divisor of the baudrate.
 *          lCtrl: A Line Control Register value.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_SetupPort(U16_T divBaudRate, U8_T lCtrl)
{
	U8_T	dll, dlh, lineCtrl;

	/* Set UART2 Baudrate */
	lineCtrl = HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lineCtrl, 1);
	dll = (U8_T)(divBaudRate & 0x00FF);
	dlh = (U8_T)((divBaudRate & 0xFF00) >> 8);
	HSUR2_RegWrite(HSDLLR, &dll, 1);
	HSUR2_RegWrite(HSDLHR , &dlh, 1);
	lineCtrl &= ~HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lineCtrl, 1);
	/* Set Line Control Register */
	lCtrl &= ~HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lCtrl, 1);
}

/*
 * ----------------------------------------------------------------------------
 * void HSUR2_SetBaudRate(U16_T divBaudRate)
 * Purpose: Setting the UART2 baudrate.
 * Params : divBaudRate: The divisor of the baudrate.
 * Returns: None.
 * Note   : None.
 * ----------------------------------------------------------------------------
 */
void HSUR2_SetBaudRate(U16_T divBaudRate)
{
	U8_T	dll, dlh, lineCtrl;

	/* Set UART2 Baudrate */
	lineCtrl = HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lineCtrl, 1);
	dll = (U8_T)(divBaudRate & 0x00FF);
	dlh = (U8_T)((divBaudRate & 0xFF00) >> 8);
	HSUR2_RegWrite(HSDLLR, &dll, 1);
	HSUR2_RegWrite(HSDLHR , &dlh, 1);
	lineCtrl &= ~HSLCR_DLAB_ENB;
	HSUR2_RegWrite(HSLCR, &lineCtrl, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_AutoCtsRtsFlowCtrl(U8_T set)
 * Purpose: To enable/disable the automatic CTS/RTS flow control executed by hardware.
 * Params : set: A value 1 to enable the auto-CTS/RTS flow control function.
 *               A value 0 to disable.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_AutoCtsRtsFlowCtrl(U8_T set)
{
	U8_T	reg8b;
	
	/* If turn HW CTS/RTS, the software flow control need not be enabled */
	HSUR2_RegRead(HSAFCSR, &reg8b, 1);
	if (set)
		reg8b = HSAFCSR_ACTSE_ENB | HSAFCSR_ARTSE_ENB; // only use HW CTS/RTS */
	else
		reg8b &= ~(HSAFCSR_ACTSE_ENB | HSAFCSR_ARTSE_ENB);
	HSUR2_RegWrite(HSAFCSR, &reg8b, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_AutoSoftwareFlowCtrl(U8_T setFlow, U8_T modeFlow, U16_T chXon, U16_T chXoff, U16_T chEsc)
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
void HSUR2_AutoSoftwareFlowCtrl(U8_T setFlow, U8_T modeFlow, U16_T chXon, U16_T chXoff, U16_T chEsc)
{
	U8_T	reg8b;
	
	/* If turn HW CTS/RTS, the software flow control need not be enabled */
	HSUR2_RegRead(HSAFCSR, &reg8b, 1);
	if (setFlow)
	{
		reg8b = HSAFCSR_ASFCE_ENB; // only use Automatic Software flow control Enable
		
		HSUR2_RegWrite(HSXONR, (U8_T *)&chXon, 2);

		HSUR2_RegWrite(HSXOFFR, (U8_T *)&chXoff, 2);
		
		HSUR2_RegWrite(HSECR, (U8_T *)&chEsc, 2);
			
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
	HSUR2_RegWrite(HSAFCSR, &reg8b, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_WakeUpEvent(void)
 * Purpose: An interrupt service rounine that should be included in MCPU
 *          Wake-Up interrupt service routine within the AX220xx MCPU module.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_WakeUpEvent(void)
{
	U8_T	reg8b;

	hsur2_IntRegRead(HSDSR, &reg8b, 1);
	// Add wakeup process from here...
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_ForceSendCtrlXon(void)
 * Purpose: Force AX220xx UART2 to send the Xon character with single or double.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_ForceSendCtrlXon(void)
{
	U8_T	reg8b;
	
	HSUR2_RegRead(HSAFCSR,&reg8b, 1);
	reg8b |= HSAFCSR_SXON;
	HSUR2_RegWrite(HSAFCSR,&reg8b, 1);
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_ForceSendCtrlXoff(void)
 * Purpose: Force AX220xx UART2 to send the Xoff character with single or double.
 * Params : None.
 * Returns: None.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
void HSUR2_ForceSendCtrlXoff(void)
{
	U8_T	reg8b;
	
	HSUR2_RegRead(HSAFCSR,&reg8b, 1);
	reg8b |= HSAFCSR_SXOFF;
	HSUR2_RegWrite(HSAFCSR,&reg8b, 1);
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_XmitOutEnq(U8_T* pAppData, U16_T appLength)
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
BOOL HSUR2_XmitOutEnq(U8_T* pAppData, U16_T appLength)
{
    U16_T txUr2Len;

    while (appLength)
    {
	    /* Check the transmitting buffer for free to use */
	    if (hsur2TxPageInfo[hsur2TxEnqPage].txBusyFlag & UR2_BUF_BUSY)
        {
		    return FALSE;
        }
        txUr2Len = appLength;
        if (txUr2Len > UR2_TX_PAGE_BUF)
        {
            txUr2Len = UR2_TX_PAGE_BUF;
        }
        appLength -= txUr2Len;

	    hsur2TxPageInfo[hsur2TxEnqPage].txBusyFlag |= UR2_BUF_BUSY;
	    hsur2TxPageInfo[hsur2TxEnqPage].txDataLen = txUr2Len;
        if (txUr2Len)
	        DMA_Grant(hsur2TxPageInfo[hsur2TxEnqPage].txBufArray, pAppData, txUr2Len);

	    pAppData += txUr2Len;
	    hsur2TxEnqPage ++;
	    if (hsur2TxEnqPage >= UR2_TX_PAGE_NUM)
		    hsur2TxEnqPage = 0;
    }
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL HSUR2_XmitOutDeq(void)
 * Purpose: A transmitting de-queue function is used by an application up-layer.
 *          The up-layer software can call this function to transmit data from the
 *          TX buffer to FIFO. This function will use the function, HSUR2_TxDmaTranmit( ),
 *          to send data stream out. An application must include this de-queue
 *          function into a transmitting task that should have a responsibility
 *          for sending data to serial bus.
 * Params : None.
 * Returns: TRUE (1) is success ; FALSE (0) is failure.
 * Note   : None.
 *--------------------------------------------------------------------------------
 */
BOOL HSUR2_XmitOutDeq(void)
{
	U8_T	*pTxOut;
	U16_T	txLength;
		
	/* Check the transmitting buffer for valid to send */
	if (hsur2TxPageInfo[hsur2TxDeqPage].txBusyFlag == UR2_BUF_EMPTY)
		return FALSE;

    pTxOut = hsur2TxPageInfo[hsur2TxDeqPage].txBufArray;
    txLength = hsur2TxPageInfo[hsur2TxDeqPage].txDataLen;
    HSUR2_TxDmaTranmit(pTxOut, txLength);
    hsur2TxPageInfo[hsur2TxDeqPage].txBusyFlag = UR2_BUF_EMPTY;

    hsur2TxDeqPage ++;
    if (hsur2TxDeqPage >= UR2_TX_PAGE_NUM)
        hsur2TxDeqPage = 0;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR2_CopyUartToApp(U8_T* pAppData, U16_T appGetLen)
 * Purpose: If the up-layer application has a memory resource as a RX buffer,
 *          this function can help to copy the received data block from the firmware
 *          receiving buffer ring to up-layer buffer by a SW DMA function. After
 *          completing the DMA copying, the function, HSUR2_RxDmaSwReadPointerUpdated,
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
void HSUR2_CopyUartToApp(U8_T* pAppData, U16_T appGetLen)
{
	U8_T	*pRxStart, *pRxCur, *pRxEnd;
	U16_T	rxCount, tempCount1, tempCount2;
    BIT	oldEintBit;

    if (!appGetLen)
        return;

	rxCount = appGetLen;
	pRxStart = ptHsur2RxDmaRingStart;
	pRxCur = ptHsur2RxDmaRingSwRead;
	pRxEnd = ptHsur2RxDmaRingEnd;
	
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

	HSUR2_RxDmaSwReadPointerUpdated(rxCount);
    oldEintBit = EA;
	EA = 0;
    hsur2RingFull=0;
	hsur2RxDataAvail = 0;
	EA = oldEintBit;
}
/* End of hsuart2.c */
