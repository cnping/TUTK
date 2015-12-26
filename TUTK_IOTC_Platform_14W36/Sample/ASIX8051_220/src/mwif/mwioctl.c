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
 * Module Name: wioctl.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: wioctl.c,v $
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "ax22000.h"
#include "mwioctl.h"
#include <string.h>
#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
#include "uart0.h"
#include "printd.h"
#include "mcpu.h"
#include "stoe.h"
#else			/***** Middle of IO_CPU_TYPE, Below are for WCPU use *****/
#include <stdio.h>
#include "uart.h"
#include "wcpu.h"
#include "mac.h"
#include "hw.h"
#include "pcib.h"
#include "mgr.h"
#include "wifi.h"
#include "wpsave.h"
#include "supplicant.h"
#include "delay.h"
#endif			/***** End of IO_CPU_TYPE macro *****/

#if IO_CPU_TYPE
  #if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
    #define IO_SHOWCMD						0	// show the command detail byte by byte

	#include "mwifapp.h"
  #else
    #define IO_SHOWCMD						0	// always 0  
  #endif
#else /* for WiFi CPU */
    #define IO_SHOWCMD						0
#endif

/* GLOBAL VARIABLES DECLARATIONS */
U8_T XDATA	IO_RxBuf[MAX_IO_RX_BUFFER_SIZE] = {0};
U16_T XDATA	IO_RxHead;
U16_T XDATA	IO_RxTail;
//U8_T XDATA	IO_CmdFlag = 0;

#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
  #if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
U8_T XDATA	IO_WifiRFEnable = 0;
U8_T XDATA	IO_WifiBasebandMode = 1; /* 802.11b+g mode*/
U8_T XDATA	IO_WifiNetworkMode = 1; /* 802.11 ad hoc mode */
U8_T const FAR	IO_WiFiSupportedCh[] =
{0/*auto*/,1,2,3,4,5,6,7,8,9,10,11,36,40,44,48,149,153,157,161,165};
U8_T XDATA	IO_WiFiSupportedChBufSize = sizeof (IO_WiFiSupportedCh);
U8_T XDATA	IO_WifiChannel = 11; /* channel = 11 */
U8_T XDATA	IO_WiFiSsid[33] = {'A','X','2','2','0','x','x',' ','D','e','m','o',' ','F','i','r','m','w','a','r','e'};
U8_T XDATA	IO_WifiSsidLen = 21;
U8_T XDATA	IO_WiFiTxRate = 0; /* auto */
U8_T const FAR	IO_WiFiSupportTxRate[] =
{0/*auto*/,2/*1M*/,4/*2M*/,11/*5.5M*/,22/*11M*/,12/*6M*/,18,24,36,48,72,96,108};
U8_T XDATA	IO_WiFiTxPowerLevel = 0; /* default 100% */
U8_T XDATA	IO_GBProtection = 0; /* auto */
U8_T XDATA	IO_PreambleMode = 0; /* auto */
U16_T XDATA	IO_BcnInterval = 100; /* 100 ms */
U16_T XDATA	IO_RtsThreshold = 2432; /* bytes */
U8_T XDATA	IO_AutoPowerCtrl = 1; /* on */
SISRVY_T XDATA	IO_SiteSurveyTable[20];	/* max num = 20 */
U8_T XDATA	IO_FoundBssNum = 0;
U8_T XDATA	IO_JbssIndex = 0xff;
U8_T XDATA	IO_SiteSurveyStatusFlag = 0; /*0:nothing, 1:to start sisrvy, 1:had finish*/
U8_T XDATA	IO_EncryptMode = 0; /* 0:disable, 1:wep64, 2:wep128, 3:tkip, 4:aes */
U8_T XDATA	IO_WepKeyIndex = 0;
U8_T XDATA	IO_WepKeyLength = 0; /* 0:64 bits, 1:128 bits*/
U8_T XDATA	IO_Wep64Key[4][5] = {0x12,0x34,0x56,0x78,0x90, 0x09,0x87,0x65,0x43,0x21,
0xA1,0xB2,0xC3,0xD4,0xE5,0x01, 0x23,0x45,0x67,0x89};
U8_T XDATA	IO_Wep128Key[4][13] = {
0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34};
U8_T XDATA	IO_PreShareKey[64] = {'1','2','3','4','5','6','7','8'};
U8_T XDATA	IO_PreShareKeyLen = 8;
  #endif /* (MAC_ARBIT_MODE & MAC_ARBIT_WIFI) */
#else			/***** Middle of IO_CPU_TYPE, Below are for WCPU use *****/
U8_T XDATA	IO_SiteSurveyFromMcpu = 0;
U8_T XDATA	IO_ScanFromMcpu = 0;
#endif			/***** End of IO_CPU_TYPE macro *****/

/* LOCAL VARIABLES DECLARATIONS */
static U8_T	XDATA	io_TxBuf[MAX_IO_TX_BUFFER_SIZE] = {0};

/************************************************************/
/* LOCAL SUBPROGRAM DECLARATIONS */
#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
  #if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
static void io_ScanReportAck(void);
static void io_SensitivityTestReportACK(void);
static void io_WlanDebug1Resp(void);
  #endif /* (MAC_ARBIT_MODE & MAC_ARBIT_WIFI) */
#else			/***** Middle of IO_CPU_TYPE, Below are for WCPU use *****/
static void io_ResetAck(void);
static void io_ScanAck(void);
static void io_SiteSurveyAck(void);
static void io_JoinBssAck(void);
static void io_SsidAckRW(U8_T type, U8_T len, U8_T XDATA* pReg);
static void io_OperateModeAckRW(U8_T type, U8_T value);
static void io_ChannelAckRW(U8_T type, U8_T value);
static void io_NetworkModeAckRW(U8_T type, U8_T value);
static void io_SetMulticastMacAck(void);
static void io_WiFiMacAckRW(U8_T type, U8_T XDATA* pReg);
static void io_RadioOnOffAck(U8_T state);
static void io_BgProtectionRespRW(U8_T rw, U8_T state);
static void io_EncryptionAckRW(U8_T type, U8_T value);
static void io_OpenSysAckRW(U8_T type, U8_T value);
static void io_Wep64KeyAckRW(U8_T type, U8_T XDATA* pReg);
static void io_Wep128KeyAckRW(U8_T type, U8_T XDATA* pReg);
static void io_WepKeyIndexKeyRW(U8_T type, U8_T value);
static void io_TkipAesAckRW(U8_T type, U8_T len, U8_T XDATA* pReg);
static void io_FragmentThresholdAckRW(U8_T type, U8_T XDATA* pValue);
static void io_RtsIntervalAckRW(U8_T type, U8_T XDATA* pValue);
static void io_PreambleAckRW(U8_T type, U8_T value);
static void io_TxGainRW(U8_T rw, U8_T txGain);
static void io_BeaconIntervalAckRW(U8_T type, U8_T* pValue);
static void io_CountryRegionAckRW(U8_T type, U16_T value);
static void io_AtimIntervalAckRW(U8_T type, U8_T XDATA* pValue);
static void io_TxDataRateRW(U8_T rw, U8_T txRate);
static void io_AutoPowerCtrlRW(U8_T rw, U8_T state);
static void io_AutoPowerCtrlDetailW(void);
static void io_RoamingRW(U8_T rw, U8_T state);
static void io_WifiMultimediaSetRW(U8_T rw, U8_T value);
static void io_ListenIntervalRW(U8_T rw, U16_T value);
static void io_MinContentionWindowRW(U8_T rw, U16_T value);
static void io_MaxContentionWindowRW(U8_T rw, U16_T value);
static void io_ShowRssiResp(U8_T rssi);
static void io_PayloadLenAddInWifiRW(U8_T rw, U8_T value);
static void io_RetainLlcInWifiRW(U8_T rw, U8_T value);
static void io_2LlcInWiFiVlanPacketRW(U8_T rw, U8_T value);
static void io_TrafficProfileRW(U8_T rw, U8_T state);
static void io_TxDefaultPowerLevelRW(U8_T rw, U8_T level);
static void io_QueuePrioritySetRW(U8_T rw, U8_T priority);
static void io_SelfCtsRW(U8_T rw, U8_T value);
static void io_BssidR(U8_T *bssid);
static void io_WifiIpR(U8_T *wifiIp);
static void io_WifiDriverVerR(U8_T *pVer);
#if MASS_PRODUCTION
static void io_ContinuousTxAck(U8_T mode);
static void io_SensitivityTestAck(U8_T *ptr);
static void io_SensitivityTestReportAck(void);
#endif
static U8_T io_SfrIO(U8_T rw, U8_T index, U8_T XDATA* pValue);
static void io_SfrAckRW(U8_T type, U8_T value);
static void io_WcpuIndirectAckRW(U8_T type, U8_T len, U8_T XDATA* pReg);
static void io_MiibIndirectAckRW(U8_T type, U8_T len, U8_T XDATA* pReg);
static void io_PcibIndirectRegAckRW(U8_T type, U8_T len, U8_T XDATA* pReg);
static void io_PciConfigAckRW(U8_T type, U8_T XDATA* pReg);
static void io_WiFiRegAckRW(U8_T type, U8_T XDATA* pValue);
static void io_WlanPktFilterAckRW(U8_T rw, U8_T type, U16_T subtype, U8_T XDATA* pBssid);
static void io_SynthesizerSetAck(void);
static void io_RetryAckRW(U8_T type, U8_T value);
static void io_Gpio0RW(U8_T rw, U8_T value);
static void io_Gpio1RW(U8_T rw, U8_T value);
static void io_Gpio2RW(U8_T rw, U8_T value);
static void io_Gpio3RW(U8_T rw, U8_T value);
static U8_T io_WifiRegIO(U8_T rw, U16_T index, U32_T XDATA* pValue);
#endif			/***** End of IO_CPU_TYPE macro *****/
#if IO_SHOWCMD
static void io_ShowCmd(U8_T TxRx, U8_T XDATA* pReg);
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Init
 * Purpose: To initiate "MCPU / WCPU interface" module
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Init(void)
{
	IO_RxHead = 0;
	IO_RxTail = 0;
#if MWIF_TASK_HOOKUP
    MWIFAPP_TaskInit();
#endif
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_CmdError
 * Purpose: To send a error message to MCPU when MCPU sent a wrong command
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_CmdError(void)
{
	MWIF_Send(IO_ERROR_COMMAND, NULL, 0);
	
}	/* End of IO_CmdError */

/************************************************************************/
#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
  #if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_CmdParsing
 * Purpose: To parse the command it gets and do the reaction
 * Params: none
 * Returns: 0x00 - IO_COMMAND_DONE
 *          0xFF - IO_COMMAND_ERROR
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T IO_CmdParsing(U8_T* pbuf)
{
	U8_T XDATA	i, temp;

	if ((pbuf[0] + pbuf[1]) != 0xFF)
		return IO_COMMAND_ERROR;
	{
		switch (pbuf[0])
		{
		case IO_MCPU_WCPU_READY:
			MCPU_WcpuReady = 1;
			IO_NoticeWcpu(IO_MCPU_WCPU_READY_ACK, NULL, 0);
			printd ("Rcv WCPU Ready Command.\n");
			break;
		case IO_MCPU_WCPU_READY_ACK:
			MCPU_WcpuReady = 1;
			printd ("Rcv MCPU Ready ACK Command.\n");
			break;
		case IO_WCPU_RESET_ACK:
			printd ("Rcv WCPU Reset ACK Command.\n");
			// need to add reset wcpu function.........
			break;
		case IO_MCPU_WCPU_CONNECT:
			STOE_ConnectState |= STOE_WIRELESS_LINK;
			IO_NoticeWcpu(IO_MCPU_WCPU_CONNECT_ACK, NULL, 0);
			printd ("Rcv WiFi Connect Command.\n");

			MWIFAPP_TaskIOWiFiConnect();
			break;
		case IO_MCPU_WCPU_DISCONNECT:
			STOE_ConnectState &= ~STOE_WIRELESS_LINK;
			temp = 1;
			IO_NoticeWcpu(IO_MCPU_WCPU_DISCONNECT_RESP, &temp, 1);
			printd ("Rcv WiFi Disconnect Command.\n");
			break;
		case IO_SCAN_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_SCAN_REPORT:
			if (!pbuf[2])
				return IO_COMMAND_ERROR;
			if (pbuf[3] == 1)
			{
				printd ("Start a new IBSS\n\r");
			}
			else if (pbuf[3] == 2)
			{
				printd ("Rescan\n\r");
			}
			else if (pbuf[3] == 3)
			{
				printd ("BSS is found:");
				for (i = 0; i < 72; i++)
				{
					if ((i%16) == 0)
					{
						printd ("\n\r%2bx ", pbuf[i + 4]);
					}
					else
					{
						printd ("%2bx ", pbuf[i + 4]);
					}
				}
				printd ("\n\r");
			}
			else
			{
				return IO_COMMAND_ERROR;
			}
			io_ScanReportAck();
			break;
		case IO_SITE_SURVEY_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_SITE_SURVEY_REPORT:
			if (!pbuf[2])
				return IO_COMMAND_ERROR;
			if (pbuf[2] == 1)
			{
				if (pbuf[3] == 0)
					printd ("\n\rIt can not find any BSS.\n\r");
				else
					return IO_COMMAND_ERROR;
			}
			else
			{
				U8_T XDATA*		point;

				if (pbuf[2] < 11)
				{
					return IO_COMMAND_ERROR;
				}

				if (pbuf[3] == 0)
				{
					printd ("\nid BSSID             SSID                             TYPE CH  RSSI SECURITY\n\r");
				}
				printd ("\r%02bx %02bx-%02bx-%02bx-%02bx-%02bx-%02bx ", pbuf[3], pbuf[4], pbuf[5],
						pbuf[6], pbuf[7], pbuf[8], pbuf[9]);
				temp = pbuf[2] - 12;

				IO_FoundBssNum = pbuf[3];
				point = IO_SiteSurveyTable[IO_FoundBssNum].Bssid;
				point[0] = pbuf[4];
				point[1] = pbuf[5];
				point[2] = pbuf[6];
				point[3] = pbuf[7];
				point[4] = pbuf[8];
				point[5] = pbuf[9];

				IO_SiteSurveyTable[IO_FoundBssNum].SsidLen = temp;
				point = IO_SiteSurveyTable[IO_FoundBssNum].Ssid;
				
				if (pbuf[15])
				{
					for (i = 0; i < temp; i++)
					{
						printd ("%c", pbuf[15 + i]);
						point[i] = pbuf[15 + i];
					}
					
					temp = 33 - temp;
					for (i = 0; i < temp; i++)
					{
						printd (" ");
					}
				}
				else
				{
					printd ("                                 ");
				}

				if (pbuf[10])
				{
					printd ("ap   %03bu %02bx   %s\n\r", pbuf[11], pbuf[12], pbuf[13] ? "yes":"no");
				}
				else
				{
					printd ("sta  %03bu %02bx   %s\n\r", pbuf[11], pbuf[12], pbuf[13] ? "yes":"no");
				}

				IO_SiteSurveyTable[IO_FoundBssNum].InfraMode = pbuf[10];
				IO_SiteSurveyTable[IO_FoundBssNum].Channel = pbuf[11];
				IO_SiteSurveyTable[IO_FoundBssNum].Rssi = pbuf[12];
				IO_SiteSurveyTable[IO_FoundBssNum].SecurFlag = pbuf[13];
				IO_FoundBssNum++;

				IO_SiteSurveyReportAck();

				if (pbuf[14])
				{
					IO_SiteSurveyStatusFlag = 2;
				}
			}
			break;
		case IO_JOIN_BSS_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			printd ("\r BSS joined              \n\r");
			break;
		case IO_SSID_READ_RESP:
			if (pbuf[2] > 32)
				return IO_COMMAND_ERROR;
			printd ("\r Read SSID: ");
			for (temp = 3; temp < (pbuf[2] + 3); temp++)
			{
				printd ("%c", pbuf[temp]);
			}
			if (temp < 15)
				printd ("          \n\r");
			else
				printd ("\n\r");
			break;
		case IO_SSID_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_OPERATE_MODE_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;

				IO_SiteSurveyStatusFlag = 1;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("\r Read Baseband Mode: %bd", pbuf[4]);
				switch (pbuf[4])
				{
					case 1:
						printd (" (802.11b/g mixed mode)\n\r");
						break;
					case 2:
						printd (" (Pure 802.11g mode)\n\r");
						break;
					case 3:
						printd (" (Pure 802.11b mode)\n\r");
						break;
					case 4:
						printd (" (Pure 802.11a mode)\n\r");
						break;
					default:
						printd ("\n Not the correct baseband Mode!\n\r");
						return IO_COMMAND_ERROR;
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_CHANNEL_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("\r Read Current Channel: %bu\n\r", pbuf[4]);

				MWIFAPP_TaskIOReadChannel(pbuf);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_NETWORK_MODE_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("\r Read Network Type: %bu    \n\r", pbuf[4]);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_MULTICAST_SET_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_WIFI_MAC_ADDR_SET_RESP:	// 0xB6
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 7)
					return IO_COMMAND_ERROR;
				printd ("\r Read WiFi MAC address: ");
				for (temp = 4; temp < 10; temp++)
					printd (" %02bx", pbuf[temp]);
				printd ("\n\r");

			    MWIFAPP_TaskIOWiFiMacAddrSet(pbuf);
			}
			break;
		case IO_WIFI_RADIO_ON_OFF_ACK:
			if (pbuf[3] == 0)
			{
				IO_WifiRFEnable = 0;
				printd("\r Disable RF.                       \n");
			}
			else if (pbuf[3] == 1)
			{
				IO_WifiRFEnable = 1;
				printd("\r Enble RF.                         \n");
			}
			else if (pbuf[3] == 2)
				printd("\r RF is enabling now.               \n");
			else if (pbuf[3] == 3)
				printd("\r RF is disabling now.              \n");
			break;
		case IO_WIFI_B_G_PROTECTION_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("\r BG Protection: %s", pbuf[4] ? "auto" : "off");
			}
			else
			{
				return IO_COMMAND_ERROR;
			}
			break;
		case IO_ENCRYPTION_TYPE_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;

				/* do not know encryption type, be used in web,site survey,jbss case */
				if (IO_EncryptMode == 5)
				{
					switch (pbuf[4])
					{
					case 0: /* No encrypt */
						IO_EncryptMode = 0;
						break;
					case 1: /* WEP64 */
						IO_EncryptMode = 1;
						break;
					case 2: /* TKIP */
						IO_EncryptMode = 3;
						break;
					case 4: /* WPA/WPA2 */
						IO_EncryptMode = 4;
						break;
					case 5: /* WEP128 */
						IO_EncryptMode = 2;
						break;
					default:
						return IO_COMMAND_ERROR;
					}

					MWIFAPP_TaskIOReadEncryptType();
				}
				else
				{
					printd ("\r Read Encryption Type: %bd", pbuf[4]);
					switch (pbuf[4])
					{
					case 0:
	
						IO_EncryptMode = 0;
						printd (" (No encrypt)\n\r");
						break;
					case 1:
						IO_EncryptMode = 1;
						printd (" (WEP64)\n\r");
						break;
					case 2:
						IO_EncryptMode = 3;
						printd (" (TKIP)\n\r");
						break;
					case 4:
						IO_EncryptMode = 4;
						printd (" (AES)\n\r");
						break;
					case 5:
						IO_EncryptMode = 2;
						printd (" (WEP128)\n\r");
						break;
					default:
						printd ("\n Not the correct encryption type!\n\r");
						return IO_COMMAND_ERROR;
					}
				}
			}
			else
			{
				return IO_COMMAND_ERROR;
			}
			break;
		case IO_OPEN_SYSTEM_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("\r Read Authentication Algorithm: %bd", pbuf[4]);
				switch (pbuf[4])
				{
					case 0:
						printd (" (Open system)\n\r");
						break;
					case 1:
						printd (" (Shared key)\n\r");
						break;
					case 2:
						printd (" (WPA1WPA2 only)\n\r");
						break;
					case 3:
						printd (" (WPA1WPA2 mixed)\n\r");
						break;
					default:
						printd ("\n Not the correct Authentication Algorithm!\n\r");
						return IO_COMMAND_ERROR;
				}
			}
			else
			{
				return IO_COMMAND_ERROR;
			}
			break;
		case IO_WEP64_KEY_READ_RESP:
			if (pbuf[2] != 5)
				return IO_COMMAND_ERROR;
			printd ("\r");
			for (temp = 3; temp < (pbuf[2] + 3); temp++)
			{
				printd (" %02bx", pbuf[temp]);
			}
			printd ("               \n\r");
			break;
		case IO_WEP64_KEY_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_WEP128_KEY_READ_RESP:
			if (pbuf[2] != 13)
				return IO_COMMAND_ERROR;
			printd ("\r");
			for (temp = 3; temp < (pbuf[2] + 3); temp++)
			{
				printd (" %02bx", pbuf[temp]);
			}
			printd ("\n\r");
			break;
		case IO_WEP128_KEY_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_WEP_KEY_INDEX_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd("\r Read Key index: %bd          \n\r", pbuf[4]);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_TKIP_AES_PASSPHASE_STRING_READ_RESP:
			if (!pbuf[2] || pbuf[2] > 64)
				return IO_COMMAND_ERROR;
			printd ("\r Read tkip aes Passphrase: ");
			for (temp = 3; temp < (pbuf[2] + 3); temp++)
			{
				printd ("%c", pbuf[temp]);
			}
			printd ("\n\r");
			break;
		case IO_TKIP_AES_PASSPHASE_STRING_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_ERROR_COMMAND:	// 0x0B
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			printd("Drop bad command\n\r");
			break;
		case IO_FRAGMENT_THRESHOLD_READ_RESP:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			printd ("\r Read Fragment threshold: %u\n", *((U16_T XDATA*)&pbuf[3]));
			break;
		case IO_FRAGMENT_THRESHOLD_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_RTS_THRESHOLD_READ_RESP:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			printd ("\r Read RTS threshold: %u\n", *((U16_T XDATA*)&pbuf[3]));
			break;
		case IO_RTS_THRESHOLD_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_PREAMBLE_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("\r Read Preamble mode: %bd", pbuf[4] >> 5);
				switch (pbuf[4])
				{
				case 0:
					printd (" (Long)\n\r");
					break;
				case BIT5:
					printd (" (Short)\n\r");
					break;
				default:
					printd ("\n Not the correct mode!\n\r");
					return IO_COMMAND_ERROR;
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_TX_POWER_LEVEL_READ_RESP:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			printd ("\r Read Tx power gain: 0x%02bx\n\r", pbuf[3]);
			break;
		case IO_TX_POWER_LEVEL_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_BEACON_INTERVAL_READ_RESP:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			IO_BcnInterval = *((U16_T XDATA*)&pbuf[3]);
			printd ("\r Read Beacon Interval: %u ms\n\r", IO_BcnInterval);

			MWIFAPP_TaskIOReadBeaconInterval();

			break;
		case IO_BEACON_INTERVAL_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_COUNTRY_REGION_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 3)
					return IO_COMMAND_ERROR;
				printd ("\r Read Country Region: 0x%02bX%02bX", pbuf[4], pbuf[5]);
				switch (*((U16_T XDATA *)(&pbuf[4])))
				{
				case 0x0010:
					printd (" (USA)\n\r");
					break;
				case 0x0020:
					printd (" (Canada)\n\r");
					break;
				case 0x0030:
					printd (" (Most of Europe1)\n\r");
					break;
				case 0x0083:
					printd (" (Most of Europe2)\n\r");
					break;
				case 0x0031:
					printd (" (Spain)\n\r");
					break;
				case 0x0032:
					printd (" (France1)\n\r");
					break;
				case 0x0084:
					printd (" (France2)\n\r");
					break;
				case 0x0040:
					printd (" (Japan1)\n\r");
					break;
				case 0x0041:
					printd (" (Japan2)\n\r");
					break;
				case 0x0049:
					printd (" (Japan3)\n\r");
					break;
				case 0x0043:
					printd (" (China)\n\r");
					break;
				case 0x0048:
					printd (" (Taiwan)\n\r");
					break;
				case 0x0000:
					printd (" (All areas)\n\r");
					break;
				default:
					printd ("\n Not the correct Country Region number!\n\r");
					break;
				}
			}
			else
			{
				return IO_COMMAND_ERROR;
			}
			break;
		case IO_ATIM_INTERVAL_READ_RESP:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			printd ("\r Read ATIM Windows: %u ms\n\r", *((U16_T XDATA*)&pbuf[3]));
			break;
		case IO_ATIM_INTERVAL_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_TX_DATA_RATE_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)	// write
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("\r Read current Tx data rate: %2bx", pbuf[4]);
				switch (pbuf[4])
				{
				case 0:
					printd (" (Auto)\n\r");
					break;
				case 0x02:
					printd (" (1M)\n\r");
					break;
				case 0x04:
					printd (" (2M)\n\r");
					break;
				case 0x0B:
					printd (" (5.5M)\n\r");
					break;
				case 0x0C:
					printd (" (6M)\n\r");
					break;
				case 0x12:
					printd (" (9M)\n\r");
					break;
				case 0x16:
					printd (" (11M)\n\r");
					break;
				case 0x18:
					printd (" (12M)\n\r");
					break;
				case 0x24:
					printd (" (18M)\n\r");
					break;
				case 0x30:
					printd (" (24M)\n\r");
					break;
				case 0x48:
					printd (" (36M)\n\r");
					break;
				case 0x60:
					printd (" (48M)\n\r");
					break;
				case 0x6C:
					printd (" (54M)\n\r");
					break;
				default:
					printd ("\n Not the correct Tx data rate!\n\r");
					break;
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_AUTO_POWER_CONTROL_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				printd ("successfully!!\n\r");
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4])
				{
					printd("Enable auto power control.\n\r");
				}
				else
				{
					printd("Disable auto power control.\n\r");
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WIFI_ROAMING_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4])
				{
					printd("Enable roaming.\n\r");
				}
				else
				{
					printd("Disable roaming.\n\r");
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WIFI_MULTIMEDIA_SET_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4])
				{
					printd("Enable WMM.\n\r");
				}
				else
				{
					printd("Disable WMM.\n\r");
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_LISTEN_INTERVAL_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 3)
					return IO_COMMAND_ERROR;
				printd("Listen Interval: %d.\n\r", *((U16_T*)&pbuf[4]));
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_MINIMUM_CONTENTION_WINDOW:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 3)
					return IO_COMMAND_ERROR;
				printd("Minimum contention window: %d.\n\r", *((U16_T*)&pbuf[4]));
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_MAXIMUM_CONTENTION_WINDOW:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 3)
					return IO_COMMAND_ERROR;
				printd("Maximum contention window: %d.\n\r", *((U16_T*)&pbuf[4]));
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_SHOW_RSSI_RESP:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			printd ("\r Read RSSI value: %bx    \n\r", pbuf[3]);
			break;
		case IO_PAYLOAD_LENGTH_ATTACH_IN_WIFI_PACKET_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4])
				{
					printd("Attach payload length in WiFi packet.\n\r");
				}
				else
				{
					printd("Don't attach payload length in WiFi packet.\n\r");
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_RETAIN_LLC_IN_WIFI_PACKET_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4])
				{
					printd("Retain LLC in WiFi packet.\n\r");
				}
				else
				{
					printd("Don't retain LLC in WiFi packet.\n\r");
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_2_LLC_IN_WIFI_VLAN_PACKET_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4])
				{
					printd("Add 2 LLC in WiFi VLAN packet.\n\r");
				}
				else
				{
					printd("Don't add 2 LLC in WiFi VLAN packet.\n\r");
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_TRAFFIC_PROFILE_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				printd("Set traffic profile successful!\n\r");
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				switch (pbuf[4])
				{
					case 0:
						printd("Traffic profile is VOICE.\n\r");
						break;
					case 1:
						printd("Traffic profile is VIDEO.\n\r");
						break;
					case 2:
						printd("Traffic profile is BEST_EFFORT.\n\r");
						break;
					case 3:
						printd("Traffic profile is BACK_GROUND.\n\r");
						break;
					case 4:
						printd("Traffic profile is DEFAULT.\n\r");
						break;
					default:
						printd("Incorrect profile number !!\n\r");
						break;
				}
			}
			break;
		case IO_TX_DEFAULT_POWER_LEVEL_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4] == 0)
				{
					printd("Current power level: 100%%\n\r");
				}
				else if (pbuf[4] == 1)
				{
					printd("Current power level: 50%%\n\r");
				}
				else if (pbuf[4] == 2)
				{
					printd("Current power level: 25%%\n\r");
				}
				else if (pbuf[4] == 3)
				{
					printd("Current power level: 12.5%%\n\r");
				}
				else if (pbuf[4] == 4)
				{
					printd("Current power level: 6.25%%\n\r");
				}
				else if (pbuf[4] == 0xff)
				{
					printd("Power level is the same!!\n\r");
				}
				else
				{
					printd("Not the correct Power level!!\n\r");
					return IO_COMMAND_ERROR;
				}
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4] == 0)
				{
					printd("Current power level: 100%%\n\r");
				}
				else if (pbuf[4] == 1)
				{
					printd("Current power level: 50%%\n\r");
				}
				else if (pbuf[4] == 2)
				{
					printd("Current power level: 25%%\n\r");
				}
				else if (pbuf[4] == 3)
				{
					printd("Current power level: 12.5%%\n\r");
				}
				else if (pbuf[4] == 4)
				{
					printd("Current power level: 6.25%%\n\r");
				}
				else
				{
					printd("Not the correct Power level!!\n\r");
					return IO_COMMAND_ERROR;
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_QUEUE_PRIORITY_SET_RESP:
			break;
		case IO_SELF_CTS_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				printd ("Success to set!!\r\n");
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4] == 0)
				{
					printd ("Self-CTS disabled in OFDM\r\n");
				}
				else if (pbuf[4] == 1)
				{
					printd ("Self-CTS enable in OFDM\r\n");
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_BSSID_RESP:
			if (pbuf[3] == IO_COMMAND_READ)
			{
				if (pbuf[2] != 7)
					return IO_COMMAND_ERROR;
				
				if ((pbuf[4] == 0) && (pbuf[5] == 0) && (pbuf[6] == 0) && (pbuf[7] == 0) && (pbuf[8] == 0) && (pbuf[9] == 0))
				{
					printd ("BSSID: 00 00 00 00 00 00, wireless does not connect.\r\n");
				}
				else
				{
					printd ("BSSID: %02bX %02bX %02bX %02bX %02bX %02bX\r\n", pbuf[4], pbuf[5], pbuf[6], pbuf[7], pbuf[8], pbuf[9]);
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WIFI_IP_READ_RESP:
			if (pbuf[2] != 4)
				return IO_COMMAND_ERROR;
			printd ("Wifi IP: %02bu.%02bu.%02bu.%02bu\r\n", pbuf[3], pbuf[4], pbuf[5], pbuf[6]);
			break;
		case IO_WIFI_DRIVER_VER_READ_RESP:
			if (pbuf[2] == 0)
				return IO_COMMAND_ERROR;
			printd ("WCPU driver version: %s\n\r", &pbuf[3]);

            MWIFAPP_TaskIOWiFiDriverVer(pbuf);
			break;
#if MASS_PRODUCTION
		case IO_CONTINUOUS_TX_ACK:	// 0xC0
			if (pbuf[3] == 0)
				printd("\r Stop ct command !!                \n");
			else if (pbuf[3] == 1)
				printd("\r Launch continuous tx with CCK ... \n");
			else if (pbuf[3] == 2)
				printd("\r Launch continuous tx with OFDM ...\n");
			else if (pbuf[3] == 3)
				printd("\r Launch single tone ...            \n");
			else if (pbuf[3] == 4)
				printd("\r Launch tx suppression ...         \n");
			else if (pbuf[3] == 5)
				printd("\r Launch burst CCK ...              \n");
			break;
		case IO_SENSITIVITY_TEST_ACK:
			if (pbuf[2] == 2)
			{
				printd("\r Total received frames should be %05u\n", *((U16_T *)&pbuf[3]));
				printd("\r Start sensitivity test ...        \n");
			}
			break;
		case IO_SENSITIVITY_TEST_REPORT_ACK:
			if (pbuf[2])
			{
				printd("\r");
				temp = pbuf[2];
				for (i = 0; i < temp; i++)
				{
					printd("%c", pbuf[i + 3]);
				}
				printd("    \n");
			}
			break;
#endif
		case IO_DBG_WCPU_SFR_READ_RESP:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			printd ("\rvalue = %bx              \n", pbuf[3]);
			break;
		case IO_DBG_WCPU_SFR_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_DBG_WCPU_SUB_SYS_REG_READ_RESP:
			if (pbuf[2] < 1)
				return IO_COMMAND_ERROR;
			printd ("\r");
			for (temp = 3; temp < (3 + pbuf[2]); temp++)
			{
				printd (" %02bx", pbuf[temp]);
			}
			printd ("                       \n");
			break;
		case IO_DBG_WCPU_SUB_SYS_REG_WRITE_RESP:	// 0xD3
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_DBG_WCPU_MIIB_REG_READ_RESP:
			if (pbuf[2] < 1)
				return IO_COMMAND_ERROR;
			printd ("\r");
			for (temp = 3; temp < (3 + pbuf[2]); temp++)
			{
				printd (" %02bx", pbuf[temp]);
			}
			printd ("                         \n");
			break;
		case IO_DBG_WCPU_MIIB_REG_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_DBG_WCPU_PCIB_REG_READ_RESP:
			if (pbuf[2] < 1)
				return IO_COMMAND_ERROR;
			printd ("\r");
			for (temp = 3; temp < (3 + pbuf[2]); temp++)
			{
				printd (" %02bx", pbuf[temp]);
			}
			printd ("                          \n");
			break;
		case IO_DBG_WCPU_PCIB_REG_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_DBG_PCI_CONFIGURATION_READ_RESP:	// 0xD8
			if (pbuf[2] != 4)
				return IO_COMMAND_ERROR;
			printd ("\r");
			printd ("%08lx", *((U32_T XDATA*)&pbuf[3]));
			printd ("                          \n");
			break;
		case IO_DBG_PCI_CONFIGURATION_WRITE_ACK:	// 0xD9
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_DBG_WIFI_REG_READ_RESP:
			if (pbuf[2] != 4)
				return IO_COMMAND_ERROR;
			printd ("\r");
			printd ("%08lx", *((U32_T XDATA*)&pbuf[3]));
			printd ("                          \n");
			break;
		case IO_DBG_WIFI_REG_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_DBG_WIFI_PACKET_FILTERING_WRITE_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			printd("\n Set packet filtering successful!!\n");
			break;
		case IO_DBG_WIFI_PACKET_FILTERING_READ_RESP:
			if (pbuf[2] != 9)
				return IO_COMMAND_ERROR;
			if (!(pbuf[3] & 0x07))
			{
				printd ("\n No filtering!!!\n");
				break;
			}
			if (pbuf[3] & BIT0)
			{
				printd ("\n Filtering type:\n");
				printd (" 1.Filter management frame.\n");
				if (pbuf[5] & BIT0)
					printd ("\tFilter Association request.\n");
				if (pbuf[5] & BIT1)
					printd ("\tFilter Association response.\n");
				if (pbuf[5] & BIT2)
					printd ("\tFilter Re-associate request.\n");
				if (pbuf[5] & BIT3)
					printd ("\tFilter Re-associate response.\n");
				if (pbuf[5] & BIT4)
					printd ("\tFilter Probe request.\n");
				if (pbuf[5] & BIT5)
					printd ("\tFilter Probe response.\n");
				if (pbuf[5] & BIT6)
					printd ("\tFilter Reserve 1.\n");
				if (pbuf[5] & BIT7)
					printd ("\tFilter Reserve 2.\n");
				if (pbuf[4] & BIT0)
					printd ("\tFilter Beacon.\n");
				if (pbuf[4] & BIT1)
					printd ("\tFilter ATIM.\n");
				if (pbuf[4] & BIT2)
					printd ("\tFilter De-associate.\n");
				if (pbuf[4] & BIT3)
					printd ("\tFilter Authentication.\n");
				if (pbuf[4] & BIT4)
					printd ("\tFilter De-authentication.\n");
				if (pbuf[4] & BIT5)
					printd ("\tFilter Action.\n");
				if (pbuf[4] & BIT6)
					printd ("\tFilter Reserve 3.\n");
				if (pbuf[4] & BIT7)
					printd ("\tFilter Reserve 4.\n");
			}
			if (pbuf[3] & BIT1)
				printd (" 2.Filter data frame.\n");
			if (pbuf[3] & BIT2)
				printd (" 3.Filter broadcast/multicast.\n");
			printd (" BSSID to filter:\n\t");
			for (temp = 6; temp < 12; temp++)
				printd (" %02bx", pbuf[temp]);
			printd ("\n");
			break;
		case IO_DBG_SYNTHESIZER_SET_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			printd("\r Set synthesizer successful!!\n");
			break;
		case IO_DBG_RETRY_NUMBER_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("\r Read Max Retry Counter: %bu\n\r", pbuf[4]);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_GPIO_ZERO_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("P0: %bX\n\r", pbuf[4]);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_GPIO_ONE_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("P1: %bX\n\r", pbuf[4]);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_GPIO_TWO_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("P2: %bX\n\r", pbuf[4]);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_GPIO_THREE_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				printd ("P3: %bX\n\r", pbuf[4]);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_DBG_AUTO_POWER_CONTROL_RESP:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				printd ("successfully.\n\r");
			}
			else
				return IO_COMMAND_ERROR;
			break;
		default:
			return IO_COMMAND_ERROR;
		}
	}

	return IO_COMMAND_DONE;
}

void io_WlanDebug1Resp(void)
{
	MWIF_Send(IO_WLAN_DEBUG1_RESP, NULL, 0);

} /* End of IO_WlanDebug1 */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_NoticeWcpu
 * Purpose: To management CPU connect, disconnect, and ready commands
 * Params: U8_T  type - The type of the command
 *         U8_T* pbuf - The start pointer of the payload buffer
 *         U8_T  len  - Length of the payload
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_NoticeWcpu(U8_T type, U8_T* pbuf, U8_T len)
{
	if (MCPU_WcpuReady || type == IO_MCPU_WCPU_READY)
	{
		MWIF_Send(type, pbuf, len);
	}

} /* End of IO_NoticeWcpu() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Reset
 * Purpose: To ask WCPU reset itself
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Reset(void)
{
	MWIF_Send(IO_WCPU_RESET, NULL, 0);

}	/* End of IO_Reset */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Scan
 * Purpose: To ask WCPU to do scanning
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Scan(void)
{
	MWIF_Send(IO_SCAN, NULL, 0);

}	/* End of IO_Scan */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SiteSurvey
 * Purpose: To ask WCPU to do site survey
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SiteSurvey(void)
{
	MWIF_Send(IO_SITE_SURVEY, NULL, 0);

}	/* End of IO_SiteSurvey */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_JoinBss
 * Purpose: To ask WCPU to join the BSS
 * Params: U8_T value - The index of the BSS
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_JoinBss(U8_T value)
{
	MWIF_Send(IO_JOIN_BSS, &value, 1);

	MWIFAPP_TaskIOJoinBss(value);

}	/* End of IO_JoinBss */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SsidRW
 * Purpose: To ask WCPU to write SSID string or get the SSID string from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T  len - The length of SSID string
 *         U8_T* pReg - Start pointer of the buffer to save SSID string
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SsidRW(U8_T type, U8_T len, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_SSID_WRITE, pReg, len);
	}
	else
	{
		MWIF_Send(IO_SSID_READ, NULL, 0);
	}
}	/* End of IO_SsidRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_OperateModeRW
 * Purpose: To ask WCPU to set operation mode or get operation mode from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - the mode to operate
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_OperateModeRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_OPERATE_MODE, io_TxBuf, len);
	
}	/* End of IO_OperateModeRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_ChannelRW
 * Purpose: To ask WCPU to set channel number or get channel number from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - The channel number
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_ChannelRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_CHANNEL, io_TxBuf, len);
	
}	/* End of IO_ChannelRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_NetworkModeRW
 * Purpose: To ask WCPU to set network mode or get network mode from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - the mode of network
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_NetworkModeRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_NETWORK_MODE, io_TxBuf, len);
	
}	/* End of IO_NetworkModeRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SetMulticastMac
 * Purpose: To ask WCPU to set multicast MAC support or get Tx data rate from WCPU
 * Params: U8_T  rw - 0: read command 1: write command
 *         U8_T* pMulticastMac - Start pointer of the buffer to save the multicast
 *                               MAC setting
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SetMulticastMac(U8_T XDATA* pMulticastMac)
{
	MWIF_Send(IO_MULTICAST_SET, pMulticastMac, 6);
	
}	/* End of IO_SetMulticastMac */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_WiFiMacRW
 * Purpose: To ask WCPU to set WiFi MAC address or get WiFi MAC address from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T* pReg - Start pointer of the buffer to WiFi MAC address
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_WiFiMacRW(U8_T type, U8_T XDATA* pReg)
{
	U8_T	len;
		
	if (type == IO_COMMAND_WRITE)
	{
		len = 7;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		memcpy(&io_TxBuf[1], pReg, 6);
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_WIFI_MAC_ADDR_SET, io_TxBuf, len);
	
}	/* End of IO_WiFiMacRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_RadioOnOff
 * Purpose: To ask WCPU to enable or disable the RF or read the state
 * Params: U8_T state - 0: disable RF, 1: enable RF
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T IO_RadioOnOff(U8_T state)
{	
	MWIF_Send(IO_WIFI_RADIO_ON_OFF, &state, 1);

	return 0;

}	/* End of IO_RadioOnOff */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_BgProtection
 * Purpose: To ask WCPU to set the mode of bg protection
 * Params: U8_T state - xxxxx
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_BgProtectionRW(U8_T type, U8_T state)
{
	U8_T	len;
	
	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = state;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_WIFI_B_G_PROTECTION, io_TxBuf, len);

}	/* End of IO_BgProtection */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_EncryptionRW
 * Purpose: To ask WCPU to set encryption status or get encryption status from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - 0: disable encryption 1: enable encryption
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_EncryptionRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_ENCRYPTION_TYPE, io_TxBuf, len);
	
}	/* End of IO_EncryptionRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_OpenSysRW
 * Purpose: To ask WCPU to enable/disable open system or get the status of
 *          open system from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - enable/disable open system
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_OpenSysRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_OPEN_SYSTEM, io_TxBuf, len);
	
}	/* End of IO_OpenSysRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Wep64KeyRW
 * Purpose: To ask WCPU to write WEP64 key or get the WEP64 key from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T  index - The index of WEP64 key
 *         U8_T* pReg - Start pointer of the buffer to save WEP64 key
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Wep64KeyRW(U8_T type, U8_T index, U8_T XDATA* pReg)
{
	io_TxBuf[0] = index;
	
	if (type == IO_COMMAND_WRITE)
	{
		memcpy(&io_TxBuf[1], pReg, 5);

		MWIF_Send(IO_WEP64_KEY_WRITE, io_TxBuf, 6);
	}
	else
	{
		MWIF_Send(IO_WEP64_KEY_READ, io_TxBuf, 1);
	}
}	/* End of IO_Wep64KeyRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Wep128KeyRW
 * Purpose: To ask WCPU to write WEP128 key or get the WEP128 key from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T  index - The index of WEP128 key
 *         U8_T* pReg - Start pointer of the buffer to save WEP128 key
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Wep128KeyRW(U8_T type, U8_T index, U8_T XDATA* pReg)
{
	io_TxBuf[0] = index;

	if (type == IO_COMMAND_WRITE)
	{
		memcpy(&io_TxBuf[1], pReg, 13);

		MWIF_Send(IO_WEP128_KEY_WRITE, io_TxBuf, 14);
	}
	else
	{
		MWIF_Send(IO_WEP128_KEY_READ, io_TxBuf, 1);
	}
}	/* End of IO_Wep128KeyRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_WepKeyIndexRW
 * Purpose: To ask WCPU to set WEP key index or get WEP key index from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - WEP key index
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_WepKeyIndexRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_WEP_KEY_INDEX, io_TxBuf, len);
	
}	/* End of IO_WepKeyIndexRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_TkipAesRW
 * Purpose: To ask WCPU to write TKIP/AES string or get the TKIP/AES string from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T  len - The length of TKIP/AES string
 *         U8_T* pReg - Start pointer of the buffer to save TKIP/AES string
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_TkipAesRW(U8_T type, U8_T len, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_TKIP_AES_PASSPHASE_STRING_WRITE, pReg, len);
	}
	else
	{
		MWIF_Send(IO_TKIP_AES_PASSPHASE_STRING_READ, NULL, 0);
	}
}	/* End of IO_TkipAesRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_FragmentThresholdRW
 * Purpose: To ask WCPU to set fragment threshold or reply a acknowledgement to WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T* pValue - Start pointer of the buffer to save fragment threshold value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_FragmentThresholdRW(U8_T type, U8_T XDATA* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = *pValue;
		io_TxBuf[1] = *(pValue + 1);

		MWIF_Send(IO_FRAGMENT_THRESHOLD_WRITE, io_TxBuf, 2);
	}
	else
	{
		MWIF_Send(IO_FRAGMENT_THRESHOLD_READ, NULL, 0);
	}

}	/* End of IO_FragmentThresholdRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_RtsThresholdRW
 * Purpose: To ask WCPU to set RTS threshold or reply a acknowledgement to WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T* pValue - Start pointer of the buffer to save RTS threshold value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_RtsThresholdRW(U8_T type, U8_T XDATA* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = *pValue;
		io_TxBuf[1] = *(pValue + 1);

		MWIF_Send(IO_RTS_THRESHOLD_WRITE, io_TxBuf, 2);
	}
	else
	{
		MWIF_Send(IO_RTS_THRESHOLD_READ, NULL, 0);
	}

}	/* End of IO_RtsThresholdRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_PreambleRW
 * Purpose: To ask WCPU to set long/short preamble or get the preamble status
 *          from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - long/short preamble
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_PreambleRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_PREAMBLE, io_TxBuf, len);
	
}	/* End of IO_PreambleRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_TxGainRW
 * Purpose: To ask WCPU to set Tx power gain or get Tx power gain from WCPU
 * Params: U8_T rw - 0: read command 1: write command
 *         U8_T txGain - the Tx power gain value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_TxGainRW(U8_T rw, U8_T txGain)
{
	if (rw == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = txGain;

		MWIF_Send(IO_TX_POWER_LEVEL_WRITE, io_TxBuf, 1);
	}
	else
	{
		MWIF_Send(IO_TX_POWER_LEVEL_READ, NULL, 0);
	}

}	/* End of IO_TxGainRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_BeaconIntervalRW
 * Purpose: To ask WCPU to set beacon interval or reply a acknowedgement to WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T* pValue - Start pointer of the buffer to save beacon interval value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_BeaconIntervalRW(U8_T type, U8_T* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = *pValue;
		io_TxBuf[1] = *(pValue + 1);

		MWIF_Send(IO_BEACON_INTERVAL_WRITE, io_TxBuf, 2);
	}
	else
	{
		MWIF_Send(IO_BEACON_INTERVAL_READ, NULL, 0);
	}

}	/* End of IO_BeaconIntervalRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_CountryRegionRW
 * Purpose: To ask WCPU to set country region or get country region from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - The country region value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_CountryRegionRW(U8_T type, U16_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 3;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value >> 8;
		io_TxBuf[2] = (U8_T)value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_COUNTRY_REGION, io_TxBuf, len);

}	/* End of IO_CountryRegionRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_AtimIntervalRW
 * Purpose: To ask WCPU to set ATIM interval or reply a acknowedgement to WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T* pValue - Start pointer of the buffer to save ATIM interval value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_AtimIntervalRW(U8_T type, U8_T XDATA* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = *pValue;
		io_TxBuf[1] = *(pValue + 1);

		MWIF_Send(IO_ATIM_INTERVAL_WRITE, io_TxBuf, 2);
	}
	else
	{
		MWIF_Send(IO_ATIM_INTERVAL_READ, NULL, 0);
	}

}	/* End of IO_AtimIntervalRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_TxDataRateRW
 * Purpose: To ask WCPU to set Tx data rate or get Tx data rate from WCPU
 * Params: U8_T rw - 0: read command 1: write command
 *         U8_T txRate - the Tx data rate
 * Returns: U8_T result - 0x00: success 
 *                        0xFF: fail
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T IO_TxDataRateRW(U8_T rw, U8_T txRate)
{
	U8_T XDATA result = 0xff;
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		switch (txRate)
		{
		case 0:
		case 0x02:
		case 0x04:
		case 0x0B:
		case 0x0C:
		case 0x12:
		case 0x16:
		case 0x18:
		case 0x24:
		case 0x30:
		case 0x48:
		case 0x60:
		case 0x6C:
			io_TxBuf[1] = txRate;
			break;
		default:
			return result;
		}
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	result = 0;
	
	MWIF_Send(IO_TX_DATA_RATE,io_TxBuf, len);
	
	return result;
	
}	/* End of IO_TxDataRateRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_AutoPowerCtrlRW
 * Purpose: To ask WCPU to enable/disable "WiFi Tx auto power control" function
 *          or get the status from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T OnOff - 0: Off 1: ON
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_AutoPowerCtrlRW(U8_T type, U8_T OnOff)
{
	U8_T	len;
	
	if (type == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = OnOff;
		switch (OnOff)
		{
		case 0:
		case 1:
			len = 2;
			break;
		}
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_AUTO_POWER_CONTROL, io_TxBuf, len);
	
}	/* End of IO_AutoPowerCtrlRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_AutoPowerCtrlDetailW
 * Purpose: To ask WCPU to enable/disable "WiFi Tx auto power control" function
 *          or get the status from WCPU
 * Params: U8_T value - the set point value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_AutoPowerCtrlDetailW(U8_T value)
{
	U8_T	len;
	
	io_TxBuf[0] = IO_COMMAND_WRITE;
	len = 3;
	io_TxBuf[1] = 2;
	io_TxBuf[2] = value;
	
	MWIF_Send(IO_DBG_AUTO_POWER_CONTROL, io_TxBuf, len);
	
}	/* End of IO_AutoPowerCtrlDetailW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Roaming
 * Purpose: To ask WCPU to enable/disable roaming function or get the status from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - 0: Off 1: ON
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Roaming(U8_T type, U8_T value)
{
	U8_T	len;
	
	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_WIFI_ROAMING, io_TxBuf, len);
	
}	/* End of IO_Roaming */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_MultimediaSet
 * Purpose: To ask WCPU to enable/disable WiFi Multimedia (WMM) function or get
 *          the status from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - 0: Off 1: ON
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_WifiMultimediaSet(U8_T type, U8_T value)
{
	U8_T	len;
	
	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_WIFI_MULTIMEDIA_SET ,io_TxBuf, len);
	
}	/* End of IO_MultimediaSet */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_ListenIntervalRW
 * Purpose: To ask WCPU to write listen interval value or get
 *          the it from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U16_T value - listen interval value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_ListenIntervalRW(U8_T type, U16_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 3;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		*((U16_T*)&io_TxBuf[1]) = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_LISTEN_INTERVAL, io_TxBuf, len);
	
}	/* End of IO_ListenIntervalRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_MinContentWindowRW
 * Purpose: To ask WCPU to write minimum contention window value or get
 *          the it from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U16_T value - minimum contention window value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_MinContentWindowRW(U8_T type, U16_T value)
{
	U8_T	len;
	
	if (type == IO_COMMAND_WRITE)
	{
		len = 3;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		*((U16_T*)&io_TxBuf[1]) = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_MINIMUM_CONTENTION_WINDOW, io_TxBuf, len);
	
}	/* End of IO_MinContentWindowRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_MaxContentWindowRW
 * Purpose: To ask WCPU to write maximum contention window value or get
 *          the it from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U16_T value - minimum contention window value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_MaxContentWindowRW(U8_T type, U16_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 3;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		*((U16_T*)&io_TxBuf[1]) = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_MAXIMUM_CONTENTION_WINDOW, io_TxBuf, len);
	
}	/* End of IO_MaxContentWindowRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_ShowRssi
 * Purpose: To ask WCPU the RSSI value
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_ShowRssi(void)
{
	MWIF_Send(IO_SHOW_RSSI, NULL, 0);

}	/* End of IO_ShowRssi */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_PayloadLenAddInWifi
 * Purpose: To ask WCPU to enable/disable "Payload Length Attach in WiFi Packet"
 *          function or get the status from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - 0: Off 1: ON
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_PayloadLenAddInWifi(U8_T type, U8_T value)
{
	U8_T	len;
	
	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_PAYLOAD_LENGTH_ATTACH_IN_WIFI_PACKET, io_TxBuf, len);
		
}	/* End of IO_PayloadLenAddInWifi */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_RetainLlcInWifi
 * Purpose: To ask WCPU to enable/disable "Retain LLC in WiFi Packet" function
 *          or get the status from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - 0: Off 1: ON
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_RetainLlcInWifi(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_RETAIN_LLC_IN_WIFI_PACKET, io_TxBuf, len);
	
}	/* End of IO_RetainLlcInWifi */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_2LlcInWifiVlan
 * Purpose: To ask WCPU to enable/disable "2LLC in WiFi VLAN Packet" function
 *          or get the status from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - 0: Off 1: ON
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_2LlcInWifiVlan(U8_T type, U8_T value)
{
	U8_T	len;
	
	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_2_LLC_IN_WIFI_VLAN_PACKET, io_TxBuf, len);
	
}	/* End of IO_2LlcInWifiVlan */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_TrafficProfile
 * Purpose: To ask WCPU to write traffic profile
 *          or get the status from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - 0: voice 1: video 2: best effort 3: back ground
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_TrafficProfile(U8_T type, U8_T value)
{
	U8_T	len;
	
	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_TRAFFIC_PROFILE, io_TxBuf, len);
	
}	/* End of IO_TrafficProfile */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_TxDefaultPowerLevelRW
 * Purpose: To ask WCPU to set Tx default power level or get Tx default power level from WCPU
 * Params: U8_T rw - 0: read command 1: write command
 *         U8_T level - 0: 100%, 1: 50%, 2: 25%, 3: 12.5%, 4: 6.25%
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_TxDefaultPowerLevelRW(U8_T rw, U8_T level)
{
	if (rw == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = level;

		MWIF_Send(IO_TX_DEFAULT_POWER_LEVEL, io_TxBuf, 2);
	}
	else
	{
		io_TxBuf[0] = IO_COMMAND_READ;

		MWIF_Send(IO_TX_DEFAULT_POWER_LEVEL, io_TxBuf, 1);
	}

}	/* End of IO_TxDefaultPowerLevelRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SelfCtsRW
 * Purpose: To ask WCPU to set the self-CTS from WCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  value - 0: OFF, 1: ON
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SelfCtsRW(U8_T rw, U8_T value)
{
	U8_T	len;
	
	if (rw == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}
	
	MWIF_Send(IO_SELF_CTS, io_TxBuf, len);
	
}	/* End of IO_SelfCtsRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_BssidR
 * Purpose: To query BSSID from WCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_BssidR(void)
{
	io_TxBuf[0] = IO_COMMAND_READ;
	
	MWIF_Send(IO_BSSID, io_TxBuf, 1);
	
}	/* End of IO_SelfCtsRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_WifiIpR
 * Purpose: To query Wifi IP from WCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_WifiIpR(void)
{
	MWIF_Send(IO_WIFI_IP_READ, io_TxBuf, 0);
	
}	/* End of IO_WifiIpR */

#if MASS_PRODUCTION
/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_ContinuousTx
 * Purpose: To ask WCPU to enable/disable WiFi continuous Tx function or get
 *          the status from WCPU
 * Params: U8_T mode - 0: Off, 1: CCK, 2: OFDM, 3: CW wave, 4: , 5:
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T IO_ContinuousTx(U8_T mode)
{
	if (mode > 5)
		return IO_COMMAND_ERROR;

	io_TxBuf[0] = mode;

	MWIF_Send(IO_CONTINUOUS_TX, io_TxBuf, 1);

	return IO_COMMAND_DONE;
	
}	/* End of IO_ContinuousTx */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SensitivityTest
 * Purpose: To ask WCPU to enable/disable WiFi sensitivity test function or get
 *          the status from WCPU
 * Params: U16_T counts - Amount of the broadcast packet to send
 *         U8_T source - The Golden Unit to send broadcast packet
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SensitivityTest(U16_T counts, U8_T source)
{
	io_TxBuf[0] = (U8_T)(counts >> 8);
	io_TxBuf[1] = (U8_T)counts;
	io_TxBuf[2] = source;

	MWIF_Send(IO_SENSITIVITY_TEST, io_TxBuf, 3);
	
}	/* End of IO_SensitivityTest */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SensitivityTestReport
 * Purpose: To ask WCPU to report the result of WiFi sensitivity test
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SensitivityTestReport(void)
{
	MWIF_Send(IO_SENSITIVITY_TEST_REPORT, NULL, 0);

}	/* End of IO_SensitivityTestReport */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SfrRW
 * Purpose: To ask WCPU to write SFR or get the SFR value from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T index - The index of SFR
 *         U8_T value - The SFR value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SfrRW(U8_T type, U8_T index, U8_T XDATA value)
{
	if (type == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = index;
		io_TxBuf[1] = value;

		MWIF_Send(IO_DBG_WCPU_SFR_WRITE, io_TxBuf, 2);
	}
	else
	{
		io_TxBuf[0] = index;

		MWIF_Send(IO_DBG_WCPU_SFR_READ, io_TxBuf, 1);
	}
}	/* End of IO_SfrRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_WcpuIndirectRW
 * Purpose: To ask WCPU to write WCPU indirect register or get WCPU indirect
 *          register value from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T  index - index of WCPU indirect register
 *         U8_T* pReg - Start pointer of the buffer to save WCPU indirect register value
 * Returns: U8_T - 0x00: IO_COMMAND_DONE
 *                 0xFF: IO_COMMAND_ERROR
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T IO_WcpuIndirectRW(U8_T type, U8_T index, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		U8_T	len;

		io_TxBuf[0] = index;
		switch (index)
		{
		case WCPU_MAIN_CTL_REG:				// 0x00
		case WCPU_WIFI_MAIN_CTL_REG:		// 0x01
		case WCPU_DMA_BYTE_COUNT_LOW_REG:	// 0x06
		case WCPU_DMA_BYTE_COUNT_CMD_REG:	// 0x07
		case WCPU_DMA_CONTROL_REG:			// 0x08
		case WCPU_DMA_INTR_MASK:			// 0x0B
		case WCPU_SWTIMER_TIMER_CTRL:		// 0x0C
		case WCPU_SYSTEM_SETTING_REG:		// 0x0D
			len = 2;
			io_TxBuf[1] = *pReg;
			break;
		case WCPU_DMA_SOUR_ADDR_REG:		// 0x02
		case WCPU_DMA_TARGET_ADDR_REG:		// 0x04
		case WCPU_SWTIMER_TIMER_REG:		// 0x09
			len = 3;
			io_TxBuf[1] = *pReg;
			io_TxBuf[2] = *(pReg + 1);
			break;
		default:
			return IO_COMMAND_ERROR;
		}

		MWIF_Send(IO_DBG_WCPU_SUB_SYS_REG_WRITE, io_TxBuf, len);
	}
	else
	{
		io_TxBuf[0] = index;

		MWIF_Send(IO_DBG_WCPU_SUB_SYS_REG_READ, io_TxBuf, 1);
	}

	return IO_COMMAND_DONE;

} /* End of IO_WcpuIndirectRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_MiibIndirectRW
 * Purpose: To ask WCPU to write MII indirect register or get MII indirect
 *          register value from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T  index - index of MII indirect register
 *         U8_T* pReg - Start pointer of the buffer to save MII indirect register value
 * Returns: U8_T - 0x00: IO_COMMAND_DONE
 *                 0xFF: IO_COMMAND_ERROR
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T IO_MiibIndirectRW(U8_T type, U8_T index, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		U8_T	len;

		io_TxBuf[0] = index;
		switch (index)
		{
		case MIB_TXMIC_PRIORITY:	// 0x08
		case MIB_TX_CONTROL:		// 0x0F
		case MIB_RXMIC_PRIORITY:	// 0x18
		case MIB_RXMIC_FAIL_CNT:	// 0x19
		case MIB_BOUNDARY_SIZE:		// 0x1A
			len = 2;
			io_TxBuf[1] = *pReg;
			break;
		case MIB_TX_THRESHOLD:		// 0x09
		case MIB_TX_START_ADDR:		// 0x0B
		case MIB_TX_END_ADDR:		// 0x0D
			len = 3;
			io_TxBuf[1] = *pReg;
			io_TxBuf[2] = *(pReg + 1);
			break;
		case MIB_LLC_PATTERN:		// 0x1B
			len = 7;
			memcpy(&io_TxBuf[1], pReg, 6);
			break;
		case MIB_TXMIC_KEY:			// 0x00
		case MIB_RXMIC_KEY:			// 0x10
			len = 9;
			memcpy(&io_TxBuf[1], pReg, 8);
			break;
		default:
			return IO_COMMAND_ERROR;
		}

		MWIF_Send(IO_DBG_WCPU_MIIB_REG_WRITE, io_TxBuf, len);
	}
	else
	{
		io_TxBuf[0] = index;

		MWIF_Send(IO_DBG_WCPU_MIIB_REG_READ, io_TxBuf, 1);
	}

	return IO_COMMAND_DONE;
	
}	/* End of IO_MiibIndirectRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_PcibIndirectRegRW
 * Purpose: To ask WCPU to write PCI indirect register or get PCI indirect
 *          register value from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T  index - index of PCI indirect register
 *         U8_T* pReg - Start pointer of the buffer to save PCI indirect register value
 * Returns: U8_T - 0x00: IO_COMMAND_DONE
 *                 0xFF: IO_COMMAND_ERROR
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T IO_PcibIndirectRegRW(U8_T type, U8_T index, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		U8_T	len;

		io_TxBuf[0] = index;
		switch (index)
		{
		case PCIB_RFD_BNDRY_SIZE:	// 0x04
		case PCIB_FILTER_CONTROL:	// 0x05
			len = 2;
			io_TxBuf[1] = pReg[0];
			break;
		case PCIB_RFD_BASE_ADDR:	// 0x00
		case PCIB_RFD_END_ADDR:		// 0x02
		case PCIB_FILTER_MANG_FRAME:// 0x06
			len = 3;
			io_TxBuf[1] = pReg[0];
			io_TxBuf[2] = pReg[1];
			break;
		case PCIB_FILTER_BSSID:		// 0x08
			len = 7;
			memcpy(&io_TxBuf[1], pReg, 6);
			break;
		default:
			return IO_COMMAND_ERROR;
		}

		MWIF_Send(IO_DBG_WCPU_PCIB_REG_WRITE, io_TxBuf, len);
	}
	else
	{
		io_TxBuf[0] = index;

		MWIF_Send(IO_DBG_WCPU_PCIB_REG_READ, io_TxBuf, 1);
	}

	return IO_COMMAND_DONE;
	
}	/* End of IO_PcibIndirectRegRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_PciConfigRW
 * Purpose: To ask WCPU to write PCI configuration value or get the PCI
 *          configuration value from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T  index - The index of PCI configuration register
 *         U8_T* pReg - Start pointer of the buffer to save PCI configuration value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_PciConfigRW(U8_T type, U8_T index, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = index;
		io_TxBuf[1] = pReg[0];
		io_TxBuf[2] = pReg[1];
		io_TxBuf[3] = pReg[2];
		io_TxBuf[4] = pReg[3];

		MWIF_Send(IO_DBG_PCI_CONFIGURATION_WRITE, io_TxBuf, 5);
	}
	else
	{
		io_TxBuf[0] = index;

		MWIF_Send(IO_DBG_PCI_CONFIGURATION_READ, io_TxBuf, 1);
	}
}	/* End of IO_PciConfigRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_WiFiRegRW
 * Purpose: To ask WCPU to write WiFi register or get WiFi register value from WCPU
 * Params: U8_T  type - 0: read command 1: write command
 *         U8_T* pValue - Start pointer of the index of WiFi register
 *         U8_T* pReg - Start pointer of the buffer to WiFi register
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_WiFiRegRW(U8_T type, U8_T XDATA* pValue, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = pValue[0];
		io_TxBuf[1] = pValue[1];
		io_TxBuf[2] = pReg[0];
		io_TxBuf[3] = pReg[1];
		io_TxBuf[4] = pReg[2];
		io_TxBuf[5] = pReg[3];

		MWIF_Send(IO_DBG_WIFI_REG_WRITE, io_TxBuf, 6);
	}
	else
	{
		io_TxBuf[0] = pValue[0];
		io_TxBuf[1] = pValue[1];

		MWIF_Send(IO_DBG_WIFI_REG_READ, io_TxBuf, 2);
	}
}	/* End of IO_WiFiRegRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_WlanPktFilterRW
 * Purpose: To ask WCPU to set WiFi packet filtering or get WiFi packet filtering
 *          status from WCPU
 * Params: U8_T  rw - 0: read command 1: write command
 *         U8_T  type - The type of WiFi packet (i.e. Data frame, Managment frame, or Control frame)
 *         U8_T  subtype - The subtype of WiFi packet (i.e. PS-Pull, association, etc)
 *         U8_T* pBssid - Start pointer of the buffer to save BSSID
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_WlanPktFilterRW(U8_T rw, U8_T type, U16_T subtype, U8_T XDATA* pBssid)
{
	if (rw == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = type;
		io_TxBuf[1] = (U8_T)(subtype >> 8);
		io_TxBuf[2] = (U8_T)subtype;
		io_TxBuf[3] = pBssid[0];
		io_TxBuf[4] = pBssid[1];
		io_TxBuf[5] = pBssid[2];
		io_TxBuf[6] = pBssid[3];
		io_TxBuf[7] = pBssid[4];
		io_TxBuf[8] = pBssid[5];

		MWIF_Send(IO_DBG_WIFI_PACKET_FILTERING_WRITE, io_TxBuf, 9);
	}
	else
	{
		MWIF_Send(IO_DBG_WIFI_PACKET_FILTERING_READ, NULL, 0);
	}
}	/* End of IO_WlanPktFilterRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SynthesizerSet
 * Purpose: To ask WCPU to set the synthesizer
 * Params: U8_T* pValue - Start pointer of the buffer to save synthesizer value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SynthesizerSet(U8_T XDATA* pValue)
{
	io_TxBuf[0] = *pValue;
	io_TxBuf[1] = *(pValue + 1);
	io_TxBuf[2] = *(pValue + 2);
	io_TxBuf[3] = *(pValue + 3);

	MWIF_Send(IO_DBG_SYNTHESIZER_SET, io_TxBuf, 4);

}	/* End of IO_SynthesizerSet */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_RetryRW
 * Purpose: To ask WCPU to set WiFi packet retry times or get the WiFi packet 
 *          retry times from WCPU
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - WiFi packet retry times
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_RetryRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_DBG_RETRY_NUMBER, io_TxBuf, len);
	
}	/* End of IO_RetryRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Gpio0RW
 * Purpose: To ask WCPU to set GPIO 0 or get the value of GPIO 0
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - value of GPIO 0
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Gpio0RW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_GPIO_ZERO, io_TxBuf, len);
	
}	/* End of IO_Gpio0RW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Gpio1RW
 * Purpose: To ask WCPU to set GPIO 1 or get the value of GPIO 1
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - value of GPIO 1
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Gpio1RW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_GPIO_ONE , io_TxBuf, len);
	
}	/* End of IO_Gpio1RW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Gpio2RW
 * Purpose: To ask WCPU to set GPIO 2 or get the value of GPIO 2
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - value of GPIO 2
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Gpio2RW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_GPIO_TWO, io_TxBuf, len);
	
}	/* End of IO_Gpio2RW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_Gpio3RW
 * Purpose: To ask WCPU to set GPIO 3 or get the value of GPIO 3
 * Params: U8_T type - 0: read command 1: write command
 *         U8_T value - value of GPIO 3
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_Gpio3RW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = value;
	}
	else
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_READ;
	}

	MWIF_Send(IO_GPIO_THREE, io_TxBuf, len);
	
}	/* End of IO_Gpio3RW */

void IO_ZPHYTestW(void)
{
	MWIF_Send(0x6d, NULL, 0);
}

void IO_WMIBTestW(U8_T XDATA enable)
{
	MWIF_Send(0x6e, &enable, 1);
}

void IO_APNFTW(void)
{
	MWIF_Send(0x6f, NULL, 0);
}

void IO_ENCTW(void)
{
	io_TxBuf[0] = 0x01;

	MWIF_Send(0x6f, io_TxBuf, 1);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_WifiDriverVer
 * Purpose: To read the WCPU firmware version
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_WifiDriverVer(void)
{
	MWIF_Send(IO_WIFI_DRIVER_VER_READ, NULL, 0);
	
}	/* End of IO_WifiDriverVer */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_ScanReportAck
 * Purpose: To reply a scan report acknowedgement to WCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_ScanReportAck(void)
{
	MWIF_Send(IO_SCAN_REPORT_ACK, NULL, 0);
	
}	/* End of io_ScanReportAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SiteSurveyReportAck
 * Purpose: To reply a site survey acknowedgement to WCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SiteSurveyReportAck(void)
{
	MWIF_Send(IO_SITE_SURVEY_REPORT_ACK, NULL, 0);

}	/* End of IO_SiteSurveyReportAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SensitivityTestReportACK
 * Purpose: To reply a sensitivity test report acknowedgement to WCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SensitivityTestReportACK(void)
{
	MWIF_Send(IO_SENSITIVITY_TEST_REPORT_ACK, NULL, 0);

}	/* End of io_SensitivityTestReportACK */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SetTimerCharCmd
 * Purpose: 
 * Params:
 * Returns:
 * Note: Should be removed after debugging.
 * ----------------------------------------------------------------------------
 */
void IO_SetTimerCharCmd(U8_T num, U8_T mode, U16_T range, U8_T divider)
{
	io_TxBuf[0] = num;
	io_TxBuf[1] = mode;
	io_TxBuf[2] = (U8_T)(range >> 8);
	io_TxBuf[3] = (U8_T)range;
	io_TxBuf[4] = divider;

	MWIF_Send(0x67, io_TxBuf, 5);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SetTimer2CharCmd
 * Purpose: 
 * Params:
 * Returns:
 * Note: Should be removed after debugging.
 * ----------------------------------------------------------------------------
 */
void IO_SetTimer2CharCmd(U8_T mode, U16_T range, U8_T divider)
{
	io_TxBuf[0] = mode;
	io_TxBuf[1] = (U8_T)(range >> 8);
	io_TxBuf[2] = (U8_T)range;
	io_TxBuf[3] = divider;

	MWIF_Send(0x68, io_TxBuf, 4);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SetTimerRunCmd
 * Purpose: 
 * Params:
 * Returns:
 * Note: Should be removed after debugging.
 * ----------------------------------------------------------------------------
 */
void IO_SetTimerRunCmd(U8_T timer, U8_T sw)
{
	io_TxBuf[0] = timer;
	io_TxBuf[1] = sw;

	MWIF_Send(0x69, io_TxBuf, 2);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SetTimerInterruptCmd
 * Purpose: 
 * Params:
 * Returns:
 * Note: Should be removed after debugging.
 * ----------------------------------------------------------------------------
 */
void IO_SetTimerInterruptCmd(U8_T timer, U8_T run)
{
	io_TxBuf[0] = timer;
	io_TxBuf[1] = run;

	MWIF_Send(0x6a, io_TxBuf, 2);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SetUartCharCmd
 * Purpose: 
 * Params:
 * Returns:
 * Note: Should be removed after debugging.
 * ----------------------------------------------------------------------------
 */
void IO_SetUartCharCmd(U8_T uart, U8_T mode, U8_T timer, U8_T baud, U8_T db, U8_T clkdiv)
{
	io_TxBuf[0] = uart;
	io_TxBuf[1] = mode;
	io_TxBuf[2] = timer;
	io_TxBuf[3] = baud;
	io_TxBuf[4] = db;
	io_TxBuf[5] = clkdiv;

	MWIF_Send(0x6b, io_TxBuf, 6);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SetUartInterruptCmd
 * Purpose: 
 * Params:
 * Returns:
 * Note: Should be removed after debugging.
 * ----------------------------------------------------------------------------
 */
void IO_SetUartInterruptCmd(U8_T uart, U8_T sw)
{
	io_TxBuf[0] = uart;
	io_TxBuf[1] = sw;

	MWIF_Send(0x6c, io_TxBuf, 2);
}

  #endif /* (MAC_ARBIT_MODE & MAC_ARBIT_WIFI) */
/*****************************************************************/
#else	/***** Middle of IO_CPU_TYPE, Below are for WCPU use *****/


/*
 * ----------------------------------------------------------------------------
 * Function Name: 
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T IO_CmdParsing(U8_T* pbuf)
{
	U8_T XDATA len, value8;
	U8_T XDATA reg[64];
	U16_T value16;
	U32_T value32;
	
#if IO_SHOWCMD
	io_ShowCmd(0, pbuf);
#endif

	if ((pbuf[0] | pbuf[1]) != 0xFF)
		return IO_COMMAND_ERROR;
	{
		switch (pbuf[0])
		{
		case IO_MCPU_WCPU_READY:
			WCPU_McpuReady = 1;
			IO_NoticeMcpu(IO_MCPU_WCPU_READY_ACK, NULL, 0);
			printf ("Rcv MCPU Ready Command.\n\r");
			break;
		case IO_MCPU_WCPU_READY_ACK:
			WCPU_McpuReady = 1;
			printf ("Rcv WCPU Ready ACK Command.\n\r");
			break;
		case IO_WCPU_RESET:
			io_ResetAck();
			printf ("Rcv MCPU Reset Command.\n\r");
			break;
		case IO_MCPU_WCPU_CONNECT_ACK:
			printf ("Rcv WiFi Connect Command Ack.\n\r");
			break;
		case IO_MCPU_WCPU_DISCONNECT_RESP:
			printf ("Rcv WiFi Disconnect Command Response.\n\r");
			break;
		case IO_SCAN:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			IO_ScanFromMcpu = 1;
			MGR_IOScan();
			io_ScanAck();
			break;
		case IO_SCAN_REPORT_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_SITE_SURVEY:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			io_SiteSurveyAck();
			IO_SiteSurveyFromMcpu = 1;
			MGR_IOSiteSurvey();
			break;
		case IO_SITE_SURVEY_REPORT_ACK:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			break;
		case IO_JOIN_BSS:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			printf ("Join BSS index: %bd\n\r", pbuf[3]);
			io_JoinBssAck();
			MGR_IOJoinBss(pbuf[3]);
			break;
		case IO_SSID_READ:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			MGR_IOSsid(IO_COMMAND_READ, reg, &len);
			io_SsidAckRW(IO_COMMAND_READ, len, reg);
			break;
		case IO_SSID_WRITE:
			if (pbuf[2] > 32)
				return IO_COMMAND_ERROR;
			io_SsidAckRW(IO_COMMAND_WRITE, len, reg);
			MGR_IOSsid(IO_COMMAND_WRITE, &pbuf[3], &pbuf[2]);
			break;
		case IO_OPERATE_MODE:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				io_OperateModeAckRW(IO_COMMAND_WRITE, value8);
				MGR_IOBBMode(IO_COMMAND_WRITE, &pbuf[4]);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IOBBMode(IO_COMMAND_READ, &value8);
				io_OperateModeAckRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_CHANNEL:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MGR_IOChannel(IO_COMMAND_WRITE, &pbuf[4]);
				io_ChannelAckRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IOChannel(IO_COMMAND_READ, &value8);
				io_ChannelAckRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_NETWORK_MODE:
			if (pbuf[3] == IO_COMMAND_WRITE)	// write
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MGR_IONetWorkType(IO_COMMAND_WRITE, &pbuf[4]);
				io_NetworkModeAckRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IONetWorkType(IO_COMMAND_READ, &value8);
				io_NetworkModeAckRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_MULTICAST_SET:
			if (pbuf[2] != 6)
				return IO_COMMAND_ERROR;
			WIFI_SetMultiFilter(1, &pbuf[3]);
			io_SetMulticastMacAck();
			break;
		case IO_WIFI_MAC_ADDR_SET:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 7)
					return IO_COMMAND_ERROR;
				MGR_IOMacAddr(IO_COMMAND_WRITE, &pbuf[4]);
				io_WiFiMacAckRW(IO_COMMAND_WRITE, reg);
			}
			else if (!pbuf[3])
			{
				MGR_IOMacAddr(IO_COMMAND_READ, reg);
				io_WiFiMacAckRW(IO_COMMAND_READ, reg);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WIFI_RADIO_ON_OFF:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			if (pbuf[3] == 0)
			{
				io_RadioOnOffAck(HW_RadioOnOff(1, 0));
#if POWER_SAVING
				PS_BusyTimeout = 0xFF;
#endif
				IO_NoticeMcpu(IO_MCPU_WCPU_DISCONNECT, NULL, 0);
				MGR_ChangeSetting();
				WCPU_LedCtl(LED_UNLINK);
			}
			else if (pbuf[3] == 1)
			{
				io_RadioOnOffAck(HW_RadioOnOff(1, 1));
				if (MAC_Inform.Mode == MAC_MODE_IBSS_STA)
				{
					WCPU_LedCtl(LED_SCAN);
				}
			}
			else if (pbuf[3] == 2)
			{
				if (HW_RadioOnOff(0, 0))
					io_RadioOnOffAck(2);
				else
					io_RadioOnOffAck(3);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WIFI_B_G_PROTECTION:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MGR_IOBGProtection(1, &pbuf[4]);
				io_BgProtectionRespRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IOBGProtection(0, &value8);
				io_BgProtectionRespRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_ENCRYPTION_TYPE:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MGR_IOEncryptType(IO_COMMAND_WRITE, &pbuf[4]);
				io_EncryptionAckRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IOEncryptType(IO_COMMAND_READ, &value8);
				io_EncryptionAckRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_OPEN_SYSTEM:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MGR_IOAuthenAlgorithm(IO_COMMAND_WRITE, &pbuf[4]);
				io_OpenSysAckRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IOAuthenAlgorithm(IO_COMMAND_READ, &value8);
				io_OpenSysAckRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WEP64_KEY_READ:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			MGR_IOWep64(IO_COMMAND_READ, pbuf[3], reg);
			io_Wep64KeyAckRW(IO_COMMAND_READ, reg);
			break;
		case IO_WEP64_KEY_WRITE:
			if (pbuf[2] != 6)
				return IO_COMMAND_ERROR;
			MGR_IOWep64(IO_COMMAND_WRITE, pbuf[3], &pbuf[4]);
			io_Wep64KeyAckRW(IO_COMMAND_WRITE, reg);
			break;
		case IO_WEP128_KEY_READ:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			MGR_IOWep128(IO_COMMAND_READ, pbuf[3], reg);
			io_Wep128KeyAckRW(IO_COMMAND_READ, reg);
			break;
		case IO_WEP128_KEY_WRITE:
			if (pbuf[2] != 14)
				return IO_COMMAND_ERROR;
			MGR_IOWep128(IO_COMMAND_WRITE, pbuf[3], &pbuf[4]);
			io_Wep128KeyAckRW(IO_COMMAND_WRITE, reg);
			break;
		case IO_WEP_KEY_INDEX:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MGR_IOKeyId(IO_COMMAND_WRITE, &pbuf[4]);
				io_WepKeyIndexKeyRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IOKeyId(IO_COMMAND_READ, &value8);
				io_WepKeyIndexKeyRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
#if SUPPLICANT_SUPPORT
		case IO_TKIP_AES_PASSPHASE_STRING_READ:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			MGR_IOPassphrase(IO_COMMAND_READ, reg, &len);
			io_TkipAesAckRW(IO_COMMAND_READ, len, reg);
			break;
		case IO_TKIP_AES_PASSPHASE_STRING_WRITE:
			if (!pbuf[2] || pbuf[2] > 64)
				return IO_COMMAND_ERROR;
			MGR_IOPassphrase(IO_COMMAND_WRITE, &pbuf[3], &pbuf[2]);
			io_TkipAesAckRW(IO_COMMAND_WRITE, len, reg);
			break;
#endif
		case IO_ERROR_COMMAND:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			printf("Drop bad command\n\r");
			break;
		case IO_FRAGMENT_THRESHOLD_READ:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			MGR_IOFragSize(IO_COMMAND_READ, &value16);
			io_FragmentThresholdAckRW(IO_COMMAND_READ, (U8_T XDATA*)&value16);
			break;
		case IO_FRAGMENT_THRESHOLD_WRITE:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			MGR_IOFragSize(IO_COMMAND_WRITE, (U16_T XDATA*)&pbuf[3]);
			io_FragmentThresholdAckRW(IO_COMMAND_WRITE, (U8_T XDATA*)&value16);
			break;
		case IO_RTS_THRESHOLD_READ:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			MGR_IORtsThreshold(IO_COMMAND_READ, &value16);
			io_RtsIntervalAckRW(IO_COMMAND_READ, (U8_T XDATA*)&value16);
			break;
		case IO_RTS_THRESHOLD_WRITE:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			MGR_IORtsThreshold(IO_COMMAND_WRITE, (U16_T XDATA*)&pbuf[3]);
			io_RtsIntervalAckRW(IO_COMMAND_WRITE, (U8_T XDATA*)&value16);
			break;
		case IO_PREAMBLE:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MGR_IOPreamble(IO_COMMAND_WRITE, &pbuf[4]);
				io_PreambleAckRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IOPreamble(IO_COMMAND_READ, &value8);
				io_PreambleAckRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_TX_POWER_LEVEL_READ:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			io_TxGainRW(IO_COMMAND_READ, HW_Read_TxGain());
			break;
		case IO_TX_POWER_LEVEL_WRITE:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			HW_Write_TxGain(pbuf[3]);
			io_TxGainRW(IO_COMMAND_WRITE, 0);
			break;
		case IO_BEACON_INTERVAL_READ:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			MGR_IOBcnInterval(IO_COMMAND_READ, &value16);
			io_BeaconIntervalAckRW(IO_COMMAND_READ, (U8_T XDATA*)&value16);
			break;
		case IO_BEACON_INTERVAL_WRITE:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			MGR_IOBcnInterval(IO_COMMAND_WRITE, (U16_T XDATA*)&pbuf[3]);
			io_BeaconIntervalAckRW(IO_COMMAND_WRITE, (U8_T XDATA*)&value16);
			break;
		case IO_COUNTRY_REGION:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] == 3)
				{
					MGR_IOCountryRegion(IO_COMMAND_WRITE, (U16_T XDATA*)(&pbuf[4]));
					io_CountryRegionAckRW(IO_COMMAND_WRITE, value16);
				}
				else
				{
					return IO_COMMAND_ERROR;
				}
			}
			else if (pbuf[3] == IO_COMMAND_READ)
			{
				if (pbuf[2] == 1)
				{
					MGR_IOCountryRegion(IO_COMMAND_READ, &value16);
					io_CountryRegionAckRW(IO_COMMAND_READ, value16);
				}
				else
				{
					return IO_COMMAND_ERROR;
				}
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_ATIM_INTERVAL_READ:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			MGR_IOAtimInterval(IO_COMMAND_READ, &value16);
			io_AtimIntervalAckRW(IO_COMMAND_READ, (U8_T XDATA*)&value16);
			break;
		case IO_ATIM_INTERVAL_WRITE:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			MGR_IOAtimInterval(IO_COMMAND_WRITE, (U16_T XDATA*)&pbuf[3]);
			io_AtimIntervalAckRW(IO_COMMAND_WRITE, (U8_T XDATA*)&value16);
			break;
		case IO_TX_DATA_RATE:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (0xff == HW_SetTxRate(pbuf[4]))
					return IO_COMMAND_ERROR;
				io_TxDataRateRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_TxDataRateRW(IO_COMMAND_READ, HW_Inform.FixSpeed);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_AUTO_POWER_CONTROL:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] < 2)
					return IO_COMMAND_ERROR;
				switch (pbuf[4])
				{
					case 0:
						MAC_CurSet.SwProfile[MAC_UserProfile].AutoPowerContol = 0;
						WIFI_apcNeed = 0;
						WIFI_AutoPowerControl(0, 0, 0, 0);
						break;
					case 1:
						MAC_CurSet.SwProfile[MAC_UserProfile].AutoPowerContol = 1;
						WIFI_apcNeed = 1;
						break;
					default:
						return IO_COMMAND_ERROR;
				}
				io_AutoPowerCtrlRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_AutoPowerCtrlRW(IO_COMMAND_READ, MAC_CurSet.SwProfile[MAC_UserProfile].AutoPowerContol);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WIFI_ROAMING:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				//need roaming function...
				io_RoamingRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				//need roaming function...
				io_RoamingRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WIFI_MULTIMEDIA_SET:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				//need WMM function...
				io_WifiMultimediaSetRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				//need WMM function...
				io_WifiMultimediaSetRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_LISTEN_INTERVAL:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 3)
					return IO_COMMAND_ERROR;
				//need listen interval function...
				io_ListenIntervalRW(IO_COMMAND_WRITE, value16);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				//need listen interval function...
				io_ListenIntervalRW(IO_COMMAND_READ, value16);
			}
			else
				return IO_COMMAND_ERROR;
			break;
        case IO_MINIMUM_CONTENTION_WINDOW:
            if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 3)
					return IO_COMMAND_ERROR;
				//need contention window function...
				io_MinContentionWindowRW(IO_COMMAND_WRITE, value16);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				//need contention window function...
				io_MinContentionWindowRW(IO_COMMAND_READ, value16);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_MAXIMUM_CONTENTION_WINDOW:
            if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 3)
					return IO_COMMAND_ERROR;
				//need contention window function...
				io_MaxContentionWindowRW(IO_COMMAND_WRITE, value16);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				//need contention window function...
				io_MaxContentionWindowRW(IO_COMMAND_READ, value16);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_SHOW_RSSI:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			value8 = MGR_IORssi();
			io_ShowRssiResp(value8);
			break;
		case IO_PAYLOAD_LENGTH_ATTACH_IN_WIFI_PACKET:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				//need payload length in WiFi function...
				io_PayloadLenAddInWifiRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				//need payload length in WiFi function...
				io_PayloadLenAddInWifiRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_RETAIN_LLC_IN_WIFI_PACKET:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				//need retain LLC in WiFi function...
				io_RetainLlcInWifiRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				//need retain LLC in WiFi function...
				io_RetainLlcInWifiRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_2_LLC_IN_WIFI_VLAN_PACKET:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				//need to add 2 LLC in WiFi VLAN function...
				io_2LlcInWiFiVlanPacketRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				//need to add 2 LLC in WiFi VLAN function...
				io_2LlcInWiFiVlanPacketRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_TRAFFIC_PROFILE:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;

				if (pbuf[4] == MAC_CurSet.SwProfile[MAC_UserProfile].TrafficProfile)
				{
					printf("Same as current traffic profile.\n\r");
				}
				else if (MAC_Inform.State >= MAC_STATE_JOINED)
				{
					MAC_SetTrafficProfile(pbuf[4]);
				}
				else
				{
					if (pbuf[4] <= AC_DF)
					{
						MAC_CurSet.SwProfile[MAC_UserProfile].TrafficProfile = pbuf[4];
					}
					else
					{
						printf("Incorrect profile number !!\n\r");
					}
				}
				io_TrafficProfileRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_TrafficProfileRW(IO_COMMAND_READ, MAC_CurSet.SwProfile[MAC_UserProfile].TrafficProfile);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_TX_DEFAULT_POWER_LEVEL:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				if (pbuf[4] == MAC_CurSet.SwProfile[MAC_UserProfile].PowerLevel)
				{
					io_TxDefaultPowerLevelRW(IO_COMMAND_WRITE, 0xff);
				}
				else
				{
					MAC_IOTxPowerLevel(1, pbuf[4]);
					io_TxDefaultPowerLevelRW(IO_COMMAND_WRITE, MAC_CurSet.SwProfile[MAC_UserProfile].PowerLevel);
				}
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_TxDefaultPowerLevelRW(IO_COMMAND_READ, MAC_CurSet.SwProfile[MAC_UserProfile].PowerLevel);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_QUEUE_PRIORITY_SET:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				// please modify the code you need and call "io_QueuePrioritySetRW(U8_T rw, U8_T index, U8_T priority)" function
#if (QOS_SUPPORT - QOS_ONE_QUEUE)
				io_QueuePrioritySetRW(IO_COMMAND_WRITE, MAC_ExchangePriority(1, pbuf[4]));
#else
				io_QueuePrioritySetRW(IO_COMMAND_WRITE, 0xFE);
#endif
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				// please modify the code you need and call "io_QueuePrioritySetRW(U8_T rw, U8_T index, U8_T priority)" function
#if (QOS_SUPPORT - QOS_ONE_QUEUE)
				io_QueuePrioritySetRW(IO_COMMAND_READ, MAC_ExchangePriority(0, 0));
#else
				io_QueuePrioritySetRW(IO_COMMAND_READ, 0xFE);
#endif
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_SELF_CTS:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MAC_CurSet.SwProfile[MAC_UserProfile].SelfCTS = pbuf[4];
				io_SelfCtsRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_SelfCtsRW(IO_COMMAND_READ, MAC_CurSet.SwProfile[MAC_UserProfile].SelfCTS);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_BSSID:
			if (pbuf[3] == IO_COMMAND_READ)
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IOGetBssid(reg);
				io_BssidR(reg);
			}
			break;
		case IO_WIFI_IP_READ:
			if (pbuf[2] != 0)
				return IO_COMMAND_ERROR;
			io_WifiIpR(MAC_IOGetSecIp());
			break;
		case IO_WIFI_DRIVER_VER_READ:
			if (pbuf[2] != 0)
				return IO_COMMAND_ERROR;
			io_WifiDriverVerR(MAC_IOGetDriverVer());
			break;
#if MASS_PRODUCTION
		case IO_CONTINUOUS_TX:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			DBG_ContinuousTx(pbuf[3]);
			io_ContinuousTxAck(pbuf[3]);
			break;
		case IO_SENSITIVITY_TEST:
			if (!pbuf[2])
				return IO_COMMAND_ERROR;
			
			if (pbuf[5] == 1)
			{
				dbg_senTestSource = 0; //Inspire
			}
			else if (pbuf[5] == 2)
			{
				dbg_senTestSource = 1; //N4010A
			}
			else
			{
				dbg_senTestSource = 0; //Inspire
			}

			DBG_SensitivityTest(1, *((U16_T *)&pbuf[3]));
			io_SensitivityTestAck(&pbuf[3]);
			break;
		case IO_SENSITIVITY_TEST_REPORT:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			DBG_SensitivityTest(0, *((U16_T *)&pbuf[3]));
			io_SensitivityTestReportAck();
			break;
#endif
		case IO_DBG_WCPU_SFR_READ:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			if (0xff == io_SfrIO(IO_COMMAND_READ, pbuf[3], &value8))
				return IO_COMMAND_ERROR;
			io_SfrAckRW(IO_COMMAND_READ, value8);
			break;
		case IO_DBG_WCPU_SFR_WRITE:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			if (0xff == io_SfrIO(IO_COMMAND_WRITE, pbuf[3], &pbuf[4]))
				return IO_COMMAND_ERROR;
			io_SfrAckRW(IO_COMMAND_WRITE, value8);
			break;
		case IO_DBG_WCPU_SUB_SYS_REG_READ:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			len = WCPU_IndirectIO(IO_COMMAND_READ, pbuf[3], reg);
			if (0xff == len)
				return IO_COMMAND_ERROR;
			io_WcpuIndirectAckRW(IO_COMMAND_READ, len, reg);
			break;
		case IO_DBG_WCPU_SUB_SYS_REG_WRITE:
			if (pbuf[2] < 2)
				return IO_COMMAND_ERROR;
			len = WCPU_IndirectIO(IO_COMMAND_WRITE, pbuf[3], &pbuf[4]);
			if (0xff == len)
				return IO_COMMAND_ERROR;
			io_WcpuIndirectAckRW(IO_COMMAND_WRITE, len, reg);
			break;
		case IO_DBG_WCPU_MIIB_REG_READ:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			len = MIB_IndirectIO(IO_COMMAND_READ, pbuf[3], reg);
			if (0xff == len)
				return IO_COMMAND_ERROR;
			io_MiibIndirectAckRW(IO_COMMAND_READ, len, reg);
			break;
		case IO_DBG_WCPU_MIIB_REG_WRITE:
			if (pbuf[2] < 2)
				return IO_COMMAND_ERROR;
			len = MIB_IndirectIO(IO_COMMAND_WRITE, pbuf[3], &pbuf[4]);
			if (0xff == len)
				return IO_COMMAND_ERROR;
			io_MiibIndirectAckRW(IO_COMMAND_WRITE, len, reg);
			break;
		case IO_DBG_WCPU_PCIB_REG_READ:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			len = PCIB_IndirectIO(IO_COMMAND_READ, pbuf[3], reg);
			if (0xff == len)
			return IO_COMMAND_ERROR;
			io_PcibIndirectRegAckRW(IO_COMMAND_READ, len, reg);
			break;
		case IO_DBG_WCPU_PCIB_REG_WRITE:
			if (pbuf[2] < 2)
				return IO_COMMAND_ERROR;
			len = PCIB_IndirectIO(IO_COMMAND_WRITE, pbuf[3], &pbuf[4]);
			if (0xff == len)
			return IO_COMMAND_ERROR;
			io_PcibIndirectRegAckRW(IO_COMMAND_WRITE, len, reg);
			break;
		case IO_DBG_PCI_CONFIGURATION_READ:
			if (pbuf[2] != 1)
				return IO_COMMAND_ERROR;
			PCIB_PciRead(PCIB_CONFIG_MODE, pbuf[3], reg);
			io_PciConfigAckRW(IO_COMMAND_READ, reg);
			break;
		case IO_DBG_PCI_CONFIGURATION_WRITE:
			if (pbuf[2] < 2)
				return IO_COMMAND_ERROR;
			PCIB_PciWrite(PCIB_CONFIG_MODE, pbuf[3], &pbuf[4]);
			io_PciConfigAckRW(IO_COMMAND_WRITE, reg);
			break;
		case IO_DBG_WIFI_REG_READ:
			if (pbuf[2] != 2)
				return IO_COMMAND_ERROR;
			if (0xff == io_WifiRegIO(IO_COMMAND_READ, *((U16_T*)&pbuf[3]), &value32))
				return IO_COMMAND_ERROR;
			io_WiFiRegAckRW(IO_COMMAND_READ, (U8_T XDATA*)&value32);
			break;
		case IO_DBG_WIFI_REG_WRITE:
			if (pbuf[2] != 6)
				return IO_COMMAND_ERROR;
			if (0xff == io_WifiRegIO(IO_COMMAND_WRITE, *((U16_T*)&pbuf[3]), (U32_T XDATA*)&pbuf[5]))
				return IO_COMMAND_ERROR;
			io_WiFiRegAckRW(IO_COMMAND_WRITE, (U8_T XDATA*)&value32);
			break;
		case IO_DBG_WIFI_PACKET_FILTERING_WRITE:
			if (pbuf[2] != 9)
				return IO_COMMAND_ERROR;
			PCIB_WlanPktFilterIO(IO_COMMAND_WRITE, &pbuf[3], (U16_T XDATA*)&pbuf[4], &pbuf[6]);
			io_WlanPktFilterAckRW(IO_COMMAND_WRITE, value8, value16, reg);
			break;
		case IO_DBG_WIFI_PACKET_FILTERING_READ:
			if (pbuf[2])
				return IO_COMMAND_ERROR;
			PCIB_WlanPktFilterIO(IO_COMMAND_READ, &value8, &value16, reg);
			io_WlanPktFilterAckRW(IO_COMMAND_READ, value8, value16, reg);
			break;
		case IO_DBG_SYNTHESIZER_SET:
			if (pbuf[2] != 4)
				return IO_COMMAND_ERROR;

			HW_3W_IF_Synthesizer(dbg_SYNBitReverse(*((U32_T XDATA*)&pbuf[3])));
			io_SynthesizerSetAck();
			break;
		case IO_DBG_RETRY_NUMBER:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				MGR_IORetry(IO_COMMAND_WRITE, &pbuf[4]);
				io_RetryAckRW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				MGR_IORetry(IO_COMMAND_READ, &value8);
				io_RetryAckRW(IO_COMMAND_READ, value8);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_GPIO_ZERO:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				P0 = pbuf[4];
				io_Gpio0RW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_Gpio0RW(IO_COMMAND_READ, P0);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_GPIO_ONE:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				P1 = pbuf[4];
				io_Gpio1RW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_Gpio1RW(IO_COMMAND_READ, P1);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_GPIO_TWO:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				P2 = pbuf[4];
				io_Gpio2RW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_Gpio2RW(IO_COMMAND_READ, P2);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_GPIO_THREE:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if (pbuf[2] != 2)
					return IO_COMMAND_ERROR;
				P3 = pbuf[4];
				io_Gpio3RW(IO_COMMAND_WRITE, value8);
			}
			else if (!pbuf[3])
			{
				if (pbuf[2] != 1)
					return IO_COMMAND_ERROR;
				io_Gpio3RW(IO_COMMAND_READ, P3);
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_DBG_AUTO_POWER_CONTROL:
			if (pbuf[3] == IO_COMMAND_WRITE)
			{
				if ((pbuf[2] != 3) && (pbuf[4] != 2))
					return IO_COMMAND_ERROR;
				if (MAC_Inform.State >= MAC_STATE_JOINED)
				{
					MAC_CurSet.SwProfile[MAC_UserProfile].AutoPowerContol = 1;
					WIFI_AutoPowerControl(0, 0, 0, 1);
					value32 = (U32_T)pbuf[5];
					if (0xff == io_WifiRegIO(IO_COMMAND_WRITE, 0x007c, &value32))
						return IO_COMMAND_ERROR;
				}
				io_AutoPowerCtrlDetailW();
			}
			else
				return IO_COMMAND_ERROR;
			break;
		case IO_WLAN_DEBUG1_RESP:
			break;
		default:
			return IO_COMMAND_ERROR;
		}
	}

	return IO_COMMAND_DONE;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_NoticeMcpu
 * Purpose: To management CPU connect, disconnect, and ready commands
 * Params: U8_T  type - The type of the command
 *         U8_T* pbuf - The start pointer of the payload buffer
 *         U8_T  len  - Length of the payload
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_NoticeMcpu(U8_T type, U8_T* pbuf, U8_T len)
{
	U8_T XDATA			i;
	static U8_T XDATA	avoidRepeat = 0;

	if (type == IO_MCPU_WCPU_CONNECT)
	{
#if SUPPLICANT_SUPPORT
		SUP_notice = 0;
#endif
#if QOS_SUPPORT
		if (MAC_Inform.CurWmmMode)
		{
			HW_QosSetting();
		}
#endif
		/* Check out if need to set custom traffic profile */
		{
			MAC_SetTrafficProfile(MAC_CurSet.SwProfile[MAC_UserProfile].TrafficProfile);
		}
		
		if (avoidRepeat == 1)
		{
			return;
		}
		
		/* Enable receiving PKT of MIB */
		MIB_IndirectIO(0, MIB_TX_CONTROL, &i);
		i |= MIB_RCV_ENABLE;
		MIB_IndirectIO(1, MIB_TX_CONTROL, &i);

		avoidRepeat = 1;
	}
	else if (type == IO_MCPU_WCPU_DISCONNECT)
	{
#if SUPPLICANT_SUPPORT
		SUP_notice = 1;
#endif
		if (avoidRepeat == 2)
		{
			return;
		}
		
		/* Disable receiving PKT of MIB */
		MIB_IndirectIO(0, MIB_TX_CONTROL, &i);
		i &= ~(MIB_RCV_ENABLE);
		MIB_IndirectIO(1, MIB_TX_CONTROL, &i);
		
		avoidRepeat = 2;
	}

	if (WCPU_McpuReady || type == IO_MCPU_WCPU_READY)
	{
		MWIF_Send(type, pbuf, len);
	}

} /* End of IO_NoticeMcpu() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_ScanReport
 * Purpose: To reply the scanning result to MCPU
 * Params: U8_T result - 1: no find any BSS, generate itself
 *                       2: no find any BSS, rescan
 *                       3: find a BSS
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_ScanReport(U8_T result)
{
	U8_T	len = 0;

	switch (result)
	{
	case 1:	// Not found any BSS, generate a new IBSS
	case 2:	// Not found any BSS, rescan
		len = 1;
		io_TxBuf[0] = result;
		break;
	case 3:	// BSS is found
		io_TxBuf[0] = result;
		len = sizeof (MGR_KNOWN_BSS);
		memcpy(&io_TxBuf[1], &MGR_Inform.BssList[0], len);
		len += 1;
		break;
	}

	MWIF_Send(IO_SCAN_REPORT, io_TxBuf, len);
	
}	/* End of IO_ScanReport */

/*
 * ----------------------------------------------------------------------------
 * Function Name: IO_SiteSurveyReport
 * Purpose: To reply site survey result to MCPU
 * Params: U8_T BssNum - The order of BSS
 *         U8_T IsTheLast - 0: Not the last BSS
 *                          1: Is the last BSS
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void IO_SiteSurveyReport(U8_T BssNum, U8_T IsTheLast)
{
	MGR_BSSID XDATA*	pBSS = &MGR_Inform.BssInform[BssNum];
	U8_T				i, len;
	U8_T XDATA*			ptr;
	
	if (IsTheLast != 2)	// find a BSS at least
	{
		io_TxBuf[0] = BssNum;
		io_TxBuf[1] = pBSS->MacAddr[0];
		io_TxBuf[2] = pBSS->MacAddr[1];
		io_TxBuf[3] = pBSS->MacAddr[2];
		io_TxBuf[4] = pBSS->MacAddr[3];
		io_TxBuf[5] = pBSS->MacAddr[4];
		io_TxBuf[6] = pBSS->MacAddr[5];
		io_TxBuf[7] = pBSS->InfrastructureMode;
		io_TxBuf[8] = pBSS->Config.Channel;
		io_TxBuf[9] = pBSS->Rssi;
		io_TxBuf[10] = pBSS->EncryptMode;
		io_TxBuf[11] = IsTheLast;
		ptr = &io_TxBuf[12];
		len = pBSS->Ssid.SsidLength;
		for (i = 0; i < len; i++)
		{
			ptr[i] = pBSS->Ssid.Ssid[i];
		}
		len = len + 12;
	}
	else	// can't find any BSS
	{
		len = 1;
		io_TxBuf[0] = 0;
	}	

	MWIF_Send(IO_SITE_SURVEY_REPORT, io_TxBuf, len);

}	/* End of IO_SiteSurveyReport */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_ResetAck
 * Purpose: To reply a acknowedgement to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_ResetAck(void)
{
	MWIF_Send(IO_WCPU_RESET_ACK, NULL, 0);
	
}	/* End of io_ResetAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_ScanAck
 * Purpose: To reply a acknowedgement to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_ScanAck(void)
{
	MWIF_Send(IO_SCAN_ACK, NULL, 0);
	
}	/* End of io_ScanAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SiteSurveyAck
 * Purpose: To reply a acknowedgement to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SiteSurveyAck(void)
{
	MWIF_Send(IO_SITE_SURVEY_ACK, NULL, 0);
	
}	/* End of io_SiteSurveyAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_JoinBssAck
 * Purpose: To reply a acknowedgement to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_JoinBssAck(void)
{
	MWIF_Send(IO_JOIN_BSS_ACK, NULL, 0);
	
}	/* End of io_JoinBssAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SsidAckRW
 * Purpose: To reply a writing acknowedgement or the SSID to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T  len - The SSID length
 *         U8_T* pReg - Start pointer of the buffer to save SSID
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SsidAckRW(U8_T type, U8_T len, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_SSID_WRITE_ACK, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pReg, len);

		MWIF_Send(IO_SSID_READ_RESP, io_TxBuf, len);
	}
}	/* End of io_SsidRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_OperateModeAckRW
 * Purpose: To reply a writing acknowledgement or the WiFi operating mode to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WiFi operating mode
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_OperateModeAckRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_OPERATE_MODE_RESP, io_TxBuf, len);
	
}	/* End of io_OperateModeAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_ChannelAckRW
 * Purpose: To reply a writing acknowledgement or the WiFi channel to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WiFi channel value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_ChannelAckRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_CHANNEL_RESP, io_TxBuf, len);
	
}	/* End of io_ChannelAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_NetworkModeAckRW
 * Purpose: To reply a writing acknowledgement or the WiFi network mode to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WiFi network mode
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_NetworkModeAckRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_NETWORK_MODE_RESP, io_TxBuf, len);
	
}	/* End of io_NetworkModeAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SetMulticastMacAck
 * Purpose: To reply a writing acknowledgement or the WiFi multicast MAC address to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SetMulticastMacAck(void)
{
	MWIF_Send(IO_MULTICAST_SET_ACK, NULL, 0);
	
}	/* End of io_SetMulticastMacAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WiFiMacAckRW
 * Purpose: To reply a writing acknowledgement or the WiFi MAC address to MCPU
 * Params: U8_T  type - 1: write command 0: read command
 *         U8_T* pReg - Start pointer of the WiFi MAC buffer
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_WiFiMacAckRW(U8_T type, U8_T XDATA* pReg)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = 1;
	}
	else
	{
		len = 7;
		io_TxBuf[0] = 0;
		memcpy(&io_TxBuf[1], pReg, 6);
	}

	MWIF_Send(IO_WIFI_MAC_ADDR_SET_RESP, io_TxBuf, len);
	
}	/* End of io_WiFiMacRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_RadioOnOffAck
 * Purpose: To reply a writing acknowledgement or the WiFi RF state to MCPU
 * Params: U8_T state - The state of WiFi RF
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_RadioOnOffAck(U8_T state)
{
	io_TxBuf[0] = state;

	MWIF_Send(IO_WIFI_RADIO_ON_OFF_ACK, io_TxBuf, 1);

}	/* End of io_RadioOnOffAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_BgProtectionRespRW
 * Purpose: To reply a writing acknowledgement or the WiFi data rate to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  state - The state of bg protection mode
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_BgProtectionRespRW(U8_T rw, U8_T state)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = state;
	}
	
	MWIF_Send(IO_WIFI_B_G_PROTECTION_RESP, io_TxBuf, len);

}	/* End of io_BgProtectionRespRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_EncryptionAckRW
 * Purpose: To reply a writing acknowledgement or the WiFi encryption type to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WiFi encryption type
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_EncryptionAckRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_ENCRYPTION_TYPE_RESP, io_TxBuf, len);
	
}	/* End of io_EncryptionAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_OpenSysAckRW
 * Purpose: To reply a writing acknowledgement or the open system mode to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WiFi open system mode
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_OpenSysAckRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_OPEN_SYSTEM_RESP, io_TxBuf, len);
	
}	/* End of io_OpenSysAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_Wep64KeyRW
 * Purpose: To reply a writing acknowedgement or the WEP64 key to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T* pReg - Start pointer of the buffer to save WEP64 key
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_Wep64KeyAckRW(U8_T type, U8_T XDATA* pReg)
{
	if (type)// write
	{
		MWIF_Send(IO_WEP64_KEY_WRITE_ACK, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pReg, 5);

		MWIF_Send(IO_WEP64_KEY_READ_RESP, io_TxBuf, 5);
	}
}	/* End of io_Wep64KeyRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_Wep128KeyAckRW
 * Purpose: To reply a writing acknowedgement or the WEP128 key to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T* pReg - Start pointer of the buffer to save WEP128 key
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_Wep128KeyAckRW(U8_T type, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_WEP128_KEY_WRITE_ACK, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pReg, 13);

		MWIF_Send(IO_WEP128_KEY_READ_RESP, io_TxBuf, 13);
	}
}	/* End of io_Wep128KeyRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WepKeyIndexKeyRW
 * Purpose: To reply a writing acknowledgement or the WEP key index to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WEP key index
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_WepKeyIndexKeyRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_WEP_KEY_INDEX_RESP, io_TxBuf, len);
	
}	/* End of io_WepKeyIndexKeyRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_TkipAesAckRW
 * Purpose: To reply a writing acknowedgement or the TKIP/AES key to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T  len - The TKIP/AES key length
 *         U8_T* pReg - Start pointer of the buffer to save TKIP/AES key
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_TkipAesAckRW(U8_T type, U8_T len, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_TKIP_AES_PASSPHASE_STRING_WRITE_ACK, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pReg, len);

		MWIF_Send(IO_TKIP_AES_PASSPHASE_STRING_READ_RESP, io_TxBuf, len);
	}
}	/* End of io_TkipAesRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_FragmentThresholdRW
 * Purpose: To reply a writing acknowedgement or the fragment threshold value to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T* pValue - Start pointer of the butter to save fragment threshold value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_FragmentThresholdAckRW(U8_T type, U8_T XDATA* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_FRAGMENT_THRESHOLD_WRITE_ACK, NULL, 0);
	}
	else
	{
		io_TxBuf[0] = *pValue;
		io_TxBuf[1] = *(pValue + 1);

		MWIF_Send(IO_FRAGMENT_THRESHOLD_READ_RESP, io_TxBuf, 2);
	}
}	/* End of io_FragmentThresholdRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_RtsThresholdRW
 * Purpose: To reply a writing acknowedgement or the RTS threshold value to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T* pValue - Start pointer of the butter to save RTS threshold value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_RtsIntervalAckRW(U8_T type, U8_T XDATA* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_RTS_THRESHOLD_WRITE_ACK, NULL, 0);
	}
	else
	{
		io_TxBuf[0] = *pValue;
		io_TxBuf[1] = *(pValue + 1);

		MWIF_Send(IO_RTS_THRESHOLD_READ_RESP, io_TxBuf, 2);
	}
}	/* End of io_RtsThresholdRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_PreambleAckRW
 * Purpose: To reply a writing acknowledgement or the preamble mode to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WiFi preamble mode
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_PreambleAckRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_PREAMBLE_RESP, io_TxBuf, len);
	
}	/* End of io_PreambleAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_TxGainRW
 * Purpose: To reply a writing acknowledgement or the WiFi Tx power gain to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  txGain - The value of WiFi Tx power gain
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_TxGainRW(U8_T rw, U8_T txGain)
{
	if (rw == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_TX_POWER_LEVEL_WRITE_ACK, NULL, 0);
	}
	else
	{
		io_TxBuf[0] = txGain;

		MWIF_Send(IO_TX_POWER_LEVEL_READ_RESP, io_TxBuf, 1);
	}
}	/* End of io_TxGainRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_BeaconIntervalRW
 * Purpose: To reply a writing acknowedgement or the beacon interval value to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T* pValue - Start pointer of the butter to save beacon interval value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_BeaconIntervalAckRW(U8_T type, U8_T* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_BEACON_INTERVAL_WRITE_ACK, NULL, 0);
	}
	else
	{
		io_TxBuf[0] = *pValue;
		io_TxBuf[1] = *(pValue + 1);

		MWIF_Send(IO_BEACON_INTERVAL_READ_RESP, io_TxBuf, 2);
	}
}	/* End of io_BeaconIntervalRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_CountryRegionAckRW
 * Purpose: To reply a writing acknowledgement or the WiFi country region to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WiFi country region
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_CountryRegionAckRW(U8_T type, U16_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 3;
		io_TxBuf[0] = IO_COMMAND_READ;
		*((U16_T XDATA *)(&io_TxBuf[1])) = value;
	}

	MWIF_Send(IO_COUNTRY_REGION_RESP, io_TxBuf, len);
	
}	/* End of io_CountryRegionAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_AtimIntervalRW
 * Purpose: To reply a writing acknowedgement or the ATIM interval value to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T* pValue - Start pointer of the butter to save ATIM interval value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_AtimIntervalAckRW(U8_T type, U8_T XDATA* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_ATIM_INTERVAL_WRITE_ACK, NULL, 0);
	}
	else
	{
		io_TxBuf[0] = *pValue;
		io_TxBuf[1] = *(pValue + 1);

		MWIF_Send(IO_ATIM_INTERVAL_READ_RESP, io_TxBuf, 2);
	}
}	/* End of io_AtimIntervalRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_TxDataRateRW
 * Purpose: To reply a writing acknowledgement or the WiFi data rate to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  txRate - The value of WiFi data rate
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_TxDataRateRW(U8_T rw, U8_T txRate)
{
	U8_T	len;	

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = txRate;
	}

	MWIF_Send(IO_TX_DATA_RATE_RESP, io_TxBuf, len);
	
}	/* End of io_TxDataRateRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_AutoPowerCtrlRW
 * Purpose: To reply a writing acknowledgement or the auto power control state to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  state - The state of auto power control
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_AutoPowerCtrlRW(U8_T rw, U8_T state)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = state;
	}
	
	MWIF_Send(IO_AUTO_POWER_CONTROL_RESP, io_TxBuf, len);

}	/* End of io_AutoPowerCtrlRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_AutoPowerCtrlDetailW
 * Purpose: To reply a writing acknowledgement or the auto power control state to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  state - The state of auto power control
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_AutoPowerCtrlDetailW(void)
{
	U8_T	len;

	len = 1;
	io_TxBuf[0] = IO_COMMAND_WRITE;
	
	MWIF_Send(IO_DBG_AUTO_POWER_CONTROL_RESP, io_TxBuf, len);

}	/* End of io_AutoPowerCtrlDetailW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_RoamingRW
 * Purpose: To reply a writing acknowledgement or the roaming state to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  state - The state of roaming
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_RoamingRW(U8_T rw, U8_T state)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = state;
	}
	
	MWIF_Send(IO_WIFI_ROAMING_RESP, io_TxBuf, len);

}	/* End of io_RoamingRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WifiMultimediaSetRW
 * Purpose: To reply a writing acknowledgement or the WMM state to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  state - The state of WMM
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_WifiMultimediaSetRW(U8_T rw, U8_T state)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = state;
	}
	
	MWIF_Send(IO_WIFI_MULTIMEDIA_SET_RESP, io_TxBuf, len);

}	/* End of io_WifiMultimediaSetRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_ListenIntervalRW
 * Purpose: To reply a writing acknowledgement or the listen interval to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  value - The value of listen interval
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_ListenIntervalRW(U8_T rw, U16_T value)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 3;
		io_TxBuf[0] = IO_COMMAND_READ;
		*((U16_T*)&io_TxBuf[1]) = value;
	}
	
	MWIF_Send(IO_LISTEN_INTERVAL_RESP, io_TxBuf, len);

}	/* End of io_ListenIntervalRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_MinContentionWindowRW
 * Purpose: To reply a writing acknowledgement or the minimum contention window to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  value - The value of minimum contention window
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_MinContentionWindowRW(U8_T rw, U16_T value)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 3;
		io_TxBuf[0] = IO_COMMAND_READ;
		*((U16_T*)&io_TxBuf[1]) = value;
	}
	
	MWIF_Send(IO_MINIMUM_CONTENTION_WINDOW_RESP, io_TxBuf, len);

}	/* End of io_MinContentionWindowRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_MaxContentionWindowRW
 * Purpose: To reply a writing acknowledgement or the maximum contention window to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  value - The value of minimum contention window
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_MaxContentionWindowRW(U8_T rw, U16_T value)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 3;
		io_TxBuf[0] = IO_COMMAND_READ;
		*((U16_T*)&io_TxBuf[1]) = value;
	}
	
	MWIF_Send(IO_MAXIMUM_CONTENTION_WINDOW_RESP, io_TxBuf, len);

}	/* End of io_MaxContentionWindowRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_ShowRssiResp
 * Purpose: To reply RSSI value to MCPU
 * Params: U8_T rssi - Input of the RSSI value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_ShowRssiResp(U8_T rssi)
{
	io_TxBuf[0] = rssi;

	MWIF_Send(IO_SHOW_RSSI_RESP, io_TxBuf, 1);
	
}	/* End of io_ShowRssiResp */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_PayloadLenAddInWifiRW
 * Purpose: To reply a writing acknowledgement or the payload length in WiFi packet to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  value - 0: Disable payload length in WiFi packet
 *                       1: Enable payload length in WiFi packet
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_PayloadLenAddInWifiRW(U8_T rw, U8_T state)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = state;
	}
	
	MWIF_Send(IO_PAYLOAD_LENGTH_ATTACH_IN_WIFI_PACKET_RESP, io_TxBuf, len);
	
}	/* End of io_PayloadLenAddInWifiRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_RetainLlcInWifiRW
 * Purpose: To reply a writing acknowledgement or the retain LLC in WiFi packet to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  value - 0: Disable retain LLC in WiFi packet
 *                       1: Enable retain LLC in WiFi packet
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_RetainLlcInWifiRW(U8_T rw, U8_T state)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[1] = IO_COMMAND_READ;
		io_TxBuf[2] = state;
	}
	
	MWIF_Send(IO_RETAIN_LLC_IN_WIFI_PACKET_RESP, io_TxBuf, len);
	
}	/* End of io_RetainLlcInWifiRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_2LlcInWiFiVlanPacketRW
 * Purpose: To reply a writing acknowledgement or add 2 LLC in WiFi VLAN packet to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  value - 0: Disable 2 LLC in WiFi VLAN packet
 *                       1: Enable 2 LLC in WiFi VLAN packet
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_2LlcInWiFiVlanPacketRW(U8_T rw, U8_T state)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = state;
	}
	
	MWIF_Send(IO_2_LLC_IN_WIFI_VLAN_PACKET, io_TxBuf, len);
	
}	/* End of io_2LlcInWiFiVlanPacketRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_TrafficProfileRW
 * Purpose: To reply a writing acknowledgement or send the traffic profile value to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  state - 0: voice        1: vedio
 *                       2: best effort  3: back ground
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_TrafficProfileRW(U8_T rw, U8_T state)
{
	U8_T	len;

	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len= 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = state;
	}
	
	MWIF_Send(IO_TRAFFIC_PROFILE_RESP, io_TxBuf, len);
	
}	/* End of io_TrafficProfileRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_TxDefaultPowerLevelRW
 * Purpose: To reply a writing acknowledgement or current Tx default power level to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  level - 0: 100%, 1: 50%, 2: 25%, 3: 12.5%, 4: 6.25%
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_TxDefaultPowerLevelRW(U8_T rw, U8_T level)
{
	if (rw == IO_COMMAND_WRITE)
	{
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = level;
	}
	else
	{
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = level;
	}
	
	MWIF_Send(IO_TX_DEFAULT_POWER_LEVEL_RESP, io_TxBuf, 2);
	
}	/* End of io_TxDefaultPowerLevelRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_QueuePrioritySetRW
 * Purpose: To reply a queue priority to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  index - 0~3: Queue index
 *         U8_T  priority - 0~3: voice, vedio, best effort, background
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_QueuePrioritySetRW(U8_T rw, U8_T priority)
{
	U8_T	len;
	
	if (rw == IO_COMMAND_WRITE)
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_WRITE;
		io_TxBuf[1] = priority;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = priority;
	}
	
	MWIF_Send(IO_QUEUE_PRIORITY_SET_RESP, io_TxBuf, len);
	
}	/* End of io_TxDefaultPowerLevelRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SelfCtsRW
 * Purpose: To reply the self-CTS from MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  value - 0: OFF, 1: ON
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SelfCtsRW(U8_T rw, U8_T value)
{
	U8_T	len;
	
	if (rw == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}
	
	MWIF_Send(IO_SELF_CTS_RESP, io_TxBuf, len);
	
}	/* End of io_SelfCtsRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_BssidR
 * Purpose: To reply the BSSID to MCPU
 * Params: U8_T  *bssid
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_BssidR(U8_T *bssid)
{
	io_TxBuf[0] = IO_COMMAND_READ;
	memcpy(&io_TxBuf[1], bssid, 6);
	
	MWIF_Send(IO_BSSID_RESP, io_TxBuf, 7);
	
}	/* End of io_BssidR */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WifiIpR
 * Purpose: To reply the Wifi IP address to MCPU
 * Params: U8_T  *wifiIp
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_WifiIpR(U8_T *wifiIp)
{
	memcpy(&io_TxBuf[0], wifiIp, 4);
	
	MWIF_Send(IO_WIFI_IP_READ_RESP, io_TxBuf, 4);
	
}	/* End of io_WifiIpR */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WifiDriverR
 * Purpose: To reply the Wifi driver version to MCPU
 * Params: U8_T  *pVer
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_WifiDriverVerR(U8_T *pVer)
{
	memcpy(&io_TxBuf[0], pVer, (U8_T)strlen(pVer));

	MWIF_Send(IO_WIFI_DRIVER_VER_READ_RESP, io_TxBuf, (U8_T)strlen(pVer));
	
}	/* End of io_WifiDriverVerR */

#if MASS_PRODUCTION
/*
 * ----------------------------------------------------------------------------
 * Function Name: io_ContinuousTxAck
 * Purpose: To reply a writing acknowledgement to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_ContinuousTxAck(U8_T mode)
{
	io_TxBuf[0] = mode;

	MWIF_Send(IO_CONTINUOUS_TX_ACK, io_TxBuf, 1);

}	/* End of io_ContinuousTxAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SensitivityTestAck
 * Purpose: To reply a acknowledgement to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SensitivityTestAck(U8_T *ptr)
{
	io_TxBuf[0] = ptr[0];
	io_TxBuf[1] = ptr[1];
	
	MWIF_Send(IO_SENSITIVITY_TEST_ACK, io_TxBuf, 2);

}	/* End of io_SensitivityTestAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SensitivityTestReportAck
 * Purpose: To reply a acknowledgement to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SensitivityTestReportAck(void)
{
	U8_T	i, j;

	j = (U8_T)strlen(DBG_senTestResult);

	for (i = 0; i < j; i++)
	{
		io_TxBuf[i] = DBG_senTestResult[i];
	}

	MWIF_Send(IO_SENSITIVITY_TEST_REPORT_ACK, io_TxBuf, j);
	
}	/* End of io_SensitivityTestReportAck */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SfrIO
 * Purpose: To write the value into specific SFR or to read the value from specific SFR
 * Params: U8_T  rw - 0: read 1: write
 *         U8_T  index - The index of the specific SFR
 *         U8_T* pValue - The pointer of the buffer to save SFR value
 * Returns: 0x00: success 0xFF: fault
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T io_SfrIO(U8_T rw, U8_T index, U8_T XDATA* pValue)
{
	if (rw == IO_COMMAND_READ)
	{
		switch(index)
		{
		case 0x80:
			*pValue = P0;
			break;
		case 0x81:
			*pValue = SP;
			break;
		case 0x82:
			*pValue = DPL0;
			break;
		case 0x83:
			*pValue = DPH0;
			break;
		case 0x84:
			*pValue = DPL1;
			break;
		case 0x85:
			*pValue = DPH1;
			break;
		case 0x86:
			*pValue = DPS;
			break;
		case 0x87:
			*pValue = PCON;
			break;
		case 0x88:
			*pValue = TCON;
			break;
		case 0x89:
			*pValue = TMOD;
			break;
		case 0x8a:
			*pValue = TL0;
			break;
		case 0x8b:
			*pValue = TL1;
			break;
		case 0x8c:
			*pValue = TH0;
			break;
		case 0x8d:
			*pValue = TH1;
			break;
		case 0x8e:
			*pValue = CKCON;
			break;
		case 0x90:
			*pValue = P1;
			break;
		case 0x91:
			*pValue = EIF;
			break;
		case 0x92:
			*pValue = WTST;
			break;
		case 0x93:
			*pValue = DPX0;
			break;
		case 0x95:
			*pValue = DPX1;
			break;
		case 0x98:
			*pValue = SCON0;
			break;
	   	case 0x99:
			*pValue = SBUF0;
			break;
		case 0x9d:
			*pValue = ACON;
			break;
	    case 0xa0:
			*pValue = P2;
			break;
		case 0xa1:
			*pValue = PMALR;
			break;
	    case 0xa2:
			*pValue = PMAHR;
			break;
		case 0xa3:
			*pValue = PMDR0;
			break;
		case 0xa4:
			*pValue = PMDR1;
			break;
		case 0xa5:
			*pValue = PMDR2;
			break;
		case 0xa6:
			*pValue = PMDR3;
			break;
		case 0xa7:
			*pValue = PBIER;
			break;
		case 0xa8:
			*pValue = IE;
			break;
		case 0xa9:
			*pValue = RHPLR;
			break;
		case 0xaa:
			*pValue = RHPHR;
			break;
		case 0xab:
			*pValue = RSPLR;
			break;
		case 0xac:
			*pValue = RSPHR;
			break;
		case 0xae:
			*pValue = PBSR;
			break;
		case 0xaf:
			*pValue = PBISR;
			break;
		case 0xb0:
			*pValue = P3;
			break;
		case 0xb1:
			*pValue = MTWPL;
			break;
		case 0xb2:
			*pValue = MTWPH;
			break;
		case 0xb3:
			*pValue = CTRPL;
			break;
		case 0xb4:
			*pValue = CTRPH;
			break;
		case 0xb5:
			*pValue = TXBS;
			break;
		case 0xb8:
			*pValue = IP;
			break;
		case 0xb9:
			*pValue = MRPLL;
			break;
		case 0xba:
			*pValue = MRPLH;
			break;
		case 0xbb:
			*pValue = MRBLL;
			break;
		case 0xbc:
			*pValue = MRBLH;
			break;
		case 0xbd:
			*pValue = MRBSAL;
			break;
		case 0xbe:
			*pValue = MRBSAH;
			break;
		case 0xbf:
			*pValue = MRCR;
			break;
		case 0xc0:
			*pValue = SCON1;
			break;
		case 0xc1:
			*pValue = SBUF1;
			break;
		case 0xc2:
			*pValue = MISR;
			break;
		case 0xc3:
			*pValue = MIMR;
			break;
		case 0xc8:
			*pValue = T2CON;
			break;
		case 0xc9:
			*pValue = T2IF;
			break;
		case 0xca:
			*pValue = RLDL;
			break;
		case 0xcb:
			*pValue = RLDH;
			break;
		case 0xcc:
			*pValue = TL2;
			break;
		case 0xcd:
			*pValue = TH2;
			break;
		case 0xce:
			*pValue = WI2CCIR;
			break;
		case 0xcf:
			*pValue = WI2CDR;
			break;
		case 0xd0:
			*pValue = PSW;
			break;
		case 0xd8:
			*pValue = WDCON;
			break;
		case 0xd9:
			*pValue = SINTMR;
			break;
		case 0xda:
			*pValue = HASHCR;
			break;
		case 0xdb:
			*pValue = HASHDR;
			break;
		case 0xe0:
			*pValue = ACC;
			break;
		case 0xe1:
			*pValue = SINTSR;
			break;
		case 0xe2:
			*pValue = SSCIR;
			break;
		case 0xe3:
			*pValue = SSDR;
			break;
		case 0xe4:
			*pValue = MBCIR;
			break;
		case 0xe5:
			*pValue = MBDR;
			break;
		case 0xe6:
			*pValue = PBCIR;
			break;
		case 0xe7:
			*pValue = PBDR;
			break;
		case 0xe8:
			*pValue = EIE;
			break;
		case 0xe9:
			*pValue = STATUS;
			break;
		case 0xea:
			*pValue = MXAX;
			break;
		case 0xeb:
			*pValue = TA;
			break;
//		case 0xec:
//			*pValue = SHACIR;
//			break;
//		case 0xed:
//			*pValue = SHADR;
//			break;
		case 0xee:
			*pValue = RNO;
			break;
		case 0xef:
			*pValue = RNDP;
			break;
		case 0xf0:
			*pValue = B;
			break;
		case 0xf1:
			*pValue = RC0;
			break;
		case 0xf2:
			*pValue = RC1;
			break;
		case 0xf3:
			*pValue = RC2;
			break;
		case 0xf4:
			*pValue = RC3;
			break;
		case 0xf5:
			*pValue = RC4;
			break;
		case 0xf6:
			*pValue = MOP;
			break;
		case 0xf7:
			*pValue = MD;
			break;
		case 0xf8:
			*pValue = EIP;
			break;
		case 0xf9:
			*pValue = WC0;
			break;
		case 0xfa:
			*pValue = WC1;
			break;
		case 0xfb:
			*pValue = WC2;
			break;
		case 0xfc:
			*pValue = WC3;
			break;
		case 0xfd:
			*pValue = WC4;
			break;
		case 0xfe:
			*pValue = CCTRL;
			break;
		case 0xff:
			*pValue = CSSR;
			break;
		default:
			return IO_COMMAND_ERROR;
		}
	}
	else if (rw == 1) // write sfr
	{
		switch(index)
		{
		case 0x80:
			P0 = *pValue;
			break;
		case 0x81:
			SP = *pValue;
			break;
		case 0x82:
			DPL0 = *pValue;
			break;
		case 0x83:
			DPH0 = *pValue;
			break;
		case 0x84:
			DPL1 = *pValue;
			break;
		case 0x85:
			DPH1 = *pValue;
			break;
		case 0x86:
			DPS = *pValue;
			break;
		case 0x87:
			PCON = *pValue;
			break;
		case 0x88:
			TCON = *pValue;
			break;
		case 0x89:
			TMOD = *pValue;
			break;
		case 0x8a:
			TL0 = *pValue;
			break;
		case 0x8b:
			TL1 = *pValue;
			break;
		case 0x8c:
			TH0 = *pValue;
			break;
		case 0x8d:
			TH1 = *pValue;
			break;
		case 0x8e:
			CKCON = *pValue;
			break;
		case 0x90:
			P1 = *pValue;
			break;
		case 0x91:
			EIF = *pValue;
			break;
		case 0x92:
			WTST = *pValue;
			break;
		case 0x93:
			DPX0 = *pValue;
			break;
		case 0x95:
			DPX1 = *pValue;
			break;
		case 0x98:
			SCON0 = *pValue;
			break;
	   	case 0x99:
			SBUF0 = *pValue;
			break;
		case 0x9d:
			ACON = *pValue;
			break;
	    case 0xa0:
			P2 = *pValue;
			break;
		case 0xa1:
			PMALR = *pValue;
			break;
	    case 0xa2:
			PMAHR = *pValue;
			break;
		case 0xa3:
			PMDR0 = *pValue;
			break;
		case 0xa4:
			PMDR1 = *pValue;
			break;
		case 0xa5:
			PMDR2 = *pValue;
			break;
		case 0xa6:
			PMDR3 = *pValue;
			break;
		case 0xa7:
			PBIER = *pValue;
			break;
		case 0xa8:
			IE = *pValue;
			break;
		case 0xa9:
			RHPLR = *pValue;
			break;
		case 0xaa:
			RHPHR = *pValue;
			break;
		case 0xab:
			RSPLR = *pValue;
			break;
		case 0xac:
			RSPHR = *pValue;
			break;
		case 0xae:
			PBSR = *pValue;
			break;
		case 0xaf:
			PBISR = *pValue;
			break;
		case 0xb0:
			P3 = *pValue;
			break;
		case 0xb1:
			MTWPL = *pValue;
			break;
		case 0xb2:
			MTWPH = *pValue;
			break;
		case 0xb3:
			CTRPL = *pValue;
			break;
		case 0xb4:
			CTRPH = *pValue;
			break;
		case 0xb5:
			TXBS = *pValue;
			break;
		case 0xb8:
			IP = *pValue;
			break;
		case 0xb9:
			MRPLL = *pValue;
			break;
		case 0xba:
			MRPLH = *pValue;
			break;
		case 0xbb:
			MRBLL = *pValue;
			break;
		case 0xbc:
			MRBLH = *pValue;
			break;
		case 0xbd:
			MRBSAL = *pValue;
			break;
		case 0xbe:
			MRBSAH = *pValue;
			break;
		case 0xbf:
			MRCR = *pValue;
			break;
		case 0xc0:
			SCON1 = *pValue;
			break;
		case 0xc1:
			SBUF1 = *pValue;
			break;
		case 0xc2:
			MISR = *pValue;
			break;
		case 0xc3:
			MIMR = *pValue;
			break;
		case 0xc8:
			T2CON = *pValue;
			break;
		case 0xc9:
			T2IF = *pValue;
			break;
		case 0xca:
			RLDL = *pValue;
			break;
		case 0xcb:
			RLDH = *pValue;
			break;
		case 0xcc:
			TL2 = *pValue;
			break;
		case 0xcd:
			TH2 = *pValue;
			break;
		case 0xce:
			WI2CCIR = *pValue;
			break;
		case 0xcf:
			WI2CDR = *pValue;
			break;
		case 0xd0:
			PSW = *pValue;
			break;
		case 0xd8:
			WDCON = *pValue;
			break;
		case 0xd9:
			SINTMR = *pValue;
			break;
		case 0xda:
			HASHCR = *pValue;
			break;
		case 0xdb:
			HASHDR = *pValue;
			break;
		case 0xe0:
			ACC = *pValue;
			break;
		case 0xe1:
			SINTSR = *pValue;
			break;
		case 0xe2:
			SSCIR = *pValue;
			break;
		case 0xe3:
			SSDR = *pValue;
			break;
		case 0xe4:
			MBCIR = *pValue;
			break;
		case 0xe5:
			MBDR = *pValue;
			break;
		case 0xe6:
			PBCIR = *pValue;
			break;
		case 0xe7:
			PBDR = *pValue;
			break;
		case 0xe8:
			EIE = *pValue;
			break;
		case 0xe9:
			STATUS = *pValue;
			break;
		case 0xea:
			MXAX = *pValue;
			break;
		case 0xeb:
			TA = *pValue;
			break;
		case 0xee:
			RNO = *pValue;
			break;
		case 0xef:
			RNDP = *pValue;
			break;
		case 0xf0:
			B = *pValue;
			break;
		case 0xf1:
			RC0 = *pValue;
			break;
		case 0xf2:
			RC1 = *pValue;
			break;
		case 0xf3:
			RC2 = *pValue;
			break;
		case 0xf4:
			RC3 = *pValue;
			break;
		case 0xf5:
			RC4 = *pValue;
			break;
		case 0xf6:
			MOP = *pValue;
			break;
		case 0xf7:
			MD = *pValue;
			break;
		case 0xf8:
			EIP = *pValue;
			break;
		case 0xf9:
			WC0 = *pValue;
			break;
		case 0xfa:
			WC1 = *pValue;
			break;
		case 0xfb:
			WC2 = *pValue;
			break;
		case 0xfc:
			WC3 = *pValue;
			break;
		case 0xfd:
			WC4 = *pValue;
			break;
		case 0xfe:
			CCTRL = *pValue;
			break;
		case 0xff:
			CSSR = *pValue;
			break;
		default:
			return IO_COMMAND_ERROR;
		}
	}
	else // command error
		return IO_COMMAND_ERROR;

	return IO_COMMAND_DONE;

}	/* End of io_SfrIO */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SfrAckRW
 * Purpose: To reply a writing acknowedgement or the SFR value to MCPU
 * Params: U8_T type - 0: read 1: write
 *         U8_T value - The value of specific SFR
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SfrAckRW(U8_T type, U8_T value)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_DBG_WCPU_SFR_WRITE_ACK, NULL, 0);
	}
	else
	{
		io_TxBuf[0] = value;

		MWIF_Send(IO_DBG_WCPU_SFR_READ_RESP, io_TxBuf, 1);
	}
}	/* End of io_SfrRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WcpuIndirectAckRW
 * Purpose: To reply a writing acknowledgement or the WCPU system register value to MCPU
 * Params: U8_T  type - 1: write command 0: read command
 *         U8_T  len - The buffer length of WCPU system register
 *         U8_T* pReg - The start pointer of the buffer to save WCPU system register value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_WcpuIndirectAckRW(U8_T type, U8_T len, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_DBG_WCPU_SUB_SYS_REG_WRITE_RESP, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pReg, len);

		MWIF_Send(IO_DBG_WCPU_SUB_SYS_REG_READ_RESP, io_TxBuf, len);
	}
}	/* End of io_WcpuIndirectAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_MiibIndirectAckRW
 * Purpose: To reply a writing acknowledgement or the MII bridge register value to MCPU
 * Params: U8_T  type - 1: write command 0: read command
 *         U8_T  len - The buffer length of MII bridge register
 *         U8_T* pReg - The start pointer of the buffer to save MII bridge register value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_MiibIndirectAckRW(U8_T type, U8_T len, U8_T XDATA* pReg)
{
	if (type== IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_DBG_WCPU_MIIB_REG_WRITE_ACK, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pReg, len);

		MWIF_Send(IO_DBG_WCPU_MIIB_REG_READ_RESP, io_TxBuf, len);
	}
}	/* End of io_MiibIndirectAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_PcibIndirectRegAckRW
 * Purpose: To reply a writing acknowledgement or the PCI bridge register value to MCPU
 * Params: U8_T  type - 1: write command 0: read command
 *         U8_T  len - The buffer length of PCI bridge register
 *         U8_T* pReg - The start pointer of the buffer to save PCI bridge register value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_PcibIndirectRegAckRW(U8_T type, U8_T len, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_DBG_WCPU_PCIB_REG_WRITE_ACK, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pReg, len);

		MWIF_Send(IO_DBG_WCPU_PCIB_REG_READ_RESP, io_TxBuf, len);
	}
}	/* End of io_PcibIndirectRegAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_PciConfigRW
 * Purpose: To reply a writing acknowedgement or the PCI configuration value to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T* pValue - Start pointer of the butter to save PCI configuration value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_PciConfigAckRW(U8_T type, U8_T XDATA* pReg)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_DBG_PCI_CONFIGURATION_WRITE_ACK, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pReg, 4);

		MWIF_Send(IO_DBG_PCI_CONFIGURATION_READ_RESP, io_TxBuf, 4);
	}
}	/* End of io_PciConfigRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WiFiRegRW
 * Purpose: To reply a writing acknowedgement or the WiFi register value to MCPU
 * Params: U8_T  type - 0: read 1: write
 *         U8_T* pValue - Start pointer of the buffer to save register value
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_WiFiRegAckRW(U8_T type, U8_T XDATA* pValue)
{
	if (type == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_DBG_WIFI_REG_WRITE_ACK, NULL, 0);
	}
	else
	{
		memcpy(&io_TxBuf[0], pValue, 4);

		MWIF_Send(IO_DBG_WIFI_REG_READ_RESP, io_TxBuf, 4);
	}
}	/* End of io_WiFiRegRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WlanPktFilterAckRW
 * Purpose: To reply a writing acknowledgement or the WiFi packet filtering result to MCPU
 * Params: U8_T  rw - 1: write command 0: read command
 *         U8_T  type - the type of WiFi packet
 *         U8_T  subtype - The subtype of WiFi packet
 *         U8_T* pBssid - The start pointer of the buffer to save BSSID
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_WlanPktFilterAckRW(U8_T rw, U8_T type, U16_T subtype, U8_T XDATA* pBssid)
{
	if (rw == IO_COMMAND_WRITE)
	{
		MWIF_Send(IO_DBG_WIFI_PACKET_FILTERING_WRITE_ACK, NULL, 0);
	}
	else
	{
		io_TxBuf[0] = type;
		io_TxBuf[1] = *((U8_T XDATA *)&subtype);
		io_TxBuf[2] = (U8_T)subtype;
		io_TxBuf[3] = pBssid[0];
		io_TxBuf[4] = pBssid[1];
		io_TxBuf[5] = pBssid[2];
		io_TxBuf[6] = pBssid[3];
		io_TxBuf[7] = pBssid[4];
		io_TxBuf[8] = pBssid[5];

		MWIF_Send(IO_DBG_WIFI_PACKET_FILTERING_READ_RESP, io_TxBuf, 9);
	}

}	/* End of io_WlanPktFilterAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_SynthesizerSetAck
 * Purpose: To reply a acknowedgement to MCPU
 * Params: none
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_SynthesizerSetAck(void)
{
	MWIF_Send(IO_DBG_SYNTHESIZER_SET_ACK, NULL, 0);
	
}	/* End of io_SynthesizerSetAck */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_RetryAckRW
 * Purpose: To reply a writing acknowledgement or the retry count to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - WiFi packet retry count
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_RetryAckRW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_DBG_RETRY_NUMBER_RESP, io_TxBuf, len);
	
}	/* End of io_RetryAckRW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_Gpio0RW
 * Purpose: To reply a writing acknowledgement or the P0 value to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - value of P0
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_Gpio0RW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_GPIO_ZERO_RESP, io_TxBuf, len);
	
}	/* End of io_Gpio0RW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_Gpio1RW
 * Purpose: To reply a writing acknowledgement or the P1 value to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - value of P1
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_Gpio1RW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_GPIO_ONE_RESP, io_TxBuf, len);
	
}	/* End of io_Gpio1RW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_Gpio2RW
 * Purpose: To reply a writing acknowledgement or the P2 value to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - value of P2
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_Gpio2RW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_GPIO_TWO_RESP, io_TxBuf, len);
	
}	/* End of io_Gpio2RW */

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_Gpio3RW
 * Purpose: To reply a writing acknowledgement or the P3 value to MCPU
 * Params: U8_T type - 1: write command 0: read command
 *         U8_T value - value of P3
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_Gpio3RW(U8_T type, U8_T value)
{
	U8_T	len;

	if (type == IO_COMMAND_WRITE)
	{
		len = 1;
		io_TxBuf[0] = IO_COMMAND_WRITE;
	}
	else
	{
		len = 2;
		io_TxBuf[0] = IO_COMMAND_READ;
		io_TxBuf[1] = value;
	}

	MWIF_Send(IO_GPIO_THREE_RESP, io_TxBuf, len);
	
}	/* End of io_Gpio3RW */

#if ALLEN_DBG
void IO_WlanDebug1(U8_T value)
{
	MWIF_Send(IO_WLAN_DEBUG1, &value, 1);

} /* End of IO_WlanDebug1 */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: io_WifiRegIO
 * Purpose: To read or to write the value into specific WiFi register
 * Params: U8_T  rw - 0: read 1: write
 *         U8_T  index - The index of the specific WiFi register
 *         U8_T* pValue - Start pointer of the WiFi value buffer
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
U8_T io_WifiRegIO(U8_T rw, U16_T index, U32_T XDATA* pValue)
{
	if ((index > 0xeff) || (index % 4))
		return IO_COMMAND_ERROR;

	if (index < 0x400)
	{
		LockPhyReg();
	}

	if (rw == 0)
	{
		*pValue = ZD_Readl(index);
	}
	else if (rw == 1)
	{
		ZD_Writel(*pValue, index);
	}
	else
	{
		if (index < 0x400)
		{
			UnLockPhyReg();
		}
		return IO_COMMAND_ERROR;
	}

	if (index < 0x400)
	{
		UnLockPhyReg();
	}
	return IO_COMMAND_DONE;

} /* End of dbg_WifiRegIO */

/*******************************************************************/
#endif	/**************** End of IO_CPU_TYPE macro *****************/

#if IO_SHOWCMD
/*
 * ----------------------------------------------------------------------------
 * Function Name: io_ShowCmd
 * Purpose: To list the contents of the MCPU/WCPU communication command 
 * Params: U8_T  TxRx - 1: To send command to the other CPU
 *                      0: To receive command from the other CPU
 *         U8_T* pReg - Start pointer of the buffer to save the command
 * Returns: none
 * Note: none
 * ----------------------------------------------------------------------------
 */
void io_ShowCmd(U8_T TxRx, U8_T XDATA* pReg)
{
	U8_T i, len;


	len = pReg[2] + 3;
	printd ("\n\r*------------------------------------*\n\r");
	if (TxRx)
		printd ("| Sent IO Cmd.. ==>\n\r");
	else
		printd ("| Received IO Cmd.. <==\n\r");
	printd ("| Len: %bd\n\r", len);
	printd ("| header: %02bx %02bx %02bx\n\r", pReg[0], pReg[1], pReg[2]);
	printd ("| payload:");
	for (i = 3; i < len; i++)
	{
		printd(" %02bx", pReg[i]);
		if (!((i + 1) % 16))
			printd("\n\r|         ");
	}
	printd ("\n\r*------------------------------------*\n\r");
}
#endif
