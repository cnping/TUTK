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
 * Module Name : at24c01a.c
 * Purpose     : This module handles the I2C serial interface driver.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * $Log: at24c01a.c,v $
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"types.h"
#include	"ax22000.h"
#include	"interrupt.h"
#include	"i2c.h"
#include	"at24c02.h"
#include	"uart0.h"
#include    "printd.h"


/* STATIC VARIABLE DECLARATIONS */
static U8_T		i2cApiRxPkt[MST_MAX_PKT_NUM];
static U8_T		i2cApiTxPkt[MST_MAX_PKT_NUM];

//NDBG
extern U8_T XDATA	I2C_IntrHang;

/* LOCAL SUBPROGRAM DECLARATIONS */
U32_T	DbgLostCount = 0;


/* LOCAL SUBPROGRAM BODIES */


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * BOOL ByteWrite(U16_T AddrOfDev, U16_T AddrOfMem,
 *                U8_T ByteData, U8_T EndCond)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ByteWrite(U16_T AddrOfDev, U16_T AddrOfMem, U8_T ByteData, U8_T EndCond)
{
	I2C_BUF		*ptTxPkt = NULL;
	U8_T		AddrMode = 0;

	ptTxPkt = (I2C_BUF *)i2cApiTxPkt;
	ptTxPkt->I2cEnd = EndCond;
	ptTxPkt->I2cDir = I2C_MST_XMIT;
	ptTxPkt->DataLen = 0x02;

	/* Device Address */
	I2C_Cmd(SI_RD, I2CCTL, &AddrMode);
	if (AddrMode & I2C_10BIT)
	{
		ptTxPkt->I2cAddr.TenBitAddr = (U16_T)(AddrOfDev | ((AddrOfMem & 0x0700) >> 8));
	}
	else
	{
		ptTxPkt->I2cAddr.SevenBitAddr = (U8_T)(AddrOfDev | ((AddrOfMem & 0x0700) >> 8));
	}
	/* Word Address */
	ptTxPkt->I2cData[0] = (U8_T)(AddrOfMem & 0x00FF);
	/* Access Data */
	ptTxPkt->I2cData[1] = ByteData;
	I2C_FlagEnb(I2C_BUSY);
	I2C_PktBuf(ptTxPkt);
	while (I2C_FlagChk(I2C_BUSY))
	{
	}
	if (I2C_FlagChk(I2C_NACK))
	{
		I2C_FlagClr(I2C_NACK);
		return FALSE;
	}
	if (I2C_FlagChk(I2C_LOST))
	{
		I2C_FlagClr(I2C_LOST);
		DbgLostCount ++;
		return FALSE;
	}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL PageWrite(U16_T AddrOfDev, U16_T AddrOfMem,
 *                U16_T WriteLen, U8_T EndCond)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL PageWrite(U16_T AddrOfDev, U16_T AddrOfMem, U8_T *ptPageData, U16_T WriteLen, U8_T EndCond)
{
	
	I2C_BUF XDATA	*ptTxPkt = NULL;
	U16_T			i;
	U8_T XDATA		AddrMode = 0;

	ptTxPkt = (I2C_BUF *)i2cApiTxPkt;
	ptTxPkt->I2cEnd = EndCond;
	ptTxPkt->I2cDir = I2C_MST_XMIT;
	ptTxPkt->DataLen = WriteLen + 1;
	/* Device Address */
	I2C_Cmd(SI_RD, I2CCTL, &AddrMode);
	if (AddrMode & I2C_10BIT)
	{
		ptTxPkt->I2cAddr.TenBitAddr = (U16_T)(AddrOfDev | ((AddrOfMem & 0x0700) >> 8));
	}
	else
	{
		ptTxPkt->I2cAddr.SevenBitAddr = (U8_T)(AddrOfDev | ((AddrOfMem & 0x0700) >> 8));
	}
	/* Word Address */
	ptTxPkt->I2cData[0] = (U8_T)(AddrOfMem & 0x00FF);
	/* Access Data */
	for (i = 0 ; i < WriteLen ; i ++)
	{
		ptTxPkt->I2cData[i + 1] = *(ptPageData + i);
	}
	I2C_FlagEnb(I2C_BUSY);
	I2C_PktBuf(ptTxPkt);
	while (I2C_FlagChk(I2C_BUSY))
	{

	}

	if (I2C_FlagChk(I2C_NACK))
	{
		I2C_FlagClr(I2C_NACK);
		return FALSE;
	}
	if (I2C_FlagChk(I2C_LOST))
	{
		I2C_FlagClr(I2C_LOST);
		DbgLostCount ++;
		return FALSE;
	}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL CurRead(U16_T AddrOfDev, I2C_BUF *ptRxPkt)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL CurRead(U16_T AddrOfDev, I2C_BUF *ptRxPkt)
{
	U8_T XDATA			AddrMode = 0;

	ptRxPkt = (I2C_BUF *)i2cApiRxPkt;
	ptRxPkt->I2cEnd = I2C_STOP_COND;
	ptRxPkt->I2cDir = I2C_MST_RCVR;
	ptRxPkt->DataLen = 0x01;
	/* Device Address */
	I2C_Cmd(SI_RD, I2CCTL, &AddrMode);
	if (AddrMode & I2C_10BIT)
	{
		ptRxPkt->I2cAddr.TenBitAddr = (U16_T)AddrOfDev;
	}
	else
	{
		ptRxPkt->I2cAddr.SevenBitAddr = (U8_T)AddrOfDev;
	}
	/* No Word Address */
	/* Access Data */
	I2C_FlagEnb(I2C_BUSY);
	I2C_PktBuf(ptRxPkt);
	while (I2C_FlagChk(I2C_BUSY)) {}
	if (I2C_FlagChk(I2C_NACK))
	{
		I2C_FlagClr(I2C_NACK);
		return FALSE;
	}
	if (I2C_FlagChk(I2C_LOST))
	{
		I2C_FlagClr(I2C_LOST);
		DbgLostCount ++;
		return FALSE;
	}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL DummyWrite(U16_T AddrOfDev, U16_T AddrOfMem, U8_T EndCond)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL DummyWrite(U16_T AddrOfDev, U16_T AddrOfMem, U8_T EndCond)
{
	I2C_BUF XDATA	*ptTxPkt = NULL;
	U8_T XDATA		AddrMode = 0;

	ptTxPkt = (I2C_BUF *)i2cApiTxPkt;
	ptTxPkt->I2cEnd = EndCond;
	ptTxPkt->I2cDir = I2C_MST_XMIT;
	ptTxPkt->DataLen = 0x01;
	/* Device Address */
	I2C_Cmd(SI_RD, I2CCTL, &AddrMode);
	if (AddrMode & I2C_10BIT)
	{
		ptTxPkt->I2cAddr.TenBitAddr = (U16_T)(AddrOfDev | ((AddrOfMem & 0x0700) >> 8));
	}
	else
	{
		ptTxPkt->I2cAddr.SevenBitAddr = (U8_T)(AddrOfDev | ((AddrOfMem & 0x0700) >> 8));
	}
	/* Word Address */
	ptTxPkt->I2cData[0] = (U8_T)(AddrOfMem & 0x00FF);
	/* No Access Data */
	I2C_FlagEnb(I2C_BUSY);
	I2C_FlagClr(I2C_RESTART);
	I2C_PktBuf(ptTxPkt);

	while (I2C_FlagChk(I2C_BUSY))
	{
	}
	if (I2C_FlagChk(I2C_NACK))
	{
		I2C_FlagClr(I2C_NACK);

		return FALSE;
	}
	if (I2C_FlagChk(I2C_LOST))
	{
		I2C_FlagClr(I2C_LOST);
		DbgLostCount ++;
		return FALSE;
	}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL RdmRead(U16_T AddrOfDev, U16_T AddrOfMem ,I2C_BUF *ptPktTemp, U16_T ReadLen, U8_T EndCond)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL RdmRead(U16_T AddrOfDev, U16_T AddrOfMem ,I2C_BUF *ptPktTemp, U16_T ReadLen, U8_T EndCond)
{
	I2C_BUF		*ptRxPkt = NULL;
	U16_T		i;
	U8_T XDATA	AddrMode = 0;

	ptRxPkt = (I2C_BUF *)i2cApiRxPkt;
	ptRxPkt->I2cEnd = I2C_STOP_COND;
	ptRxPkt->I2cDir = I2C_MST_RCVR;
	ptRxPkt->DataLen = ReadLen;
	/* Device Address */
	I2C_Cmd(SI_RD, I2CCTL, &AddrMode);
	if (AddrMode & I2C_10BIT)
	{
		ptRxPkt->I2cAddr.TenBitAddr = (U16_T)(AddrOfDev | ((AddrOfMem & 0x0700) >> 8));
	}
	else
	{
		ptRxPkt->I2cAddr.SevenBitAddr = (U8_T)(AddrOfDev | ((AddrOfMem & 0x0700) >> 8));
	}
	if (EndCond & I2C_STOP_COND)
	{
		if (DummyWrite(AddrOfDev, AddrOfMem, I2C_STOP_COND))
		{
			I2C_FlagEnb(I2C_BUSY);
			I2C_PktBuf(ptRxPkt);
			while (I2C_FlagChk(I2C_BUSY))
			{
			}
			if (I2C_FlagChk(I2C_NACK))
			{
				I2C_FlagClr(I2C_NACK);
				return FALSE;
			}
			if (I2C_FlagChk(I2C_LOST))
			{
				I2C_FlagClr(I2C_LOST);
				DbgLostCount ++;
				return FALSE;
			}
			for (i=0 ; i<=ReadLen ; i++)
			{
				ptPktTemp->I2cData[i] = ptRxPkt->I2cData[i];
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if (DummyWrite(AddrOfDev, AddrOfMem, EndCond))
		{
			while (!I2C_FlagChk(I2C_RESTART))
			{
				if (!I2C_FlagChk(I2C_BUSY))
				{
					if (I2C_FlagChk(I2C_NACK))
					{
						I2C_FlagClr(I2C_NACK);
						return FALSE;
					}
					if (I2C_FlagChk(I2C_LOST))
					{
						I2C_FlagClr(I2C_LOST);
						DbgLostCount ++;
						return FALSE;
					}
				}
			}

			I2C_FlagClr(I2C_RESTART);
			while (I2C_FlagChk(I2C_BUSY))
			{
			}
			if (I2C_FlagChk(I2C_NACK))
			{
				I2C_FlagClr(I2C_NACK);
				return FALSE;
			}
			if (I2C_FlagChk(I2C_LOST))
			{
				I2C_FlagClr(I2C_LOST);
				DbgLostCount ++;
				return FALSE;
			}

			I2C_FlagEnb(I2C_BUSY);
			I2C_PktBuf(ptRxPkt);
			while (I2C_FlagChk(I2C_BUSY))
			{
			}

			if (I2C_FlagChk(I2C_NACK))
			{
				I2C_FlagClr(I2C_NACK);
				return FALSE;
			}
			if (I2C_FlagChk(I2C_LOST))
			{
				I2C_FlagClr(I2C_LOST);
				DbgLostCount ++;
				return FALSE;
			}
			for (i=0 ; i<=ReadLen ; i++)
			{
				ptPktTemp->I2cData[i] = ptRxPkt->I2cData[i];
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

/*
 *--------------------------------------------------------------------------------
 * U8_T EEPROM_Init()
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void EEPROM_Init(void)
{
	U16_T	sys_clk;

	/* Enable I2C interrupt */
	EXTINT4(1);

#if 0
	/* get system clock */
	switch (AX11000_GetSysClk())
	{
		case SCS_100M :
			sys_clk = 8000;
			break;
		case SCS_50M :
			sys_clk = 4000;
			break;
		case SCS_25M :
			sys_clk = 2000;
			break;
	}
#else
	sys_clk = 100;
#endif

	sys_clk--;
	switch (CSREPR & BIT7)
	{
		case SCS_80M :
			/* I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 80M */
			I2C_Setup(I2C_ENB|I2C_FAST|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, 0x0030, 0x005A);
			printd("80M\r\n");
			break;
		case SCS_40M :
			/* I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 40M */
			I2C_Setup(I2C_ENB|I2C_FAST|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, 0x0018, 0x005A);
			printd("40M\r\n");
			break;
		default :
			/* I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 40M */
			I2C_Setup(I2C_ENB|I2C_FAST|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, 0x0030, 0x005A);
			break;
	}

} /* End of EEPROM_Init */

/*
 *--------------------------------------------------------------------------------
 * U8_T EEPROM_Read(U16_T addr, U8_T len, U8_T* pBuf)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T EEPROM_Read(U16_T addr, U8_T len, U8_T* pBuf)
{
	U16_T	i;
	I2C_BUF	dumpdata;

	dumpdata.I2cEnd = 0;
	dumpdata.I2cDir = 0;
	dumpdata.DataLen = 0;
	dumpdata.I2cAddr.SevenBitAddr = 0;
	dumpdata.I2cAddr.TenBitAddr = 0;
	for (i=0; i<MST_MAX_PKT_NUM; i++)
	{
		dumpdata.I2cData[i] = 0;
	}
	
	if (RdmRead((U16_T)(0x0050), (U16_T)addr, &dumpdata, (U16_T)len, I2C_NO_STOP))
	{
		for (i = 0 ; i < len ; i ++)
		{
			pBuf[i] = dumpdata.I2cData[i+1];
		}
	}
	else
	{
		return 0;
	}
	return 1;

}

/*
 *--------------------------------------------------------------------------------
 * U8_T EEPROM_Write(U16_T addr, U8_T len, U8_T* pBuf)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T EEPROM_Write(U16_T addr, U8_T* pBuf)
{
	return ByteWrite((U16_T)(0x0050), (U16_T)addr, *pBuf, I2C_STOP_COND);

}

/*
 *--------------------------------------------------------------------------------
 * U8_T EEPROM_BatchWrite(U16_T addr, U8_T len, U8_T* pBuf)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T EEPROM_BatchWrite(U16_T addr, U16_T len, U8_T* pBuf)
{
	return PageWrite((U16_T)(0x0050), (U16_T)addr, pBuf, len, I2C_STOP_COND);
}

/* End of at24c01a.c */
