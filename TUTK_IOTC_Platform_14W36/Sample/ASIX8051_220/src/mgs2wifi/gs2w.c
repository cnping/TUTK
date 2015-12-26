/*
 ******************************************************************************
 *     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 /*============================================================================
 * Module Name: gs2w.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gs2w.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "main.h"
#include "adapter.h"
#include "gs2w.h"
#include "gtcpdat.h"
#include "gudpdat.h"
#include "gudpbc.h"
#include "gudpmc.h"
#include "gudpuc.h"
#include "httpd.h"
#include "gconfig.h"
#include "printd.h"
#include "mac.h"
#include "console.h"
#if (INCLUDE_TELNET_SERVER)
#include "telnet.h"
#endif
#if (INCLUDE_EVENT_DETECT)
#include "gevent.h"
#endif
#if (INCLUDE_DNS_CLIENT)
  #include "dnsc.h"
#endif

#if (INCLUDE_DHCP_SERVER)
#include "dhcps.h"
#endif

#include "at24c02.h"
#include "ping.h"

#if (INCLUDE_HTTP_SERVER)
#include "httpdap.h"
#endif
/* NAMING CONSTANT DECLARATIONS */

/* MACRO DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static U8_T XDATA gs2w_State;
static U8_T XDATA gs2w_ConnType;

/* LOCAL SUBPROGRAM DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GS2W_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GS2W_Task(void)
{
	GCONFIG_Task();
    if (gs2w_ConnType == GS2W_CONN_UDP)
	    GUDPDAT_Task();
    else if (gs2w_ConnType == GS2W_CONN_TCP)
	    GTCPDAT_Task();
	GUDPMC_Task();
	GUDPBC_Task();
	GUDPUC_Task();
#if (INCLUDE_HTTP_SERVER)
	HTTPAP_Task();
#endif
    
#if (INCLUDE_TELNET_SERVER)
	CONSOLE_Task();
    TELNET_Task();
#endif

#if (INCLUDE_EVENT_DETECT)
	GEVENT_Task();
#endif

#if (INCLUDE_DNS_CLIENT)
	DNSC_Task();
#endif	

	PING_Task();

#if (INCLUDE_DHCP_SERVER)
    if (GCONFIG_GetDhcpSrvStatus())
        DHCPS_ConnsCheck();
#endif
} /* End of GS2W_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GS2W_Init()
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GS2W_Init(void)
{
    U8_T state;

	gs2w_State = GS2W_STATE_IDLE; 

	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_PROTO_UDP_MCAST) == GCONFIG_NETWORK_PROTO_UDP_MCAST)
		GUDPMC_Init(GCONFIG_UDP_MCAST_SERVER_PORT);

	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_PROTO_UDP_BCAST) == GCONFIG_NETWORK_PROTO_UDP_BCAST ||
		(GCONFIG_GetNetwork() & GCONFIG_NETWORK_PROTO_UDP_MCAST) != GCONFIG_NETWORK_PROTO_UDP_MCAST
	   )
	{
		if((GCONFIG_GetNetwork() & GCONFIG_NETWORK_PROTO_UDP_MCAST) != GCONFIG_NETWORK_PROTO_UDP_MCAST)
		{
			MAC_SetRxFilter(MAC_RCV_BROADCAST);
		}
		GUDPBC_Init(GCONFIG_UDP_BCAST_SERVER_PORT);
	}

	GUDPUC_Init(GCONFIG_UDP_UCAST_SERVER_PORT);

#if (INCLUDE_HTTP_SERVER)
	HTTP_Init();
#endif

	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_PROTO_UDP) == GCONFIG_NETWORK_PROTO_UDP)
	{
        gs2w_ConnType = GS2W_CONN_UDP;
		GUDPDAT_Init(GCONFIG_GetServerDataPktListenPort());
	}
	else
	{
        gs2w_ConnType = GS2W_CONN_TCP;
		GTCPDAT_Init(GCONFIG_GetServerDataPktListenPort());
	}

    
#if (INCLUDE_TELNET_SERVER)
	CONSOLE_Init();
    TELNET_Init();
#endif

#if (INCLUDE_EVENT_DETECT)
	GEVENT_Init();
#endif

#if (INCLUDE_DNS_CLIENT)
	DNSC_Init();
	DNSC_SetServerIP(GCONFIG_GetDNS());
	//DNSC_SetServerIP(0xc0a80201);
#endif	

	PING_Init();

#if (INCLUDE_DHCP_SERVER)
    if (GCONFIG_GetDhcpSrvStatus())
    {
        state = DHCPS_GetServerStatus();
        DHCPS_Init(GCONFIG_GetDhcpSrvStartIp(), GCONFIG_GetDhcpSrvEndIp(),
                   GCONFIG_GetDhcpSrvNetmask(), GCONFIG_GetDhcpSrvDefGateway(),
                   GCONFIG_GetDhcpSrvLeaseTime(), GCONFIG_GetDhcpSrvStatus());
        if (state != 0xFF)
            DHCPS_SetServerStatus(state);
    }
#endif
} /* End of GS2W_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GS2W_GetTaskState
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GS2W_GetTaskState(void)
{
	return gs2w_State;
} /* End of GS2W_GetTaskState() */
/*
 * ----------------------------------------------------------------------------
 * Function Name: GS2W_SetTaskState
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GS2W_SetTaskState(U8_T state)
{
	gs2w_State = state;
} /* End of GS2W_SetTaskState() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GS2W_GetConnType
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GS2W_GetConnType(void)
{
	return gs2w_ConnType;
} /* End of GS2W_GetConnType() */


/* End of gs2w.c */
