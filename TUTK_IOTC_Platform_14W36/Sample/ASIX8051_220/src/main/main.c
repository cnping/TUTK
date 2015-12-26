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
 * Module Name: main.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: main.c,v $
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include "mcpu_cfg.h"
#include "main.h"
#include "mcpu.h"
#include "adapter.h"
#include "mstimer.h"
#include "ax22000.h"
#include "interrupt.h"
#include "stoe.h"
#include "mac.h"
#include "dma.h"
#include "tcpip.h"
#include "hsuart.h"
#include "hsuart2.h"
#include "printd.h"
#include "delay.h"
#include "uart0.h"
#include "mwif.h"
#include "mwioctl.h"
#include "filesys.h"
#include "httpd.h"
#include "at24c02.h"
#include <stdlib.h>
#include "IOTCAPIs.h"

#if (INCLUDE_DHCP_CLIENT)
  #include "dhcpc.h"
#endif
#if (INCLUDE_DHCP_SERVER)
  #include "dhcps.h"
#endif
#include "gs2w.h"
#include "gconfig.h"
#include <string.h>

/* NAMING CONSTANT DECLARATIONS */
#ifdef DEBUG
#define DBGMSG(A) {A}
#else
#define DBGMSG(A) {}
#endif

#define TIME_OUT_COUNTER	(250/SWTIMER_INTERVAL)

/* GLOBAL VARIABLES DECLARATIONS */
U8_T InitWifi = 1;
/* LOCAL VARIABLES DECLARATIONS */
static U8_T main_EnableFactorySetting=0;
static U8_T main_AppInit=1;
#if (INCLUDE_DHCP_CLIENT)
U8_T  cmdDhcpFlag = 0;
U32_T dhcpTimeStart = 0;
#endif

U32_T main_MDefaultTimeStart = 0;
U32_T main_MDefaultTimeNow = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void UpdateIpSettings(U32_T);
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
extern U8_T GUDPMC_Multicast[MAC_ADDRESS_LEN];
static void main_ConfigWifi(void);
#endif //#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)

/*
 * ----------------------------------------------------------------------------
 * Function Name: UpdateIpSettings
 * Purpose: Update IP address, subnet mak, gateway IP address and DNS IP address 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void UpdateIpSettings(U32_T ip)
{
	if (ip > 0)	
	{
		printd("IP: %bu.%bu.%bu.%bu\n\r", (U8_T) ((ip >> 24) & 0x000000FF),
									      (U8_T) ((ip >> 16) & 0x000000FF),
								  	      (U8_T) ((ip >> 8) & 0x000000FF),
									      (U8_T) (ip & 0x000000FF));
	}
	else
	{
		U32_T mask = GCONFIG_GetNetmask();
		U32_T gateway = GCONFIG_GetGateway();
		ip = GCONFIG_GetServerStaticIP();
		
#if (INCLUDE_DHCP_CLIENT)
		TCPIP_SetIPAddr(ip);
#endif
		STOE_SetIPAddr(ip);
#if (INCLUDE_DHCP_CLIENT)
		TCPIP_SetSubnetMask(mask);
#endif
		STOE_SetSubnetMask(mask);
#if (INCLUDE_DHCP_CLIENT)
		TCPIP_SetGateway(gateway);
#endif
		STOE_SetGateway(gateway);
	}
    if (main_AppInit)
    {
	    GS2W_Init();
        main_AppInit = 0;
    }
} /* End of UpdateIpSettings */

#if (INCLUDE_DHCP_CLIENT)
/*
 * ----------------------------------------------------------------------------
 * Function Name: MAIN_RestartDhcpClient
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MAIN_RestartDhcpClient()
{
    if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_DHCP_ENABLE) == 0)
        return;

    cmdDhcpFlag = 1;
    dhcpTimeStart = SWTIMER_Tick();
#if (STOE_TRANSPARENT)
    ETH_SetDhcpFlag(1);
#endif
#if (INCLUDE_DHCP_SERVER)
    if (GCONFIG_GetDhcpSrvStatus())
        DHCPS_SetServerStatus(0);
#endif
    DHCP_ResetState();
    DHCP_Start();

} /* End of MAIN_RestartDhcpClient() */
#endif

void SessionRead(void)
{
	struct st_SInfo Sinfo;
	int status, nRead;
	int SID;
	char buf[32];
	
	for(SID =0; SID < MAX_CLIENT_NUMBER; SID++)
	{
		status = IOTC_Session_Check(SID, &Sinfo);
		if((status == IOTC_ER_SESSION_CLOSE_BY_REMOTE) || (status == IOTC_ER_REMOTE_TIMEOUT_DISCONNECT))
		{
			printd("Session already close = %d\n", SID);
			IOTC_Session_Close(SID);
			continue;
		}
		
		nRead = IOTC_Session_Read(SID, buf, IOTC_MAX_PACKET_SIZE, 10, 0);
		
		if(nRead > 0)
		{
			unsigned char i, val;
			
			buf[nRead] = 0;
			#if 0
			if(Sinfo.Mode ==0)
				printd("Client is from %s:%d[%s] Mode=P2P\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
			else
				printd("Client is from %s:%d[%s] Mode=RLY\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
			#endif
			if(strncmp(buf, "DO", 2) == 0)
			{
				i = atoi(&buf[4]);
				
				switch(i)
				{
					case 0:
					val = P3_0;
					P3_0 = !val;
					break;
					case 1:
					val = P3_1;
					P3_1 = !val;
					break;
					case 2:
					val = P3_2;
					P3_2 = !val;
					break;
					case 3:
					val = P3_3;
					P3_3 = (!val);
					break;
				}
				
			}
			
		}
	}
}

#define BROCAST_DIO_TIME_COUNT  10

void BroadCastDIO(void)
{
	static unsigned long preTimeCount = 0;
	unsigned long nowTimeCount = SWTIMER_Tick();
	if((nowTimeCount - preTimeCount) > BROCAST_DIO_TIME_COUNT)
	{
		int SID;
		char far buf[16];
		struct st_SInfo Sinfo;
		unsigned char val = P3;
		preTimeCount = nowTimeCount;
		sprintf(buf,"%bu%bu%bu%bu%bu%bu%bu%bu", !(val & 0x01), !(val & (0x01 << 1)), !(val & (0x01<<2)), !(val & (0x01<<3)), !(P3 & (0x01<<4)), !(val & (0x01<<5)), !(val & (0x01<<6)), !(val & (0x01<<7)));
		
		buf[8] = 0;
		
		for(SID =0; SID < MAX_CLIENT_NUMBER; SID++)
		{
			if(IOTC_Session_Check(SID, &Sinfo) != IOTC_ER_NoERROR)
				continue;			
			IOTC_Session_Write(SID,buf,8, 0);
		}
		
	}
}

void EnterAdHocMode()
{
	char ssid[16];
	strcpy(ssid, "AXIS220");
	#if (INCLUDE_DHCP_SERVER)
	DHCPS_SetServerStatus(1);
	#endif
	IO_SsidRW(IO_COMMAND_WRITE, strlen(ssid), ssid);
	IO_OpenSysRW(1, 0);
	IO_NetworkModeRW(1, 2);/* 2/3: Ad-hoc/infra */
	printd("NO IP into Ad-hoc mode\n");
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: main
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void main(void)
{
#if (!AX_ETH_INT_ENABLE)
	ERROR: Must enable ethernet module in this driver.
#endif
#if (INCLUDE_DHCP_CLIENT)
    U32_T 	dhcpTimeStop = 0;
	U8_T    dhcpArpRequest = 0;
	U8_T    dhcpDeclineCount = 0;
#endif
	U32_T	timeCount,preTimeCount, timeElapse, ip;

	/* System Initiation */
	MCPU_Init();

	/* UART0 Console Interface Initiation */
	UART_Init();
	printd("UART-0 init ok.\n\r");
	/* UART2 High Speed Serial Interface Initiation */
	HSUR2_Setup(HS_BR80M_115200, (HSLCR_CHAR_8|HSLCR_STOP_10),
	(HSIER_RDI_ENB|HSIER_RLSI_ENB|HSIER_RBRFI_ENB|HSIER_DOEI_ENB|HSIER_FCCRI_ENB|HSIER_TDCI_ENB),
	(HSFCR_HSUART_ENB|HSFCR_FIFOE|HSFCR_RFR|HSFCR_TFR|HSFCR_TRIG_08), (HSMCR_RTS|HSMCR_DEREC_MASTER));
	printd("UART-2 init ok.\n\r");
		
	printd("AX220xx RS-232 to WiFi Device Server Firmware - Version %s\n\r", GCONFIG_VERSION_STRING);

	EEPROM_Init();

	/* Initialize Network adapter */
	ETH_Init();

#if (INCLUDE_DHCP_CLIENT)
	if (DHCP_Init())
		printd("DHCP client init ok.\n\r");
#endif

	MWIF_Init();

	/* Generic serial-to-ethernet module initialization */
	GCONFIG_Init();

	SWTIMER_Init();

#if (INCLUDE_DHCP_CLIENT)	
	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_DHCP_ENABLE) == GCONFIG_NETWORK_DHCP_ENABLE)
	{
		printd("DHCP client request... ");
		DHCP_Start();
#if (!STOE_TRANSPARENT)
		STOE_DisableIpFilter();
#else
        ETH_SetDhcpFlag(1);
#endif
		cmdDhcpFlag = 1;
	}
	else
	{
		printd("DHCP client disable.\n\r");
		UpdateIpSettings(0);
	}
#else
	printd("DHCP client disable.\n\r");
	GS2W_Init();
#endif
	ETH_Start();	
	MWIF_Start();	
	SWTIMER_Start();

	DELAY_Ms(1000);

#if (INCLUDE_DHCP_CLIENT)
	if (cmdDhcpFlag == 1)
	{
		dhcpTimeStart = SWTIMER_Tick();
	}
#endif

	HSUR2_Start();
	
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
	/* Notice WCPU that MCPU had ready */
	IO_NoticeWcpu(IO_MCPU_WCPU_READY, NULL, 0);

    /* Check the WiFi link has established */
    preTimeCount = (U16_T)SWTIMER_Tick();
    while (1)
    {
		if ((InitWifi) && (MCPU_WcpuReady))
		{
			main_ConfigWifi(); /* WiFi default setting */
			InitWifi = 0;
		}

        if (STOE_ConnectState & STOE_WIRELESS_LINK)
            break;

		timeCount = (U16_T)SWTIMER_Tick();
		if ((timeCount- preTimeCount) > (SWTIMER_COUNT_SECOND * 5))
            break;

		MWIF_Sys();
    }

    preTimeCount = 0;
#endif
	
	while (1)
	{
		HSUR2_ErrorRecovery();
		STOE_DeferredProcess();
		
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
		if ((InitWifi) && (MCPU_WcpuReady))
		{
			main_ConfigWifi(); /* WiFi default setting */
			InitWifi = 0;
		}
#endif

		ETH_SendArpToGateway(ETH_CONTINUE_ARP_REQUEST_TO_GATEWAY_AFTER_REPLY);

#if (INCLUDE_DHCP_CLIENT)
		if (cmdDhcpFlag == 1)
		{
			if (DHCP_GetState() > DHCP_IDLE_STATE)
			{
				DHCP_Send();
				dhcpTimeStop = SWTIMER_Tick();

				if ((dhcpTimeStop - dhcpTimeStart) > (SWTIMER_COUNT_SECOND * 5))
				{
					printd(" fail!\n\r");
					UpdateIpSettings(0);
#if (STOE_TRANSPARENT)
                    ETH_SetDhcpFlag(0);
#endif
#if (INCLUDE_DHCP_SERVER)
                    if (GCONFIG_GetDhcpSrvStatus())
                        DHCPS_SetServerStatus(1);
#endif
					cmdDhcpFlag = 0;
					
					EnterAdHocMode();
				}
			}
			else
			{
                ip = STOE_GetIPAddr();
				// Check if the IP address is already in use.
				if (dhcpArpRequest == 0) // 0: Not yet send out the ARP request
				{
					ETH_SendDhcpArpRequest();
					dhcpArpRequest = 1;  // 1: Have sent out the ARP request 

					dhcpTimeStart = SWTIMER_Tick();
				}

				dhcpTimeStop = SWTIMER_Tick();

                if (uip_findarptable((U16_T *)&ip) != 0xFF)
				{ // Received an ARP reply, IP address confliction occurs					
					if (dhcpDeclineCount < MAX_DHCP_DECLINE_COUNT)
					{
						printd("DHCP client decline. \n\r");
						DHCP_RejectOffer(); // Reject the IP address and request a new one
						dhcpDeclineCount++;
						dhcpArpRequest = 0;
				
						printd("DHCP client request... ");
						DHCP_Start();
						dhcpTimeStart = SWTIMER_Tick();
				 	}
					else
					{
						printd(" fail!\n\r");						
						UpdateIpSettings(0); // Failed to get a valid IP address, use the static IP address
#if (!STOE_TRANSPARENT)
						STOE_EnableIpFilter();
#else
                        ETH_SetDhcpFlag(0);
#endif
#if (INCLUDE_DHCP_SERVER)
                        if (GCONFIG_GetDhcpSrvStatus())
                            DHCPS_SetServerStatus(1);
#endif
						cmdDhcpFlag = 0;
						
						EnterAdHocMode();
					}
				}
				else
				{		 
					if ((dhcpTimeStop - dhcpTimeStart) > (SWTIMER_COUNT_SECOND * 3)) // Timeout, i.e., no IP address confliction
					{
						ip = STOE_GetIPAddr();
						
						if (ip == 0)
						{
							printd(" fail!\n\r");
						}
						UpdateIpSettings(ip);
#if (!STOE_TRANSPARENT)
						STOE_EnableIpFilter();
#else
                        ETH_SetDhcpFlag(0);
#endif
#if (INCLUDE_DHCP_SERVER)
                        /* when the dhcp client got IP, then disable the dhcp server */
                        if (ip)
                            DHCPS_SetServerStatus(0);
                        else
                        {
                        	EnterAdHocMode();
                        }
#endif
						cmdDhcpFlag = 0;
						
						IOTC_Initialize(0, "m1.iotcplatform.com", "m2.iotcplatform.com", "m4.iotcplatform.com", "m5.iotcplatform.com");
						printd("IOTC_Initialize OK!!\n");
						IOTC_Device_Login("W0000000000000000009", NULL, NULL);
						printd("IOTC_Device_Login OK\n");
					}
				}
			}
		}
#endif

		timeCount = (U16_T)SWTIMER_Tick();
		if ((timeCount- preTimeCount)>= TIME_OUT_COUNTER)
		{
			preTimeCount = timeCount;
			TCPIP_PeriodicCheck();

#if (!STOE_TRANSPARENT)
			ETH_ArpTableTimer(); /* update for every 250ms */
#endif
		}
		
#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)	/* interrupt mode */
		if (STOE_GetInterruptFlag())
		{
			STOE_ProcessInterrupt();
		}
#else	/* polling mode */
		STOE_ProcessInterrupt();
#endif //#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)

#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
		if (MAC_GetInterruptFlag())
		{
#if (!MAC_GET_INTSTATUS_MODE)
			if (MAC_GetInterruptFlag())
			{
				MAC_ProcessInterrupt();
			}
#else
			MAC_LinkSpeedChk();
#endif //#if (!MAC_GET_INTSTATUS_MODE)
		}
#endif //#if (MAC_ARBIT_MODE & MAC_ARBIT_ETH)
#if (INCLUDE_DHCP_CLIENT)
		if (cmdDhcpFlag == 0)
		{
			IOTC_Listen(0);
			SessionRead();
			BroadCastDIO();
			IOTC_PeriodicRun();
			GS2W_Task();
	   	}
#else
			GS2W_Task();
#endif //#if (INCLUDE_DHCP_CLIENT)

		HSUR2_XmitOutDeq();
		
		MWIF_Sys();

        /* Check the button is pressed still. */
        if (TCON & BIT1)
        {
            if (!main_EnableFactorySetting)
            {
                main_EnableFactorySetting = 1;
                main_MDefaultTimeStart = SWTIMER_Tick();
            }

            main_MDefaultTimeNow = SWTIMER_Tick();
            if (main_MDefaultTimeNow >= main_MDefaultTimeStart)
                timeElapse = main_MDefaultTimeNow - main_MDefaultTimeStart;
            else
                timeElapse = (0xFFFFFFFF - main_MDefaultTimeStart) + main_MDefaultTimeNow;

            if (timeElapse >= (5 * SWTIMER_COUNT_SECOND))
            {
		        GCONFIG_ReadDefaultConfigData();
		        GCONFIG_WriteConfigData();
		        DELAY_Ms(500);
		        MCPU_SoftReboot();
            }
        }
        else
            main_EnableFactorySetting = 0;
	}
} /* End of main() */

#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
/*
 * ----------------------------------------------------------------------------
 * Function Name: main_ConfigWifi
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void main_ConfigWifi(void)
{
	U8_T i, Buf[64];
	U16_T j;

	/* Set baseband mode */
	IO_OperateModeRW(1, GCONFIG_GetWifiBasebandMode() ? 1:1);/* 1/1: 802.11b+g */

	/* set network type */
	IO_NetworkModeRW(1, GCONFIG_GetWifiNetworkMode() ? 2:3);/* 2/3: Ad-hoc/infra */
	
	/* set channel */
	IO_ChannelRW(1, IO_WiFiSupportedCh[GCONFIG_GetChannel()]);
	
	/* set ssid */
	i = GCONFIG_GetSsid(Buf);
	IO_SsidRW(1, i, Buf);
			
	/* set tx rate */
	IO_TxDataRateRW(1, GCONFIG_GetWifiTxRate());
		
	/* set tx power level */
	IO_TxDefaultPowerLevelRW(1, GCONFIG_GetWifiTxPowerLevel());

	/* set preamble mode */
	IO_PreambleRW(1, GCONFIG_GetWifiPreambleMode() ? 20:0); /* 0/20: long/auto */	
	
	/* set beacon interval */
	j = GCONFIG_GetWifiBcnInterval();
	IO_BeaconIntervalRW(1, (U8_T XDATA*)&j);
	
	/* set RTS threshold */
	j = GCONFIG_GetWifiRtsThreshold();
	IO_RtsThresholdRW(1, (U8_T XDATA*)&j);

	/* set auto power control ON/OFF */
	IO_AutoPowerCtrlRW(1, GCONFIG_GetWifiAutoPowerCtrl() ? 1:0);

	/* set wep key index */
	IO_WepKeyIndexRW(1, GCONFIG_GetWifiWepKeyIndex());

	/* set wep 64-bits key */
	GCONFIG_GetWifiWep64Key(0, Buf);
	IO_Wep64KeyRW (1, 0, Buf);
	
	GCONFIG_GetWifiWep64Key(1, Buf);
	IO_Wep64KeyRW (1, 1, Buf);
		
	GCONFIG_GetWifiWep64Key(2, Buf);
	IO_Wep64KeyRW (1, 2, Buf);
		
	GCONFIG_GetWifiWep64Key(3, Buf);
	IO_Wep64KeyRW (1, 3, Buf);		

	/* set wep 128-bits key */
	GCONFIG_GetWifiWep128Key(0, Buf);	
	IO_Wep128KeyRW (1, 0, Buf);	
	
	GCONFIG_GetWifiWep128Key(1, Buf);
	IO_Wep128KeyRW (1, 1, Buf);	
	
	GCONFIG_GetWifiWep128Key(2, Buf);
	IO_Wep128KeyRW (1, 2, Buf);	
	
	GCONFIG_GetWifiWep128Key(3, Buf);
	IO_Wep128KeyRW (1, 3, Buf);
	
	/* set tkip/aes pre-share key */
	i = GCONFIG_GetWifiPreShareKey(Buf);
	IO_TkipAesRW(1, i, Buf);

    /* set encryption type */
   	i = GCONFIG_GetWifiEncryptMode();

	if (i == 2)
	{
		IO_OpenSysRW(1, 0); /* open system */
		IO_EncryptionRW(1, 5); /* wep128 */
	}
	else if (i == 3)
	{
		IO_OpenSysRW(1, 3); /* mixed mode */
		IO_EncryptionRW(1, 2); /* tkip */
	}
	else if (i == 1) /* do not know current encrypt mode */
	{
		IO_OpenSysRW(1, 0); /* open system */
		IO_EncryptionRW(1, 1); /* wep64 */
	}
	else if (i == 4)
	{
		IO_OpenSysRW(1, 3); /* mixed mode */
		IO_EncryptionRW(1, 4);
	}
	else /* 0:disable, 1:wep64 */
	{
		IO_OpenSysRW(1, 0); /* open system */
		IO_EncryptionRW(1, i);
	}
	
	/* radio on */
	IO_RadioOnOff(1);
	
    /* enable the WCPU firmware to receive mcast packet */
    if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_PROTO_UDP_MCAST) == GCONFIG_NETWORK_PROTO_UDP_MCAST)
        IO_SetMulticastMac(GUDPMC_Multicast);

    /* set traffic profile */
	IO_TrafficProfile(1, 0);

	/* get WCPU MAC address */
	IO_WiFiMacRW(0, 0);
	
	/* get WCPU F/W version */	
	IO_WifiDriverVer();
	
	printd("WiFi init ok.\n\r");	
} /* End of main_ConfigWifi() */
#endif
/* End of adapter.c */
