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
/*============================================================================
 * Module name: cmime64.c
 * Purpose:	Multipurpos Internet Mail Extensions process.
 * Author:
 * Date:
 * Notes:
 * $Log: CMIME64.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

// Global variable
extern S8_T	XDATA	each3toc;		/* Buffer each 3 byte to coding Base64 format*/
extern S8_T	XDATA	b64[4];			/* memory output 4 base64 data*/

/* LOCAL VARIABLE DECLARATIONS */
U32_T buffer24;		/* recode the buffer 24 bits */


/*--------------------------------------------------------------------------*
 * Initialization of the coder.                                             *
 *--------------------------------------------------------------------------*/
void Cmime64_Init(void)
{
	each3toc = 0;
	buffer24 = 0;

} /* End of Cmime64_Init() */

void cmime64(S8_T* pindata)
{
  U32_T	buffer;
  U32_T	buffer_out;
  S8_T		indata;
  S8_T		cvtb64[4];

  U32_T   	i;
  indata = *pindata;
  buffer = (U32_T)indata & 0xFF;
  buffer <<= ((2-each3toc)*8);  	//first-in source data keep in MSB
  buffer24 |= buffer;

  if(each3toc == 2)
  {
  	for(i=0;i<4;i++)
  	{
  		buffer_out = buffer24>>(i*6);//encode LSB to b64[first==>0..3]array
  		b64[i] = (S8_T)buffer_out&0x3F;
  		cvtb64[i] = b64[i];
                if (cvtb64[i] <= 0x19 )
                   b64[i] += 65;

                if ( (cvtb64[i] > 0x19 ) && (cvtb64[i] <= 0x33) )
                   b64[i] += 71;	//(97-26)

                if ( (cvtb64[i] > 0x33 ) && (cvtb64[i] <= 0x3d) )
                   b64[i] -= 4;

                if ( cvtb64[i] == 0x3e )
                   b64[i] = '+';
                if ( cvtb64[i] == 0x3f )
                   b64[i] = '/';
  	}
  	buffer24 = 0;
  }
}
