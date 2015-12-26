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
 * Module Name: pppoe.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: pppoe.h,v $
 *=============================================================================
 */

#ifndef __PPPOE_H__
#define __PPPOE_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* Data length definition */
#define ETH_MAX_DATA     1500
#define PPPOE_MAX_DATA   (ETH_MAX_DATA - 8)

/* Code of PPPoE */
#define PPPOE_CODE_PADI  0x09    /* Active Discovery Initiation */
#define PPPOE_CODE_PADO  0x07    /* Active Discovery Offer packet */
#define PPPOE_CODE_PADR  0x19    /* Active Discovery Request */
#define PPPOE_CODE_PADS  0x65    /* Active Discovery Session-confirmation */
#define PPPOE_CODE_PADT  0xA7    /* Active Discovery Terminate */
#define PPPOE_CODE_PADM  0xD3    /* Active Discovery Message packet */
#define PPPOE_CODE_PADN  0xD4    /* Active Discovery Network packet */
#define PPPOE_CODE_SESS  0x00    /* code when we have a session */

/* Special definition for handshake of session */
#define PPPOE_CODE_PADS_1	0x66
#define PPPOE_CODE_PADS_2	0x67
#define PPPOE_CODE_PADS_3	0x68
#define PPPOE_CODE_PADS_4	0x69
#define PPPOE_CODE_PADS_5	0x6a
#define PPPOE_CODE_PADS_6	0x6b
#define PPPOE_CODE_PADS_7	0x6c
#define PPPOE_CODE_PADS_8	0x6d

/* These 16-bit tags are on network order (RFC2516) */
#define PPPOE_TAG_END_LIST       0x0000
#define PPPOE_TAG_SERVICE_NAME   0x0101
#define PPPOE_TAG_AC_NAME        0x0102
#define PPPOE_TAG_HOST_UNIQ      0x0103
#define PPPOE_TAG_AC_COOKIE      0x0104
#define PPPOE_TAG_VENDOR_SPES    0x0105
#define PPPOE_TAG_RELAY_SESS     0x0106
#define PPPOE_TAG_HOST_URL       0x0111
#define PPPOE_TAG_MOTM           0x0112
#define PPPOE_TAG_IP_ROUTE_ADD   0x0121
#define PPPOE_TAG_SERVICE_ERR    0x0201
#define PPPOE_TAG_AC_SYSTEM_ERR  0x0202
#define PPPOE_TAG_GENERIC_ERR    0x0203

/* Below 16-bit definition are used protocol by PPP handshake */
#define LCP		0xc021	/* Link Control Protocol */
#define PAP		0xc023	/* Password Authentication Protocol */
#define CHAP	0xc223	/* Challenge Handshake Authentication Protocol */
#define IPCP	0x8021	/* IP Control Protocol */

/* PPPoE data struct */
typedef struct _pppoe_hdr {
	/* Ethernet header */
	U8_T	dest_addr[6];
	U8_T	src_addr[6];
	U16_T	ether_type;
	/* PPPoE header. */
	U8_T	vhl;
	U8_T	pppoe_code;
	U16_T	session_id;
	U16_T	payload_length;
	/* Payload */
	U8_T	payload_data[PPPOE_MAX_DATA];
}pppoe_hdr;

/* Pointer definition */
//#define PPPOEBUF ((struct pppoe_hdr *)&uip_buf[0])

/* PPPoE tag */
typedef struct _tag {
	U16_T	name;
	U16_T	length;
	U8_T	value[PPPOE_MAX_DATA-4];
}tag;

/* PPP handshake */
typedef struct _ppp_handshake_session {
	U16_T	protocol;
	U8_T	ppp_code;
	U8_T	ppp_id;
	U16_T	length;
	U8_T	option[PPPOE_MAX_DATA-10];
}ppp_handshake_session;

/* PPPoE Information */
typedef struct _pppoe_info {
	/* PPPoE state */
	U8_T	state;
	U8_T	identifier;
	U8_T	wait_count_master;
	U16_T	wait_count_slave;
	U16_T	lcp_length;
} PPPOE_INFO;


/* GLOBAL VARIABLES */
extern U8_T PPPOE_GetDnsFlag;
#define	PPPOE_GetDNSFlag()		(PPPOE_GetDnsFlag)
#define PPPOE_ClearDNSFlag()	(PPPOE_GetDnsFlag = 0)


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	PPPOE_Init(void);
void	PPPOE_Command(void);
U8_T	PPPOE_GetState(void);
void	PPPOE_Send(U8_T* buf, U16_T length);
U8_T	PPPOE_Receive(U8_T *buf, U16_T length);
void	PPPOE_Setup_Account(U8_T *username, U8_T *password);
void	PPPOE_Get_DNS(U8_T *pri_dns, U8_T *sec_dns);


#endif /* End of __PPPOE_H__ */

/* End of pppoe.h */