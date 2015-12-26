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
/*================================================================================
 * Module Name : uart0.c
 * Purpose     : The UART0 module driver. It manages the character
 *               buffer and handles the ISR. This driver includes UART0 and UART1.
 * Author      : Robin Lee
 * Date        :
 * Notes       : None.
 * $Log: uart.c,v $
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"ax22000.h"
#include	"types.h"
#include	"uart0.h"

/* GLOBAL VARIABLES DECLARATIONS */
U16_T XDATA	UART0_RxCount = 0;

/* STATIC VARIABLE DECLARATIONS */
static U8_T		far uart0_TxBuf[MAX_TX_UART0_BUF_SIZE];
static U16_T	uart0_TxHead = 0;
static U16_T	uart0_TxTail = 0;
static U16_T	uart0_TxCount = 0;
static U8_T		uart0_TxFlag = 0;
static U8_T		far uart0_RxBuf[MAX_RX_UART0_BUF_SIZE];
static U16_T	uart0_RxHead = 0;
static U16_T	uart0_RxTail = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void		uart0_Init(void);


/* LOCAL SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * static void UART0_ISR(void)
 * Purpose : UART0 interrupt service routine. For sending out, it puts data
 *           from software buffer into hardware serial buffer register (SBUF0).
 *           For receiving, it gets data from hardware serial buffer register
 *           (SBUF0) and stores into software buffer.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static void UART0_ISR(void) interrupt 4
{
	if (RI0)
	{
		if (UART0_RxCount != MAX_RX_UART0_BUF_SIZE) 
		{
			uart0_RxBuf[uart0_RxHead] = SBUF0;
			UART0_RxCount++;
			uart0_RxHead++;
			uart0_RxHead &= MAX_RX_UART0_MASK;
	    }
		RI0 = 0;
	} /* End of if(RI0) */

	if (TI0)
	{
		uart0_TxTail++;
		uart0_TxTail &= MAX_TX_UART0_MASK;
		uart0_TxCount--;
		if (uart0_TxCount > 0)
		{
			SBUF0 = uart0_TxBuf[uart0_TxTail];
		}
		else
			uart0_TxFlag = 0;

		TI0 = 0;

	} /* End of if(TI0) */

	if (PCON & PMMS_)
	{
		if (OCGCR & BIT3)
		{
			PCON |= (PMM_ | PMMS_ | SWB_);
		}
	}

} /* End of UART_Int */

/*
 * ----------------------------------------------------------------------------
 * static void uart0_Init(void)
 * Purpose : Setting operation mode of UART0 and initiating the global values.
 * Params  : none
 * Returns : none
 * Note    : TI0 must be 0 in interrupt mode
 * ----------------------------------------------------------------------------
 */
static void uart0_Init(void)
{
	U16_T	i;

	uart0_TxHead = 0;
	uart0_TxTail = 0;
	uart0_TxCount = 0;
	uart0_TxFlag = 0;
	uart0_RxHead = 0;
	uart0_RxTail = 0;
	UART0_RxCount = 0;
	for (i=0 ; i<MAX_TX_UART0_BUF_SIZE ; i++)
		uart0_TxBuf[i] = 0;
	for (i=0 ; i<MAX_RX_UART0_BUF_SIZE ; i++)
		uart0_RxBuf[i] = 0;

	// Initialize TIMER1 for standard 8051 UART clock
	PCON  = 0;			// Disable BaudRate doubler.
	SM01  = 1;			// Use serial port 0 in mode 1 with 8-bits data.
	REN0  = 1;			// Enable UART0 receiver.
	TMOD  = 0x20;		// Use timer 1 in mode 2, 8-bit counter with auto-reload.

	switch (CSREPR & SCS_80M)
	{
		case SCS_80M :
			TH1 = 0xEA;		// Baud rate = 9600 @ 80MHz.
			break;
		default :
			TH1 = 0xF5;		// Baud rate = 9600 @ 40MHz.
			break;
	}

	ES0	= 1;				// Enable serial port Interrupt request
	TR1 = 1;				// Run Timer 1
	TI0 = 0;

} /* End of UART_Init */

/*
 * ----------------------------------------------------------------------------
 * S8_T UART0_PutChar(S8_T c)
 * Purpose : UART0 output function. This function puts one byte data into the
 *           software character buffer.
 * Params  : c - one byte character.
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
S8_T UART0_PutChar(S8_T c)
{
	U16_T	count = 0;

	if (c == 0xa)
	{
		do
		{
			EA = 0;
			count = uart0_TxCount; 
			EA = 1;
		} while (count == MAX_TX_UART0_BUF_SIZE);
		uart0_TxBuf[uart0_TxHead] = 0xd;
		EA = 0;
		uart0_TxCount++;
		EA = 1;
		uart0_TxHead++;
		uart0_TxHead &= MAX_TX_UART0_MASK;
	}
	do
	{
		EA = 0;
		count = uart0_TxCount; 
		EA = 1;
	} while (count == MAX_TX_UART0_BUF_SIZE);
	uart0_TxBuf[uart0_TxHead] = c;
	EA = 0;
	uart0_TxCount++;
	EA = 1;
	uart0_TxHead++;
	uart0_TxHead &= MAX_TX_UART0_MASK;

	if (!uart0_TxFlag)
	{
		uart0_TxFlag = 1;
		SBUF0 = uart0_TxBuf[uart0_TxTail];
	}

	return c;
} /* End of UART0_PutChar() */

/*
 * ----------------------------------------------------------------------------
 * S8_T UART0_GetKey(void)
 * Purpose : UART0 input function. This function replies one byte data from the
 *           software character buffer.
 * Params  : none
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
S8_T UART0_GetKey (void)
{
    S8_T	c = 0;

	while(UART0_RxCount == 0);
	EA = 0;
	UART0_RxCount--;
	EA = 1;
	c = uart0_RxBuf[uart0_RxTail];
	uart0_RxTail++;
	uart0_RxTail &= MAX_RX_UART0_MASK;

	return c;
} /* End of UART0_GetKey() */

/*
 * ----------------------------------------------------------------------------
 * S8_T UART0_NoBlockGetkey(void)
 * Purpose : UART0 input function. This function replies one byte data from the
 *           software character buffer. But it only check the buffer one time.
 *           If no data, it will reply a FALSE condition.
 * Params  : none
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
S8_T UART0_NoBlockGetkey (void)
{
    char c = 0;

	if (UART0_RxCount != 0)
	{
		ES0 = 0;
		UART0_RxCount--;
		ES0 = 1;
		c = uart0_RxBuf[uart0_RxTail];
		uart0_RxTail++;
		uart0_RxTail &= MAX_RX_UART0_MASK;

		return c;
	}
	else
	{
		return FALSE;
	}
}


/* EXPORTED SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * S8_T _getkey (void)
 * Purpose : UART getkey function. This function is the entry of getting
 *           characters from software buffer of system's UART ports,
 *           UART0, UART1 and HSUR. 
 * Params  : none
 * Returns : ch - one byte character.
 * Note    : The default UART port is UART0.
 * ----------------------------------------------------------------------------
 */
S8_T _getkey (void)
{
	S8_T	ch = 0;

	ch = UART0_GetKey();

	return ch;
} /* End of _getkey */

/*
 * ----------------------------------------------------------------------------
 * S8_T putchar(S8_T c)
 * Purpose : UART putchar function. This function is the entry of putting
 *           characters into software buffer of system's UART ports,
 *           UART0, UART1 and HSUR. 
 * Params  : c - one byte character to be put.
 * Returns : ch - the same character to be replied.
 * Note    : The default UART port is UART0.
 * ----------------------------------------------------------------------------
 */
S8_T putchar(S8_T c)
{
	S8_T	ch = 0;

	ch = UART0_PutChar(c);

	return ch;
} /* End of putchar */

/*
 * ----------------------------------------------------------------------------
 * void UART_Init(U8_T port)
 * Purpose : UART initial function. It will call a real initial function
 *           corresponding to the used UART port.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UART_Init(void)
{
	uart0_Init();
} /* End of UART_Init() */


/* End of uart0.c */