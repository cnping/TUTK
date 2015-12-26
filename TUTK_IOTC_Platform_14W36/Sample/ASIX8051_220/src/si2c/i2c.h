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
 * Module Name : i2c.h
 * Purpose     : A header file of I2C module.
 * Author      : Robin Lee
 * Date        :
 * Notes       :
 * $Log: i2c.h,v $
 * no message
 *
 *================================================================================
 */
#ifndef I2C_H
#define I2C_H

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "i2c_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
/* I2C Register */
#define	I2CCPR					0x00	// I2C Clock Pre-scale Register
#define	I2CTR					0x02	// I2C Transmit Register
#define	I2CRR					0x03	// I2C Receive Register
#define	I2CCTL					0x04	// I2C Control Register
#define	I2CCR					0x05	// I2C Command Register
#define	I2CMSR					0x06	// I2C Master Status Register
#define	I2CSDAR					0x08	// I2C Slave Device Address Register
#define	I2CSSR					0x0A	// I2C Slave Status Register

/* I2CCTLR */
#define	I2C_MST_IE				BIT0	// I2C master mode interrupt enable
#define	I2C_ENB					BIT1	// I2C module enable
#define	I2C_DISB				0		// I2C module disable
#define	I2C_STANDARD			BIT2	// I2C standard speed in slave mode
#define	I2C_FAST				0		// I2C fast speed in slave mode
#define	I2C_10BIT				BIT3	// I2C 10-bit device address mode
#define	I2C_7BIT				0		// I2C 7-bit device address mode
#define	I2C_RELOAD				BIT4	// Reload I2C configuration EEPROM
#define	I2C_SLV_IE				BIT6	// I2C slave mode interrupt enable
#define	I2C_MASTER_MODE			BIT7	// I2C operated as master mode
#define	I2C_SLAVE_MODE			0		// I2C operated as slave mode

/* I2CCR */
#define	I2C_RLS					BIT0	// release SCLK, only for slave
#define	I2C_SLAVE_GO			BIT1	// I2C slave go command
#define	I2C_MASTER_GO			BIT3	// I2C master go command
#define	I2C_CMD_WRITE			BIT4	// Request to send data in master mode
#define	I2C_CMD_READ			BIT5	// Request to receive data in master mode
#define	I2C_STOP_COND			BIT6	// Generate a STOP condition in master mode
#define	I2C_NO_STOP				0		// No STOP condition
#define	I2C_START_COND			BIT7	// Generate a START condition in master mode

/* I2CMSR */
#define	I2C_INTR_FLAG			BIT0	// Interrupt flag
#define	I2C_TIP					BIT1	// Transfer in progress
#define	I2C_RLE_ING				BIT3	// EEPROM reloading
#define I2C_BL_DONE				BIT4	// Bootloader done
#define	I2C_ARB_LOST			BIT5	// Arbitration lost
#define	I2C_BUS_BUSY			BIT6	// I2C bus busy
#define	I2C_NO_ACK				BIT7	// No ack after a byte transfer.

/* I2CSSR */
#define	I2C_SLV_TXR_OK			BIT0	// Slave transfer complete
#define	I2C_SLV_NACK			BIT1	// No ack condition
#define	I2C_SLV_WR				BIT2	// Slave write command
#define	I2C_SLV_RD				BIT3	// Slave read command
#define	I2C_SLV_RESTART			BIT4	// Restart condition detected
#define	I2C_SLV_START			BIT5	// Start condition detected
#define	I2C_SLV_STOP			BIT6	// Stop condition detected
#define	I2C_SLV_ERR				BIT7	// Error occur

/* I2C packet direction */
#define	I2C_MASTER				BIT1
#define	I2C_XMIT				BIT0
#define	I2C_MST_XMIT			BIT1 + BIT0	//11 (BIT1:Master, BIT0:Xmit)
#define	I2C_MST_RCVR			BIT1		//10
#define	I2C_SLV_XMIT			BIT0		//01
#define	I2C_SLV_RCVR			0			//00

#define	I2C_BUSY				BIT0	// I2C busy flag
#define	I2C_RESTART				BIT1	// I2C restart flag
#define	I2C_LOST				BIT2	// I2C arbitration lost flag
#define	I2C_NACK				BIT7	// I2C NACK flag

/* I2C Master Clock */
#define	I2C_FAST_25M			0x0C	// Fast mode pre-scale at 25M
#define	I2C_FAST_50M			0x18	// Fast mode pre-scale at 50M
#define	I2C_FAST_100M			0x31	// Fast mode pre-scale at 100M
#define	I2C_STD_25M				0x3A	// Standard mode pre-scale at 25M
#define	I2C_STD_50M				0x76	// Standard mode pre-scale at 50M
#define	I2C_STD_100M			0xC7	// Standard mode pre-scale at 100M


/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */
typedef	union	addr_mode {
	U8_T		SevenBitAddr;				// 7-bit device address
	U16_T		TenBitAddr;					// 10-bit device address
}	ADDR_MODE;

typedef	struct	i2c_pkt {
	ADDR_MODE	I2cAddr;					// device address
	U8_T 		I2cData[MST_MAX_PKT_NUM];  	// i2c data field
}	I2C_PKT;

typedef	struct	i2c_buf {
	U8_T			I2cEnd;						//packet condition
	U8_T 			I2cDir;						//packet direction
	U16_T 			DataLen;					//packet length including address and data
	ADDR_MODE		I2cAddr;					//device address (7-bit or 10-bit)
	U8_T 			I2cData[MST_MAX_PKT_NUM];	//data field
}	I2C_BUF;

/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	I2C_Setup(U8_T ctrlCmd, U16_T preClk, U16_T axIdAddr);
void	I2C_IntrEntryFunc(void);
void	I2C_PktBuf(I2C_BUF *ptI2cBuf);
BOOL	I2C_FlagChk(U8_T chkBit);
void	I2C_FlagEnb(U8_T enbBit);
void	I2C_FlagClr(U8_T clrBit);
void	I2C_Cmd(U8_T cmdType, U8_T i2cCmdIndex, U8_T *i2cData);

#endif /* End of I2C_H */




