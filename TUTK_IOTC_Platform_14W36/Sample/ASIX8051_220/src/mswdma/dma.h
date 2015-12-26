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
 * Module Name:dma.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

#ifndef __DMA_H__
#define __DMA_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "dma_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
/* SFR 0x94 definitions*/
#define DMA_COMPLETE			BIT0

/* DMA register definitions */
#define SW_DMA_CMD_REG			0x00
#define SW_DMA_SOURCE_ADDR_REG	0x02
#define SW_DMA_TARGET_ADDR_REG	0x06
#define SW_DMA_BYTE_COUNT_REG	0x0A

/* Bit definitions: SW_DMA_CMD_REG */
#define DMA_CMD_GO				BIT0
#define DMA_FORCE_STOP			BIT1
#define DMA_ERROR_BIT			BIT2
#define	DMA_SA_IN_PROG_MEM		BIT3
#define DMA_SA_IN_RPBR			BIT4
#define DMA_TA_IN_TPBR			BIT5
#define	DMA_TA_IN_PROG_MEM		BIT6
#define	DMA_COMPLETE_INT_ENABLE	BIT7

/* for software definition */
#define DMA_NORMAL				0
#define DMA_FROM_RPBR			BIT4
#define DMA_TO_TPBR				BIT5


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/*-------------------------------------------------------------*/
void DMA_InterruptService(U8_T intrStatus);
void DMA_ReadReg(U8_T, U8_T XDATA*, U8_T);
void DMA_WriteReg(U8_T, U8_T XDATA*, U8_T);
void DMA_Init(void);
void DMA_Start(void);
U8_T DMA_CheckStatus(void);
BOOL DMA_DataToData(U32_T addrSour, U32_T addrDest, U16_T length);
BOOL DMA_DataToProgram(U32_T addrSour, U32_T addrDest, U16_T length);
BOOL DMA_ProgramToProgram(U32_T addrSour, U32_T addrDest, U16_T length);
BOOL DMA_ProgramToData(U32_T addrSour, U32_T addrDest, U16_T length);
U8_T XDATA* DMA_GrantXdata(U8_T XDATA*, U8_T XDATA*, U16_T);
#if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K)
U8_T* DMA_Grant(U8_T*, U8_T*, U16_T);
#endif
/* only for uart console debug */
U8_T DMA_IndirectIO(U8_T, U16_T, U8_T XDATA*);


#endif /* End of __DMA_H__ */


/* End of dma.h */