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
 * Module Name : printd.c
 * Purpose     : AX22000 initialization and globe values setting.
 * Author      : 
 * Date        : 
 * Notes       : 
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdarg.h>
#include "types.h"
#include "printd.h"
#include "uart0.h"

/* GLOBAL VARIABLES DECLARATIONS */
U8_T DEBUG_MsgEnable = 1;

/* STATIC VARIABLE DECLARATIONS */

/* LOCAL SUBPROGRAM DECLARATIONS */
static int prints(const char *string, int width, int pad);
static int printi(long i, int b, int sg, int width, int pad, int letbase);

/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static int prints(const char *string, int width, int pad)
 * Purpose : None
 * Params  : None
 * Returns : None
 * Note    : None
 *--------------------------------------------------------------------------------
 */
static int prints(const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';


	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			DEBUG_PORT_PUTCHR(padchar);
			++pc;
		}
	}
	
	for ( ; *string ; ++string) {
		DEBUG_PORT_PUTCHR(*string);
		++pc;
	}
	
	for ( ; width > 0; --width) {
		DEBUG_PORT_PUTCHR ( padchar);
		++pc;
	}

	return pc;
}

/*
 *--------------------------------------------------------------------------------
 * static int printi(long i, int b, int sg, int width, int pad, int letbase)
 * Purpose : None
 * Params  : None
 * Returns : None
 * Note    : None
 *--------------------------------------------------------------------------------
 */
static int printi(long i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register long t, neg = 0, pc = 0;
	register unsigned long u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints ( print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			DEBUG_PORT_PUTCHR ('-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}
	return pc + prints ( s, width, pad);
}


/* EXPORTED SUBPROGRAM BODIES */
/*
 *--------------------------------------------------------------------------------
 * int printd(const char *format, ...)
 * Purpose : None
 * Params  : None
 * Returns : None
 * Note    : None
 *--------------------------------------------------------------------------------
 */
int printd(const char *format, ...)
{
	register int width, pad;
	register int pc = 0;
 	char scr[2];
	
    va_list args;
        
    if (DEBUG_MsgEnable == 0)
        return 0;

    va_start( args, format );

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			switch(*format)
			{
				case 's':
				    {
					register char *s = (char *)va_arg( args, char * );
					pc += prints ( s?s:"(null)", width, pad);
					continue;
					}
				case 'b':
					++format;
					if (*format == 'd')
						pc += printi ( va_arg( args, char ), 10, 1, width, pad, 'a');
					else if (*format == 'u')
						pc += printi ( va_arg( args, unsigned char ), 10, 0, width, pad, 'a');
					else if (*format == 'x')
						pc += printi ( va_arg( args, unsigned char ), 16, 0, width, pad, 'a');
					continue;
				case 'd':
					pc += printi ( va_arg( args, int ), 10, 1, width, pad, 'a');
					continue;
				case 'x':
					pc += printi ( va_arg( args, int ), 16, 0, width, pad, 'a');
					continue;
				case 'u' :
					pc += printi ( va_arg( args, int ), 10, 0, width, pad, 'a');
					continue;
				case 'l':
					++format;
					if (*format == 'x')
						pc += printi ( va_arg( args, long ), 16, 1, width, pad, 'a');
					else if (*format == 'u')
						pc += printi ( va_arg( args, long ), 10, 0, width, pad, 'a');
					continue;
				case 'c' :
					scr[0] = (char)va_arg( args, char );
					scr[1] = '\0';
					pc += prints ( scr, width, pad);
					continue;
			}
		}
		else {
		out:
			DEBUG_PORT_PUTCHR(*format);
			++pc;
		}
	}
	va_end( args );
	return pc;
}