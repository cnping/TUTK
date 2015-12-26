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
 * Module Name : interrupt_cfg.h
 * Purpose     : Configuration setting of interrupt module.
 * Author      : Robin Lee
 * Date        :
 * Notes       : None.
 * $Log: interrupt_cfg.h,v $
 * no message
 *
 *================================================================================
 */
#ifndef INTERRUPT_CFG_H
#define INTERRUPT_CFG_H

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
#define		AX_LBI_INT_ENABLE				0 // 1:enable Local Bus ISR ; 0:disable Local Bus ISR
#define		AX_ETH_INT_ENABLE				1 // 1:enable Ethernet ISR ; 0:disable Ethernet ISR
#define		AX_I2C_INT_ENABLE				1 // 1:enable I2C ISR ; 0:disable I2C ISR
#define		AX_SPI_INT_ENABLE				0 // 1:enable SPI ISR ; 0:disable SPI ISR
#define		AX_ONEWIRE_INT_ENABLE			0 // 1:enable OneWire ISR ; 0:disable OneWire ISR
#define		AX_HSUART2_INT_ENABLE			1 // 1:enable High Speed Uart 2 ISR ; 0:disable High Speed Uart 2 ISR
#define		AX_HSUART3_INT_ENABLE			0 // 1:enable High Speed Uart 3 ISR ; 0:disable High Speed Uart 3 ISR
#define		AX_I2S_INT_ENABLE				0 // 1:enable I2S ISR ; 0:disable I2S ISR
#define		AX_WATCHDOG_INT_ENB				0 // 1:enable watchdog ISR ; 0:disable watchdog ISR;
#define		AX_RTC_INT_ENABLE				0 // 1:enable RTC ISR ; 0:disable RTC ISR


/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */

#endif /* End of INTERRUPT_CFG_H */
