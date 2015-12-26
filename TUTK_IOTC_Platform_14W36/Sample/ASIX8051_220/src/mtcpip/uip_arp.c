/**
 * \addtogroup uip
 * @{
 */

/**
 * \defgroup uiparp uIP Address Resolution Protocol
 * @{
 * 
 * The Address Resolution Protocol ARP is used for mapping between IP
 * addresses and link level addresses such as the Ethernet MAC
 * addresses. ARP uses broadcast queries to ask for the link level
 * address of a known IP address and the host which is configured with
 * the IP address for which the query was meant, will respond with its
 * link level address.
 *
 * \note This ARP implementation only supports Ethernet.
 */
 
/**
 * \file
 * Implementation of the ARP Address Resolution Protocol.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 */

/*
 * Copyright (c) 2001-2003, Adam Dunkels.
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
 * $Id: uip_arp.c,v$
 *
 */

/*       浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
         ?   TITLE:  Keil C51 v7.00 port of Adam Dunkels' uIP v0.9 
         ?REVISION:  VER 0.0                                       
         ?REV.DATE:  30-01-05                                      
         ? ARCHIVE:                                                
         ?  AUTHOR:  Murray R. Van Luyn, 2005.                     
         藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕        */

#include "uip_arp.h"
#include "tcpip.h"
#include "tcpip_uip.h"
#include <string.h>

struct arp_hdr {
	struct uip_eth_hdr ethhdr;
	U16_T hwtype;
	U16_T protocol;
	U8_T hwlen;
	U8_T protolen;
	U16_T opcode;
	struct uip_eth_addr shwaddr;
	U16_T sipaddr[2];
	struct uip_eth_addr dhwaddr;
	U16_T dipaddr[2]; 
};

struct ethip_hdr {
	struct uip_eth_hdr ethhdr;
	/* IP header. */
	U8_T vhl,
	tos,          
	len[2],       
	ipid[2],        
	ipoffset[2],  
	ttl,          
	proto;     
	U16_T ipchksum;
	U16_T srcipaddr[2], 
	destipaddr[2];
};

#define ARP_REQUEST 1
#define ARP_REPLY   2

#define ARP_HWTYPE_ETH 1

struct arp_entry {
	U16_T ipaddr[2];
	struct uip_eth_addr ethaddr;
	U8_T time;
};

struct uip_eth_addr xdata uip_ethaddr = {{UIP_ETHADDR0,
										  UIP_ETHADDR1,
										  UIP_ETHADDR2,
										  UIP_ETHADDR3,
										  UIP_ETHADDR4,
										  UIP_ETHADDR5}};
struct uip_eth_addr xdata uip_gatewaymacaddr = {0, 0, 0, 0, 0, 0};
U8_T xdata uip_getgatewaymacaddrflag = FALSE;

static struct arp_entry xdata arp_table[UIP_ARPTAB_SIZE];
static U16_T xdata ipaddr[2];
static U8_T xdata i, c;

static U8_T xdata arptime;
static U8_T xdata tmpage;

#define BUF   ((struct arp_hdr *)&uip_buf[0])
#define IPBUF ((struct ethip_hdr *)&uip_buf[0])

/*-----------------------------------------------------------------------------------*/
/**
 * Initialize the ARP module.
 *
 */
/*-----------------------------------------------------------------------------------*/
void uip_arp_init(void)
{
	for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
		memset(arp_table[i].ipaddr, 0, 4);
	}
}

/*-----------------------------------------------------------------------------------*/
/**
 * Periodic ARP processing function.
 *
 * This function performs periodic timer processing in the ARP module
 * and should be called at regular intervals. The recommended interval
 * is 10 seconds between the calls.
 *
 */
/*-----------------------------------------------------------------------------------*/
void uip_arp_timer(void)
{
	struct arp_entry *tabptr;

	++arptime;
	for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
		tabptr = &arp_table[i];
		if((tabptr->ipaddr[0] | tabptr->ipaddr[1]) != 0 &&
			arptime - tabptr->time >= UIP_ARP_MAXAGE)
		{
			memset(tabptr->ipaddr, 0, 4);
		}
	}
}

/*-----------------------------------------------------------------------------------*/

static void uip_arp_update(U16_T *ipaddr, struct uip_eth_addr *ethaddr)
{
	register struct arp_entry *tabptr;

	/* Walk through the ARP mapping table and try to find an entry to
	   update. If none is found, the IP -> MAC address mapping is
	   inserted in the ARP table. */
	for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
		tabptr = &arp_table[i];
		/* Only check those entries that are actually in use. */
		if(tabptr->ipaddr[0] != 0 && tabptr->ipaddr[1] != 0) {
			/* Check if the source IP address of the incoming packet matches
			   the IP address in this ARP table entry. */
			if(ipaddr[0] == tabptr->ipaddr[0] && ipaddr[1] == tabptr->ipaddr[1]) {
				/* An old entry found, update this and return. */
				memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
				tabptr->time = arptime;
				return;
			}
		}
	}

	/* If we get here, no existing ARP table entry was found, so we create one. */

	/* First, we try to find an unused entry in the ARP table. */
	for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
		tabptr = &arp_table[i];
		if(tabptr->ipaddr[0] == 0 && tabptr->ipaddr[1] == 0) {
			break;
		}
	}

	/* If no unused entry is found, we try to find the oldest entry and throw it away. */
	if(i == UIP_ARPTAB_SIZE) {
		tmpage = 0;
		c = 0;
		for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
			tabptr = &arp_table[i];
			if(arptime - tabptr->time > tmpage) {
				tmpage = arptime - tabptr->time;
				c = i;
			}
		}
		i = c;
		tabptr = &arp_table[i];
	}

	/* Now, i is the ARP table entry which we will fill with the new information. */
	memcpy(tabptr->ipaddr, ipaddr, 4);
	memcpy(tabptr->ethaddr.addr, ethaddr->addr, 6);
	tabptr->time = arptime;
}

/*-----------------------------------------------------------------------------------*/
/**
 * ARP processing for incoming IP packets
 *
 * This function should be called by the device driver when an IP
 * packet has been received. The function will check if the address is
 * in the ARP cache, and if so the ARP cache entry will be
 * refreshed. If no ARP cache entry was found, a new one is created.
 *
 * This function expects an IP packet with a prepended Ethernet header
 * in the uip_buf[] buffer, and the length of the packet in the global
 * variable uip_len.
 */
/*-----------------------------------------------------------------------------------*/
void uip_arp_ipin(void)
{
	/* Only insert/update an entry if the source IP address of the
	   incoming IP packet comes from a host on the local network. */
	if((IPBUF->srcipaddr[0] & uip_arp_netmask[0]) != (uip_hostaddr[0] & uip_arp_netmask[0])) {
    	return;
	}
	if((IPBUF->srcipaddr[1] & uip_arp_netmask[1]) != (uip_hostaddr[1] & uip_arp_netmask[1])) {
	    return;
	}
	
	/* had got the gateway mac address and ip address */
	if (uip_getgatewaymacaddrflag == TRUE)
	{
		/* source ip address is not gateway ip address */
		if ((IPBUF->srcipaddr[0]!=uip_arp_draddr[0])||(IPBUF->srcipaddr[1]!=uip_arp_draddr[1]))
		{
			/* source mac address is equal to gateway mac address */
			if (0 == memcmp(uip_gatewaymacaddr.addr, IPBUF->ethhdr.src.addr, 6))
				return;
		}
	}

	uip_arp_update(IPBUF->srcipaddr, &(IPBUF->ethhdr.src));

	return;
}

/*-----------------------------------------------------------------------------------*/
/**
 * ARP processing for incoming ARP packets.
 *
 * This function should be called by the device driver when an ARP
 * packet has been received. The function will act differently
 * depending on the ARP packet type: if it is a reply for a request
 * that we previously sent out, the ARP cache will be filled in with
 * the values from the ARP reply. If the incoming ARP packet is an ARP
 * request for our IP address, an ARP reply packet is created and put
 * into the uip_buf[] buffer.
 *
 * When the function returns, the value of the global variable uip_len
 * indicates whether the device driver should send out a packet or
 * not. If uip_len is zero, no packet should be sent. If uip_len is
 * non-zero, it contains the length of the outbound packet that is
 * present in the uip_buf[] buffer.
 *
 * This function expects an ARP packet with a prepended Ethernet
 * header in the uip_buf[] buffer, and the length of the packet in the
 * global variable uip_len.
 */
/*-----------------------------------------------------------------------------------*/
void uip_arp_arpin(void)
{
	register struct arp_entry *tabptr;

	if(uip_len < sizeof(struct arp_hdr)) {
		uip_len = 0;
		return;
	}

	uip_len = 0;

	switch(BUF->opcode)
	{
		case HTONS(ARP_REQUEST):
			/* ARP request. If it asked for our address, we send out a reply. */
		    if(BUF->dipaddr[0] == uip_hostaddr[0] && BUF->dipaddr[1] == uip_hostaddr[1]) {
				/* The reply opcode is 2. */
				BUF->opcode = HTONS(2);

				memcpy(BUF->dhwaddr.addr, BUF->shwaddr.addr, 6);
				memcpy(BUF->shwaddr.addr, uip_ethaddr.addr, 6);
				memcpy(BUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
				memcpy(BUF->ethhdr.dest.addr, BUF->dhwaddr.addr, 6);

				BUF->dipaddr[0] = BUF->sipaddr[0];
				BUF->dipaddr[1] = BUF->sipaddr[1];
				BUF->sipaddr[0] = uip_hostaddr[0];
				BUF->sipaddr[1] = uip_hostaddr[1];

				BUF->ethhdr.type = HTONS(UIP_ETHTYPE_ARP);
				uip_len = sizeof(struct arp_hdr);
			}
			break;

		case HTONS(ARP_REPLY):
			/* ARP reply. We insert or update the ARP table if it was meant for us. */
			if(BUF->dipaddr[0] == uip_hostaddr[0] && BUF->dipaddr[1] == uip_hostaddr[1]) {
				if (uip_getgatewaymacaddrflag == FALSE) /* get gateway mac address */
				{
					if ((uip_arp_draddr[0] == BUF->sipaddr[0]) && (uip_arp_draddr[1] == BUF->sipaddr[1]))
					{
						uip_getgatewaymacaddrflag = TRUE;
						memcpy(uip_gatewaymacaddr.addr, BUF->shwaddr.addr, 6);

						for(i = 0; i < UIP_ARPTAB_SIZE; ++i)
						{
							tabptr = &arp_table[i];
							/* Only check those entries that are actually in use. */
							if(tabptr->ipaddr[0] != 0 && tabptr->ipaddr[1] != 0)
							{
								if (0 == memcmp(uip_gatewaymacaddr.addr, tabptr->ethaddr.addr, 6))
								{
									if ((uip_arp_draddr[0] != tabptr->ipaddr[0]) || (uip_arp_draddr[1]) != tabptr->ipaddr[1])
									{
										tabptr->ipaddr[0] = 0;
										tabptr->ipaddr[1] = 0;
									}
								}
							}
						}
					}
				}
				uip_arp_update(BUF->sipaddr, &BUF->shwaddr);
			}
			break;
		default:
			break;
	}

	return;
}

/*-----------------------------------------------------------------------------------*/
/**
 * Prepend Ethernet header to an outbound IP packet and see if we need
 * to send out an ARP request.
 *
 * This function should be called before sending out an IP packet. The
 * function checks the destination IP address of the IP packet to see
 * what Ethernet MAC address that should be used as a destination MAC
 * address on the Ethernet.
 *
 * If the destination IP address is in the local network (determined
 * by logical ANDing of netmask and our IP address), the function
 * checks the ARP cache to see if an entry for the destination IP
 * address is found. If so, an Ethernet header is prepended and the
 * function returns. If no ARP cache entry is found for the
 * destination IP address, the packet in the uip_buf[] is replaced by
 * an ARP request packet for the IP address. The IP packet is dropped
 * and it is assumed that they higher level protocols (e.g., TCP)
 * eventually will retransmit the dropped packet.
 *
 * If the destination IP address is not on the local network, the IP
 * address of the default router is used instead.
 *
 * When the function returns, a packet is present in the uip_buf[]
 * buffer, and the length of the packet is in the global variable
 * uip_len.
 */
/*-----------------------------------------------------------------------------------*/
void uip_arp_out(void)
{
	struct arp_entry *tabptr;
	/* Find the destination IP address in the ARP table and construct
	   the Ethernet header. If the destination IP addres isn't on the
	   local network, we use the default router's IP address instead.

	   If not ARP table entry is found, we overwrite the original IP
	   packet with an ARP request for the IP address. */

	if (IPBUF->destipaddr[0] == 0xffff)
	{
		memset(BUF->ethhdr.dest.addr, 0xff, 6);
	}
	else if (IPBUF->destipaddr[0] > 0xe000)
	{
		BUF->ethhdr.dest.addr[0] = 1;
		BUF->ethhdr.dest.addr[1] = 0;
		BUF->ethhdr.dest.addr[2] = 0x5e;
		BUF->ethhdr.dest.addr[3] = (U8_T)IPBUF->destipaddr[0] & 0x7f;
		BUF->ethhdr.dest.addr[4] = (U8_T)(IPBUF->destipaddr[1] >> 8);
		BUF->ethhdr.dest.addr[5] =(U8_T)IPBUF->destipaddr[1];
	}
	else
	{
		/* Check if the destination address is on the local network. */
		if((IPBUF->destipaddr[0] & uip_arp_netmask[0]) != (uip_hostaddr[0] & uip_arp_netmask[0]) ||
			(IPBUF->destipaddr[1] & uip_arp_netmask[1]) != (uip_hostaddr[1] & uip_arp_netmask[1])) 
		{
			/* Destination address was not on the local network, so we need to
			   use the default router's IP address instead of the destination
			   address when determining the MAC address. */
			ipaddr[0] = uip_arp_draddr[0];
			ipaddr[1] = uip_arp_draddr[1];
		} else {
			/* Else, we use the destination IP address. */
			ipaddr[0] = IPBUF->destipaddr[0];
			ipaddr[1] = IPBUF->destipaddr[1];
		}

		/* dest ip address is 0 or local ip address is 0 */
		if (((ipaddr[0] == 0) && (ipaddr[1] == 0)) || ((uip_hostaddr[0] == 0) && (uip_hostaddr[1] == 0)))
		{
			uip_len = 0;
			return;
		}

		for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
			tabptr = &arp_table[i];
			if(ipaddr[0] == tabptr->ipaddr[0] && ipaddr[1] == tabptr->ipaddr[1])
				break;
		}

		if(i == UIP_ARPTAB_SIZE) {
			/* The destination address was not in our ARP table, so we
			   overwrite the IP packet with an ARP request. */
			memset(BUF->ethhdr.dest.addr, 0xff, 6);
			memset(BUF->dhwaddr.addr, 0x00, 6);
			memcpy(BUF->ethhdr.src.addr, uip_ethaddr.addr, 6);
			memcpy(BUF->shwaddr.addr, uip_ethaddr.addr, 6);

			BUF->dipaddr[0] = ipaddr[0];
			BUF->dipaddr[1] = ipaddr[1];
			BUF->sipaddr[0] = uip_hostaddr[0];
			BUF->sipaddr[1] = uip_hostaddr[1];
			BUF->opcode = HTONS(ARP_REQUEST); /* ARP request. */
			BUF->hwtype = HTONS(ARP_HWTYPE_ETH);
			BUF->protocol = HTONS(UIP_ETHTYPE_IP);
			BUF->hwlen = 6;
			BUF->protolen = 4;
			BUF->ethhdr.type = HTONS(UIP_ETHTYPE_ARP);

			uip_appdata = &uip_buf[40 + TCPIP_EthHeadLen];

			uip_len = sizeof(struct arp_hdr);
			return;
		}
		/* Build an ethernet header. */
		memcpy(IPBUF->ethhdr.dest.addr, tabptr->ethaddr.addr, 6);
	}

	memcpy(IPBUF->ethhdr.src.addr, uip_ethaddr.addr, 6);

	IPBUF->ethhdr.type = HTONS(UIP_ETHTYPE_IP);

	uip_len += sizeof(struct uip_eth_hdr);
}

/*-----------------------------------------------------------------------------------*/
/**
 * To find the assigned ip address in the uip arp table. If not found, 
 * this function return 0xff, or it return the index of uip arp table.
 */
/*-----------------------------------------------------------------------------------*/
U8_T uip_findarptable(U16_T* ipaddr)
{
	register struct arp_entry *tabptr;

	/* Walk through the ARP mapping table and try to find an entry
	   with the same ip address*/
	for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
		tabptr = &arp_table[i];
		/* Only check those entries that are actually in use. */
		if(tabptr->ipaddr[0] != 0 && tabptr->ipaddr[1] != 0) {
			/* Check if the source IP address of the incoming packet matches
			   the IP address in this ARP table entry. */
			if(ipaddr[0] == tabptr->ipaddr[0] && ipaddr[1] == tabptr->ipaddr[1]) {
				/* An old entry found, return the index. */
				return i;
			}
		}
	}

	return 0xff;
} /* End of uip_findarptable() */
/*-----------------------------------------------------------------------------------*/


/** @} */
/** @} */
