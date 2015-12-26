/*
 * Copyright (c) 2001, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: uip_arch.c,v$
 *
 */

/*       浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
         ?   TITLE:  Keil C51 v7.00 port of Adam Dunkels' uIP v0.9 
         ?REVISION:  VER 0.0                                       
         ?REV.DATE:  14-01-05                                      
         ? ARCHIVE:                                                
         ?  AUTHOR:  Murray R. Van Luyn, 2005.                     
         藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕        */

#include "uip.h"
#include "uip_arch.h"

#define BUF_TRANSP ((uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define BUF ((uip_tcpip_hdr *)&uip_buf[0])
#define IP_PROTO_TCP    6

/*-----------------------------------------------------------------------------------*/
void
uip_add32(U8_T *op32, U16_T op16)
{
  
  uip_acc32[3] = op32[3] + (op16 & 0xff);
  uip_acc32[2] = op32[2] + (op16 >> 8);
  uip_acc32[1] = op32[1];
  uip_acc32[0] = op32[0];
  
  if(uip_acc32[2] < (op16 >> 8)) {
    ++uip_acc32[1];    
    if(uip_acc32[1] == 0) {
      ++uip_acc32[0];
    }
  }
  
  
  if(uip_acc32[3] < (op16 & 0xff)) {
    ++uip_acc32[2];  
    if(uip_acc32[2] == 0) {
      ++uip_acc32[1];    
      if(uip_acc32[1] == 0) {
	++uip_acc32[0];
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
#if (!CHECKSUM_OFFLOAD)
U16_T
uip_chksum(U16_T *sdata, U16_T len)
{
  U16_T acc;
  
  for(acc = 0; len > 1; len -= 2) {
    acc += *sdata;
    if(acc < *sdata) {
      /* Overflow, so we add the carry to acc (i.e., increase by
         one). */
      ++acc;
    }
    ++sdata;
  }

  /* add up any odd byte */
  if(len == 1) {
    acc += htons(((U16_T)(*(U8_T *)sdata)) << 8);
    if(acc < htons(((U16_T)(*(U8_T *)sdata)) << 8)) {
      ++acc;
    }
  }

  return acc;
}
#endif
/*-----------------------------------------------------------------------------------*/
#if (!CHECKSUM_OFFLOAD)
U16_T
uip_ipchksum(void)
{
  return uip_chksum((U16_T *)&uip_buf[TCPIP_EthHeadLen], 20);
}
#endif
/*-----------------------------------------------------------------------------------*/
#if (!CHECKSUM_OFFLOAD)
U16_T
uip_tcpchksum(void)
{
  U16_T hsum, sum;

  
  /* Compute the checksum of the TCP header. */
  hsum = uip_chksum((U16_T *)&uip_buf[20 + TCPIP_EthHeadLen], 20);

  /* Compute the checksum of the data in the TCP packet and add it to
     the TCP header checksum. */
  if (TCPIP_EthHeadLen)
  {
	sum = uip_chksum((U16_T *)uip_appdata,
	  (U16_T)(((((U16_T)(BUF_TRANSP->len[0]) << 8) + BUF_TRANSP->len[1]) - 40)));
  }
  else
  {
	sum = uip_chksum((U16_T *)uip_appdata,
	  (U16_T)(((((U16_T)(BUF->len[0]) << 8) + BUF->len[1]) - 40)));
  }

  if((sum += hsum) < hsum) {
    ++sum;
  }

  if (TCPIP_EthHeadLen)
  {
	if((sum += BUF_TRANSP->srcipaddr[0]) < BUF_TRANSP->srcipaddr[0]) {
		++sum;
	}
	if((sum += BUF_TRANSP->srcipaddr[1]) < BUF_TRANSP->srcipaddr[1]) {
    	++sum;
	}
	if((sum += BUF_TRANSP->destipaddr[0]) < BUF_TRANSP->destipaddr[0]) {
    	++sum;
	}
	if((sum += BUF_TRANSP->destipaddr[1]) < BUF_TRANSP->destipaddr[1]) {
    	++sum;
	}
  }
  else
  {  
	if((sum += BUF->srcipaddr[0]) < BUF->srcipaddr[0]) {
		++sum;
	}
	if((sum += BUF->srcipaddr[1]) < BUF->srcipaddr[1]) {
    	++sum;
	}
	if((sum += BUF->destipaddr[0]) < BUF->destipaddr[0]) {
    	++sum;
	}
	if((sum += BUF->destipaddr[1]) < BUF->destipaddr[1]) {
    	++sum;
	}
  }
  if((sum += (U16_T)htons((U16_T)IP_PROTO_TCP)) < (U16_T)htons((U16_T)IP_PROTO_TCP)) {
    ++sum;
  }

  if (TCPIP_EthHeadLen)
  {
	hsum = (U16_T)htons((((U16_T)(BUF_TRANSP->len[0]) << 8) + BUF_TRANSP->len[1]) - 20);
  }
  else
  {
	hsum = (U16_T)htons((((U16_T)(BUF->len[0]) << 8) + BUF->len[1]) - 20);
  }
  
  if((sum += hsum) < hsum) {
    ++sum;
  }
  
  return sum;
}
#endif
