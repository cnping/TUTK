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
 * Module Name : i2c.c
 * Purpose     : This module handles the I2C serial interface driver.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * $Log: i2c.c,v $
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"ax22000.h"
#include	"types.h"
#include	"interrupt.h"
#include	"i2c.h"


/* STATIC VARIABLE DECLARATIONS */
static U8_T		i2cCtrl = 0;
static U8_T		i2cEndCond = 0;
static U8_T		i2cActF = 0;	// indicate the condition of a transfer
static U16_T	i2cDataLen = 0;	// The I2cPktLen includes address and data
static U16_T	i2cDataLenCnt = 0;	// Packet's counter of transferring 
static U8_T		i2cPktDir = 0;	// Packet's direction
static I2C_BUF 	*ptI2cTxBuf = 0;
static I2C_BUF 	*ptI2cRxBuf = 0;
static U8_T		i2cState = 0;	// I2C master state flag


/* LOCAL SUBPROGRAM DECLARATIONS */
static void i2c_MstStatus(U8_T i2cStatus);
static void i2c_MasterXmit(U8_T wrData, U8_T mstCmd);
static void i2c_MasterRcvr(U8_T *rdData, U8_T mstCmd);


/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void i2c_MstStatus(U8_T i2cStatus)
 * Purpose : Checks the interrupt status of I2C master mode.
 * Params  : i2cStatus - master status when interrupt occured.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
static void i2c_MstStatus(U8_T i2cStatus)
{
	U8_T	lostStatus = 0;
	U16_T	lostCnt = 0;

	if (i2cPktDir == I2C_MST_XMIT)
	{
		/* Check the current byte ack */
		if (i2cStatus & I2C_ARB_LOST)
		{
			I2C_FlagEnb(I2C_LOST);

			i2cDataLenCnt = 0;

			I2C_FlagClr(I2C_BUSY);
		}
		else if ((!(i2cStatus & I2C_NO_ACK)) && (!(i2cStatus & I2C_TIP)))
		{
			if (i2cActF & I2C_STOP_COND)
			{
				i2cDataLenCnt = 0;
				I2C_FlagClr(I2C_BUSY);
				return;
			}

			if (!I2C_FlagChk(I2C_BUSY))
			{
				I2C_FlagEnb(I2C_RESTART);
			}
			else
			{
				if ((i2cCtrl & I2C_10BIT) && (i2cActF & I2C_START_COND))
				{
					i2c_MasterXmit((U8_T)(ptI2cTxBuf->I2cAddr.TenBitAddr & 0x00FF), I2C_MASTER_GO | I2C_CMD_WRITE);
				}
				else
				{
					if (i2cDataLenCnt < (i2cDataLen-1))
					{
						/* transmit the first data byte */
						i2c_MasterXmit(ptI2cTxBuf->I2cData[i2cDataLenCnt], I2C_MASTER_GO | I2C_CMD_WRITE);
						i2cDataLenCnt ++;
					}
					else if (i2cDataLenCnt == (i2cDataLen-1))
					{
						/* transmit the last data byte */
						if (i2cEndCond & I2C_STOP_COND)
						{
							i2c_MasterXmit(ptI2cTxBuf->I2cData[i2cDataLenCnt], I2C_MASTER_GO | I2C_CMD_WRITE | I2C_STOP_COND);
							i2cDataLenCnt = 0;
						}
						else // the last data without stop condition
						{
							i2c_MasterXmit(ptI2cTxBuf->I2cData[i2cDataLenCnt], I2C_MASTER_GO | I2C_CMD_WRITE);
							i2cDataLenCnt = 0;
							I2C_FlagClr(I2C_BUSY);
						}
					}
				}
			}
		}
		else if (i2cStatus & I2C_NO_ACK)
		{
			if (i2cActF & I2C_START_COND)
			{
				/* transmit the STOP condition */
				I2C_FlagEnb(I2C_NACK);
				i2cDataLenCnt = 0;
				i2c_MasterXmit(0, I2C_MASTER_GO | I2C_STOP_COND);
			}
			else
			{
				I2C_FlagEnb(I2C_NACK);
				i2cDataLenCnt = 0;
#if MULTI_MASTER
				i2c_MasterXmit(0, I2C_MASTER_GO | I2C_STOP_COND);
#endif
			}
		}
		else
		{
			if (i2cActF & I2C_STOP_COND)
			{
				i2cDataLenCnt = 0;
				I2C_FlagClr(I2C_BUSY);
			}
		}
	}
	else if (i2cPktDir == I2C_MST_RCVR)
	{
		/* Check the current byte ack */
		if (i2cStatus & I2C_ARB_LOST)
		{
			I2C_FlagEnb(I2C_LOST);

			i2cDataLenCnt = 0;

			I2C_FlagClr(I2C_BUSY);
		}
		else if ((!(i2cStatus & I2C_NO_ACK)) && (!(i2cStatus & I2C_TIP)))
		{
			if (i2cActF & I2C_START_COND)
			{
				if (i2cDataLenCnt == (i2cDataLen - 1))
					i2c_MasterRcvr(&(ptI2cRxBuf->I2cData[0]), I2C_MASTER_GO | I2C_CMD_READ | I2C_STOP_COND);
				else
					i2c_MasterRcvr(&(ptI2cRxBuf->I2cData[0]), I2C_MASTER_GO | I2C_CMD_READ);
			}
			else if (i2cActF & I2C_STOP_COND)
			{
				i2c_MasterRcvr(&(ptI2cRxBuf->I2cData[i2cDataLenCnt]), 0);
				I2C_FlagClr(I2C_BUSY);
			}
			else
			{
				if (i2cDataLenCnt < (i2cDataLen - 1))
				{
					/* reveive the next byte */
					i2c_MasterRcvr(&(ptI2cRxBuf->I2cData[i2cDataLenCnt]), I2C_MASTER_GO | I2C_CMD_READ);
				}
				else if (i2cDataLenCnt == (i2cDataLen - 1))
				{
					/* receive the last byte */
					i2c_MasterRcvr((U8_T *)(&(ptI2cRxBuf->I2cData[i2cDataLenCnt])), I2C_MASTER_GO | I2C_CMD_READ | I2C_STOP_COND);
				}
			}
		}
		else if (i2cStatus & I2C_NO_ACK)
		{
			if (i2cActF & I2C_START_COND)
			{
				EA = 0;
				I2C_FlagEnb(I2C_NACK);
				EA = 1;
				i2cDataLenCnt = 0;
				i2c_MasterXmit(0, I2C_MASTER_GO | I2C_STOP_COND);
			}
			else
			{
				EA = 0;
				I2C_FlagEnb(I2C_NACK);
				EA = 1;
				i2cDataLenCnt = 0;
#if MULTI_MASTER
				i2c_MasterXmit(0, I2C_MASTER_GO | I2C_STOP_COND);
#endif
			}
		}
		else
		{
			if (i2cActF & I2C_STOP_COND)
			{
				i2c_MasterRcvr(&(ptI2cRxBuf->I2cData[i2cDataLenCnt]), 0);
				I2C_FlagClr(I2C_BUSY);
			}
		}
	}
}

/*
 *--------------------------------------------------------------------------------
 * static void i2c_MasterXmit(U8_T wrData, U8_T mstCmd)
 * Purpose : Putting the data into i2c transmitting register and setting the
 *           master's command and condition in I2C master mode.
 * Params  : wrData - one byte data to transmit.
 *           mstCmd - master command of the current byte data.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
static void i2c_MasterXmit(U8_T wrData, U8_T mstCmd)
{
	/* Record the globe flag of command condition */
	i2cActF = mstCmd;
	/* First the master flipper sends the slave address to access */
	I2C_Cmd(SI_WR, I2CTR, &wrData);
	/* Order command to I2CCR */
	I2C_Cmd(SI_WR, I2CCR, &mstCmd);
}

/*
 *--------------------------------------------------------------------------------
 * static void i2c_MasterRcvr(U8_T *rdData, U8_T mstCmd)
 * Purpose : Getting the receiving byte data in I2C master mode.
 * Params  : *rdData - a pointer to store receiving data.
 *           mstCmd - master command of the current byte data.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
static void i2c_MasterRcvr(U8_T *rdData, U8_T mstCmd)
{
	/* Record the globe flag of command condition */
	i2cActF = mstCmd;
	/* After ACK, read data from I2CRR */
	I2C_Cmd(SI_RD, I2CRR, rdData);
	/* Then, reply ACK to slave */
	I2C_Cmd(SI_WR, I2CCR, &mstCmd);

	i2cDataLenCnt ++;
}

/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void I2C_Setup(U8_T ctrlCmd, U16_T preClk, U16_T axIdAddr)
 * Purpose : This function is used to setup the I2C module and
 *           to initial the globe values in this module.
 * Params  : ctrlCmd - control register value.
 *           preClk - a pre-scale parameter calculates the bus speed.
 *           axIdAddr - a device address of local AX11000 I2C module.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
void I2C_Setup(U8_T ctrlCmd, U16_T preClk, U16_T axIdAddr)
{
	/* Values initial */
	i2cCtrl = 0;
	i2cEndCond = 0;
	i2cActF = 0;
	i2cDataLen = 0;
	i2cDataLenCnt = 0;
	i2cPktDir = 0;
	ptI2cTxBuf = NULL;
	ptI2cRxBuf = NULL;
	i2cState = 0;

	/* Pre-scale Clock */
	I2CDR = (U8_T)(0x00FF & preClk);
	I2CDR = (U8_T)((0xFF00 & preClk) >> 8);
	I2CCIR = I2CCPR;
	/* Flipper device address for slave mode */
	I2CDR = (U8_T)(axIdAddr & 0x00FF);
	I2CDR = (U8_T)((axIdAddr & 0xFF00) >> 8);
	I2CCIR = I2CSDAR;
	/* Setup I2C mode */
	I2C_Cmd(SI_WR, I2CCTL, &ctrlCmd);
}

/*
 *--------------------------------------------------------------------------------
 * void I2C_IntrEntryFunc(void)
 * Purpose : Handling serial interface I2C interrupt function.
 * Params  : none
 * Returns : none
 * Note    : The I2C_RLE_ING (reloading I2C EEPROM) bit and
 *           the I2C_BL_DONE (Bootloader done) bit must always be checked,
 *           whichever in master mode or slave mode.
 *--------------------------------------------------------------------------------
 */
void I2C_IntrEntryFunc(void)
{
	U8_T	i2cStatus;

	I2C_Cmd(SI_RD, I2CCTL, &i2cCtrl);

	if (i2cCtrl & I2C_MASTER_MODE)
	{
		EA = 0;
		I2C_Cmd(SI_RD, I2CMSR, &i2cStatus);
		EA = 1;
		if (!(i2cStatus & I2C_RLE_ING) && (i2cStatus & I2C_BL_DONE))
		{
			if (i2cCtrl & I2C_MST_IE)
			{
				if (i2cStatus & I2C_INTR_FLAG)
				{
					i2c_MstStatus(i2cStatus);
				}
			}
			else
			{
				if (!(i2cStatus & I2C_TIP))
				{
					i2c_MstStatus(i2cStatus);
				}
			}
		}
	}
}

/*
 *--------------------------------------------------------------------------------
 * void I2C_PktBuf(I2C_BUF *ptI2cBuf)
 * Purpose : Packeting a packet and transmitting the first byte
 * Params  : ptI2cBuf - a pointer stores the packet.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
void I2C_PktBuf(I2C_BUF *ptI2cBuf)
{
	U8_T	firstAddr;
	U8_T	i2cStatus = 0;

	I2C_Cmd(SI_RD, I2CCTL, &i2cCtrl); 
	if (i2cCtrl & I2C_MASTER_MODE) // I2C Master Mode
	{
		i2cDataLenCnt = 0;
		i2cEndCond = ptI2cBuf->I2cEnd;
		i2cPktDir = ptI2cBuf->I2cDir;
		i2cDataLen = ptI2cBuf->DataLen;


			if (i2cCtrl & I2C_10BIT)
			{
				firstAddr = ((U8_T)((ptI2cBuf->I2cAddr.TenBitAddr & 0x0300) >> 7) | 0xF0);
				if (i2cPktDir & I2C_XMIT)
				{
					ptI2cTxBuf = ptI2cBuf;
					i2c_MasterXmit(firstAddr & ~BIT0, I2C_MASTER_GO | I2C_CMD_WRITE | I2C_START_COND);
				}
				else
				{
					ptI2cRxBuf = ptI2cBuf;
					i2c_MasterXmit(firstAddr | BIT0, I2C_MASTER_GO | I2C_CMD_WRITE | I2C_START_COND);
				}
			}
			else
			{
				firstAddr = ptI2cBuf->I2cAddr.SevenBitAddr << 1;
				if (i2cPktDir & I2C_XMIT)
				{
					ptI2cTxBuf = ptI2cBuf;
					i2c_MasterXmit(firstAddr & ~BIT0, I2C_MASTER_GO | I2C_CMD_WRITE | I2C_START_COND);
				}
				else
				{
					ptI2cRxBuf = ptI2cBuf;
					i2c_MasterXmit(firstAddr | BIT0, I2C_MASTER_GO | I2C_CMD_WRITE | I2C_START_COND);
				}
			}
	}
}

/*
 *--------------------------------------------------------------------------------
 * BOOL I2C_FlagChk(U8_T chkBit)
 * Purpose : Checking the current state in I2C driver.
 * Params  : chkBit - the state bit to be checked.
 * Returns : TRUE - the state value meets the chkBit.
 * Note    : none
 *--------------------------------------------------------------------------------
 */
BOOL I2C_FlagChk(U8_T chkBit)
{
	if (i2cState & chkBit)
		return TRUE;
	else
		return FALSE;
}

/*
 *--------------------------------------------------------------------------------
 * void I2C_FlagEnb(U8_T enbBit)
 * Purpose : Setting the current state in I2C driver.
 * Params  : enbBit - the state bit to be set.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
void I2C_FlagEnb(U8_T enbBit)
{
	BIT		oldEint4 = EINT4;

	EXTINT4_DISABLE;
	i2cState = i2cState | enbBit;
	EINT4 = oldEint4;
}

/*
 *--------------------------------------------------------------------------------
 * void I2C_FlagClr(U8_T clrBit)
 * Purpose : Clearing the current state in I2C driver.
 * Params  : clrBit - the state bit to be cleared.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
void I2C_FlagClr(U8_T clrBit)
{
	BIT		oldEint4 = EINT4;

	EXTINT4_DISABLE;
	i2cState = i2cState & ~clrBit;
	EINT4 = oldEint4;
}

/*
 *--------------------------------------------------------------------------------
 * void I2C_Cmd(U8_T cmdType, U8_T i2cCmdIndex, U8_T *i2cData)
 * Purpose : Accessing the I2C interface indirectly through I2C's SFR.
 * Params  : cmdType - command type of writing or reading.
 *           i2cCmdIndex - I2C register address.
 *           *i2cData - a pointer store the data.
 * Returns : none
 * Note    : none
 *--------------------------------------------------------------------------------
 */
void I2C_Cmd(U8_T cmdType, U8_T i2cCmdIndex, U8_T *i2cData)
{
	BIT		oldEintBit = EINT4;
	
	if (cmdType == SI_WR)
	{
		EXTINT4_DISABLE;
		I2CDR = *i2cData;
		I2CCIR = i2cCmdIndex;
		EINT4 = oldEintBit;
	}
	else if (cmdType == SI_RD)
	{
		EXTINT4_DISABLE;
		I2CCIR = i2cCmdIndex;
		*i2cData = I2CDR;
		EINT4 = oldEintBit;
	}
}

/* End of i2c.c */
