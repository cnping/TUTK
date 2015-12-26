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
 * Module Name: gconfig.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gconfig.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "adapter.h"
#include "gconfig.h"
#include "gs2w.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart0.h"
#include "ax22000.h"
#include "mcpu.h"
#include "printd.h"
#include "hsuart.h"
#include "hsuart2.h"
#include "filesys.h"
#include "mwioctl.h"
#include <absacc.h>
#include <string.h>

#include "main.h"
#if (INCLUDE_DNS_CLIENT)
#include "dnsc.h"
#endif
/* NAMING CONSTANT DECLARATIONS */
#define DEVICE_STATUS_IDLE			0
#define DEVICE_STATUS_CONNECTED		1

/* MACRO DECLARATIONS */

/* TYPE DECLARATIONS */
typedef U8_T (*pFlashParaData)(U8_T * addr, U16_T len);

/* GLOBAL VARIABLES DECLARATIONS */
GCONFIG_GID XDATA GCONFIG_Gid = {'A','S','I','X','X','I','S','A'};
static const U8_T default_Wep64Key[4][5] = {0x12,0x34,0x56,0x78,0x90, 0x09,0x87,0x65,0x43,0x21,
                                            0xA1,0xB2,0xC3,0xD4,0xE5, 0x01,0x23,0x45,0x67,0x89};
static const U8_T default_Wep128Key[4][13] = {
0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34};

/* LOCAL VARIABLES DECLARATIONS */
U8_T XDATA gconfig_SysClock;
GCONFIG_CFG_DATA XDATA gconfig_ConfigData;
U8_T IDATA FirmwareUpgradeFlag[4] _at_ 0x31;
static GCONFIG_MAC_ADDR macAddr;

/* LOCAL SUBPROGRAM DECLARATIONS */
static BOOL	gconfig_ReStoreParameter(U32_T addr, GCONFIG_CFG_DATA *pConfig, U16_T len);
static BOOL gconfig_StoreParameter(GCONFIG_CFG_DATA *pSramBase, U16_T len);
static U16_T gconfig_Checksum(U16_T *pBuf, U32_T length);
static void	gconfig_DisplayConfigData(GCONFIG_CFG_DATA *pConfig);

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_Task
 * Purpose: Main function
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_Task(void)
{

} /* End of GCONFIG_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_Init
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_Init(void)
{
	U16_T portCfg;
	U16_T baudRateDiv;
	U8_T  dataStopParityBits = 0;
	U16_T val;
	U8_T  flowCtrl, rs485;
	
	// Restore last-saved configuration if applicable	
	GS2W_SetTaskState(GS2W_STATE_IDLE);
    memcpy((char *)&macAddr, (char *)MAC_GetMacAddr(), MAC_ADDRESS_LEN);
	GCONFIG_ReadConfigData();
    
	// UART 2 parameters setting
	portCfg = GCONFIG_GetSerialPort();
	
	gconfig_SysClock = MCPU_GetSysClk();
	
	val = (portCfg >> 11) & 0x001F;
 
	baudRateDiv = GCONFIG_GetBaudRate(val);

	val = (portCfg >> 8) & 0x0007; 
	if (val == 0) 		dataStopParityBits |= HSLCR_CHAR_5;
	else if (val == 1) 	dataStopParityBits |= HSLCR_CHAR_6;
	else if (val == 2) 	dataStopParityBits |= HSLCR_CHAR_7;
	else 				dataStopParityBits |= HSLCR_CHAR_8;

	val = (portCfg >> 5) & 0x0007; 
	if (val == 0 || val == 1) dataStopParityBits |= HSLCR_PE_ENB;
	if (val == 1) 			  dataStopParityBits |= HSLCR_EPS_EVEN;

	val = portCfg & 0x0003; 
	if (val == 0) 	dataStopParityBits |= HSLCR_STOP_10;
	else 			dataStopParityBits |= HSLCR_STOP_15;
	
	val = (portCfg >> 2) & 0x0007; 
	if (val == 0) 		flowCtrl = UR2_FLOW_CTRL_X;
	else if (val == 1) 	flowCtrl = UR2_FLOW_CTRL_HW;
	else 				flowCtrl = UR2_FLOW_CTRL_NO;

    rs485 = (HSMCR_DTR | HSMCR_RTS);
    val = GCONFIG_GetRs485Mode();

    if (val == 1)
        rs485 |= UR2_RS485_RECEIVE;
    else if (val == 2)
        rs485 |= UR2_RS485_DRIVE;
    else if (val == 3)
        rs485 |= (UR2_RS485_DRIVE | UR2_RS485_RECEIVE);

	HSUR2_Setup((baudRateDiv),
				(dataStopParityBits),
				(HSIER_RDI_ENB|HSIER_RLSI_ENB|HSIER_RBRFI_ENB|HSIER_DOEI_ENB|HSIER_FCCRI_ENB|HSIER_TDCI_ENB),
				(HSFCR_HSUART_ENB|HSFCR_FIFOE|HSFCR_RFR|HSFCR_TFR|HSFCR_TRIG_08),
				(rs485));
 
	/* Config the Flow Control Mode */
	if (flowCtrl == UR2_FLOW_CTRL_HW)
		HSUR2_AutoCtsRtsFlowCtrl(1);
    else if (flowCtrl == UR2_FLOW_CTRL_X)
		HSUR2_AutoSoftwareFlowCtrl(1, (HSAFCSR_ASFCE_ENB), ASCII_XON, ASCII_XOFF, 0);
    else
    {
		HSUR2_AutoCtsRtsFlowCtrl(0);
        HSUR2_AutoSoftwareFlowCtrl(0, 0, 0, 0, 0);
    }

} /* End of GCONFIG_Init() */

/////////////////////////////////////////////////////////////////////////////////

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetConfigPacket
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetConfigPacket(GCONFIG_CFG_PKT* pCfgPkt)
{
	gconfig_ConfigData.Option = pCfgPkt->Option;
 	gconfig_ConfigData.DevName = pCfgPkt->DevName;
	gconfig_ConfigData.Network = pCfgPkt->Network;
	gconfig_ConfigData.ServerStaticIP = pCfgPkt->ServerStaticIP;
	gconfig_ConfigData.ServerDataPktListenPort = pCfgPkt->ServerDataPktListenPort;
	gconfig_ConfigData.ServerMulticastListenPort = pCfgPkt->ServerMulticastListenPort;
	gconfig_ConfigData.ServerBroadcastListenPort = pCfgPkt->ServerBroadcastListenPort;
	gconfig_ConfigData.ClientDestIP = pCfgPkt->ClientDestIP;
	gconfig_ConfigData.ClientDestPort = pCfgPkt->ClientDestPort; 
	gconfig_ConfigData.Netmask = pCfgPkt->Netmask;
	gconfig_ConfigData.Gateway = pCfgPkt->Gateway;
	gconfig_ConfigData.Dns = pCfgPkt->Dns;
	gconfig_ConfigData.SerialPort = pCfgPkt->SerialPort;		
	gconfig_ConfigData.EthernetTxTimer = pCfgPkt->EthernetTxTimer;

    gconfig_ConfigData.Rs485Mode = pCfgPkt->Rs485Mode;
    memcpy((char *)&gconfig_ConfigData.AccessibleIP[0], (char *)&pCfgPkt->AccessibleIP[0], (GCONFIG_ACCESSIBLE_IP_NUM*4));
    memcpy((char *)&gconfig_ConfigData.Smtp, (char *)&pCfgPkt->Smtp, sizeof(GCONFIG_SMTP_CONFIG));
    memcpy((char *)gconfig_ConfigData.DestHostName, (char *)pCfgPkt->DestHostName, 36);
 	
    gconfig_ConfigData.UpgradeCfg.TftpServerIpForMcpu = pCfgPkt->TftpServerIp;
    memcpy((char *)gconfig_ConfigData.UpgradeCfg.FileNameMcpu, (char *)pCfgPkt->Filename, 63);
    gconfig_ConfigData.UpgradeCfg.FileNameMcpu[63] = 0;
    gconfig_ConfigData.UpgradeCfg.FileNameLenMcpu = strlen(gconfig_ConfigData.UpgradeCfg.FileNameMcpu);
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
    gconfig_ConfigData.WifiConfig.WifiBasebandMode = pCfgPkt->WifiConfig.WifiBasebandMode;
    gconfig_ConfigData.WifiConfig.WifiNetworkMode = pCfgPkt->WifiConfig.WifiNetworkMode;
    gconfig_ConfigData.WifiConfig.WifiChannel = pCfgPkt->WifiConfig.WifiChannel;
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiSsid, (char *)pCfgPkt->WifiConfig.WifiSsid, 32);
    gconfig_ConfigData.WifiConfig.WifiSsidLen = pCfgPkt->WifiConfig.WifiSsidLen;
    gconfig_ConfigData.WifiConfig.WifiTxRate = pCfgPkt->WifiConfig.WifiTxRate;
    gconfig_ConfigData.WifiConfig.WifiTxPowerLevel = pCfgPkt->WifiConfig.WifiTxPowerLevel;
    gconfig_ConfigData.WifiConfig.WifiPreambleMode = pCfgPkt->WifiConfig.WifiPreambleMode;
    gconfig_ConfigData.WifiConfig.WifiBcnInterval = pCfgPkt->WifiConfig.WifiBcnInterval;
    gconfig_ConfigData.WifiConfig.WifiRtsThreshold = pCfgPkt->WifiConfig.WifiRtsThreshold;
    gconfig_ConfigData.WifiConfig.WifiAutoPowerCtrl = pCfgPkt->WifiConfig.WifiAutoPowerCtrl;
    gconfig_ConfigData.WifiConfig.WifiEncryptMode = pCfgPkt->WifiConfig.WifiEncryptMode;
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiWep64Key[0], (char *)pCfgPkt->WifiConfig.WifiWep64Key[0],  5);
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiWep64Key[1], (char *)pCfgPkt->WifiConfig.WifiWep64Key[1], 5);
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiWep64Key[2], (char *)pCfgPkt->WifiConfig.WifiWep64Key[2], 5);
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiWep64Key[3], (char *)pCfgPkt->WifiConfig.WifiWep64Key[3], 5);
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiWep128Key[0], (char *)pCfgPkt->WifiConfig.WifiWep128Key[0], 13);
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiWep128Key[1], (char *)pCfgPkt->WifiConfig.WifiWep128Key[1], 13);
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiWep128Key[2], (char *)pCfgPkt->WifiConfig.WifiWep128Key[2], 13);
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiWep128Key[3], (char *)pCfgPkt->WifiConfig.WifiWep128Key[3], 13);
    gconfig_ConfigData.WifiConfig.WifiWepKeyIndex = pCfgPkt->WifiConfig.WifiWepKeyIndex;
    gconfig_ConfigData.WifiConfig.WifiWepKeyLength = pCfgPkt->WifiConfig.WifiWepKeyLength;
    memcpy((char *)gconfig_ConfigData.WifiConfig.WifiPreShareKey, (char *)pCfgPkt->WifiConfig.WifiPreShareKey, 64);
    gconfig_ConfigData.WifiConfig.WifiPreShareKeyLen = pCfgPkt->WifiConfig.WifiPreShareKeyLen;

#endif
    gconfig_ConfigData.DhcpSrv.IpPoolAddrStart = pCfgPkt->DhcpSrv.IpPoolAddrStart;
    gconfig_ConfigData.DhcpSrv.IpPoolAddrEnd = pCfgPkt->DhcpSrv.IpPoolAddrEnd;
    gconfig_ConfigData.DhcpSrv.Netmask = pCfgPkt->DhcpSrv.Netmask;
    gconfig_ConfigData.DhcpSrv.DefaultGateway = pCfgPkt->DhcpSrv.DefaultGateway;
    gconfig_ConfigData.DhcpSrv.LeaseTime = pCfgPkt->DhcpSrv.LeaseTime;
    gconfig_ConfigData.DhcpSrv.Status = pCfgPkt->DhcpSrv.Status;

	GCONFIG_WriteConfigData();
} /* End of GCONFIG_SetConfigPacket() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetConfigPacket
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_GetConfigPacket(GCONFIG_CFG_PKT* pCfgPkt)
{
    memcpy((char *)&pCfgPkt->Gid, (char *)&GCONFIG_Gid, sizeof(GCONFIG_Gid));
	pCfgPkt->Option = gconfig_ConfigData.Option;
    pCfgPkt->DevName = gconfig_ConfigData.DevName;
    pCfgPkt->MacAddr = macAddr;
	pCfgPkt->Network = gconfig_ConfigData.Network;
    pCfgPkt->ServerDynamicIP = STOE_GetIPAddr();
	pCfgPkt->ServerStaticIP = gconfig_ConfigData.ServerStaticIP;
	pCfgPkt->ServerDataPktListenPort = gconfig_ConfigData.ServerDataPktListenPort;
	pCfgPkt->ServerMulticastListenPort = gconfig_ConfigData.ServerMulticastListenPort;
	pCfgPkt->ServerBroadcastListenPort = gconfig_ConfigData.ServerBroadcastListenPort;
	pCfgPkt->ClientDestIP = gconfig_ConfigData.ClientDestIP;
	pCfgPkt->ClientDestPort = gconfig_ConfigData.ClientDestPort; 
	pCfgPkt->Netmask = gconfig_ConfigData.Netmask;
	pCfgPkt->Gateway = gconfig_ConfigData.Gateway;
	pCfgPkt->Dns = gconfig_ConfigData.Dns;
	pCfgPkt->SerialPort = gconfig_ConfigData.SerialPort;		
	pCfgPkt->EthernetTxTimer = gconfig_ConfigData.EthernetTxTimer;

    pCfgPkt->Rs485Mode = gconfig_ConfigData.Rs485Mode;
    memcpy((char *)&pCfgPkt->AccessibleIP[0], (char *)&gconfig_ConfigData.AccessibleIP[0], (GCONFIG_ACCESSIBLE_IP_NUM*4));
    memcpy((char *)&pCfgPkt->Smtp, (char *)&gconfig_ConfigData.Smtp, sizeof(GCONFIG_SMTP_CONFIG));
    memcpy((char *)pCfgPkt->DestHostName, (char *)gconfig_ConfigData.DestHostName, 36);
 	
    pCfgPkt->TftpServerIp = gconfig_ConfigData.UpgradeCfg.TftpServerIpForMcpu;
    memcpy((char *)&pCfgPkt->Filename[0], (char *)&gconfig_ConfigData.UpgradeCfg.FileNameMcpu[0], 64);

	if (GS2W_GetTaskState() == GS2W_STATE_IDLE)
		pCfgPkt->DeviceStatus = DEVICE_STATUS_IDLE;
	else
		pCfgPkt->DeviceStatus = DEVICE_STATUS_CONNECTED;

#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
    pCfgPkt->WifiConfig.WifiBasebandMode = gconfig_ConfigData.WifiConfig.WifiBasebandMode;
    pCfgPkt->WifiConfig.WifiNetworkMode = gconfig_ConfigData.WifiConfig.WifiNetworkMode;
    pCfgPkt->WifiConfig.WifiChannel = gconfig_ConfigData.WifiConfig.WifiChannel;
    memcpy((char *)pCfgPkt->WifiConfig.WifiSsid, (char *)gconfig_ConfigData.WifiConfig.WifiSsid, 32);
    pCfgPkt->WifiConfig.WifiSsidLen = gconfig_ConfigData.WifiConfig.WifiSsidLen;
    pCfgPkt->WifiConfig.WifiTxRate = gconfig_ConfigData.WifiConfig.WifiTxRate;
    pCfgPkt->WifiConfig.WifiTxPowerLevel = gconfig_ConfigData.WifiConfig.WifiTxPowerLevel;
    pCfgPkt->WifiConfig.WifiPreambleMode = gconfig_ConfigData.WifiConfig.WifiPreambleMode;
    pCfgPkt->WifiConfig.WifiBcnInterval = gconfig_ConfigData.WifiConfig.WifiBcnInterval;
    pCfgPkt->WifiConfig.WifiRtsThreshold = gconfig_ConfigData.WifiConfig.WifiRtsThreshold;
    pCfgPkt->WifiConfig.WifiAutoPowerCtrl = gconfig_ConfigData.WifiConfig.WifiAutoPowerCtrl;
    pCfgPkt->WifiConfig.WifiEncryptMode = gconfig_ConfigData.WifiConfig.WifiEncryptMode;
    memcpy((char *)pCfgPkt->WifiConfig.WifiWep64Key[0], (char *)gconfig_ConfigData.WifiConfig.WifiWep64Key[0], 5);
    memcpy((char *)pCfgPkt->WifiConfig.WifiWep64Key[1], (char *)gconfig_ConfigData.WifiConfig.WifiWep64Key[1], 5);
    memcpy((char *)pCfgPkt->WifiConfig.WifiWep64Key[2], (char *)gconfig_ConfigData.WifiConfig.WifiWep64Key[2], 5);
    memcpy((char *)pCfgPkt->WifiConfig.WifiWep64Key[3], (char *)gconfig_ConfigData.WifiConfig.WifiWep64Key[3], 5);
    memcpy((char *)pCfgPkt->WifiConfig.WifiWep128Key[0], (char *)gconfig_ConfigData.WifiConfig.WifiWep128Key[0], 13);
    memcpy((char *)pCfgPkt->WifiConfig.WifiWep128Key[1], (char *)gconfig_ConfigData.WifiConfig.WifiWep128Key[1], 13);
    memcpy((char *)pCfgPkt->WifiConfig.WifiWep128Key[2], (char *)gconfig_ConfigData.WifiConfig.WifiWep128Key[2], 13);
    memcpy((char *)pCfgPkt->WifiConfig.WifiWep128Key[3], (char *)gconfig_ConfigData.WifiConfig.WifiWep128Key[3], 13);
    pCfgPkt->WifiConfig.WifiWepKeyIndex = gconfig_ConfigData.WifiConfig.WifiWepKeyIndex;
    pCfgPkt->WifiConfig.WifiWepKeyLength = gconfig_ConfigData.WifiConfig.WifiWepKeyLength;
    memcpy((char *)pCfgPkt->WifiConfig.WifiPreShareKey, (char *)gconfig_ConfigData.WifiConfig.WifiPreShareKey, 64);
    pCfgPkt->WifiConfig.WifiPreShareKeyLen = gconfig_ConfigData.WifiConfig.WifiPreShareKeyLen;

#endif
    pCfgPkt->DhcpSrv.IpPoolAddrStart = gconfig_ConfigData.DhcpSrv.IpPoolAddrStart;
    pCfgPkt->DhcpSrv.IpPoolAddrEnd = gconfig_ConfigData.DhcpSrv.IpPoolAddrEnd;
    pCfgPkt->DhcpSrv.Netmask = gconfig_ConfigData.DhcpSrv.Netmask;
    pCfgPkt->DhcpSrv.DefaultGateway = gconfig_ConfigData.DhcpSrv.DefaultGateway;
    pCfgPkt->DhcpSrv.LeaseTime = gconfig_ConfigData.DhcpSrv.LeaseTime;
    pCfgPkt->DhcpSrv.Status = gconfig_ConfigData.DhcpSrv.Status;

} /* End of GCONFIG_GetConfigPacket() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_ReadConfigData
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_ReadConfigData(void)
{
#ifdef RuntimeCodeAt32KH
	BOOL bRet;

	printd("Read configuration data");
	bRet = gconfig_ReStoreParameter(0, &gconfig_ConfigData, sizeof(gconfig_ConfigData));

	if (bRet == FALSE || (gconfig_ConfigData.ServerStaticIP & 0x000000FF) == 0)
	{
		GCONFIG_ReadDefaultConfigData();
		GCONFIG_WriteConfigData();
		gconfig_DisplayConfigData(&gconfig_ConfigData);
	}

#else
		GCONFIG_ReadDefaultConfigData();
		gconfig_DisplayConfigData(&gconfig_ConfigData);
#endif

} /* End of GCONFIG_ReadConfigData() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_WriteConfigData
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_WriteConfigData(void)
{
#ifdef RuntimeCodeAt32KH
	gconfig_StoreParameter(&gconfig_ConfigData, (U16_T)sizeof(gconfig_ConfigData));
#endif	
} /* End of GCONFIG_WriteConfigData() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_ReadDefaultConfigData
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_ReadDefaultConfigData(void)
{
    U8_T i;

	printd("Read default configuration data. \n\r");

	gconfig_ConfigData.Signature[0] = 'A';
	gconfig_ConfigData.Signature[1] = 'S';
	gconfig_ConfigData.Signature[2] = 'I';
	gconfig_ConfigData.Signature[3] = 'X';
    memset((char *)&gconfig_ConfigData.UpgradeCfg, 0, sizeof(GCONFIG_UPGRADE));

#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
    gconfig_ConfigData.WifiConfig.WifiBasebandMode = 1;       /* mixed mode, 802.11b+g mode, 802.11a mode */
    gconfig_ConfigData.WifiConfig.WifiNetworkMode = 1;        /* ad hoc mode, infrastructure mode */
    gconfig_ConfigData.WifiConfig.WifiChannel = 11;
    strcpy(gconfig_ConfigData.WifiConfig.WifiSsid, "R2WiFi");
    gconfig_ConfigData.WifiConfig.WifiSsidLen = 6;
    gconfig_ConfigData.WifiConfig.WifiTxRate = 0;             /* auto, 1, 2, .... 48, 54M */
    gconfig_ConfigData.WifiConfig.WifiTxPowerLevel = 0;
    gconfig_ConfigData.WifiConfig.WifiPreambleMode = 1;       /* auto, long preamble, short preamble */
    gconfig_ConfigData.WifiConfig.WifiBcnInterval = 100;      /* beacon interval, only for IBSS mode */
    gconfig_ConfigData.WifiConfig.WifiRtsThreshold = 2432;
    gconfig_ConfigData.WifiConfig.WifiAutoPowerCtrl = 1;      /* auto power control enable or disable*/
    gconfig_ConfigData.WifiConfig.WifiEncryptMode = 0;        /* 0:disable, 1:wep64, 2:wep128, 3:tkip, 4:aes */
    gconfig_ConfigData.WifiConfig.WifiWepKeyIndex = 0;
    gconfig_ConfigData.WifiConfig.WifiWepKeyLength = 0;       /* wep key length, 0:64 bits, 1:128 bits*/

    memcpy((char *)&gconfig_ConfigData.WifiConfig.WifiWep64Key[0][0], (char *)&default_Wep64Key[0][0], 5);
    memcpy((char *)&gconfig_ConfigData.WifiConfig.WifiWep64Key[1][0], (char *)&default_Wep64Key[1][0], 5);
    memcpy((char *)&gconfig_ConfigData.WifiConfig.WifiWep64Key[2][0], (char *)&default_Wep64Key[2][0], 5);
    memcpy((char *)&gconfig_ConfigData.WifiConfig.WifiWep64Key[3][0], (char *)&default_Wep64Key[3][0], 5);

    memcpy((char *)&gconfig_ConfigData.WifiConfig.WifiWep128Key[0][0], (char *)&default_Wep128Key[0][0], 13);
    memcpy((char *)&gconfig_ConfigData.WifiConfig.WifiWep128Key[1][0], (char *)&default_Wep128Key[1][0], 13);
    memcpy((char *)&gconfig_ConfigData.WifiConfig.WifiWep128Key[2][0], (char *)&default_Wep128Key[2][0], 13);
    memcpy((char *)&gconfig_ConfigData.WifiConfig.WifiWep128Key[3][0], (char *)&default_Wep128Key[3][0], 13);

    strcpy((char *)gconfig_ConfigData.WifiConfig.WifiPreShareKey,"12345678");
    gconfig_ConfigData.WifiConfig.WifiPreShareKeyLen = strlen(gconfig_ConfigData.WifiConfig.WifiPreShareKey);
#endif

	gconfig_ConfigData.Checksum = 0x00;
    gconfig_ConfigData.SwCfgLength = sizeof(gconfig_ConfigData);
	gconfig_ConfigData.UdpAutoConnectClientIp = 0x00000000;
	gconfig_ConfigData.UdpAutoConnectClientPort = 0x0000;

	gconfig_ConfigData.Option = 0;
 	memset(&gconfig_ConfigData.DevName, 0, sizeof(GCONFIG_DEV_NAME));

	gconfig_ConfigData.Network = (GCONFIG_NETWORK_SERVER | GCONFIG_NETWORK_DHCP_ENABLE |
                                  GCONFIG_NETWORK_PROTO_TCP | GCONFIG_NETWORK_PROTO_UDP_BCAST);

	gconfig_ConfigData.ServerStaticIP = 0xc0a80203;	// 192.168.2.3
	gconfig_ConfigData.ServerDataPktListenPort = GCONFIG_DEFAULT_DATA_PORT;
	gconfig_ConfigData.ServerMulticastListenPort = GCONFIG_UDP_MCAST_SERVER_PORT;
	gconfig_ConfigData.ServerBroadcastListenPort = GCONFIG_UDP_BCAST_SERVER_PORT;
	gconfig_ConfigData.ClientDestIP = 0xc0a80202; 	// 192.168.2.2
	gconfig_ConfigData.ClientDestPort = GCONFIG_DEFAULT_DATA_PORT; 
	gconfig_ConfigData.Netmask = 0xffffff00; 		// 255.255.255.0
	gconfig_ConfigData.Gateway = 0xc0a80201; 		// 192.168.2.1
	gconfig_ConfigData.Dns = 0x08080808; 			// 8.8.8.8
	//gconfig_ConfigData.Dns = 0xa85f0101; 			// 168.95.1.1

	gconfig_ConfigData.SerialPort = (GCONFIG_SPORT_BRATE_115200 |
	                   			   GCONFIG_SPORT_DATABIT_8 |
								   GCONFIG_SPORT_PARITY_NONE |
								   GCONFIG_SPORT_FLOWC_NONE |
								   GCONFIG_SPORT_STOPBIT_1
								  );

	gconfig_ConfigData.EthernetTxTimer = 100; // 100 ms

    strcpy(gconfig_ConfigData.Admin.Username, "admin");
    strcpy(gconfig_ConfigData.Admin.Passwd, "admin");
    gconfig_ConfigData.Admin.Level = 5;

    for (i = 0; i < GCONFIG_ACCESSIBLE_IP_NUM; i++)
    {
        gconfig_ConfigData.AccessibleIP[i] = 0;
    }

    strcpy(gconfig_ConfigData.Smtp.DomainName, "asix.com.tw");
    strcpy(gconfig_ConfigData.Smtp.FromAddr, "ds@asix.com.tw");
    strcpy(gconfig_ConfigData.Smtp.ToAddr1, "to1@asix.com.tw");	
    strcpy(gconfig_ConfigData.Smtp.ToAddr2, "to2@asix.com.tw");	
    strcpy(gconfig_ConfigData.Smtp.ToAddr3, "to3@asix.com.tw");
	
    gconfig_ConfigData.Smtp.EventEnableBits = 0x0000;

    gconfig_ConfigData.Rs485Mode = 0;

    strcpy(gconfig_ConfigData.DestHostName, "192.168.2.2"); /* 192.168.2.2 */

    /* DHCP server configuration */
    gconfig_ConfigData.DhcpSrv.IpPoolAddrStart = 0xc0a80204; /* 192.168.2.4  */
    gconfig_ConfigData.DhcpSrv.IpPoolAddrEnd = 0xc0a8020a;   /* 192.168.2.10 */
    gconfig_ConfigData.DhcpSrv.Netmask = 0xFFFFFF00;
    gconfig_ConfigData.DhcpSrv.DefaultGateway = 0x0;  /* 0.0.0.0  */
    gconfig_ConfigData.DhcpSrv.LeaseTime = 1440;
    gconfig_ConfigData.DhcpSrv.Status = 1;                   /* 0: disable  1: enable */

} /* End of GCONFIG_ReadDefaultConfigData */

/////////////////////////////////////////////////////////////////////////////////

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_EnableUdpAutoConnectAfterReboot
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_EnableUdpAutoConnectAfterReboot(void)
{
	if ((gconfig_ConfigData.Option & GCONFIG_OPTION_ENABLE_UDP_AUTO_CONNECT) 
	     == GCONFIG_OPTION_ENABLE_UDP_AUTO_CONNECT)
		return TRUE;
	
	return FALSE;
} /* End of GCONFIG_EnableUdpAutoConnectAfterReboot() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_EnableFirmwareUpgrade
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_EnableFirmwareUpgrade(void)
{
	FirmwareUpgradeFlag[0] = 'a';
	FirmwareUpgradeFlag[1] = 'x';
	FirmwareUpgradeFlag[2] = '_';
	FirmwareUpgradeFlag[3] = 'm';

    /* save the network configuration for boot loader */
    gconfig_ConfigData.UpgradeCfg.DeviceIp = STOE_GetIPAddr();
    gconfig_ConfigData.UpgradeCfg.DeviceNetmask = STOE_GetSubnetMask();
    gconfig_ConfigData.UpgradeCfg.DeviceGateway = STOE_GetGateway();

} /* End of GCONFIG_EnableFirmwareUpgrade() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_IpAddr2Ulong()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_IpAddr2Ulong(U8_T* pBuf, U8_T len)
{
	U32_T	ip = 0;
	U8_T*	point = (U8_T*)&ip;
	U8_T	count = 0;
	U8_T	count2 = 0;

	while (1)
	{
		count2 = 0;
		while ((*pBuf != '.') && len)
		{
			count2++;
			if (count2 > 3) return 0xffffffff;

			*point *= 10;
  
			if ((*pBuf < 0x3a) && (*pBuf > 0x2f))
				*point += (*pBuf - 0x30);
			else
				return 0xffffffff;

			pBuf++;
			len--;
		}
  
		if (len == 0) break;

		pBuf++;
		len--;
		count++;
		point++;

		if (count > 3) return 0xffffffff;
	}

	if (count != 3) return 0xffffffff;

	return ip;
} /* End of GCONFIG_IpAddr2Ulong() */
/////////////////////////////////////////////////////////////////////////////////
// Get Functions

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDeviceName
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDeviceName(GCONFIG_DEV_NAME* pDevName)
{
	memcpy(&gconfig_ConfigData.DevName, pDevName, sizeof(GCONFIG_DEV_NAME)); 
} /* End of GCONFIG_GetDeviceName() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetNetwork
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetNetwork(U16_T val)
{
	gconfig_ConfigData.Network = val;	
} /* End of GCONFIG_SetNetwork() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetServerStaticIP
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetServerStaticIP(U32_T ip)
{
	gconfig_ConfigData.ServerStaticIP = ip;	
} /* End of GCONFIG_SetServerStaticIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetServerDataPktListenPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetServerDataPktListenPort(U16_T port)
{
	gconfig_ConfigData.ServerDataPktListenPort = port;
} /* End of GCONFIG_SetServerDataPktListenPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetServerMulticastListenPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetServerMulticastListenPort(U16_T port)
{
	gconfig_ConfigData.ServerMulticastListenPort = port;
} /* End of GCONFIG_SetServerMulticastListenPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetServerBroadcastListenPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetServerBroadcastListenPort(U16_T port)
{
	gconfig_ConfigData.ServerBroadcastListenPort = port;
} /* End of GCONFIG_SetServerBroadcastListenPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetClientDestIP
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetClientDestIP(U32_T ip)
{
	gconfig_ConfigData.ClientDestIP = ip;
} /* End of GCONFIG_SetClientDestIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetClientDestPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetClientDestPort(U16_T port)
{
	gconfig_ConfigData.ClientDestPort = port;
} /* End of GCONFIG_SetClientDestPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetNetmask
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetNetmask(U32_T netmask)
{
	gconfig_ConfigData.Netmask = netmask;
} /* End of GCONFIG_SetNetmask() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetGateway
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetGateway(U32_T gateway)
{
	gconfig_ConfigData.Gateway = gateway;
} /* End of GCONFIG_SetGateway() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetDNS
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDNS(U32_T dns)
{
	gconfig_ConfigData.Dns = dns;
} /* End of GCONFIG_SetDNS() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetSerialPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetSerialPort(U16_T sPort)
{
	gconfig_ConfigData.SerialPort = sPort;
} /* End of GCONFIG_SetSerialPort() */


/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetEthernetTxTimer
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetEthernetTxTimer(U16_T timer)
{
	gconfig_ConfigData.EthernetTxTimer = timer;
} /* End of GCONFIG_SetEthernetTxTimer() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetUdpAutoConnectClientIp
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetUdpAutoConnectClientIp(U32_T ip)
{
	gconfig_ConfigData.UdpAutoConnectClientIp = ip;
} /* End of GCONFIG_SetUdpAutoConnectClientIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetUdpAutoConnectRemotePort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetUdpAutoConnectClientPort(U16_T port)
{
	gconfig_ConfigData.UdpAutoConnectClientPort = port;
} /* End of GCONFIG_SetUdpAutoConnectClientPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetTftpServerIp
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetTftpServerIp(U32_T ip)
{
	gconfig_ConfigData.UpgradeCfg.TftpServerIpForMcpu = ip;
} /* End of GCONFIG_SetTftpServerIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetFilename
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_SetFilename(U8_T *filename, U8_T len)
{
	if (len > sizeof(gconfig_ConfigData.UpgradeCfg.FileNameMcpu) - 1)
		len = sizeof(gconfig_ConfigData.UpgradeCfg.FileNameMcpu) - 1;

	memcpy(gconfig_ConfigData.UpgradeCfg.FileNameMcpu, filename, len); 
	gconfig_ConfigData.UpgradeCfg.FileNameMcpu[len] = '\0';
    gconfig_ConfigData.UpgradeCfg.FileNameLenMcpu = len; 	
    return len;
} /* End of GCONFIG_SetFilename() */

/////////////////////////////////////////////////////////////////////////////////
// Get Functions

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetTftpServerIp
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetTftpServerIp(void)
{
	return gconfig_ConfigData.UpgradeCfg.TftpServerIpForMcpu; 
} /* End of GCONFIG_GetTftpServerIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetFilename
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetFilename(U8_T *filename, U8_T len)
{
    if (!filename || len < sizeof(gconfig_ConfigData.UpgradeCfg.FileNameMcpu))
        return 0;

    memcpy(filename, gconfig_ConfigData.UpgradeCfg.FileNameMcpu, sizeof(gconfig_ConfigData.UpgradeCfg.FileNameMcpu)); 

    return gconfig_ConfigData.UpgradeCfg.FileNameLenMcpu;	
} /* End of GCONFIG_GetFilename() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDeviceName
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
GCONFIG_DEV_NAME* GCONFIG_GetDeviceName(void)
{
	return &gconfig_ConfigData.DevName;
} /* End of GCONFIG_GetDeviceName() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetMacAddress
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
GCONFIG_MAC_ADDR GCONFIG_GetMacAddress(void)
{
	return macAddr;
} /* End of GCONFIG_GetMacAddress() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetNetwork
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetNetwork(void)
{
	return gconfig_ConfigData.Network;
} /* End of GCONFIG_GetNetwork() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetServerStaticIP
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetServerStaticIP(void)
{
	return gconfig_ConfigData.ServerStaticIP;
} /* End of GCONFIG_GetServerStaticIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetServerDataPktListenPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetServerDataPktListenPort(void)
{
	return gconfig_ConfigData.ServerDataPktListenPort;
} /* End of GCONFIG_GetServerDataPktListenPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetServerMulticastListenPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetServerMulticastListenPort(void)
{
	return gconfig_ConfigData.ServerMulticastListenPort;
} /* End of GCONFIG_GetServerMulticastListenPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetServerBroadcastListenPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetServerBroadcastListenPort(void)
{
	return gconfig_ConfigData.ServerBroadcastListenPort;
} /* End of GCONFIG_GetServerBroadcastListenPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetClientDestIP
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetClientDestIP(void)
{
#if (INCLUDE_DNS_CLIENT)
	U32_T	DestIP, DestNameLen;
    DNSC_STATE state;

	DestNameLen = strlen(gconfig_ConfigData.DestHostName);
	DestIP = GCONFIG_IpAddr2Ulong(gconfig_ConfigData.DestHostName, DestNameLen);
	if (DestIP == 0xffffffff)
	{
		if ((state = DNSC_Query(gconfig_ConfigData.DestHostName, &DestIP)) == DNSC_QUERY_OK)
			return DestIP;
		else if (state == DNSC_QUERY_FREE)
			DNSC_Start(gconfig_ConfigData.DestHostName);
		return 0;		
	}
	return DestIP;
#else
	return gconfig_ConfigData.ClientDestIP;
#endif			
} /* End of GCONFIG_GetClientDestIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetClientDestPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetClientDestPort(void)
{
	return gconfig_ConfigData.ClientDestPort;
} /* End of GCONFIG_GetClientDestPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetNetmask
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetNetmask(void)
{
	return gconfig_ConfigData.Netmask;
} /* End of GCONFIG_GetNetmask() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetGateway
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetGateway(void)
{
	return gconfig_ConfigData.Gateway;
} /* End of GCONFIG_GetGateway() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDNS
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetDNS(void)
{
	return gconfig_ConfigData.Dns;
} /* End of GCONFIG_GetDNS() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetSerialPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetSerialPort(void)
{
	return gconfig_ConfigData.SerialPort;
} /* End of GCONFIG_GetSerialPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetEthernetTxTimer
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetEthernetTxTimer(void)
{
	return gconfig_ConfigData.EthernetTxTimer;
} /* End of GCONFIG_GetEthernetTxTimer() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetUdpAutoConnectClientIp
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetUdpAutoConnectClientIp(void)
{
	return gconfig_ConfigData.UdpAutoConnectClientIp;
} /* End of GCONFIG_GetUdpAutoConnectClientIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetUdpAutoConnectClientPort
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetUdpAutoConnectClientPort(void)
{
	return gconfig_ConfigData.UdpAutoConnectClientPort;
} /* End of GCONFIG_GetUdpAutoConnectClientPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetBaudRate
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetBaudRate(U8_T index)
{
	U16_T baudRate = 0;

	switch(gconfig_SysClock)
	{
		case SCS_80M:
			if 	    (index == 0) baudRate = HS_BR80M_921600;
			else if (index == 1) baudRate = HS_BR80M_115200;
			else if (index == 2) baudRate = HS_BR80M_57600;
			else if (index == 3) baudRate = HS_BR80M_38400;
			else if (index == 4) baudRate = HS_BR80M_19200;
			else if (index == 5) baudRate = HS_BR80M_9600;
			else if (index == 6) baudRate = HS_BR80M_4800;
			else if (index == 7) baudRate = HS_BR80M_2400;
			else if (index == 8) baudRate = HS_BR80M_1200;
			break;
		case SCS_40M:
			if 	    (index == 0) baudRate = HS_BR40M_921600;
			else if (index == 1) baudRate = HS_BR40M_115200;
			else if (index == 2) baudRate = HS_BR40M_57600;
			else if (index == 3) baudRate = HS_BR40M_38400;
			else if (index == 4) baudRate = HS_BR40M_19200;
			else if (index == 5) baudRate = HS_BR40M_9600;
			else if (index == 6) baudRate = HS_BR40M_4800;
			else if (index == 7) baudRate = HS_BR40M_2400;
			else if (index == 8) baudRate = HS_BR40M_1200;
			break;
		default:
			printd("Unknown system clock rate!\n\r");
			break;
	}
	
	return baudRate;
} /* End of GCONFIG_GetBaudRate() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetAdmin
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S16_T GCONFIG_GetAdmin(U8_T *username, U8_T *passwd, U8_T *level)
{
    if (!username || !passwd)
        return -1;

    memcpy(username, gconfig_ConfigData.Admin.Username, 16);
    memcpy(passwd, gconfig_ConfigData.Admin.Passwd, 16);
    *level = gconfig_ConfigData.Admin.Level;

    return 1;
} /* End of GCONFIG_GetAdmin() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetAdmin
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S16_T GCONFIG_SetAdmin(U8_T *username, U8_T *passwd)
{
    if (!username || !passwd)
        return -1;

    memcpy(gconfig_ConfigData.Admin.Username, username, 16);
    memcpy(gconfig_ConfigData.Admin.Passwd, passwd, 16);

    return 1;
} /* End of GCONFIG_SetAdmin() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetAccessibleIP
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S16_T GCONFIG_GetAccessibleIP(U8_T idx, U32_T *ip)
{
    if (!ip || idx >= GCONFIG_ACCESSIBLE_IP_NUM)
        return -1;

    *ip = gconfig_ConfigData.AccessibleIP[idx];
    return 1;

} /* End of GCONFIG_GetAccessibleIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetAccessibleIP
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S16_T GCONFIG_SetAccessibleIP(U8_T idx, U32_T ip)
{
    if (idx >= GCONFIG_ACCESSIBLE_IP_NUM)
        return -1;

    gconfig_ConfigData.AccessibleIP[idx] = ip;
    return 1;
} /* GCONFIG_SetAccessibleIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetSMTPDomainName
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetSMTPDomainName(U8_T *pIn, U8_T len)
{
	if (len > sizeof(gconfig_ConfigData.Smtp.DomainName) - 1)
		len = sizeof(gconfig_ConfigData.Smtp.DomainName) - 1;
	
	memcpy(gconfig_ConfigData.Smtp.DomainName, pIn, len);
	gconfig_ConfigData.Smtp.DomainName[len] = '\0';
} /* GCONFIG_SetSMTPDomainName() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: U8_T GCONFIG_GetSMTPDomainName
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetSMTPDomainName(U8_T *pOut)
{
	gconfig_ConfigData.Smtp.DomainName[sizeof(gconfig_ConfigData.Smtp.DomainName) - 1] = '\0';
    strcpy(pOut, gconfig_ConfigData.Smtp.DomainName);

	return strlen(gconfig_ConfigData.Smtp.DomainName);
} /* GCONFIG_GetSMTPDomainName() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetSMTPFrom
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetSMTPFrom(U8_T *pIn, U8_T len)
{
	if (len > sizeof(gconfig_ConfigData.Smtp.FromAddr) - 1)
		len = sizeof(gconfig_ConfigData.Smtp.FromAddr) - 1;
	
	memcpy(gconfig_ConfigData.Smtp.FromAddr, pIn, len);
	gconfig_ConfigData.Smtp.FromAddr[len] = '\0';
} /* GCONFIG_SetSMTPFrom() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: U8_T GCONFIG_GetSMTPFrom()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetSMTPFrom(U8_T *pOut)
{
	gconfig_ConfigData.Smtp.FromAddr[sizeof(gconfig_ConfigData.Smtp.FromAddr) - 1] = '\0';
    strcpy(pOut, gconfig_ConfigData.Smtp.FromAddr);

	return strlen(gconfig_ConfigData.Smtp.FromAddr);
} /* GCONFIG_GetSMTPFrom() */
/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetSMTPTo1
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetSMTPTo1(U8_T *pIn, U8_T len)
{
	if (len > sizeof(gconfig_ConfigData.Smtp.ToAddr1) - 1)
		len = sizeof(gconfig_ConfigData.Smtp.ToAddr1) - 1;
	
	memcpy(gconfig_ConfigData.Smtp.ToAddr1, pIn, len);
	gconfig_ConfigData.Smtp.ToAddr1[len] = '\0';
} /* GCONFIG_SetSMTPTo1() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: U8_T GCONFIG_GetSMTPTo1()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetSMTPTo1(U8_T *pOut)
{
	gconfig_ConfigData.Smtp.ToAddr1[sizeof(gconfig_ConfigData.Smtp.ToAddr1) - 1] = '\0';
    strcpy(pOut, gconfig_ConfigData.Smtp.ToAddr1);

	return strlen(gconfig_ConfigData.Smtp.ToAddr1);
} /* GCONFIG_GetSMTPTo1() */
/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetSMTPTo2
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetSMTPTo2(U8_T *pIn, U8_T len)
{
	if (len > sizeof(gconfig_ConfigData.Smtp.ToAddr2) - 1)
		len = sizeof(gconfig_ConfigData.Smtp.ToAddr2) - 1;
	
	memcpy(gconfig_ConfigData.Smtp.ToAddr2, pIn, len);
	gconfig_ConfigData.Smtp.ToAddr2[len] = '\0';
} /* GCONFIG_SetSMTPTo2() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: U8_T GCONFIG_GetSMTPTo2()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetSMTPTo2(U8_T *pOut)
{
	gconfig_ConfigData.Smtp.ToAddr2[sizeof(gconfig_ConfigData.Smtp.ToAddr2) - 1] = '\0';
    strcpy(pOut, gconfig_ConfigData.Smtp.ToAddr2);

	return strlen(gconfig_ConfigData.Smtp.ToAddr2);
} /* GCONFIG_GetSMTPTo2() */
/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetSMTPTo3
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetSMTPTo3(U8_T *pIn, U8_T len)
{
	if (len > sizeof(gconfig_ConfigData.Smtp.ToAddr3) - 1)
		len = sizeof(gconfig_ConfigData.Smtp.ToAddr3) - 1;
	
	memcpy(gconfig_ConfigData.Smtp.ToAddr3, pIn, len);
	gconfig_ConfigData.Smtp.ToAddr3[len] = '\0';
} /* GCONFIG_SetSMTPTo3() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: U8_T GCONFIG_GetSMTPTo3()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetSMTPTo3(U8_T *pOut)
{
	gconfig_ConfigData.Smtp.ToAddr3[sizeof(gconfig_ConfigData.Smtp.ToAddr3) - 1] = '\0';
    strcpy(pOut, gconfig_ConfigData.Smtp.ToAddr3);

	return strlen(gconfig_ConfigData.Smtp.ToAddr3);	
} /* GCONFIG_GetSMTPTo3() */
/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetAutoWarning
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetAutoWarning(U16_T ValueIn)
{
	gconfig_ConfigData.Smtp.EventEnableBits = ValueIn;
} /* GCONFIG_SetAutoWarning() */
/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetAutoWarning
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetAutoWarning(void)
{
	return gconfig_ConfigData.Smtp.EventEnableBits;
} /* GCONFIG_GetAutoWarning() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetRs485Mode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetRs485Mode(U8_T mode)
{
	gconfig_ConfigData.Rs485Mode = mode;
} /* GCONFIG_SetAutoWarning() */
/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetRs485Mode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetRs485Mode(void)
{
	return gconfig_ConfigData.Rs485Mode;
} /* GCONFIG_GetRs485Mode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetDestHostName
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDestHostName(U8_T *pIn, U8_T len)
{
    if (len > (sizeof(gconfig_ConfigData.DestHostName)-1))
        len = sizeof(gconfig_ConfigData.DestHostName) - 1;

    memcpy(gconfig_ConfigData.DestHostName, pIn, len);
    gconfig_ConfigData.DestHostName[len] = '\0';

} /* End of GCONFIG_SetDestHostName() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDestHostName
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetDestHostName(U8_T *pOut)
{
    gconfig_ConfigData.DestHostName[sizeof(gconfig_ConfigData.DestHostName) - 1] = '\0';
    strcpy(pOut, gconfig_ConfigData.DestHostName);

	return strlen(gconfig_ConfigData.DestHostName);
} /* End of GCONFIG_GetDestHostName() */

#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetSsid
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetSsid(U8_T *pOut)
{
    if (!pOut)
        return 0;

    memcpy(pOut, gconfig_ConfigData.WifiConfig.WifiSsid, gconfig_ConfigData.WifiConfig.WifiSsidLen);

	return gconfig_ConfigData.WifiConfig.WifiSsidLen;
} /* End of GCONFIG_GetSsid() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetSsid
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetSsid(U8_T *pIn, U8_T len)
{
    if (len > (sizeof(gconfig_ConfigData.WifiConfig.WifiSsid)-1))
        len = sizeof(gconfig_ConfigData.WifiConfig.WifiSsid)-1;

    memcpy(gconfig_ConfigData.WifiConfig.WifiSsid, pIn, len);
    gconfig_ConfigData.WifiConfig.WifiSsid[len] = '\0';
    gconfig_ConfigData.WifiConfig.WifiSsidLen = len;

} /* End of GCONFIG_SetSsid() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetChannel
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetChannel(void)
{
	return gconfig_ConfigData.WifiConfig.WifiChannel;
} /* End of GCONFIG_GetChannel() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetChannel
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetChannel(U8_T channel)
{
    if (channel > 20)
        return;

    gconfig_ConfigData.WifiConfig.WifiChannel = channel;
} /* End of GCONFIG_SetChannel() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiBasebandMode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiBasebandMode(void)
{
	return gconfig_ConfigData.WifiConfig.WifiBasebandMode;
} /* End of GCONFIG_GetWifiBasebandMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiBasebandMode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_SetWifiBasebandMode(U8_T value)
{
    if (value != 1 && value != 2)
        return 0;

    gconfig_ConfigData.WifiConfig.WifiBasebandMode = value;
    return 1;
} /* End of GCONFIG_SetWifiBasebandMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiNetworkMode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiNetworkMode(void)
{
	return gconfig_ConfigData.WifiConfig.WifiNetworkMode;
} /* End of GCONFIG_GetWifiNetworkMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiNetworkMode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiNetworkMode(U8_T value)
{
    if (value > 1)
        return;

    gconfig_ConfigData.WifiConfig.WifiNetworkMode = value;
} /* End of GCONFIG_SetWifiNetworkMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiTxRate
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiTxRate(void)
{
	return gconfig_ConfigData.WifiConfig.WifiTxRate;
} /* End of GCONFIG_GetWifiTxRate() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiTxRate
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiTxRate(U8_T value)
{
    gconfig_ConfigData.WifiConfig.WifiTxRate = value;
} /* End of GCONFIG_SetWifiTxRate() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiTxPowerLevel
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiTxPowerLevel(void)
{
	return gconfig_ConfigData.WifiConfig.WifiTxPowerLevel;
} /* End of GCONFIG_GetWifiTxPowerLevel() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiTxPowerLevel
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiTxPowerLevel(U8_T value)
{
    gconfig_ConfigData.WifiConfig.WifiTxPowerLevel = value;
} /* End of GCONFIG_SetWifiTxPowerLevel() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiPreambleMode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiPreambleMode(void)
{
	return gconfig_ConfigData.WifiConfig.WifiPreambleMode;
} /* End of GCONFIG_GetWifiPreambleMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiPreambleMode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_SetWifiPreambleMode(U8_T value)
{
    if (value > 2)
        return 0;

    gconfig_ConfigData.WifiConfig.WifiPreambleMode = value;
    return 1;
} /* End of GCONFIG_SetWifiPreambleMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiBcnInterval
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetWifiBcnInterval(void)
{
	return gconfig_ConfigData.WifiConfig.WifiBcnInterval;
} /* End of GCONFIG_GetWifiBcnInterval() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiBcnInterval
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiBcnInterval(U16_T value)
{
    gconfig_ConfigData.WifiConfig.WifiBcnInterval = value;
} /* End of GCONFIG_SetWifiBcnInterval() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiRtsThreshold
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetWifiRtsThreshold(void)
{
	return gconfig_ConfigData.WifiConfig.WifiRtsThreshold;
} /* End of GCONFIG_GetWifiRtsThreshold() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiRtsThreshold
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiRtsThreshold(U16_T value)
{
    gconfig_ConfigData.WifiConfig.WifiRtsThreshold = value;
} /* End of GCONFIG_SetWifiRtsThreshold() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiAutoPowerCtrl
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiAutoPowerCtrl(void)
{
	return gconfig_ConfigData.WifiConfig.WifiAutoPowerCtrl;
} /* End of GCONFIG_GetWifiAutoPowerCtrl() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiAutoPowerCtrl
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiAutoPowerCtrl(U8_T value)
{
    gconfig_ConfigData.WifiConfig.WifiAutoPowerCtrl = value ? 1:0;
} /* End of GCONFIG_SetWifiAutoPowerCtrl() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiEncryptMode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiEncryptMode(void)
{
	return gconfig_ConfigData.WifiConfig.WifiEncryptMode;
} /* End of GCONFIG_GetWifiEncryptMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiEncryptMode
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_SetWifiEncryptMode(U8_T value)
{
    if (value > 4)
        return 0;

    if (value == 1)
        gconfig_ConfigData.WifiConfig.WifiWepKeyLength = 0;
    else if (value == 2)
        gconfig_ConfigData.WifiConfig.WifiWepKeyLength = 1;

    gconfig_ConfigData.WifiConfig.WifiEncryptMode = value;
    return 1;
} /* End of GCONFIG_SetWifiEncryptMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiWepKeyIndex
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiWepKeyIndex(void)
{
	return gconfig_ConfigData.WifiConfig.WifiWepKeyIndex;
} /* End of GCONFIG_GetWifiWepKeyIndex() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiWepKeyIndex
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_SetWifiWepKeyIndex(U8_T value)
{
    if (value > 3)
        return 0;

    gconfig_ConfigData.WifiConfig.WifiWepKeyIndex = value;
    return 1;
} /* End of GCONFIG_SetWifiWepKeyIndex() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiWepKeyLength
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiWepKeyLength(void)
{
	return gconfig_ConfigData.WifiConfig.WifiWepKeyLength;
} /* End of GCONFIG_GetWifiWepKeyLength() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiWepKeyLength
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiWepKeyLength(U8_T value)
{
    gconfig_ConfigData.WifiConfig.WifiWepKeyLength = value ? 1:0;
} /* End of GCONFIG_SetWifiWepKeyLength() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiWep64Key
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiWep64Key(U8_T index, U8_T *pBuf)
{
    if (!pBuf)
        return 0;
    else if (index > 3)
		index = 0;

    memcpy(pBuf, gconfig_ConfigData.WifiConfig.WifiWep64Key[index], 5);
    return 5;
} /* End of GCONFIG_GetWifiWep64Key() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiWep64Key
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiWep64Key(U8_T index, U8_T *pBuf)
{
	if (index > 3)
        return;
		
	memcpy(gconfig_ConfigData.WifiConfig.WifiWep64Key[index], pBuf, 5);
} /* End of GCONFIG_SetWifiWep64Key() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiWep128Key
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiWep128Key(U8_T index, U8_T *pBuf)
{
    if (!pBuf)
        return 0;
    else if (index > 3)
		index = 0;

    memcpy(pBuf, gconfig_ConfigData.WifiConfig.WifiWep128Key[index], 13);
    return 13;
} /* End of GCONFIG_GetWifiWep128Key() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiWep128Key
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiWep128Key(U8_T index, U8_T *pBuf)
{
	if (index > 3)
        return;

	memcpy(gconfig_ConfigData.WifiConfig.WifiWep128Key[index], pBuf, 13);	
} /* End of GCONFIG_SetWifiWep128Key() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetWifiPreShareKey
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetWifiPreShareKey(U8_T *pBuf)
{
    memcpy(pBuf, gconfig_ConfigData.WifiConfig.WifiPreShareKey, gconfig_ConfigData.WifiConfig.WifiPreShareKeyLen);

	return gconfig_ConfigData.WifiConfig.WifiPreShareKeyLen;
} /* End of GCONFIG_GetWifiPreShareKey() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetWifiPreShareKey
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetWifiPreShareKey(U8_T *pBuf, U8_T len)
{
	if (len > sizeof(gconfig_ConfigData.WifiConfig.WifiPreShareKey))
		len = sizeof(gconfig_ConfigData.WifiConfig.WifiPreShareKey);
	
	memcpy(gconfig_ConfigData.WifiConfig.WifiPreShareKey, pBuf, len);
	gconfig_ConfigData.WifiConfig.WifiPreShareKeyLen = len;
		
} /* End of GCONFIG_SetWifiPreShareKey() */

#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDhcpSrvStartIp
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetDhcpSrvStartIp(void)
{
	return gconfig_ConfigData.DhcpSrv.IpPoolAddrStart;
} /* End of GCONFIG_GetDhcpSrvStartIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetDhcpSrvStartIp
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDhcpSrvStartIp(U32_T startIp)
{
	gconfig_ConfigData.DhcpSrv.IpPoolAddrStart = startIp;
} /* End of GCONFIG_SetDhcpSrvStartIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDhcpSrvEndIp
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetDhcpSrvEndIp(void)
{
	return gconfig_ConfigData.DhcpSrv.IpPoolAddrEnd;
} /* End of GCONFIG_GetDhcpSrvEndIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetDhcpSrvEndIp
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDhcpSrvEndIp(U32_T endIp)
{
	gconfig_ConfigData.DhcpSrv.IpPoolAddrEnd = endIp;
} /* End of GCONFIG_SetDhcpSrvEndIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDhcpSrvNetmask
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetDhcpSrvNetmask(void)
{
	return gconfig_ConfigData.DhcpSrv.Netmask;
} /* End of GCONFIG_GetDhcpSrvNetmask() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetDhcpSrvNetmask
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDhcpSrvNetmask(U32_T mask)
{
	gconfig_ConfigData.DhcpSrv.Netmask = mask;
} /* End of GCONFIG_SetDhcpSrvNetmask() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDhcpSrvDefGateway
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T GCONFIG_GetDhcpSrvDefGateway(void)
{
	return gconfig_ConfigData.DhcpSrv.DefaultGateway;
} /* End of GCONFIG_GetDhcpSrvDefGateway() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetDhcpSrvDefGateway
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDhcpSrvDefGateway(U32_T gateway)
{
	gconfig_ConfigData.DhcpSrv.DefaultGateway = gateway;
} /* End of GCONFIG_SetDhcpSrvDefGateway() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDhcpSrvStatus
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GCONFIG_GetDhcpSrvStatus(void)
{
	return gconfig_ConfigData.DhcpSrv.Status;
} /* End of GCONFIG_GetDhcpSrvStatus() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetDhcpSrvStatus
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDhcpSrvStatus(U8_T status)
{
    if (status > 1)
        return;

	gconfig_ConfigData.DhcpSrv.Status = status;
} /* End of GCONFIG_SetDhcpSrvStatus() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_GetDhcpSrvLeaseTime
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T GCONFIG_GetDhcpSrvLeaseTime(void)
{
	return gconfig_ConfigData.DhcpSrv.LeaseTime;
} /* End of GCONFIG_GetDhcpSrvLeaseTime() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GCONFIG_SetDhcpSrvLeaseTime
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GCONFIG_SetDhcpSrvLeaseTime(U16_T lease)
{
	gconfig_ConfigData.DhcpSrv.LeaseTime = lease;
} /* End of GCONFIG_SetDhcpSrvLeaseTime() */

/////////////////////////////////////////////////////////////////////////////////

/*
 * ----------------------------------------------------------------------------
 * Function Name: gconfig_ReStoreParameter
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static BOOL gconfig_ReStoreParameter(U32_T addr, GCONFIG_CFG_DATA *pConfig, U16_T len)
{
	U16_T	i;
	U8_T	*pParaBaseByte;
	U8_T	paraCheckSum = 0;
	U8_T	lastWtst = 0;

	pParaBaseByte = (U8_T *)pConfig;

	lastWtst = WTST;
	WTST = PROG_WTST_7;
	if (CSREPR & PMS) // SRAM shadow
	{
		CSREPR |= FAES;
		for (i = 0 ; i < len ; i++)
		{
			*(pParaBaseByte + i) = FCVAR(U8_T, GCONFIG_CFG_DATA_FLASH_ADDR + addr + i);
		}
		CSREPR &= ~FAES;
	}
	else
	{
		CSREPR &= ~FAES;
		for (i = 0 ; i < len ; i++)
		{
			*(pParaBaseByte + i) = FCVAR(U8_T, GCONFIG_CFG_DATA_FLASH_ADDR + addr + i);
		}
	}
	WTST = lastWtst;

    if (0xffff != gconfig_Checksum((U16_T *)pConfig, len))
    {
        printd(" failed! (wrong checksum)\n\r");
		return FALSE;
    }
    else if (pConfig->SwCfgLength != len)
    {
        printd(" Invalid configuration data!\n\r");
		return FALSE;
    }
	printd(" ok.\n\r");
	gconfig_DisplayConfigData(pConfig);
	return TRUE;

} /* End of gconfig_ReStoreParameter() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gconfig_StoreParameter
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static BOOL gconfig_StoreParameter(GCONFIG_CFG_DATA *pConfigData, U16_T len)
{
	U8_T			ret = FALSE;
	pFlashParaData  ptrFlashWrite = (void code *)0x003E00;

    pConfigData->Checksum = 0;
    pConfigData->SwCfgLength = len;
    pConfigData->Checksum = ~gconfig_Checksum((U16_T *)pConfigData, len);
    
	ret = (*ptrFlashWrite)((U8_T *)pConfigData, len);	

	if (ret == FALSE)
		printd("Failed to program Configration Data!!\r\n");

	return ret;
} /* End of gconfig_StoreParameter() */

/*
* -----------------------------------------------------------------------------
 * Function Name: gconfig_Checksum
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U16_T gconfig_Checksum(U16_T *pBuf, U32_T length)
{
	U32_T	cksum = 0;
	U32_T	i, count;
	U8_T*	inChar = &pBuf;
	U16_T*	point;

	count = length/2;

	for (i = 0; i < count; i++)
	{
		point = pBuf;
		cksum += *pBuf++;
		if (point > pBuf)
			(*inChar) = *inChar + 1;

		if (!(i % 0x8000))
		{
			cksum = (cksum >> 16) + (cksum & 0xffff);
			cksum += (cksum >> 16);
			cksum &= 0xffff;
		}
	}
		
	if (length % 2)
		cksum += (*pBuf & 0xff00);

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	cksum &= 0xffff;

	return ((U16_T)cksum);
} /* End of gconfig_Checksum() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gconfig_DisplayConfigData
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void	gconfig_DisplayConfigData(GCONFIG_CFG_DATA *pConfigData)
{
	U8_T i, *pData;
	U8_T dat;
	U16_T val;

	if (GS2W_GetTaskState() == GS2W_STATE_IDLE)
	{
#if (GS2W_ENABLE_FLOW_CONTROL != 0)
		printd("  Flow Control Support\r\n");
#else
		printd("  Flow Control Not Support\r\n");
#endif
		printd("  Device Name: ");
		for (i = 0; i < 16; i++)
			printd ("%02bx ", pConfigData->DevName.DevName[i]);		

		printd("\n\r");
		printd("  WiFi MAC Addr: ");
		for (i = 0; i < 6; i++)
			printd ("%02bx ", macAddr.MacAddr[i]);		
	
		printd("\n\r");
		printd("  Network Setting: ");
		printd ("%04x\r\n", pConfigData->Network);		
	
		val = pConfigData->Network;
		
		if (val & GCONFIG_NETWORK_CLIENT) printd("  SC:Cli ");
		else 					printd("  SC:Ser ");
	
		if (val & GCONFIG_NETWORK_DHCP_ENABLE) printd("DHCP:On ");
		else 					printd("DHCP:Off ");
	
		if (val & GCONFIG_NETWORK_PROTO_UDP) printd("UDP:On ");
		else 					printd("UDP:Off ");
	
		if (val & GCONFIG_NETWORK_PROTO_TCP) printd("TCP:On ");
		else 					printd("TCP:Off ");
	
		if (val & GCONFIG_NETWORK_PROTO_UDP_MCAST) printd("MC:On ");
		else 					printd("MC:Off ");
	
		if (val & GCONFIG_NETWORK_PROTO_UDP_BCAST) printd("BC:On ");
		else 					printd("BC:Off ");
	
		printd("\n\r");
//		printd("  IP:");
//        pData = (U8_T *)&pConfigData->ServerDynamicIP;
//		for (i = 0; i < 4; i++)
//			printd ("%bu ", *(pData + i));		
	
		printd("  SIP:");		
        pData = (U8_T *)&pConfigData->ServerStaticIP;
		for (i = 0; i < 4; i++)
			printd ("%bu ", *(pData + i));		

		printd(" Port: DA: %u", pConfigData->ServerDataPktListenPort);
		printd(" MC: %u", pConfigData->ServerMulticastListenPort);
		printd(" BC: %u", pConfigData->ServerBroadcastListenPort);
		printd("\n\r");

		printd("  Dest Host: %s",pConfigData->DestHostName);
		printd(" Dest Port: %u", pConfigData->ClientDestPort);
		printd("\n\r");
		printd("  Netmask:");
        pData = (U8_T *)&pConfigData->Netmask;
		for (i = 0; i < 4; i++)
			printd ("%bu ", *(pData + i));		

		printd(" Gateway:");
        pData = (U8_T *)&pConfigData->Gateway;
		for (i = 0; i < 4; i++)
			printd ("%bu ", *(pData + i));		

		printd(" DNS:");
        pData = (U8_T *)&pConfigData->Dns;
		for (i = 0; i < 4; i++)
			printd ("%bu ", *(pData + i));		

		printd("\n\r");
		printd("  Serial Port Setting: ");
	
		dat = (pConfigData->SerialPort>>8) & 0xFF;
		val = (dat >> 3) & 0x1F; 
		if      (val == 0) printd ("BR:921600 ");
		else if (val == 1) printd ("BR:115200 ");
		else if (val == 2) printd ("BR:57600 ");
		else if (val == 3) printd ("BR:38400 ");
		else if (val == 4) printd ("BR:19200 ");
		else if (val == 5) printd ("BR:9600 ");
		else if (val == 6) printd ("BR:4800 ");
		else if (val == 7) printd ("BR:2400 ");
		else if (val == 8) printd ("BR:1200 ");
	
		val = dat & 0x07; 
		if (val == 0) printd ("DB:5 ");
		else if (val == 1) printd ("DB:6 ");
		else if (val == 2) printd ("DB:7 ");
		else if (val == 3) printd ("DB:8 ");
	
		dat = (pConfigData->SerialPort & 0x00FF);
	
		val = (dat >> 5) & 0x07; 
		if (val == 0) printd ("P:Odd ");
		else if (val == 1) printd ("P:Even ");
		else if (val == 2) printd ("P:None ");
	
		val = (dat >> 2) & 0x07; 
		if (val == 0) printd ("FC:Xon-Xoff ");
		else if (val == 1) printd ("FC:Hardware ");
		else if (val == 2) printd ("FC:None ");
	
		val = dat & 0x03; 
		if (val == 0) printd ("SB:1 ");
		else if (val == 1) printd ("SB:1.5 ");

		printd("\n\r");
		printd("  Tx Time Interval:%u", pConfigData->EthernetTxTimer);
		printd(" Username:%s Password:%s",pConfigData->Admin.Username, pConfigData->Admin.Passwd);
		printd("\n\r");
		
		printd("  E-Mail Server:%s",pConfigData->Smtp.DomainName);		
		printd("\n\r");
				
		printd("  From:%s",pConfigData->Smtp.FromAddr);
		printd("  To1:%s",pConfigData->Smtp.ToAddr1);
		printd("\n\r");
		
		printd("  To2:%s",pConfigData->Smtp.ToAddr2);
		printd("  To3:%s",pConfigData->Smtp.ToAddr3);
		printd("\n\r");
		
		val = pConfigData->Smtp.EventEnableBits;
		printd("  AutoWarning:%x ", val);
		printd("CS:");
		if (val & GCONFIG_SMTP_EVENT_COLDSTART) printd("On ");
		else printd("Off ");
		
		printd("AF:");
		if (val & GCONFIG_SMTP_EVENT_AUTH_FAIL) printd("On ");
		else printd("Off ");
		
		printd("IPC:");
		if (val & GCONFIG_SMTP_EVENT_IP_CHANGED) printd("On ");
		else printd("Off ");

		printd("PC:");
		if (val & GCONFIG_SMTP_EVENT_PSW_CHANGED) printd("On ");
		else printd("Off ");
		printd("\r\n");
				
		// Add for wifi configuration //
		printd("  ---------- Wifi Configuration ---------- \r\n");
		
		// Wireless mode //		
		i = pConfigData->WifiConfig.WifiBasebandMode;		
		printd("  Baseband: %s mode ", (i==1) ? "802.11b+g" : ((i==2) ? "802.11a" : "unsupport"));
		printd("\r\n");
		
		// Network mode //		
		i = pConfigData->WifiConfig.WifiNetworkMode;		
		printd("  Network: %s mode ", (i ? "Ad-hoc" : "infrastructure"));
		printd("\r\n");
		
		// Channel //		
		i = IO_WiFiSupportedCh[pConfigData->WifiConfig.WifiChannel];
		printd("  Channel: ");
		if (i)
			printd("%bu ", i);
		else
			printd("auto ");
		printd("\r\n");

		// Service Area Name/SSID //
		printd("  SSID: ");
		for (i = 0; i < pConfigData->WifiConfig.WifiSsidLen; i ++)
		{
			printd("%c", pConfigData->WifiConfig.WifiSsid[i]);
		}	
		printd("\r\n");
		
		// Security Mode //
		i = pConfigData->WifiConfig.WifiEncryptMode;
		printd("  Security: ", i);
		if (i == 0)
			printd("no security ");
		else if (i == 1)
			printd("WEP64 bits");		
		else if (i == 2)
			printd("WEP128 bits");		
		else if (i == 3)
			printd("TKIP ");		
		else
			printd("AES ");
		printd("\r\n");					
		
		// Key Length //
		i = pConfigData->WifiConfig.WifiWepKeyLength;
		printd("  WEP Key Length: %s bits", (i ? "128" : "64"));
		printd("\r\n");	
						
		// Key Index Select //
		i = pConfigData->WifiConfig.WifiWepKeyIndex;
		printd("  WEP Key Index: %bu", i);
		printd("\r\n");	
				
		// Key Index 1 ~ 4//
		dat = pConfigData->WifiConfig.WifiWepKeyLength;		
		if (dat)
			dat = 13;
		else
			dat = 5;
			
		for (val = 0; val < 4; val ++)
		{
			printd("  WEP Key %u: 0x", val);
			for (i = 0; i < dat; i ++)
			{
				if (dat == 5)
					printd("%02bx", pConfigData->WifiConfig.WifiWep64Key[val][i]);		
				else
					printd("%02bx", pConfigData->WifiConfig.WifiWep128Key[val][i]);
			}
			printd("\r\n");	
		}
				
		// AES/TKIP Passphrase //
		dat = pConfigData->WifiConfig.WifiPreShareKeyLen;		
		printd("  AES/TKIP Passphrase: ");
		for (i = 0; i < dat; i ++)
		{
			printd("%c", pConfigData->WifiConfig.WifiPreShareKey[i]);
		}
		printd("\r\n");	
				
		// TX Data Rate //		
		i = pConfigData->WifiConfig.WifiTxRate;
		printd("  Tx Rate: ", i);
        switch(i)
        {
            case 0:
                printd("auto");
                break;
            case 1:
                printd("1 Mbps");
                break;
            case 2:
                printd("2 Mbps");
                break;
            case 3:
                printd("5.5 Mbps");
                break;
            case 4:
                printd("6 Mbps");
                break;
            case 5:
                printd("9 Mbps");
                break;
            case 6:
                printd("11 Mbps");
                break;
            case 7:
                printd("12 Mbps");
                break;
            case 8:
                printd("18 Mbps");
                break;
            case 9:
                printd("24 Mbps");
                break;
            case 10:
                printd("36 Mbps");
                break;
            case 11:
                printd("48 Mbps");
                break;
            case 12:
                printd("54 Mbps");
                break;
        }
		printd("\r\n");										
		
		// Transmission Power //
		i = pConfigData->WifiConfig.WifiTxPowerLevel;
		printd("  Transmission Power Level: ");
		if (i == 0)
			printd("100%% ");
		else if (i == 1)
			printd("50%% ");		
		else if (i == 2)
			printd("25%% ");		
		else if (i == 3)
			printd("12.5%% ");		
		else
			printd("6.25%% ");
		printd("\r\n");		
		
		// GB Protection //
		// Preamble //
		i = pConfigData->WifiConfig.WifiPreambleMode;
		printd("  Preamble Mode: ");
		printd("%s", (i ? "auto" : "long"));
		printd("\r\n");
				
		// Beacon Interval //
		val = pConfigData->WifiConfig.WifiBcnInterval;
		printd("  Beacon Interval: %u ms\r\n", val);
				
		// RTS Threshold //	
		val = pConfigData->WifiConfig.WifiRtsThreshold;
		printd("  RTS Threshold: %u \r\n", val);
		
		// Auto Power Control //
		i = pConfigData->WifiConfig.WifiAutoPowerCtrl;
		printd("  Auto Power Control: ");
		printd("%s", (i ? "enable" : "disable"));
										
		// End of power on information //		
		printd("\n\r\n\r");
	}

} /* End of gconfig_DisplayConfigData() */

/* End of gconfig.c */
