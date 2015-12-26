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
 /*============================================================================
 * Module Name: pppoe.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: pppoe.c,v $
 *=============================================================================
 */

#include "tcpip.h"
#include "pppoe.h"
#include "stoe.h"
#include "adapter.h"
#include "uart0.h"
#include "printd.h"
#include <string.h>

/* NAMING CONSTANT DECLARATIONS */
#define UIP_ETHTYPE_PPPOE_SESSION	0x8864
#define UIP_ETHTYPE_PPPOE_DISCOVERY	0x8863

/* LOCAL VARIABLES DECLARATIONS */
U8_T PPPOE_GetDnsFlag = 0;

/* Pointer of PPPOE_INFO type */
static PPPOE_INFO	pppoe_info;

/* Local static variables */
static U8_T		ac_cookie[32];
static U8_T		service_name[32];
static U16_T	as_length[2];
static U8_T		host_uniq_from_PADO[8];
static U16_T	session_id;
static U8_T		magic_number_server[4];
static U8_T		magic_number_client[4];
static U8_T		pppoe_identification_1[11] = "MSRAVS5.10";
static U8_T		pppoe_identification_2[13] = "MSRAV-0-ASIX";
static U8_T		pppoe_username[33] = "username"; //User name
static U8_T		pppoe_password[33] = "password"; //Password
static U8_T		pppoe_server_ip[4];
static U8_T		pppoe_ip[4];
static U8_T		pppoe_pri_dns[4];
static U8_T		pppoe_sec_dns[4];
static U8_T		pppoe_successful = 0;
static U8_T		pppoe_server_mac[6];
static U8_T		pppoe_MacAddr[6];

/* LOCAL SUBPROGRAM DECLARATIONS */
static void	Analysis_Tags(U8_T *tags, U16_T length);
static U8_T	PPPOE_Discovery_Handle(U8_T*, U8_T state);
static U8_T	PPPOE_Session_Handle(U8_T*, U16_T state);
static U8_T	Check_Magic(U8_T *magic, U8_T sc);
static U8_T	PPPOE_Process(U8_T *buf, U16_T length);
static U8_T	Check_Echo(U8_T *buf);
static U8_T	Check_Terminate(U8_T *buf);
static U16_T PPPOE_Get_Session(void);

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Receive
 * Purpose: If connection type is PPPoE, this function maintain any packets
 *		while receiving data.
 * Params:  *buf: Pointer of packet / length: Length of packet
 * Returns: 1: Useful packet / 0: Drop the packet
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T PPPOE_Receive(U8_T *buf, U16_T length)
{
	pppoe_hdr*	PPPOEBUF = (pppoe_hdr*)buf;

	if(PPPOEBUF->ether_type == 0x8863 || PPPOEBUF->ether_type == 0x8864)
	{
		return PPPOE_Process((U8_T*)PPPOEBUF, length);
	}
	else
	{
		return 0;
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_GetState
 * Purpose: Called by system, let it know if PPPoE connection is successful
 * Params: 
 * Returns: 0: Connection not successful / 1: Connection successful
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T PPPOE_GetState(void)
{
	return pppoe_successful;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Send
 * Purpose: Called by system while packet to send in PPPoE mode
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PPPOE_Send(U8_T* buf, U16_T length)
{
	pppoe_hdr*	PPPOEBUF = (pppoe_hdr*)buf;

	/* Ethernet header */
	memcpy(PPPOEBUF->dest_addr, pppoe_server_mac, 6);
	memcpy(PPPOEBUF->src_addr, pppoe_MacAddr, 6);
	PPPOEBUF->ether_type = htons(UIP_ETHTYPE_PPPOE_SESSION);

	/* PPPoE header */
	PPPOEBUF->vhl = 0x11;
	PPPOEBUF->pppoe_code = PPPOE_CODE_SESS;
	PPPOEBUF->session_id = session_id;
	PPPOEBUF->payload_length = length - 20;

	/* Point-to-Point Protocol */
	*(U16_T*)PPPOEBUF->payload_data = 0x0021;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Init
 * Purpose: Initialize state & identifier of pppoe_info.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PPPOE_Init(void)
{
	U8_T*	point = MAC_GetMacAddr();

	pppoe_info.state = 0;
	pppoe_info.identifier = 0;
	pppoe_info.wait_count_master = 0;
	pppoe_info.wait_count_slave = 0;
	memcpy(pppoe_MacAddr, point, 6);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Command
 * Purpose: Called by main() of adapter.c, this is a state machine function
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PPPOE_Command(void)
{
	/* Do process by checking internal state */
	switch(pppoe_info.state)
	{
		case 0:
			/* Send PADI packet in discovery stage */
			PPPOE_Discovery_Handle(TCPIP_GetXmitBuffer(), PPPOE_CODE_PADI);
			break;
		case PPPOE_CODE_PADI:
			/* Send PADI every 5 seconds while no PADO received */
			pppoe_info.wait_count_slave++;
			if(pppoe_info.wait_count_slave > 45644)
			{
				pppoe_info.wait_count_slave = 0;
				pppoe_info.wait_count_master++;
				if(pppoe_info.wait_count_master > 4)
				{
					pppoe_info.wait_count_master = 0;
					PPPOE_Discovery_Handle(TCPIP_GetXmitBuffer(), PPPOE_CODE_PADI);
				}
		   	}
			break;
		case PPPOE_CODE_PADS_1:
			/* Send LCP request in session stage */
			PPPOE_Session_Handle(TCPIP_GetXmitBuffer(), 2);
			break;
		case PPPOE_CODE_PADS_3:
			/* Send Identification_1 packet */
			PPPOE_Session_Handle(TCPIP_GetXmitBuffer(), 4);
			/* Send Identification_2 packet */
			PPPOE_Session_Handle(TCPIP_GetXmitBuffer(), 5);
			/* Create Authentication packet */
			PPPOE_Session_Handle(TCPIP_GetXmitBuffer(), 6);
			break;
		case PPPOE_CODE_PADS_5:
			/* Send ICPC request */
			PPPOE_Session_Handle(TCPIP_GetXmitBuffer(), 8);
			break;
		default:
			break;
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Process
 * Purpose: Called by PPPOE_Receive(), and process packet while system receive
 *			packet
 * Params:  *buf: Pointer of packet / length: Length of the packet
 * Returns: 0: Not wanted packet / 1: Wanted packet, and process it
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T PPPOE_Process(U8_T *buf, U16_T length)
{
	pppoe_hdr*	PPPOEBUF = (pppoe_hdr*)buf;

	length = length;

	/* Always check Echo request first */
	if(Check_Echo((U8_T XDATA*)PPPOEBUF))
	{
		return 1;
	}

	/* Check Terminate request */
	if(Check_Terminate((U8_T XDATA*)PPPOEBUF))
	{
		return 1;
	}

	switch(pppoe_info.state)
	{
		case PPPOE_CODE_PADI:
			/* Maintain ether type for PPPoE discovery */
			if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_DISCOVERY && PPPOEBUF->pppoe_code == PPPOE_CODE_PADO)
			{
				/* Send PADR while PADO received */
				return PPPOE_Discovery_Handle((U8_T*)PPPOEBUF, PPPOE_CODE_PADR);
      		}
			return 0;
		case PPPOE_CODE_PADR:
			/* Wait for Confirm session */
			if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_DISCOVERY && PPPOEBUF->pppoe_code == PPPOE_CODE_PADS)
			{
				/* Check and keep session */
				return PPPOE_Discovery_Handle((U8_T*)PPPOEBUF, PPPOE_CODE_PADS);
			}
			return 0;
		case PPPOE_CODE_PADS:
			/* Reply LCP in session stage */
			if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_SESSION && PPPOEBUF->session_id == PPPOE_Get_Session())
			{
				return PPPOE_Session_Handle((U8_T*)PPPOEBUF, 1);
			}
			return 0;
		case PPPOE_CODE_PADS_2:
			/* Wait ACK from server */
			if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_SESSION && PPPOEBUF->session_id == PPPOE_Get_Session())
			{
				return PPPOE_Session_Handle((U8_T*)PPPOEBUF, 3);
			}
			return 0;
		case PPPOE_CODE_PADS_4:
			/* Wait ICPC request */
			if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_SESSION && PPPOEBUF->session_id == PPPOE_Get_Session())
			{
				return PPPOE_Session_Handle((U8_T*)PPPOEBUF, 7);
			}
			return 0;
		case PPPOE_CODE_PADS_6:
			/* Wait IPCP Nak packet */
			if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_SESSION && PPPOEBUF->session_id == PPPOE_Get_Session())
			{
				return PPPOE_Session_Handle((U8_T*)PPPOEBUF, 9);
			}
			return 0;
		case PPPOE_CODE_PADS_7:
			if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_SESSION && PPPOEBUF->session_id == PPPOE_Get_Session())
			{
				return PPPOE_Session_Handle((U8_T*)PPPOEBUF, 10);
			}
			return 0;
		case PPPOE_CODE_PADS_8:
			if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_SESSION && PPPOEBUF->session_id == PPPOE_Get_Session())
			{
				return 1;
			}
			return 0;
		default:
			return 0;
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Discovery_Handle
 * Purpose: Handle discovery stage of PPPoE
 * Params: *PPPOEBUF: Pointer of packet / state: internal state
 * Returns: 0: Not wanted packet / 1: Wanted packet, and process it
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T PPPOE_Discovery_Handle(U8_T *PPPOEBUF, U8_T state)
{ 
	tag	*tags = (tag*)((pppoe_hdr*)PPPOEBUF)->payload_data;
	U8_T	i;

	switch(state)
	{
		case PPPOE_CODE_PADI:

			/* Create PADI packet */
			for(i = 0; i < 6; i++)
			{
				((pppoe_hdr*)PPPOEBUF)->dest_addr[i] = 0xFF;
			}
			memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
			((pppoe_hdr*)PPPOEBUF)->ether_type = htons(UIP_ETHTYPE_PPPOE_DISCOVERY);
			((pppoe_hdr*)PPPOEBUF)->vhl = 0x11;
			((pppoe_hdr*)PPPOEBUF)->pppoe_code = PPPOE_CODE_PADI;
			((pppoe_hdr*)PPPOEBUF)->session_id = PPPOE_CODE_SESS;
			((pppoe_hdr*)PPPOEBUF)->payload_length = 0x0010;
			tags->name = PPPOE_TAG_SERVICE_NAME;
			tags->length = 0;
			*(U16_T*) tags->value = PPPOE_TAG_HOST_UNIQ;
			*(U16_T*) (tags->value + 2) = 0x0008;
			*(U32_T*) (tags->value + 4) = 0x06000000;
			*(U32_T*) (tags->value + 8) = 0x0B000000;

			/* Set Process state */
			pppoe_info.state = PPPOE_CODE_PADI;

			break;
		case PPPOE_CODE_PADR:
			/* Analysis PADO packet and record needed information */
			Analysis_Tags((U8_T*)tags, ((pppoe_hdr*)PPPOEBUF)->payload_length);

			if(host_uniq_from_PADO[0] != 0x06 && host_uniq_from_PADO[4] != 0x0b)
			{
				return 0; /* If host uniq not for us, don't create/send PADR */
			}

			/* Keep mac of PPPoE server*/
			memcpy(pppoe_server_mac, ((pppoe_hdr*)PPPOEBUF)->src_addr, 6);

			/* Create PADR packet */
			memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, ((pppoe_hdr*)PPPOEBUF)->src_addr, 6);
			memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
			((pppoe_hdr*)PPPOEBUF)->ether_type = htons(UIP_ETHTYPE_PPPOE_DISCOVERY);
			((pppoe_hdr*)PPPOEBUF)->vhl = 0x11;
			((pppoe_hdr*)PPPOEBUF)->pppoe_code = PPPOE_CODE_PADR;
			((pppoe_hdr*)PPPOEBUF)->session_id = PPPOE_CODE_SESS;
			((pppoe_hdr*)PPPOEBUF)->payload_length = (U16_T)as_length[0] + (U16_T)as_length[1] + 20;
			tags->name = PPPOE_TAG_SERVICE_NAME;
			tags->length = as_length[1];
			memcpy(tags->value, service_name, tags->length);
			*(U16_T*) (tags->value + tags->length) = PPPOE_TAG_HOST_UNIQ;
			*(U16_T*) (tags->value + tags->length + 2) = 0x0008;
			*(U32_T*) (tags->value + tags->length + 4) = 0x06000000;
			*(U32_T*) (tags->value + tags->length + 8) = 0x0C000000;
			*(U16_T*) (tags->value + tags->length + 12) = PPPOE_TAG_AC_COOKIE;
			*(U16_T*) (tags->value + tags->length + 14) = as_length[0];

			memcpy((tags->value + tags->length + 16), ac_cookie, as_length[0]);

			/* Set Process state */
			pppoe_info.state = PPPOE_CODE_PADR;
			break;
		case PPPOE_CODE_PADS:
			/* Analysis PADS packet and record needed information */
			Analysis_Tags((U8_T*)tags, ((pppoe_hdr*)PPPOEBUF)->payload_length);
			if(host_uniq_from_PADO[0] != 0x06 && host_uniq_from_PADO[4] != 0x0c)
			{
				return 0; /* If host uniq not for us, don't process session stage */
			}
			/* Keep Session ID from PPPoE server */
			session_id = ((pppoe_hdr*)PPPOEBUF)->session_id;

			/* Set Process state */
			pppoe_info.state = PPPOE_CODE_PADS;

			return 1;
		default:
			printd("Nothing to do in Discovry of PPPoE !\r\n");
			return 0;
	}

	/* Set uip length */
	TCPIP_SetXmitLength(20 + ((pppoe_hdr*)PPPOEBUF)->payload_length);

	/* Send packet */
	ETH_SendPkt((U8_T XDATA*)PPPOEBUF, TCPIP_GetXmitLength());

	return 1;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Session_Handle
 * Purpose: Handle session stage of PPPoE
 * Params: *PPPOEBUF: Pointer of packet / state: internal state
 * Returns: 0: Not wanted packet / 1: Wanted packet, and process it
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T PPPOE_Session_Handle(U8_T *PPPOEBUF, U16_T state)
{
	ppp_handshake_session	*phs = (ppp_handshake_session*)&(((pppoe_hdr*)PPPOEBUF)->payload_data[0]);

	switch(state)
	{
		case 1:
			if(phs->protocol != LCP && phs->ppp_code == 0x01)
			{
				return 0;
			}

			/* Create LCP ACK */
			memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, ((pppoe_hdr*)PPPOEBUF)->src_addr, 6);
			memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
			((pppoe_hdr*)PPPOEBUF)->ether_type = htons(UIP_ETHTYPE_PPPOE_SESSION);
			((pppoe_hdr*)PPPOEBUF)->vhl = 0x11;
			((pppoe_hdr*)PPPOEBUF)->pppoe_code = PPPOE_CODE_SESS;
			((pppoe_hdr*)PPPOEBUF)->session_id = session_id;
			((pppoe_hdr*)PPPOEBUF)->payload_length = phs->length + 2;
			phs->ppp_code = 0x02;

			/* Save magic number from server */
			/* Length of Hinet is 18, length of linux-pppoe is 14 */
			if(phs->length == 14)
			{
				pppoe_info.lcp_length = 14;
				memcpy(magic_number_server, &(phs->option[6]), 4);
			}
			else
			{
				pppoe_info.lcp_length = 18;
				memcpy(magic_number_server, &(phs->option[10]), 4);
			}

			/* Copy the number to client and +1 */
			memcpy(magic_number_client, magic_number_server, 4);
			magic_number_client[3] += 1;

			pppoe_info.state = PPPOE_CODE_PADS_1;
		    break;
		case 2:
			/* Create LCP request */
			phs->protocol = LCP;
			phs->ppp_code = 0x01;
			phs->ppp_id = pppoe_info.identifier;
			phs->length = 14;
			*(U16_T*)phs->option = 0x0104;
			*(U16_T*)(phs->option + 2) = 0x05c8;
			if(pppoe_info.lcp_length != 14)
			{
				phs->option[4] = 0x05;
				phs->option[5] = 0x06;
			}
			memcpy(phs->option + 6, magic_number_client, 4);
			((pppoe_hdr*)PPPOEBUF)->payload_length = phs->length + 2;
			pppoe_info.state = PPPOE_CODE_PADS_2;
			break;
		case 3:
			if(phs->protocol == LCP && Check_Magic(&(phs->option[6]), 1) == 1 && phs->ppp_code == 0x02 && phs->ppp_id == 0)
			{
				pppoe_info.state = PPPOE_CODE_PADS_3;
				return 1;
			}
			else
			{
				return 0;
			}
		case 4:
			/* Create identification_1 packet */
			memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, pppoe_server_mac, 6);
			memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
			((pppoe_hdr*)PPPOEBUF)->ether_type = htons(UIP_ETHTYPE_PPPOE_SESSION);
			((pppoe_hdr*)PPPOEBUF)->pppoe_code = PPPOE_CODE_SESS;
			((pppoe_hdr*)PPPOEBUF)->vhl = 0x11;
			((pppoe_hdr*)PPPOEBUF)->session_id = session_id;
			((pppoe_hdr*)PPPOEBUF)->payload_length = 18 + 2;
			phs->protocol = LCP;
			phs->ppp_code = 0x0c;
			phs->ppp_id = ++pppoe_info.identifier;
			phs->length = 18;
			memcpy(phs->option, magic_number_client, 4);
			memcpy(&(phs->option[4]), pppoe_identification_1, 10);

			break;
		case 5:
			/* Create identification_2 packet */
			memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, pppoe_server_mac, 6);
			memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
			((pppoe_hdr*)PPPOEBUF)->ether_type = htons(UIP_ETHTYPE_PPPOE_SESSION);
			((pppoe_hdr*)PPPOEBUF)->pppoe_code = PPPOE_CODE_SESS;
			((pppoe_hdr*)PPPOEBUF)->vhl = 0x11;
			((pppoe_hdr*)PPPOEBUF)->session_id = session_id;
			phs->protocol = LCP;
			phs->ppp_code = 0x0c;
			phs->ppp_id = ++pppoe_info.identifier;
			phs->length = 20;
			memcpy(phs->option, magic_number_client, 4);
			memcpy(&(phs->option[4]), pppoe_identification_2, 12);

			((pppoe_hdr*)PPPOEBUF)->payload_length = phs->length + 2;

			break;
		case 6:
			/* Create Authentication request packet */
			memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, pppoe_server_mac, 6);
			memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
			((pppoe_hdr*)PPPOEBUF)->ether_type = htons(UIP_ETHTYPE_PPPOE_SESSION);
			((pppoe_hdr*)PPPOEBUF)->pppoe_code = PPPOE_CODE_SESS;
			((pppoe_hdr*)PPPOEBUF)->vhl = 0x11;
			((pppoe_hdr*)PPPOEBUF)->session_id = session_id;
			phs->protocol = PAP;
			phs->ppp_code = 0x01;
			phs->ppp_id = ++pppoe_info.identifier;
			phs->length = strlen(pppoe_username) + strlen(pppoe_password) + 6;

			phs->option[0] = strlen(pppoe_username);
			memcpy(&(phs->option[1]), pppoe_username, strlen(pppoe_username));
			phs->option[1+strlen(pppoe_username)] = strlen(pppoe_password);
			memcpy(&(phs->option[2 + strlen(pppoe_username)]), pppoe_password, strlen(pppoe_password));

			((pppoe_hdr*)PPPOEBUF)->payload_length = phs->length + 2;
			pppoe_info.state = PPPOE_CODE_PADS_4;
			break;
		case 7:
			/* Check if authentication successful */
			if(phs->protocol == PAP && phs->ppp_code == 0x03)
			{
				printd("PPPoE Account or Password not correct!\r\n");
				return 1;
			}

			/* Check if the packet is IPCP/Request */
			if(phs->protocol != IPCP && phs->ppp_code != 0x01)
			{
				return 0;
			}
			/* Create IPCP/ACK packet */
			memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, ((pppoe_hdr*)PPPOEBUF)->src_addr, 6);
			memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
			phs->ppp_code = 0x02;

			/* Keep the IP address of PPPoE server */
			memcpy(pppoe_server_ip, &(phs->option[2]), 4);

			/* Please careful below code's sequence */
			pppoe_info.state = PPPOE_CODE_PADS_5;
			break;
		case 8:
			/* Create IPCP request packet */
			memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, pppoe_server_mac, 6);
			memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
			phs->ppp_code = 0x01;
			phs->ppp_id = ++pppoe_info.identifier;
			phs->length = 22;
			((pppoe_hdr*)PPPOEBUF)->payload_length = phs->length + 2;
			*(U16_T*)(phs->option) = 0x0306;
			*(U32_T*)(phs->option + 2) = 0;
			*(U16_T*)(phs->option + 6) = 0x8106;
			*(U32_T*)(phs->option + 8) = 0;
			*(U16_T*)(phs->option + 12) = 0x8306;
			*(U32_T*)(phs->option + 14) = 0;

			pppoe_info.state = PPPOE_CODE_PADS_6;
			break;
		case 9:
			if(phs->protocol == IPCP && phs->ppp_code == 0x03)
			{
				/* Create IPCP request with IP packet */
				memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, ((pppoe_hdr*)PPPOEBUF)->src_addr, 6);
				memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
				phs->ppp_code = 0x01;
				phs->ppp_id = ++pppoe_info.identifier;
				phs->length = 22;
				((pppoe_hdr*)PPPOEBUF)->payload_length = phs->length + 2;

				/* Keep IP, Primary/Secondary DNS */
				memcpy(pppoe_ip, &(phs->option[2]), 4);
				memcpy(pppoe_pri_dns, &(phs->option[8]), 4);
				memcpy(pppoe_sec_dns, &(phs->option[14]), 4);

				pppoe_info.state = PPPOE_CODE_PADS_7;
				break;
			}
			return 0;
		case 10:
			/* Check last IPCP Ack */
			if(phs->protocol == IPCP && phs->ppp_code == 0x02 && phs->ppp_id == pppoe_info.identifier)
			{
				/* Reset identifier */
				pppoe_info.identifier = 0;

				/* Set IP to latest IP from PPPoE server */
				STOE_SetIPAddr(*(U32_T*)pppoe_ip);
				TCPIP_SetIPAddr(*(U32_T*)pppoe_ip);

				/* Set PPPoE connection successful */
				pppoe_successful = 1;
				PPPOE_GetDnsFlag = 1;

				pppoe_info.state = PPPOE_CODE_PADS_8;
				printd("Finish the PPP handshake !! %lx\r\n", *(U32_T*)pppoe_ip);
				return 1;
			}
			return 0;
		default:
			return 0;
	}

	/* Set uip length */
	TCPIP_SetXmitLength(20 + ((pppoe_hdr*)PPPOEBUF)->payload_length);

	/* Send packet */
	ETH_SendPkt((U8_T XDATA*)PPPOEBUF, TCPIP_GetXmitLength());

	return 1;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Get_Session
 * Purpose: Called by other process and return session_id
 * Params:
 * Returns: Session ID that got from PPPoE server
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T PPPOE_Get_Session(void)
{
	return session_id;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Analysis_Tags
 * Purpose: If connection type is PPPoE, this function maintain any packets
 *		while receiving data.
 * Params:  buf = Head pointer of packet, length = Length of packet
 * Returns: 1: Useful packet, 0: Drop the packet
 * Note:
 * ----------------------------------------------------------------------------
 */
void Analysis_Tags(U8_T *tags, U16_T length)
{
	U8_T	i;

	for(i = 0; i < length; i++)
	{
		/* Service name */
		if((*(tags + i) == 0x01) && (*(tags + i + 1) == 0x01))
		{
			as_length[1] = *(U16_T*)(tags+i+2);
			memcpy(service_name, (tags + i + 4), *(U16_T*)(tags+i+2));
		}

		/* Host-uniq */
		if((*(tags + i) == 0x01) && (*(tags + i + 1) == 0x03))
		{
			memcpy(host_uniq_from_PADO, (tags + i + 4), *(U16_T*)(tags+i+2));
		}

		/* AC-Cookie */
		if((*(tags + i) == 0x01) && (*(tags + i + 1) == 0x04))
		{
			as_length[0] = *(U16_T*)(tags+i+2);
			memcpy(ac_cookie, (tags + i + 4), *(U16_T*)(tags+i+2));
		}
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Check_Magic
 * Purpose: Check if magic number as same as keeped.
 * Params: magic:Pointer of magic number / sc: 0(server), 1(client)
 * Returns: 0: different / 1: Same
 * Note: *magic is a pointer of magic number, sc is flag for checking server 
 *			or client
 * ----------------------------------------------------------------------------
 */
U8_T Check_Magic(U8_T *magic, U8_T sc)
{
	U8_T	i;
	
	for(i = 0; i < 4; i++)
	{
		if(sc)
		{
			if(magic[i] != magic_number_client[i])
				return 0;
		}
		else
		{
			if(magic[i] != magic_number_server[i])
				return 0;
		}
	}
	return 1;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Check_Echo
 * Purpose: Check if echo request sent from PPPoE server, reply echo
 * Params: U8_T *buf
 * Returns: 0: Not echo request / 1: echo request, and reply it
 * Note: 
 * ----------------------------------------------------------------------------
 */
U8_T Check_Echo(U8_T *buf)
{
	pppoe_hdr*	PPPOEBUF = (pppoe_hdr*)buf;
	ppp_handshake_session	*phs = (ppp_handshake_session*)&(((pppoe_hdr*)PPPOEBUF)->payload_data[0]);

	/* See if packet is echo request of PPPoE server and for us */
	if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_SESSION && PPPOEBUF->session_id == PPPOE_Get_Session()
		&& phs->protocol == LCP && Check_Magic(&(phs->option[0]), 0) == 1 && phs->ppp_code == 0x09)
	{
		/* Create echo reply */
		memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, ((pppoe_hdr*)PPPOEBUF)->src_addr, 6);
		memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
		((pppoe_hdr*)PPPOEBUF)->ether_type = htons(UIP_ETHTYPE_PPPOE_SESSION);
		((pppoe_hdr*)PPPOEBUF)->vhl = 0x11;
		((pppoe_hdr*)PPPOEBUF)->pppoe_code = PPPOE_CODE_SESS;
		((pppoe_hdr*)PPPOEBUF)->session_id = session_id;
		phs->ppp_code = 0x0a;
		memcpy(phs->option, magic_number_client, 4);
		((pppoe_hdr*)PPPOEBUF)->payload_length = phs->length + 2;

		/* Set uip length */
		TCPIP_SetXmitLength(20 + ((pppoe_hdr*)PPPOEBUF)->payload_length);

		/* Send packet */
		ETH_SendPkt((U8_T XDATA*)PPPOEBUF, TCPIP_GetXmitLength());

		return 1;
	}
	return 0;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: Check_Terminate
 * Purpose: Check if terminate request sent from PPPoE server, reply terminate
 *			ack
 * Params: U8_T *buf
 * Returns: 0: Not terminate request / 1: terminate request, and reply ack
 * Note: 
 * ----------------------------------------------------------------------------
 */
U8_T Check_Terminate(U8_T *buf)
{
	pppoe_hdr*	PPPOEBUF = (pppoe_hdr*)buf;
	ppp_handshake_session	*phs = (ppp_handshake_session*)&(((pppoe_hdr*)PPPOEBUF)->payload_data[0]);

	/* See if packet is terminate request of PPPoE server and for us */
	if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_DISCOVERY && PPPOEBUF->session_id == PPPOE_Get_Session()
		&& PPPOEBUF->pppoe_code == 0xa7)	/* PADT in PPPoE */
	{
#if 0
		/* Create PADT packet */
		memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, ((pppoe_hdr*)PPPOEBUF)->src_addr, 6);
		memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, uip_ethaddr.addr, 6);

		/* Set uip length */
		TCPIP_SetXmitLength(20 + ((pppoe_hdr*)PPPOEBUF)->payload_length);

		/* Send packet */
		ETH_SendPkt((U8_T XDATA*)PPPOEBUF, TCPIP_GetXmitLength());
#endif
		/* Set PPPoE connection fail */
		pppoe_successful = 0;

		/* Reset internal state of PPPoE */
		PPPOE_Init();

		return 1;
	}
	else if(PPPOEBUF->ether_type == UIP_ETHTYPE_PPPOE_SESSION && PPPOEBUF->session_id == PPPOE_Get_Session()
			&& phs->protocol == LCP && phs->ppp_code == 0x05)	/* Terminate in PPP */
	{
		/* Create Terminate ack */
		memcpy(((pppoe_hdr*)PPPOEBUF)->dest_addr, ((pppoe_hdr*)PPPOEBUF)->src_addr, 6);
		memcpy(((pppoe_hdr*)PPPOEBUF)->src_addr, pppoe_MacAddr, 6);
		((pppoe_hdr*)PPPOEBUF)->ether_type = htons(UIP_ETHTYPE_PPPOE_SESSION);
		((pppoe_hdr*)PPPOEBUF)->vhl = 0x11;
		((pppoe_hdr*)PPPOEBUF)->pppoe_code = PPPOE_CODE_SESS;
		((pppoe_hdr*)PPPOEBUF)->session_id = session_id;
		phs->ppp_code = 0x06;
		((pppoe_hdr*)PPPOEBUF)->payload_length = phs->length + 2;

		/* Set uip length */
		TCPIP_SetXmitLength(20 + ((pppoe_hdr*)PPPOEBUF)->payload_length);

		/* Send packet */
		ETH_SendPkt((U8_T XDATA*)PPPOEBUF, TCPIP_GetXmitLength());

		return 1;
	}
	return 0;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Setup_Account
 * Purpose: Setup username and password in PPPoE module.
 * Params: U8_T *username U8_T *password
 * Returns:
 * Note: 
 * ----------------------------------------------------------------------------
 */
void PPPOE_Setup_Account(U8_T *username, U8_T *password)
{
	/* Update username and password by external string */
	strcpy(pppoe_username, username);
	strcpy(pppoe_password, password);
	PPPOE_Init();	
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: PPPOE_Get_DNS
 * Purpose: Get the DNS's ip address after connection successful.
 * Params: U8_T *pri_dns U8_T *sec_dns
 * Returns:
 * Note: 
 * ----------------------------------------------------------------------------
 */
void PPPOE_Get_DNS(U8_T *pri_dns, U8_T *sec_dns)
{
	memcpy(pri_dns, pppoe_pri_dns, 4);
	memcpy(sec_dns, pppoe_sec_dns, 4);
}
