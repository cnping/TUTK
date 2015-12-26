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
 * Module Name: mwif.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: mwif.c,v $
 * no message
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.H"
#include "mwif.h"
#include "mwioctl.h"
#include <string.h>
#if IO_CPU_TYPE
#include "mcpu.h"
#include "interrupt.h"
#include "mpsave.h"
#include "uart0.h"
#include "printd.h"
#if MWIF_TASK_HOOKUP
   #include "mwifapp.h"
#endif
#else
#include "wcpu.h"
#include "wpsave.h"
#include "uart.h"
#include <stdio.h>
#endif

#if !IO_CPU_TYPE
	#define MAC_ARBIT_MODE	0
	#define MAC_ARBIT_WIFI	0
#endif

#if (!IO_CPU_TYPE || (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)) /* WCPU or MCPU including WiFi */

/* STATIC VARIABLE DECLARATIONS */
static U8_T	XDATA	mwif_txbuf[MAX_MWIF_TX_BUFFER_SIZE];
static U8_T XDATA	mwif_txcmdlen;	// command length
static U8_T XDATA	mwif_rxbuf[MAX_MWIF_RX_BUFFER_SIZE];
static U8_T XDATA	mwif_rxcmdlen;	// command length

/* GLOBAL VARIABLE DECLARATIONS */

/* LOCAL SUBPROGRAM DECLARATIONS */

/* GLOBAL SUBPROGRAM BODIES */
#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIF_DocdReset
 * Purpose: To reset MWIF CPU communication module
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void MWIF_DocdReset(void)
{
	U8_T XDATA	tmp;
	
	tmp = CSSR;	// clean data in CSSR
	CCTRL &= ~CCTRL_WRITE_CMD_INTR_MASK;	//disable Read Command Interrupt from WCPU
	CCTRL |= CCTRL_READ_CMD_INTR_MASK;	//enable Write Command Interrupt from WCPU
	WC0 = 0;
	WC1 = 0;
	WC2 = 0;
	WC3 = 0;
	WC4 = 0;
	RC0 = 0;
	RC1 = 0;
	RC2 = 0;
	RC3 = 0;
	RC4 = 0;
	mwif_txcmdlen = 0;
	mwif_rxcmdlen = 0;
	
}	/* End of MWIF_DocdReset */
#endif			/***** End of IO_CPU_TYPE macro *****/

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIF_Init
 * Purpose: To initiate "MCPU / WCPU interface" module
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void MWIF_Init(void)
{
	mwif_txcmdlen = 0;
	mwif_rxcmdlen = 0;

	IO_Init();
#if POWER_SAVING
	PS_Init();
#endif

}	/* End of MWIF_Init */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIF_Start
 * Purpose: To start the "MCPU / WCPU interface" module.
 * 			The MWIF_Start() function will enable interrupt of "MCPU / WCPU interface"
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void MWIF_Start(void)
{
	U8_T	tmp;

	tmp = CSSR;

	CCTRL &= ~CCTRL_WRITE_CMD_INTR_MASK;	//disable Write Command Interrupt
	CCTRL |= CCTRL_READ_CMD_INTR_MASK;		//enable Read Command Interrupt

#if POWER_SAVING
	PS_Start();
#endif

	EXTINT2(ON);	// Enable INT2 interrupt for MCPU-to-WCPU.

}	/* End of MWIF_Start */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIF_Sys
 * Purpose: To check it receives any command message or not
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void MWIF_Sys(void)
{
	U8_T	isr, i = 0;
	U8_T*	point;

	isr = EA;
	EA = 0;
	if (IO_RxHead != IO_RxTail)
	{
		if (IO_RxBuf[IO_RxHead] == IO_REMAINDER_BUF_IS_TOO_SMALL)
			IO_RxHead = 0;

		if (IO_RxHead != IO_RxTail)
		{
			point = &IO_RxBuf[IO_RxHead];
			EA = isr;
#if MWIF_TASK_HOOKUP
            i = MWIFAPP_TaskCmdRcv(point);
	        if (i == MWIFAPP_UPLAYER_NO_WAIT)
#endif			
            {
		       if (IO_COMMAND_ERROR == IO_CmdParsing(point)) // it gets a incorrect command
			   {
				   printd("\nCPU internal command error!!");
			   }
            }
		    isr = EA;
		    EA = 0;
#if MWIF_TASK_HOOKUP			
            if (i != MWIFAPP_UPLAYER_BUSY)
#endif
            {
			   IO_RxHead += (point[2] +3);
			   if (IO_RxHead == MAX_IO_RX_BUFFER_SIZE)
			   {
				  IO_RxHead = 0;
			   }
            }                        
		}
	}

	EA = isr;
#if MWIF_TASK_HOOKUP	
	MWIFAPP_Task();
#endif
} /* End of MWIF_Sys() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIF_Intr
 * Purpose: To read MCPU / WCPU data in the SFR RC0 ~ RC4
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void MWIF_Intr(void) interrupt 7 /* interrupt 2 */
{
	U8_T IDATA	cssr;

	cssr = CSSR;

	while (1)
	{
#if POWER_SAVING
		if (cssr & PS_INTR_STATUS_DEFAULT)
		{
#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
			if (cssr & PS_WCPU_CMD_WRITE)
				PS_Intr(PS_WCPU_CMD_WRITE);
		}
		cssr &= ~(PS_WCPU_CMD_WRITE | PS_WCPU_CMD_READ);
#else			/***** Middle of IO_CPU_TYPE, Below are for WCPU use *****/
			if (cssr & PS_MCPU_CMD_WRITE)
				PS_Intr(PS_MCPU_CMD_WRITE);
		}
		cssr &= ~(PS_MCPU_CMD_WRITE | PS_MCPU_CMD_READ);
#endif			/***** End of IO_CPU_TYPE macro *****/

#endif

		if (cssr & CSR_READ_CMD_INTR)	// It issues a "Read" command
		{
			U8_T	rxOffset = 0;

			if (!(CCTRL & CCTRL_READ_CMD_POLLING))
			{
				CCTRL |= CCTRL_READ_CMD_INTR_MASK;
				return;
			}

			mwif_rxcmdlen = RC2 + 3;
			while (rxOffset < mwif_rxcmdlen)
			{
				mwif_rxbuf[rxOffset++] = RC0;
				mwif_rxbuf[rxOffset++] = RC1;
				mwif_rxbuf[rxOffset++] = RC2;
				mwif_rxbuf[rxOffset++] = RC3;
				mwif_rxbuf[rxOffset++] = RC4;

				CCTRL |= CCTRL_READ_CMD_READY;

				if (rxOffset >= mwif_rxcmdlen)
					break;

				while (!(CCTRL & CCTRL_READ_CMD_POLLING));
			}

			if (IO_RxHead > IO_RxTail)
			{
				if ((IO_RxHead - IO_RxTail) > mwif_rxcmdlen)
				{
					memcpy(&IO_RxBuf[IO_RxTail], mwif_rxbuf, mwif_rxcmdlen);
					IO_RxTail += mwif_rxcmdlen;
				}
			}
			else
			{
				if ((MAX_IO_RX_BUFFER_SIZE - IO_RxTail) > mwif_rxcmdlen)
				{
					memcpy(&IO_RxBuf[IO_RxTail], mwif_rxbuf, mwif_rxcmdlen);
					IO_RxTail += mwif_rxcmdlen;
				}
				else if ((MAX_IO_RX_BUFFER_SIZE - IO_RxTail) == mwif_rxcmdlen)
				{
					if (IO_RxHead)
					{
						memcpy(&IO_RxBuf[IO_RxTail], mwif_rxbuf, mwif_rxcmdlen);
						IO_RxTail = 0;
					}
				}
				else
				{
					if (IO_RxHead > mwif_rxcmdlen)
					{
						IO_RxBuf[IO_RxTail] = IO_REMAINDER_BUF_IS_TOO_SMALL;
						memcpy(&IO_RxBuf[0], mwif_rxbuf, mwif_rxcmdlen);
						IO_RxTail = mwif_rxcmdlen;
					}
				}
			}
		}

		cssr = CSSR;
#if POWER_SAVING
		if (!(cssr & PS_INTR_STATUS_DEFAULT) && !(cssr & CSR_READ_CMD_INTR))
#else
		if (!(cssr & CSR_READ_CMD_INTR))
#endif
			break;
	}

}	/* End of MWIF_Intr */

/*
 * ----------------------------------------------------------------------------
 * Function Name: MWIF_Send
 * Purpose: To write the command into the mwif_txbuf
 * Params: pinfo - the pointer where the data command is
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void MWIF_Send(U8_T type, U8_T* pbuf, U8_T len)
{
	U8_T	txOffset;

	mwif_txcmdlen = len + 3;
	mwif_txbuf[0] = type;
	mwif_txbuf[1] = ~type;
	mwif_txbuf[2] = len;
	if (len > 0)
	{
		memcpy(mwif_txbuf +3, pbuf, len);
	}
	txOffset = 0;

	while (txOffset < mwif_txcmdlen)
	{
		WC0 = mwif_txbuf[txOffset++];
		WC1 = mwif_txbuf[txOffset++];
		WC2 = mwif_txbuf[txOffset++];
		WC3 = mwif_txbuf[txOffset++];
		WC4 = mwif_txbuf[txOffset++];
		CCTRL |= CCTRL_WRITE_CMD_READY;

#if IO_CPU_TYPE
		if (MCPU_WcpuReady)
#else
		if (WCPU_McpuReady)
#endif
		{
			while (!(CCTRL & CCTRL_WRITE_CMD_POLLING)) /* had been read by another CPU */
			{
			}
		}
	}
}	/* End of MWIF_Send */

#endif /* #if (!IO_CPU_TYPE || (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)) */

/* LOCAL SUBPROGRAM BODIES */
