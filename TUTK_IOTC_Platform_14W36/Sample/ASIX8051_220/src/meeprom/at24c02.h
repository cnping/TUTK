/*
 *********************************************************************************
 *     Copyright (c) 2010	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : at24c02.h
 * Purpose     : 
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * $Log: at24c02.h,v $
 * no message
 *
 *================================================================================
 */
#ifndef AT24C01A_H
#define AT24C01A_H

/* INCLUDE FILE DECLARATIONS */
#include "i2c.h"


/* NAMING CONSTANT DECLARATIONS */


/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */
extern U32_T	DbgLostCount;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	EEPROM_Init(void);
BOOL	ByteWrite(U16_T AddrOfDec, U16_T AddrOfMem, U8_T ByteData, U8_T EndCond);
BOOL	PageWrite(U16_T AddrOfDev, U16_T AddrOfMem, U8_T *ptPageData, U16_T WriteLen, U8_T EndCond);
BOOL	CurRead(U16_T AddrOfDev, I2C_BUF *ptRxPkt);
BOOL	DummyWrite(U16_T AddrOfDev, U16_T AddrOfMem, U8_T EndCond);
BOOL	RdmRead(U16_T AddrOfDev, U16_T AddrOfMem ,I2C_BUF *ptRxPkt, U16_T ReadLen, U8_T EndCond);
U8_T	EEPROM_Read(U16_T addr, U8_T len, U8_T* pBuf);
U8_T	EEPROM_Write(U16_T addr, U8_T* pBuf);
U8_T	EEPROM_BatchWrite(U16_T addr, U16_T len, U8_T* pBuf);

#endif /* End of AT24C01A_H */
