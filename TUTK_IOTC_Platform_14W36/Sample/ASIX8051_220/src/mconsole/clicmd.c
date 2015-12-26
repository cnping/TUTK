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
/*=============================================================================
 * Module Name: CLICMD
 * Purpose: The purpose of this package provides the services to CLICMD
 * Author:
 * Date:
 * Notes:
 * $Log$
*=============================================================================
*/
/* INCLUDE FILE DECLARATIONS */
#include <absacc.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "main.h"
#include "printd.h"
#include "gconfig.h"
#include "gs2w.h"
#include "tcpip.h"
#include "gtcpdat.h"
#include "ax22000.h"
#include "hsuart.h"
#include "stoe.h"
#include "stoe_cfg.h"
#include "console.h"
#include "clicmd.h"
#include "at24c02.h"
#include "mcpu.h"
#include "mwioctl.h"
#include "mwifapp.h"
#include "ping.h"
#include "delay.h"
/* NAMING CONSTANT DECLARATIONS */

/* MACRO DECLARATIONS */
#define IsDigit(x) ((x < 0x3a && x > 0x2f) ? 1 : 0)
 
/* GLOBAL VARIABLES DECLARATIONS */
static S16_T clicmd_SetIp(CONSOLE_Inst *pInst);
static S16_T clicmd_SetIpHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetMask(CONSOLE_Inst *pInst);
static S16_T clicmd_SetMaskHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetGateway(CONSOLE_Inst *pInst);
static S16_T clicmd_SetGatewayHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetDnsIp(CONSOLE_Inst *pInst);
static S16_T clicmd_SetDnsIpHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetSerialPort(CONSOLE_Inst *pInst);
static S16_T clicmd_SetSerialPortHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetDeviceMode(CONSOLE_Inst *pInst);
static S16_T clicmd_SetDeviceModeHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetServerPort(CONSOLE_Inst *pInst);
static S16_T clicmd_SetServerPortHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetRemotePort(CONSOLE_Inst *pInst);
static S16_T clicmd_SetRemotePortHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetDhcpClient(CONSOLE_Inst *pInst);
static S16_T clicmd_SetDhcpClientHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetConnectType(CONSOLE_Inst *pInst);
static S16_T clicmd_SetConnectTypeHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SaveConfig(CONSOLE_Inst *pInst);
static S16_T clicmd_SaveConfigHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetEtherTimer(CONSOLE_Inst *pInst);
static S16_T clicmd_SetEtherTimerHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetUsername(CONSOLE_Inst *pInst);
static S16_T clicmd_SetUsernameHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_GetIpConfig(CONSOLE_Inst *pInst);
static S16_T clicmd_GetIpConfigHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetAccessibleIP(CONSOLE_Inst *pInst);
static S16_T clicmd_SetAccessibleIPHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_ConfigAccessibleIP(CONSOLE_Inst *pInst);
static S16_T clicmd_ConfigAccessibleIPHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetEmailServerAddr(CONSOLE_Inst *pInst);
static S16_T clicmd_SetEmailServerAddrHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetEmailFromAddr(CONSOLE_Inst *pInst);
static S16_T clicmd_SetEmailFromAddrHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetEmailTo1Addr(CONSOLE_Inst *pInst);
static S16_T clicmd_SetEmailTo1AddrHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetEmailTo2Addr(CONSOLE_Inst *pInst);
static S16_T clicmd_SetEmailTo2AddrHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetEmailTo3Addr(CONSOLE_Inst *pInst);
static S16_T clicmd_SetEmailTo3AddrHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_GetEmailConfig(CONSOLE_Inst *pInst);
static S16_T clicmd_GetEmailConfigHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetAWConfig(CONSOLE_Inst *pInst);
static S16_T clicmd_SetAWConfigHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetRs485Mode(CONSOLE_Inst *pInst);
static S16_T clicmd_SetRs485ModeHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetDestHostName(CONSOLE_Inst *pInst);
static S16_T clicmd_SetDestHostNameHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetTftpServerIp(CONSOLE_Inst *pInst);
static S16_T clicmd_SetTftpServerIpHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetFilename(CONSOLE_Inst *pInst);
static S16_T clicmd_SetFilenameHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_UpdateFirmware(CONSOLE_Inst *pInst);
static S16_T clicmd_UpdateFirmwareHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetEEP(CONSOLE_Inst *pInst);
static S16_T clicmd_SetEEPHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_DebugMsg(CONSOLE_Inst *pInst);
static S16_T clicmd_DebugMsgHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_ConnectStatus(CONSOLE_Inst *pInst);
static S16_T clicmd_ConnectStatusHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_Ping(CONSOLE_Inst *pInst);
static S16_T clicmd_PingHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetDefault(CONSOLE_Inst *pInst);
static S16_T clicmd_SetDefaultHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetBMCast(CONSOLE_Inst *pInst);
static S16_T clicmd_SetBMCastHelp(CONSOLE_Inst *pInst);

#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
static S16_T clicmd_SetSSID(CONSOLE_Inst *pInst);
static S16_T clicmd_SetSSIDHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetNetworkType(CONSOLE_Inst *pInst);
static S16_T clicmd_SetNetworkTypeHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetChannel(CONSOLE_Inst *pInst);
static S16_T clicmd_SetChannelHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetWep64Key(CONSOLE_Inst *pInst);
static S16_T clicmd_SetWep64KeyHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetWep128Key(CONSOLE_Inst *pInst);
static S16_T clicmd_SetWep128KeyHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetWepKeyIndex(CONSOLE_Inst *pInst);
static S16_T clicmd_SetWepKeyIndexHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetWPAPassphrase(CONSOLE_Inst *pInst);
static S16_T clicmd_SetWPAPassphraseHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetEncryptionType(CONSOLE_Inst *pInst);
static S16_T clicmd_SetEncryptionTypeHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SiteSurvey(CONSOLE_Inst *pInst);
static S16_T clicmd_SiteSurveyHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_JoinBSS(CONSOLE_Inst *pInst);
static S16_T clicmd_JoinBSSHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetWepKeyLength(CONSOLE_Inst *pInst);
static S16_T clicmd_SetWepKeyLengthHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetPreamble(CONSOLE_Inst *pInst);
static S16_T clicmd_SetPreambleHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetRTSThreshold(CONSOLE_Inst *pInst);
static S16_T clicmd_SetRTSThresholdHelp(CONSOLE_Inst *pInst);

//static S16_T clicmd_SetOperationMode(CONSOLE_Inst *pInst);
//static S16_T clicmd_SetOperationModeHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetTxPowerLevel(CONSOLE_Inst *pInst);
static S16_T clicmd_SetTxPowerLevelHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetDataRate(CONSOLE_Inst *pInst);
static S16_T clicmd_SetDataRateHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetAutoPowerControl(CONSOLE_Inst *pInst);
static S16_T clicmd_SetAutoPowerControlHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_SetBeaconInterval(CONSOLE_Inst *pInst);
static S16_T clicmd_SetBeaconIntervalHelp(CONSOLE_Inst *pInst);

static S16_T clicmd_GetWifiConfig(CONSOLE_Inst *pInst);
static S16_T clicmd_GetWifiConfigHelp(CONSOLE_Inst *pInst);
#endif /* (MAC_ARBIT_MODE & MAC_ARBIT_WIFI) */

static S16_T clicmd_SetDhcpServer(CONSOLE_Inst *pInst);
static S16_T clicmd_SetDhcpServerHelp(CONSOLE_Inst *pInst);
static S16_T clicmd_EnableDhcpServer(CONSOLE_Inst *pInst);
static S16_T clicmd_EnableDhcpServerHelp(CONSOLE_Inst *pInst);


CONSOLE_CmdEntry const FAR CLICMD_userCmdTable[]=
{
    {"username", clicmd_SetUsername, clicmd_SetUsernameHelp, 5},
    {"ipconfig", clicmd_GetIpConfig, clicmd_GetIpConfigHelp, 5},
    {"setip", clicmd_SetIp, clicmd_SetIpHelp, 5},
    {"setmask", clicmd_SetMask, clicmd_SetMaskHelp, 5},
    {"setgateway", clicmd_SetGateway, clicmd_SetGatewayHelp, 5},
    {"setdns", clicmd_SetDnsIp, clicmd_SetDnsIpHelp, 5},
    {"serialport", clicmd_SetSerialPort, clicmd_SetSerialPortHelp, 5},
    {"setmode", clicmd_SetDeviceMode, clicmd_SetDeviceModeHelp, 5},
    {"setsrvport", clicmd_SetServerPort, clicmd_SetServerPortHelp, 5},
    {"setdstport", clicmd_SetRemotePort, clicmd_SetRemotePortHelp, 5},
    {"dhcpclient", clicmd_SetDhcpClient, clicmd_SetDhcpClientHelp, 5},
    {"connectype", clicmd_SetConnectType, clicmd_SetConnectTypeHelp, 5},
    {"transmitimer", clicmd_SetEtherTimer, clicmd_SetEtherTimerHelp, 5},
    {"saveconfig", clicmd_SaveConfig, clicmd_SaveConfigHelp, 5},
    {"accessip", clicmd_SetAccessibleIP, clicmd_SetAccessibleIPHelp, 5},
    {"setaccip", clicmd_ConfigAccessibleIP, clicmd_ConfigAccessibleIPHelp, 5},
	
    {"setems", clicmd_SetEmailServerAddr, clicmd_SetEmailServerAddrHelp, 5},
    {"setemf", clicmd_SetEmailFromAddr, clicmd_SetEmailFromAddrHelp, 5},
    {"setemt1", clicmd_SetEmailTo1Addr, clicmd_SetEmailTo1AddrHelp, 5},
    {"setemt2", clicmd_SetEmailTo2Addr, clicmd_SetEmailTo2AddrHelp, 5},
    {"setemt3", clicmd_SetEmailTo3Addr, clicmd_SetEmailTo3AddrHelp, 5},				
    {"emconfig", clicmd_GetEmailConfig, clicmd_GetEmailConfigHelp, 5},
    {"setaw", clicmd_SetAWConfig, clicmd_SetAWConfigHelp, 5},
    {"rs485", clicmd_SetRs485Mode, clicmd_SetRs485ModeHelp, 5},	
	
    {"setdsthn", clicmd_SetDestHostName, clicmd_SetDestHostNameHelp, 5},
    {"tftpsrv", clicmd_SetTftpServerIp, clicmd_SetTftpServerIpHelp, 5},
    {"filename", clicmd_SetFilename, clicmd_SetFilenameHelp, 5},
    {"dlfirmware", clicmd_UpdateFirmware, clicmd_UpdateFirmwareHelp, 5},
    {"seteep", clicmd_SetEEP, clicmd_SetEEPHelp, 5},
    {"dbgmsg", clicmd_DebugMsg, clicmd_DebugMsgHelp, 5},
    {"connstatus", clicmd_ConnectStatus, clicmd_ConnectStatusHelp, 5},
    {"ping", clicmd_Ping, clicmd_PingHelp, 5},	
    {"setdef", clicmd_SetDefault, clicmd_SetDefaultHelp, 5},
    {"bmcast", clicmd_SetBMCast, clicmd_SetBMCastHelp, 5},	
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
	{"setssid", clicmd_SetSSID, clicmd_SetSSIDHelp, 5},
	{"setnt", clicmd_SetNetworkType, clicmd_SetNetworkTypeHelp, 5},
    {"setch", clicmd_SetChannel, clicmd_SetChannelHelp, 5},
    {"wepklen", clicmd_SetWepKeyLength, clicmd_SetWepKeyLengthHelp, 5},
	{"setw64k", clicmd_SetWep64Key, clicmd_SetWep64KeyHelp, 5},
	{"setw128k", clicmd_SetWep128Key, clicmd_SetWep128KeyHelp, 5},
	{"setwki", clicmd_SetWepKeyIndex, clicmd_SetWepKeyIndexHelp, 5},
	{"setwp", clicmd_SetWPAPassphrase, clicmd_SetWPAPassphraseHelp, 5},
	{"setet", clicmd_SetEncryptionType, clicmd_SetEncryptionTypeHelp, 5},
	{"sisrvy", clicmd_SiteSurvey, clicmd_SiteSurveyHelp, 5},
	{"jbss", clicmd_JoinBSS, clicmd_JoinBSSHelp, 5},
//	{"setopm", clicmd_SetOperationMode, clicmd_SetOperationModeHelp, 5},
	{"setpmb", clicmd_SetPreamble, clicmd_SetPreambleHelp, 5},
	{"setrts", clicmd_SetRTSThreshold, clicmd_SetRTSThresholdHelp, 5},
	{"settpl", clicmd_SetTxPowerLevel, clicmd_SetTxPowerLevelHelp, 5},
	{"setdr", clicmd_SetDataRate, clicmd_SetDataRateHelp, 5},
	{"setapc", clicmd_SetAutoPowerControl, clicmd_SetAutoPowerControlHelp, 5},
	{"setbi", clicmd_SetBeaconInterval, clicmd_SetBeaconIntervalHelp, 5},
	{"wificonfig", clicmd_GetWifiConfig, clicmd_GetWifiConfigHelp, 5},	
#endif
    {"dhcpsrv", clicmd_SetDhcpServer, clicmd_SetDhcpServerHelp, 5},
    {"setdhcpsrv", clicmd_EnableDhcpServer, clicmd_EnableDhcpServerHelp, 5},
};

CONSOLE_Account far CLICMD_userTable[MAX_USER_ACCOUNT];

/* LOCAL VARIABLES DECLARATIONS */
U8_T far StrBuf[128], StrLen;
S16_T argc;
S8_T **argv;

/* LOCAL SUBPROGRAM DECLARATIONS */
static U8_T clicmd_DecText2Char(U8_T *pbuf, U8_T *pValue, U8_T len);
static U8_T clicmd_HexText2Char(U8_T *pbuf, U8_T *pValue, U8_T len);
static U8_T clicmd_DecText2Short(U8_T *pbuf, U16_T *pValue, U8_T len);
static U8_T clicmd_HexText2Short(U8_T *pbuf, U16_T *pValue, U8_T len);
static U8_T clicmd_HexText2Long(U8_T *pbuf, U32_T *pValue, U8_T len);
static S16_T clicmd_GetIp2Ulong(U8_T *pBuf, U8_T len, U32_T *ipAddr);
static S16_T clicmd_CheckIpInput(U8_T *pBuf);
static U8_T clicmd_ValidIp(U32_T ipAddr);

/* LOCAL SUBPROGRAM BODIES */
/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_DecText2Char
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T clicmd_DecText2Char(U8_T *pbuf, U8_T *pValue, U8_T len)
{
	*pValue = 0;

	if ((len == 0) || (len > 3))
	{
		return 0xFF;
	}

	while (len--)
	{
		*pValue *= 10;
 
		if ((*pbuf < 0x3A) && (*pbuf > 0x2F))
		{
			*pValue += (*pbuf - 0x30);
		}
		else
		{
			return 0xFF;
		}

		pbuf++;
	}

	if (*pValue > 255)
	{
		return 0xFF;
	}

	return 0;

} /* End of  clicmd_DecText2Char() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_HexText2Char
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T clicmd_HexText2Char(U8_T *pbuf, U8_T *pValue, U8_T len)
{
	*pValue = 0;

	if ((len == 0) || (len > 2))
	{
		return 0xFF;
	}

	while (len--)
	{
		*pValue *= 16;
 
		if ((*pbuf < 0x3A) && (*pbuf > 0x2F))
			*pValue += (*pbuf - 0x30);
		else if ((*pbuf < 0x47) && (*pbuf > 0x40))
			*pValue += (*pbuf - 0x37);
		else if ((*pbuf < 0x67) && (*pbuf > 0x60))
			*pValue += (*pbuf - 0x57);
		else
			return 0xFF;

		pbuf++;
	}

	return 0;

} /* End of  clicmd_HexText2Char() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_DecText2Short
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T clicmd_DecText2Short(U8_T *pbuf, U16_T *pValue, U8_T len)
{
	*pValue = 0;

	if ((len == 0) || (len > 5))
		return 0xff;

	while (len--)
	{
		*pValue *= 10;
 
		if ((*pbuf < 0x3a) && (*pbuf > 0x2f))
			*pValue += (*pbuf - 0x30);
		else
			return 0xff;

		pbuf++;
	}

	if (*pValue > 65535)
		return 0xff;

	return 0;

} /* End of  clicmd_DecText2Short() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: dbg_HexaText2Short
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T clicmd_HexText2Short(U8_T *pbuf, U16_T *pValue, U8_T len)
{
	*pValue = 0;

	if ((len == 0) || (len > 4))
		return 0xff;

	while (len--)
	{
		*pValue *= 16;
 
		if ((*pbuf < 0x3a) && (*pbuf > 0x2f))
			*pValue += (*pbuf - 0x30);
		else if ((*pbuf < 0x47) && (*pbuf > 0x40))
			*pValue += (*pbuf - 0x37);
		else if ((*pbuf < 0x67) && (*pbuf > 0x60))
			*pValue += (*pbuf - 0x57);
		else
			return 0xff;

		pbuf++;
	}

	return 0;

} /* End of  clicmd_HexaText2Short() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_HexText2Long
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T clicmd_HexText2Long(U8_T *pbuf, U32_T *pValue, U8_T len)
{
	*pValue = 0;

	if ((len == 0) || (len > 8))
		return 0xff;

	while (len--)
	{
		*pValue *= 16;
 
		if ((*pbuf < 0x3a) && (*pbuf > 0x2f))
			*pValue += (*pbuf - 0x30);
		else if ((*pbuf < 0x47) && (*pbuf > 0x40))
			*pValue += (*pbuf - 0x37);
		else if ((*pbuf < 0x67) && (*pbuf > 0x60))
			*pValue += (*pbuf - 0x57);
		else
			return 0xff;

		pbuf++;
	}

	return 0;

} /* End of  clicmd_HexText2Long() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_GetIp2Ulong
 * Purpose: Transfer a ip address string to ulong number. exp: 0xc0a80003
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_GetIp2Ulong(U8_T *pBuf, U8_T len, U32_T *ipAddr)
{
	U8_T *point = (U8_T *)ipAddr;
	U8_T count = 0, digits;
    U16_T value;

	while (len > 0)
	{
		digits = 0;
        value = 0;
		while ((*pBuf != '.') && len > 0)
		{
			digits++;
			if (digits > 3)
				return -1;

            value *= 10;
  
            if (IsDigit(*pBuf))
                value += (*pBuf - 0x30);
			else
				return -1;

			pBuf++; len--;
		}

        if (value > 255)
            return -1;

        *point = (U8_T)(value & 0xFF);

		if (len == 0)
			break;

		pBuf++; len--;
		count++;
		point++;

		if (count > 3)
			return -1;
	}

	if (count != 3)
		return -1;

	return 1;
} /* End of clicmd_GetIp2Ulong */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_ValidIp
 * Purpose: Check the IP validation
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T clicmd_ValidIp(U32_T ipAddr)
{
    U8_T *point = (U8_T *)&ipAddr;
    
    /* class D/E */
    if ((point[0] & 0xF0) >= 0xE0)
        return 0;

    if (point[3] == 0xFF || point[3] == 0)
        return 0;

    /* Class C */
    if ((point[0] & 0xC0) && point[2] == 0xFF)
        return 0;

    return 1;
} /* End of clicmd_ValidIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_CheckIpInput
 * Purpose: Check the input data whether is IP format xxx.xxx.xxx.xxx
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_CheckIpInput(U8_T *pBuf)
{
	U8_T *point = pBuf;
	U8_T len = 0;

    if (!pBuf || (len=strlen(pBuf)) == 0)
        return 0;
  
	while (len > 0)
	{
		if (IsDigit(*point) || *point == '.')
        {
            point++;
            len --;
        }
        else
            return 0;
    }

	return 1;
} /* End of clicmd_CheckIpInput */


/* EXPORTED SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * Function Name: CLICMD_GetCmdTableSize()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T CLICMD_GetCmdTableSize(void)
{
    return sizeof(CLICMD_userCmdTable)/sizeof(CONSOLE_CmdEntry);
} /* End of CLICMD_GetCmdTableSize() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetIp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetIp(CONSOLE_Inst *pInst)
{
    U32_T addr;
    
    argc = pInst->Argc;
    argv = pInst->Argv;
    
    if (argc == 0) /* current setting */
    {
        addr = GCONFIG_GetServerStaticIP();
        CONSOLE_PutMessage(pInst, "IP address: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        return 1;
    }
    else if (argc > 1)
        return -1;

    if (clicmd_GetIp2Ulong(argv[0], strlen(argv[0]), &addr) > 0)
    {
        if (clicmd_ValidIp(addr))
            GCONFIG_SetServerStaticIP(addr);
        else
            return -1;
    }
    else
        return -1;

    return 1;
} /* End of clicmd_SetIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetIpHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetIpHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setip <ip addr>\r\n");
    return 1;
} /* End of clicmd_SetIpHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetMask()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetMask(CONSOLE_Inst *pInst)
{
    U32_T addr;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) /* current setting */
    {
        addr = GCONFIG_GetNetmask();
        CONSOLE_PutMessage(pInst, "Netmask: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        return 1;
    }
    else if (argc > 1)
        return -1;

    if (clicmd_GetIp2Ulong(argv[0], strlen(argv[0]), &addr) > 0)
        GCONFIG_SetNetmask(addr);
    else
        return -1;

    return 1;
} /* End of clicmd_SetMask() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetMaskHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetMaskHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setmask <netmask>\r\n");
    return 1;
} /* End of clicmd_SetMaskHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetGateway()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetGateway(CONSOLE_Inst *pInst)
{
    U32_T addr;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) /* current setting */
    {
        addr = GCONFIG_GetGateway();
        CONSOLE_PutMessage(pInst, "Gateway: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        return 1;
    }
    else if (argc > 1)
        return -1;

    if (clicmd_GetIp2Ulong(argv[0], strlen(argv[0]), &addr) > 0)
    {
        if (clicmd_ValidIp(addr))
        GCONFIG_SetGateway(addr);
        else
            return -1;
    }
    else
        return -1;

    return 1;
} /* End of clicmd_SetGateway() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetGatewayHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetGatewayHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setgateway <ip addr>\r\n");
    return 1;
} /* End of clicmd_SetGatewayHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDnsIp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDnsIp(CONSOLE_Inst *pInst)
{
    U32_T addr;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) /* current setting */
    {
        addr = GCONFIG_GetDNS();
        CONSOLE_PutMessage(pInst, "DNS IP: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        return 1;
    }
    else if (argc > 1)
        return -1;

    if (clicmd_GetIp2Ulong(argv[0], strlen(argv[0]), &addr) > 0)
    {
        if (clicmd_ValidIp(addr))
        GCONFIG_SetDNS(addr);
        else
            return -1;
    }
    else
        return -1;

    return 1;
} /* End of clicmd_SetDnsIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDnsIpHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDnsIpHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setdns <ip addr>\r\n");
    return 1;
} /* End of clicmd_SetDnsIpHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetSerialPort()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetSerialPort(CONSOLE_Inst *pInst)
{
    S16_T temp16;
    U8_T temp8, *argP;
   
    argc = pInst->Argc;
    argv = pInst->Argv;

    temp16 = GCONFIG_GetSerialPort();
    if (argc == 0) /* current setting */
    {
        CONSOLE_PutMessage(pInst, "Serial Port Settings:\r\n");
        CONSOLE_PutMessage(pInst, "    <baud rate>: ");
        temp8 = ((temp16>>11) & 0x1f);
        switch (temp8)
        {
            case 0:
                CONSOLE_PutMessage(pInst, "921600\r\n");
                break;
            case 1:
                CONSOLE_PutMessage(pInst, "115200\r\n");
                break;
            case 2:
                CONSOLE_PutMessage(pInst, "57600\r\n");
                break;
            case 3:
                CONSOLE_PutMessage(pInst, "38400\r\n");
                break;
            case 4:
                CONSOLE_PutMessage(pInst, "19200\r\n");
                break;
            case 5:
                CONSOLE_PutMessage(pInst, "9600\r\n");
                break;
            case 6:
                CONSOLE_PutMessage(pInst, "4800\r\n");
                break;
            case 7:
                CONSOLE_PutMessage(pInst, "2400\r\n");
                break;
            case 8:
                CONSOLE_PutMessage(pInst, "1200\r\n");
                break;
            default:
                CONSOLE_PutMessage(pInst, "\r\n");
                break;
        }

        CONSOLE_PutMessage(pInst, "    <data bits>: ");
        temp8 = ((temp16>>8) & 0x7);
        switch (temp8)
        {
            case 0: 
                CONSOLE_PutMessage(pInst, "5 bits\r\n");
                break;
            case 1: 
                CONSOLE_PutMessage(pInst, "6 bits\r\n");
                break;
            case 2: 
                CONSOLE_PutMessage(pInst, "7 bits\r\n");
                break;
            default: 
                CONSOLE_PutMessage(pInst, "8 bits\r\n");
                break;
        }
        
        CONSOLE_PutMessage(pInst, "    <parity>: ");
        temp8 = ((temp16>>5) & 0x7);
	    if (temp8 == 2) CONSOLE_PutMessage(pInst, "None\r\n");
	    else if (temp8 == 1) CONSOLE_PutMessage(pInst, "Even\r\n");
        else if (temp8 == 0) CONSOLE_PutMessage(pInst, "Odd\r\n");

        CONSOLE_PutMessage(pInst, "    <stop bits>: ");
        temp8 = (temp16 & 0x3);
        if (temp8 == 0) 
            CONSOLE_PutMessage(pInst, "1\r\n");
        else if (temp8 == 1)
            CONSOLE_PutMessage(pInst, "1.5\r\n");
        else if (temp8 == 2)
            CONSOLE_PutMessage(pInst, "2\r\n");

        CONSOLE_PutMessage(pInst, "    <flow ctrl>: ");
        temp8 = ((temp16>>2) & 0x7);
        if (temp8 == 0) 
            CONSOLE_PutMessage(pInst, "Xon/Xoff\r\n");
        else if (temp8 == 1)
            CONSOLE_PutMessage(pInst, "Hardware\r\n");
        else if(temp8 == 2)
            CONSOLE_PutMessage(pInst, "None\r\n");

        return 1;
    }
    else if (argc != 5)
        return -1;

    /* baud rate */
    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0x7FF;
        temp16 |= (temp8 << 11);
    }
    /* data bits */
    argP = argv[1];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0xF8FF;
        temp16 |= (temp8 << 8);
    }
    /* parity */
    argP = argv[2];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0xFF1F;
        temp16 |= (temp8 << 5);
    }

    /* stop bits */
    argP = argv[3];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0xFFFC;
        temp16 |= temp8;
    }
    
    /* flow ctrl */
    argP = argv[4];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0xFFE3;
        temp16 |= (temp8<<2);
    }

    GCONFIG_SetSerialPort(temp16);

    return 1;
} /* End of clicmd_SetSerialPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetSerialPortHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetSerialPortHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: serialport <baud rate> <data bits> <parity> <stop bits> <flow ctrl>\r\n");
    CONSOLE_PutMessage(pInst, "       <baud rate>: 0: 921600     5: 9600\r\n");
    CONSOLE_PutMessage(pInst, "                    1: 115200     6: 4800\r\n");
    CONSOLE_PutMessage(pInst, "                    2: 57600      7: 2400\r\n");
    CONSOLE_PutMessage(pInst, "                    3: 38400      8: 1200\r\n");
    CONSOLE_PutMessage(pInst, "                    4: 19200\r\n");
    CONSOLE_PutMessage(pInst, "       <data bits>: 0: 5      2: 7\r\n");
    CONSOLE_PutMessage(pInst, "                    1: 6      3: 8\r\n");
    CONSOLE_PutMessage(pInst, "       <parity>:    0: Odd    2: None\r\n");
    CONSOLE_PutMessage(pInst, "                    1: Even\r\n");
    CONSOLE_PutMessage(pInst, "       <stop bits>: 0: 1      1: 1.5      2: 2\r\n");
    CONSOLE_PutMessage(pInst, "       <flow ctrl>: 0: Xon/Xoff     2: None\r\n");
    CONSOLE_PutMessage(pInst, "                    1: Hardware\r\n");
    return 1;
} /* End of clicmd_SetBaudRateHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDeviceMode()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDeviceMode(CONSOLE_Inst *pInst)
{
    U16_T temp16;
    U8_T temp8, *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    temp16 = GCONFIG_GetNetwork();
    if (argc == 0) /* current setting */
    {
        temp16 &= GCONFIG_NETWORK_CLIENT;
        if (temp16)
            CONSOLE_PutMessage(pInst, "Device Mode: CLIENT mode\r\n");
        else
            CONSOLE_PutMessage(pInst, "Device Mode: SERVER mode\r\n");
        return 1;
    }
    else if (argc > 1)
        return -1;
    
    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
        temp8 = *argP - 0x30;
    else
        return -1;

    temp16 &= ~GCONFIG_NETWORK_CLIENT;
    if (temp8)
        temp16 |= GCONFIG_NETWORK_CLIENT;

    GCONFIG_SetNetwork(temp16);

    return 1;

} /* End of clicmd_SetDeviceMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDeviceModeHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDeviceModeHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setmode <mode>\r\n");
    CONSOLE_PutMessage(pInst, "       <mode>: 0: SERVER     1: CLIENT\r\n");

    return 1;

} /* End of clicmd_SetDeviceModeHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetServerPort()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetServerPort(CONSOLE_Inst *pInst)
{
    U16_T temp16;
    U8_T *argP;
    
    argc = pInst->Argc;
    argv = pInst->Argv;
    
    if (argc == 0) /* current setting */
    {
        temp16 = GCONFIG_GetServerDataPktListenPort();
        CONSOLE_PutMessage(pInst, "Server Port: %u\r\n", temp16);

        return 1;
    }
    else if (argc > 1)
        return -1;
    
    argP = argv[0];
    if (strlen(argP) <= 4)
        temp16 = atoi(argP);
    else
        return -1;

    if (temp16 < 1024)
        return -1;
    
    GCONFIG_SetServerDataPktListenPort(temp16);

    return 1;

} /* End of clicmd_SetServerPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetServerPortHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetServerPortHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setsrvport <port>\r\n");

    return 1;
} /* End of clicmd_SetServerPortHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEtherTimer()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEtherTimer(CONSOLE_Inst *pInst)
{
    U16_T temp16;
    U8_T *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) /* current setting */
    {
        temp16 = GCONFIG_GetEthernetTxTimer();
        CONSOLE_PutMessage(pInst, "Transmit Timer: %u\r\n", temp16);

        return 1;
    }
    else if (argc > 1)
        return -1;
    
    argP = argv[0];
    if (strlen(argP) <= 4)
        temp16 = atoi(argP);
    else
        return -1;

    if (temp16 < 10)
        return -1;

    GCONFIG_SetEthernetTxTimer(temp16);

    return 1;
} /* End of clicmd_SetEtherTimer() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEtherTimerHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEtherTimerHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: transmitimer <time>\r\n");
    CONSOLE_PutMessage(pInst, "       <time>: time in ms\r\n");

    return 1;
} /* End of clicmd_SetEtherTimerHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetRemotPort()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetRemotePort(CONSOLE_Inst *pInst)
{
    U16_T temp16;
    U8_T *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) /* current setting */
    {
        temp16 = GCONFIG_GetClientDestPort();
        CONSOLE_PutMessage(pInst, "Destination Port: %u\r\n", temp16);

        return 1;
    }
    else if (argc > 1)
        return -1;
    
    argP = argv[0];
    if (strlen(argP) <= 4)
        temp16 = atoi(argP);
    else
        return -1;

    GCONFIG_SetClientDestPort(temp16);

    return 1;

} /* End of clicmd_SetRemotPort() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetRemotePortHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetRemotePortHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setdstport <port>\r\n");

    return 1;
} /* End of clicmd_SetRemotePortHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDhcpClient()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDhcpClient(CONSOLE_Inst *pInst)
{
    U16_T temp16;
    U8_T temp8, *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    temp16 = GCONFIG_GetNetwork();
    if (argc == 0) /* current setting */
    {
        temp16 &= GCONFIG_NETWORK_DHCP_ENABLE;
        if (temp16)
            CONSOLE_PutMessage(pInst, "DHCP Client: enable\r\n");
        else
            CONSOLE_PutMessage(pInst, "DHCP Client: disable\r\n");
        return 1;
    }
    else if (argc > 1)
        return -1;
    
    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
        temp8 = *argP - 0x30;
    else
        return -1;

    temp16 &= ~GCONFIG_NETWORK_DHCP_ENABLE;
    if (temp8)
        temp16 |= GCONFIG_NETWORK_DHCP_ENABLE;

    GCONFIG_SetNetwork(temp16);

    return 1;

} /* End of clicmd_SetDhcpClient() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDhcpClientHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDhcpClientHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: dhcpclient <status>\r\n");
    CONSOLE_PutMessage(pInst, "       <status>: 0: disable     1: enable\r\n");

    return 1;

} /* End of clicmd_SetDhcpClientHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetConnectType()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetConnectType(CONSOLE_Inst *pInst)
{
    U16_T temp16;
    U8_T temp8, *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    temp16 = GCONFIG_GetNetwork();
    if (argc == 0) /* current setting */
    {
        temp16 &= (GCONFIG_NETWORK_PROTO_UDP | GCONFIG_NETWORK_PROTO_TCP);
        if (temp16 & GCONFIG_NETWORK_PROTO_UDP)
            CONSOLE_PutMessage(pInst, "Connection Type: UDP\r\n");
        else if (temp16 & GCONFIG_NETWORK_PROTO_TCP)
            CONSOLE_PutMessage(pInst, "Connection Type: TCP\r\n");

        return 1;
    }
    else if (argc > 1)
        return -1;
    
    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
        temp8 = *argP - 0x30;
    else
        return -1;

    temp16 &= ~(GCONFIG_NETWORK_PROTO_UDP | GCONFIG_NETWORK_PROTO_TCP);
    if (temp8)
        temp16 |= GCONFIG_NETWORK_PROTO_UDP;
    else
        temp16 |= GCONFIG_NETWORK_PROTO_TCP;

    GCONFIG_SetNetwork(temp16);

    return 1;

} /* End of clicmd_SetConnectType() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetConnectTypeHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetConnectTypeHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: connectype <protocol>\r\n");
    CONSOLE_PutMessage(pInst, "       <protocol>: 0: TCP     1: UDP\r\n");

    return 1;

} /* End of clicmd_SetDhcpClientHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SaveConfig()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SaveConfig(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;

    if (argc == 0) /* current setting */
    {
        CONSOLE_PutMessage(pInst, "Saving Configuration to FLASH\r\n");
        GCONFIG_WriteConfigData();
        return 1;
    }

    return 1;
} /* End of clicmd_SaveConfig() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SaveConfigHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SaveConfigHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: saveconfig\r\n");
    return 1;
} /* End of clicmd_SaveConfigHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetUsername()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetUsername(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 1) /* current setting */
    {
        if (CONSOLE_ChangeUsername(pInst, argv[0]) < 0)
            return -1;
    }

    return 1;
} /* End of clicmd_SetUsername() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetUsernameHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetUsernameHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: username <user name>\r\n");
    return 1;
} /* End of clicmd_SetUsernameHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_GetIpConfig()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_GetIpConfig(CONSOLE_Inst *pInst)
{
    U8_T addr[4];

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0)
    {
        *(U32_T *)&addr[0] = TCPIP_GetIPAddr();
        CONSOLE_PutMessage(pInst, "Current IP: %bu.%bu.%bu.%bu\r\n", addr[0], addr[1], addr[2], addr[3]);
        *(U32_T *)&addr[0] = TCPIP_GetSubnetMask();
        CONSOLE_PutMessage(pInst, "Current IP Mask: %bu.%bu.%bu.%bu\r\n", addr[0], addr[1], addr[2], addr[3]);
        *(U32_T *)&addr[0] = TCPIP_GetGateway();
        CONSOLE_PutMessage(pInst, "Current Gateway: %bu.%bu.%bu.%bu\r\n", addr[0], addr[1], addr[2], addr[3]);
    }

    return 1;

} /* End of clicmd_GetIpConfig() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_GetIpConfigHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_GetIpConfigHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: ipconfig\r\n");

    return 1;
} /* End of clicmd_GetIpConfigHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetAccessibleIP()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetAccessibleIP(CONSOLE_Inst *pInst)
{
    U8_T temp8, *argP;
    U32_T addr;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0)
    {
        CONSOLE_PutMessage(pInst, "Accessible IP List\r\n");
        for (temp8 = 0; temp8 < GCONFIG_ACCESSIBLE_IP_NUM; temp8++)
        {
            GCONFIG_GetAccessibleIP(temp8, &addr);
            CONSOLE_PutMessage(pInst, "IP(%bu): %bu.%bu.%bu.%bu\r\n", temp8, (U8_T)((addr>>24)&0x000000FF),
                               (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        }

        return 1;
    }
    else if (argc < 2)
        return -1;

    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
        temp8 = *argP - 0x30;
    else
        return -1;

    if (clicmd_GetIp2Ulong(argv[1], strlen(argv[1]), &addr) > 0)
    {
        if (clicmd_ValidIp(addr))
        {
            if (GCONFIG_SetAccessibleIP(temp8, addr) < 0)
                return -1;
        }
        else
            return -1;
    }
    else
        return -1;

    return 1;

} /* End of clicmd_SetAccessibleIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetAccessibleIPHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetAccessibleIPHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: accessip <index> <ip addr>\r\n");
    CONSOLE_PutMessage(pInst, "       <index>:   index of accessible IP\r\n");
    CONSOLE_PutMessage(pInst, "       <ip addr>: accessible IP address\r\n");

    return 1;
} /* End of clicmd_SetAccessibleIPHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_ConfigAccessibleIP()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_ConfigAccessibleIP(CONSOLE_Inst *pInst)
{
    U16_T temp16;
    U8_T temp8, *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    temp16 = GCONFIG_GetNetwork();
    if (argc == 0) /* current setting */
    {
        temp16 &= GCONFIG_NETWORK_ACC_IP_ENABLE;
        if (temp16)
            CONSOLE_PutMessage(pInst, "Accessible IP Mode: Enable\r\n");
        else
            CONSOLE_PutMessage(pInst, "Accessible IP Mode: Disable\r\n");

        return 1;
    }
    else if (argc > 1)
        return -1;
    
    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
        temp8 = *argP - 0x30;
    else
        return -1;

    temp16 &= ~GCONFIG_NETWORK_ACC_IP_ENABLE;
    if (temp8)
        temp16 |= GCONFIG_NETWORK_ACC_IP_ENABLE;

    GCONFIG_SetNetwork(temp16);

    return 1;
} /* End of clicmd_ConfigAccessibleIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_ConfigAccessibleIPHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_ConfigAccessibleIPHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setaccip <mode>\r\n");
    CONSOLE_PutMessage(pInst, "       <mode>:  0: disable    1: enable\r\n");
    
    return 1;
} /* End of clicmd_ConfigAccessibleIPHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailServerAddr()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailServerAddr(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    argv = pInst->Argv;
	
    if (argc == 0) /* current setting */
    {
		GCONFIG_GetSMTPDomainName((U8_T*)StrBuf);
 		CONSOLE_PutMessage(pInst, "Current e-mail server address: %s\r\n", StrBuf);		
        return 1;
    }
    else if (argc > 1)
        return -1;
		GCONFIG_SetSMTPDomainName(argv[0], strlen(argv[0]));
    return 1;
} /* End of clicmd_SetEmailServerAddr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailServerAddrHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailServerAddrHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setems <e-mail server domain name>\r\n");
    return 1;
} /* End of clicmd_SetEmailServerAddrHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailFromAddr()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailFromAddr(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    argv = pInst->Argv;
	
    if (argc == 0) /* current setting */
    {
		GCONFIG_GetSMTPFrom((U8_T*)StrBuf);
 		CONSOLE_PutMessage(pInst, "Current e-mail from address: %s\r\n", StrBuf);		
        return 1;
    }
    else if (argc > 1)
        return -1;
		GCONFIG_SetSMTPFrom(argv[0], strlen(argv[0]));
    return 1;
} /* End of clicmd_SetEmailFromAddr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailFromAddrHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailFromAddrHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setemf <e-mail address>\r\n");
    return 1;
} /* End of clicmd_SetEmailFromAddrHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailTo1Addr()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailTo1Addr(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    argv = pInst->Argv;
	
    if (argc == 0) /* current setting */
    {
		GCONFIG_GetSMTPTo1((U8_T*)StrBuf);
 		CONSOLE_PutMessage(pInst, "Current e-mail to1 address: %s\r\n", StrBuf);		
        return 1;
    }
    else if (argc > 1)
        return -1;
		GCONFIG_SetSMTPTo1(argv[0], strlen(argv[0]));
    return 1;
} /* End of clicmd_SetEmailTo1Addr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailTo1AddrHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailTo1AddrHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setemt1 <e-mail address>\r\n");
    return 1;
} /* End of clicmd_SetEmailTo1AddrHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailTo2Addr()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailTo2Addr(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    argv = pInst->Argv;
	
    if (argc == 0) /* current setting */
    {
		GCONFIG_GetSMTPTo2((U8_T*)StrBuf);
 		CONSOLE_PutMessage(pInst, "Current e-mail to2 address: %s\r\n", StrBuf);		
        return 1;
    }
    else if (argc > 1)
        return -1;
		GCONFIG_SetSMTPTo2(argv[0], strlen(argv[0]));
    return 1;
} /* End of clicmd_SetEmailTo2Addr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailTo2AddrHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailTo2AddrHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setemt2 <e-mail address>\r\n");
    return 1;
} /* End of clicmd_SetEmailTo2AddrHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailTo3Addr()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailTo3Addr(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    argv = pInst->Argv;
	
    if (argc == 0) /* current setting */
    {
		GCONFIG_GetSMTPTo3((U8_T*)StrBuf);
 		CONSOLE_PutMessage(pInst, "Current e-mail to3 address: %s\r\n", StrBuf);		
        return 1;
    }
    else if (argc > 1)
        return -1;
		GCONFIG_SetSMTPTo3(argv[0], strlen(argv[0]));
    return 1;
} /* End of clicmd_SetEmailTo3Addr() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEmailTo3AddrHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEmailTo3AddrHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setemt3 <e-mail address>\r\n");
    return 1;
} /* End of clicmd_SetEmailTo3AddrHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_GetEmailconfig()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_GetEmailConfig(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    if (argc == 0)
    {
		GCONFIG_GetSMTPDomainName((U8_T*)StrBuf);
 		CONSOLE_PutMessage(pInst, "Current e-mail server address: %s\r\n", StrBuf);
		GCONFIG_GetSMTPFrom((U8_T*)StrBuf);		
 		CONSOLE_PutMessage(pInst, "Current e-mail from address: %s\r\n", StrBuf);
		GCONFIG_GetSMTPTo1((U8_T*)StrBuf);		
 		CONSOLE_PutMessage(pInst, "Current e-mail To1 address: %s\r\n", StrBuf);
		GCONFIG_GetSMTPTo2((U8_T*)StrBuf);		
 		CONSOLE_PutMessage(pInst, "Current e-mail To2 address: %s\r\n", StrBuf);
		GCONFIG_GetSMTPTo3((U8_T*)StrBuf);
 		CONSOLE_PutMessage(pInst, "Current e-mail To3 address: %s\r\n", StrBuf);
    }

    return 1;

} /* End of clicmd_GetEmailconfig() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_GetEmailconfigHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_GetEmailConfigHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: emconfig\r\n");

    return 1;
} /* End of clicmd_GetEmailconfigHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetAWconfig()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetAWConfig(CONSOLE_Inst *pInst)
{
    S16_T temp16;
    U8_T temp8, *argP;
    
    argc = pInst->Argc;
    argv = pInst->Argv;

    temp16 = GCONFIG_GetAutoWarning();
    if (argc == 0) /* current setting */
    {
        	CONSOLE_PutMessage(pInst,"  Auto Warning Settings:\r\n");
		if (temp16 & GCONFIG_SMTP_EVENT_COLDSTART) 
			CONSOLE_PutMessage(pInst,"  Cold start auto warning:           Enable\r\n");
		else
			CONSOLE_PutMessage(pInst,"  Cold start auto warning:           Disable\r\n");
			
		if (temp16 & GCONFIG_SMTP_EVENT_AUTH_FAIL) 
			CONSOLE_PutMessage(pInst,"  Authentication fail auto warning:  Enable\r\n");
		else
			CONSOLE_PutMessage(pInst,"  Authentication fail auto warning:  Disable\r\n");
			
		if (temp16 & GCONFIG_SMTP_EVENT_IP_CHANGED) 
			CONSOLE_PutMessage(pInst,"  Local IP changed auto warning:     Enable\r\n");
		else
			CONSOLE_PutMessage(pInst,"  Local IP changed auto warning:     Disable\r\n");
			
		if (temp16 & GCONFIG_SMTP_EVENT_PSW_CHANGED) 
			CONSOLE_PutMessage(pInst,"  Password changed auto warning:     Enable\r\n");
		else
			CONSOLE_PutMessage(pInst,"  Password changed auto warning:     Disable\r\n");

        return 1;
    }
    else if (argc != 4)
        return -1;

    /* cold start */
    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0xFFFE;
        temp16 |= temp8;
    }

    /* authentication fail */
    argP = argv[1];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0xFFFB;
        temp16 |= (temp8 << 2);
    }

    /* IP changed */
    argP = argv[2];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0xFFF7;
        temp16 |= (temp8 << 3);
    }
    
    /* password changed */
    argP = argv[3];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        temp16 &= 0xFFEF;
        temp16 |= (temp8 << 4);
    }

    GCONFIG_SetAutoWarning(temp16);

    return 1;

} /* End of clicmd_SetAWConfig() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetAWConfigHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetAWConfigHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setaw <cold start> <authentication fail> <ip changed> <password changed>\r\n");
    CONSOLE_PutMessage(pInst, "       <cold start>:             0: Disable     1: Enable\r\n");
    CONSOLE_PutMessage(pInst, "       <authentication fail>:    0: Disable     1: Enable\r\n");
    CONSOLE_PutMessage(pInst, "       <ip changed>:             0: Disable     1: Enable\r\n");
    CONSOLE_PutMessage(pInst, "       <password changed>:       0: Disable     1: Enable\r\n");
    return 1;
} /* End of clicmd_SetAWConfigHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetRs485Mode()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetRs485Mode(CONSOLE_Inst *pInst)
{
    U8_T temp8, *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    temp8 = GCONFIG_GetRs485Mode();
    if (argc == 0) /* current setting */
    {
        CONSOLE_PutMessage(pInst, "RS 485 mode %bu \r\n", temp8);
        return 1;
    }
    else if (argc != 1)
        return -1;
    
    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
        temp8 = *argP - 0x30;
    else
        return -1;

    if (temp8 > 3)
        return -1;

    GCONFIG_SetRs485Mode(temp8);

    return 1;

} /* End of clicmd_SetRs485Mode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetRs485ModeHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetRs485ModeHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: rs485 <mode>\r\n");
    CONSOLE_PutMessage(pInst, "       <mode>:   0: Sleep                    2: Double Twisted Pair FD (Slave)\r\n");
    CONSOLE_PutMessage(pInst, "       <mode>:   1: Single Twisted Pair HD   3: Double Twisted Pair FD (Master)\r\n");

    return 1;
} /* End of clicmd_SetRs485ModeHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDestHostName()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDestHostName(CONSOLE_Inst *pInst)
{
    U32_T addr;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) /* current setting */
    {
		GCONFIG_GetDestHostName((U8_T*)StrBuf);
 		CONSOLE_PutMessage(pInst, "Current destination host name: %s\r\n", StrBuf);		
        return 1;
    }
    else if (argc > 1)
        return -1;
	
    if (clicmd_CheckIpInput(argv[0]))
    {
        if (clicmd_GetIp2Ulong(argv[0], strlen(argv[0]), &addr) < 0)
            return -1;

        if (clicmd_ValidIp(addr) == 0)
            return -1;
    }

    GCONFIG_SetDestHostName(argv[0], strlen(argv[0]));

    return 1;
} /* End of clicmd_SetDestHostName() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDestHostNameHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDestHostNameHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setdsthn <Host name/IP>\r\n");
    return 1;
} /* End of clicmd_SetDestHostNameHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetTftpServerIp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetTftpServerIp(CONSOLE_Inst *pInst)
{
    U32_T addr;

    argc = pInst->Argc;
    argv = pInst->Argv;
    if (argc == 0) /* current setting */
    {
        addr = GCONFIG_GetTftpServerIp();
        CONSOLE_PutMessage(pInst, "TFTP Server IP address: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        return 1;
    }
    else if (argc > 1)
        return -1;

    if (clicmd_GetIp2Ulong(argv[0], strlen(argv[0]), &addr) > 0)
    {
        if (clicmd_ValidIp(addr))
            GCONFIG_SetTftpServerIp(addr);
        else
            return -1;
    }
    else
        return -1;

    return 1;
} /* End of clicmd_SetTftpServerIp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetTftpServerIpHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetTftpServerIpHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: tftpsrv <ip addr>\r\n");
    return 1;
} /* End of clicmd_SetTftpServerIpHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetFilename()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetFilename(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    argv = pInst->Argv;
	
    if (argc == 0) /* current setting */
    {
		GCONFIG_GetFilename((U8_T*)StrBuf, 64);
 		CONSOLE_PutMessage(pInst, "Filename: %s\r\n", StrBuf);		
        return 1;
    }
    else if (argc > 1)
        return -1;
	
    GCONFIG_SetFilename(argv[0], strlen(argv[0]));
    return 1;

} /* End of clicmd_SetFilename() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetFilenameHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetFilenameHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: filename <file name>\r\n");
    return 1;
} /* End of clicmd_SetFilenameHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_UpdateFirmware()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_UpdateFirmware(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;

    if (argc == 0) /* current setting */
    {
        CONSOLE_PutMessage(pInst, "Download Firmware\r\n");
        GCONFIG_EnableFirmwareUpgrade();
        GCONFIG_WriteConfigData();
        MCPU_SoftReboot();
        return 1;
    }

    return 1;
} /* End of clicmd_UpdateFirmware() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_UpdateFirmwareHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_UpdateFirmwareHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: dlfirmware\r\n");
    return 1;
} /* End of clicmd_UpdateFirmwareHelp() */


/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEEP()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEEP(CONSOLE_Inst *pInst)
{
	U8_T 	CntGrp, Value, DigNum;
	U16_T	RegLen = 0;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) // current setting //
    {
		CONSOLE_PutMessage(pInst,"Current EEPROM: \r\n");
		CONSOLE_PutMessage(pInst,"     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
		for (RegLen = 0 ; RegLen <= 0x1FF ; RegLen ++)
		{
			if (!(RegLen & 0x0F))
			{
				CONSOLE_PutMessage(pInst,"\r\n%03x: ", RegLen);
			}
			
			EEPROM_Read(RegLen, 1, &DigNum);
			CONSOLE_PutMessage(pInst,"%02bx ", DigNum);
		}
		CONSOLE_PutMessage(pInst,"\r\n");
        return 1;
    }
    else if (argc < 2)
        return -1;

	for (CntGrp = 0; CntGrp < argc; CntGrp ++)
	{
		Value = 0;
		if (strlen(argv[CntGrp]) > 2)  return -1;// Check hex digit length //
				
		for (DigNum = 0 ; DigNum < strlen(argv[CntGrp]) ; DigNum ++)
		{
			if ((argv[CntGrp][DigNum] < 0x3a && argv[CntGrp][DigNum] > 0x2f) ? 1 : 0)// Check Digit //
				Value = (Value << 4) + (argv[CntGrp][DigNum] - 0x30);
			else if ((argv[CntGrp][DigNum] < 0x47 && argv[CntGrp][DigNum] > 0x40) ? 1 : 0)// Check upper case character //
				Value = (Value << 4) + (argv[CntGrp][DigNum] - 0x37);
			else if ((argv[CntGrp][DigNum] < 0x67 && argv[CntGrp][DigNum] > 0x60) ? 1 : 0)// Check lower case character //
				Value = (Value << 4) + (argv[CntGrp][DigNum] - 0x57);
			else
				return -1;
		}
		StrBuf[CntGrp] = Value;
		if (CntGrp)
		{
			RegLen ++;
			CONSOLE_PutMessage(pInst, "0x%02bx-", StrBuf[CntGrp]);
		}
		else
			CONSOLE_PutMessage(pInst, "RegAddr:0x%02bx|RegBuf:", StrBuf[CntGrp]);
	}
	CONSOLE_PutMessage(pInst, "|RegLen:%04u\n\r", RegLen);
	EEPROM_BatchWrite(StrBuf[0], RegLen, (U8_T*)&(StrBuf[1]));

    return 1;
} /* End of clicmd_SetEEP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEEPHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEEPHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: seteep <HEX RegStartAddr> <HEX Byte 0> <HEX Byte 1>...<HEX Byte N>\r\n");
    return 1;
} /* End of clicmd_SetEEPHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_DebugMsg()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_DebugMsg(CONSOLE_Inst *pInst)
{
    U8_T *argP, temp8;
    
    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) /* current setting */
    {
		CONSOLE_PutMessage(pInst,"Display Debug Message: ");
        if (DEBUG_MsgEnable)
		    CONSOLE_PutMessage(pInst,"Enable\r\n");
        else
            CONSOLE_PutMessage(pInst,"Disable\r\n");

        return 1;
    }
    else if (argc > 1)
        return -1;

   argP = argv[0];
   if ((strlen(argP) == 1) && IsDigit(*argP))
        temp8 = *argP - 0x30;
   else
        return -1;

    if (temp8 > 1)
        return -1;

    DEBUG_MsgEnable = temp8;

    return 1;	
} /* End of clicmd_DebugMsg() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_DebugMsgHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_DebugMsgHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: dbgmsg <mode>\r\n");
    CONSOLE_PutMessage(pInst, "       <mode>: 0: Disable    1: Enable\r\n");
    return 1;
} /* End of clicmd_DebugMsgHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_ConnectStatus()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_ConnectStatus(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;

    if (argc != 0) /* current setting */
        return -1;

    CONSOLE_PutMessage(pInst,"Connect Status: ");
    if (GS2W_GetTaskState() == GS2W_STATE_IDLE)
	    CONSOLE_PutMessage(pInst,"Idle\r\n");
    else
        CONSOLE_PutMessage(pInst,"Connected\r\n");

    CONSOLE_PutMessage(pInst,"TCP Tx Flag: %bu\r\n", GTCPDAT_GetTcpTxReady(0));
    return 1;

} /* End of clicmd_DebugMsg() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_ConnectStatusHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_ConnectStatusHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: connstatus\r\n");
    
    return 1;
} /* End of clicmd_ConnectStatus() */


/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_Ping()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_Ping(CONSOLE_Inst *pInst)
{
    S16_T argc = pInst->Argc;
    S8_T **argv = pInst->Argv;

	tsPING_CONFIG_SET *pPing;
	pPing = (tsPING_CONFIG_SET*)&(pInst->Cmd.Buf);
	
	if (pInst->State == CLI_STATE_COMMAND)	
	{
 		if (argc != 1)
        	return -1;
    	if (clicmd_GetIp2Ulong(argv[0], strlen(argv[0]), &(pPing->TargetIP)) < 0)
		{
			CONSOLE_PutMessage(pInst, "Invalid host address \n\r");
			return -1;
		}
		pPing->PatternLen = 32;// Set test pattern length //
		pPing->TxCount = 0;
		pPing->RxCount = 0;
		CONSOLE_PutMessage(pInst, "Pinging from %bu.%bu.%bu.%bu: with %u bytes of data:\n\r\n\r",
									pInst->Cmd.Buf[0], pInst->Cmd.Buf[1], pInst->Cmd.Buf[2], pInst->Cmd.Buf[3],
									pPing->PatternLen);
		PING_CmdEnQueue((tsPING_LINK*)&(pInst->Cmd));									
		pInst->State = CLI_STATE_COMMAND_WAIT;
	}
	else if (pInst->State == CLI_STATE_COMMAND_WAIT)
	{
		if (pInst->Cmd.WaitTime)
			return 1;
		if (pInst->Cmd.ReplyLen)
		{
			CONSOLE_PutMessage(pInst, "Reply from %bu.%bu.%bu.%bu: bytes=%u time<%u0ms TTL=%bu\n\r",
										pInst->Cmd.Buf[0], pInst->Cmd.Buf[1], pInst->Cmd.Buf[2], pInst->Cmd.Buf[3],
										pPing->PatternLen, pPing->EchoTime+1, pPing->TTL);
		}
		else
			CONSOLE_PutMessage(pInst, "Request timed out\n\r");
			
		PING_CmdDeQueue((tsPING_LINK*)&(pInst->Cmd));
		
		pPing->TxCount ++;
		if (pPing->TxCount < 4)
			PING_CmdEnQueue((tsPING_LINK*)&(pInst->Cmd));
		else
		{
			CONSOLE_PutMessage(pInst, "\n\rPing statistics for %bu.%bu.%bu.%bu:\n\r",
										pInst->Cmd.Buf[0], pInst->Cmd.Buf[1], pInst->Cmd.Buf[2], pInst->Cmd.Buf[3]);
			CONSOLE_PutMessage(pInst, "     Packets:Sent=%bu, Received=%bu, lost=%bu\n\r\n\r", 
										pPing->TxCount, pPing->RxCount, pPing->TxCount - pPing->RxCount);
			pInst->State = CLI_STATE_COMMAND;
		}
	}
	return 1;	
} /* End of clicmd_Ping() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_PingHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_PingHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: ping xxx.xxx.xxx.xxx \r\n");
    return 1;
} /* End of clicmd_PingHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDefault()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDefault(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;

    if (argc == 0)
    {
		GCONFIG_ReadDefaultConfigData();
		GCONFIG_WriteConfigData();
		DELAY_Ms(500);
		MCPU_SoftReboot();	
		return 1;
    }
    return -1;

} /* End of clicmd_SetDefault() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDefaultHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDefaultHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setdef\r\n");
    
    return 1;
} /* End of clicmd_SetDefaultHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetBMCast()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetBMCast(CONSOLE_Inst *pInst)
{
    U16_T temp16;
    U8_T temp8, *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    temp16 = GCONFIG_GetNetwork();
    if (argc == 0)
    {
        CONSOLE_PutMessage(pInst, "Bcast: ");
		if (temp16 & GCONFIG_NETWORK_PROTO_UDP_BCAST)
            CONSOLE_PutMessage(pInst, "On ");
		else 
            CONSOLE_PutMessage(pInst, "Off ");

        CONSOLE_PutMessage(pInst, "\r\nMcast: ");
		if (temp16 & GCONFIG_NETWORK_PROTO_UDP_MCAST)
            CONSOLE_PutMessage(pInst, "On\r\n");
		else 
            CONSOLE_PutMessage(pInst, "Off\r\n");

		return 1;
    }
    else if (argc != 2)
        return -1;

    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        if (temp8 > 1)
            return -1;
    }
    else
        return -1;

    if (temp8)
        temp16 |= GCONFIG_NETWORK_PROTO_UDP_BCAST;
    else
        temp16 &= ~GCONFIG_NETWORK_PROTO_UDP_BCAST;

    argP = argv[1];
    if ((strlen(argP) == 1) && IsDigit(*argP))
    {
        temp8 = *argP - 0x30;
        if (temp8 > 1)
            return -1;
    }
    else
        return -1;

   if (temp8)
        temp16 |= GCONFIG_NETWORK_PROTO_UDP_MCAST;
    else
        temp16 &= ~GCONFIG_NETWORK_PROTO_UDP_MCAST;

    GCONFIG_SetNetwork(temp16);

    return 1;

} /* End of clicmd_SetBMCast() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetBMCastHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetBMCastHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: bmcast <bcast> <mcast>\r\n");
    CONSOLE_PutMessage(pInst, "      <bcast>: 0: Off    1: On\r\n");
    CONSOLE_PutMessage(pInst, "      <mcast>: 0: Off    1: On\r\n");
    
    return 1;
} /* End of clicmd_SetBMCastHelp() */

#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetSSID()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetSSID(CONSOLE_Inst *pInst)
{
	U8_T	RegLen;

	argc = pInst->Argc;
    argv = pInst->Argv;

	if (argc == 0) // current setting //
   	{
       	RegLen = GCONFIG_GetSsid(StrBuf);
       	StrBuf[RegLen] = 0;
       	CONSOLE_PutMessage(pInst, "SSID: %s\r\n", StrBuf);
		return 1;
	}
	else if (argc > 1)
	{
		return -1;
	}

	// Check maximum 32 characters //
	if ((RegLen = strlen(argv[0])) >= 32)
	{
		return -1;
	}

	GCONFIG_SetSsid(argv[0], RegLen);
	IO_SsidRW(1, RegLen, (U8_T XDATA*)argv[0]);
	return 1;
	
} /* End of clicmd_SetSSID() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetSSIDHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetSSIDHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setssid <Maximum 32 characters>\r\n");
    return 1;

} /* End of clicmd_SetSSIDHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetNetworkType()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetNetworkType(CONSOLE_Inst *pInst)
{
	U8_T value;

	argc = pInst->Argc;
    argv = pInst->Argv;

	if (argc == 0) /* current setting */
    {
        value = GCONFIG_GetWifiNetworkMode();
        CONSOLE_PutMessage(pInst, "Network Type: ");
        if (value == 1)
            CONSOLE_PutMessage(pInst, "Ad-hoc mode\r\n");
        else if (value == 0)
            CONSOLE_PutMessage(pInst, "Infra mode\r\n");

		return 1;
	}
	else if (argc > 1 || strlen(argv[0]) > 1)
	{
		return -1;
	}

    value =  *argv[0] - 0x30;
    if (value < 2)
    {
        GCONFIG_SetWifiNetworkMode(value);
		IO_NetworkModeRW(1, value ? 2:3);
	}
	else
		return -1;

	return 1;

} /* End of clicmd_SetNetworkType() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetNetworkTypeHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetNetworkTypeHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setnt <0 = Infra mode, 1 = Ad-hoc mode>\r\n");
    return 1;

} /* End of clicmd_SetNetworkTypeHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetChannel()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetChannel(CONSOLE_Inst *pInst)
{
	U8_T RegLen, Value;

	argc = pInst->Argc;
    argv = pInst->Argv;

	if (argc == 0) /* current setting */
    {
        RegLen = GCONFIG_GetChannel();
        if (RegLen)
           CONSOLE_PutMessage(pInst, "Channel(%bu): %bu\r\n", RegLen, IO_WiFiSupportedCh[RegLen]);
        else
           CONSOLE_PutMessage(pInst, "Channel(%bu): auto\r\n", RegLen);
		return 1;
	}
	else if (argc > 1 || (RegLen = strlen(argv[0])) > 3)
	{
		return -1;
	}

	if (clicmd_DecText2Char((U8_T *)argv[0], &Value, RegLen) == 0xFF)
		return -1;
    else if (Value > 11)
        return -1;

    GCONFIG_SetChannel(Value);
	IO_ChannelRW(1, IO_WiFiSupportedCh[Value]);
	return 1;

} /* End of clicmd_SetChannel() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetChannelHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetChannelHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setch <channel index>\r\n");
    CONSOLE_PutMessage(pInst, "     <channel index>\r\n");
    CONSOLE_PutMessage(pInst, "     0: Auto 1:  1   2:  2   3:  3   4:  4   5:  5   6:  6\r\n");
    CONSOLE_PutMessage(pInst, "     7:  7   8:  8   9:  9  10: 10  11: 11\r\n");
// Support 802.11 b/g only
//    CONSOLE_PutMessage(pInst, "     7:  7   8:  8   9:  9  10: 10  11: 11  12: 36  13: 40\r\n");
//    CONSOLE_PutMessage(pInst, "    14: 44  15: 48  16:149  17:153  18:157  19:161  20:165\r\n");

    return 1;

} /* End of clicmd_SetChannelHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWep64Key()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWep64Key(CONSOLE_Inst *pInst)
{
	U8_T i, j, Value[5];

	argc = pInst->Argc;
    argv = pInst->Argv;

	if (argc == 1 && strlen(argv[0]) == 1) /* current setting */
    {
		if (clicmd_DecText2Char(argv[0], &j, 1) == 0xFF ||	j > 3)
		{
			return -1;
		}

        GCONFIG_GetWifiWep64Key(j, Value);
        CONSOLE_PutMessage(pInst, "WEP 64 Key %bu: ", j);
        for (i = 0; i < 5; i++)
        {
            CONSOLE_PutMessage(pInst, "%bx ", Value[i]);
        }
        CONSOLE_PutMessage(pInst, "\r\n");
        return 1;
	}
	else if (argc != 6)
	{
		return -1;
	}

	if (strlen(argv[0]) == 1)
	{
		if (clicmd_DecText2Char(argv[0], &j, 1) == 0xFF ||	j > 3)
		{
			return -1;
		}

		for (i = 0; i < 5; i++)
		{
			if (clicmd_HexText2Char(argv[i+1], &Value[i], 2) == 0xFF)
			{
				return -1;
			}
		}
        GCONFIG_SetWifiWep64Key(j, Value);
		IO_Wep64KeyRW (1, j, Value);
	}
	else
	{
		return -1;
	}

	return 1;

} /* End of clicmd_SetWep64Key() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWep64KeyHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWep64KeyHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setw64k <index = 0 ~ 3> <HEX Byte 0> <HEX Byte 1> ... <HEX Byte 4>\r\n");
    return 1;

} /* End of clicmd_SetWep64KeyHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWep128Key()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWep128Key(CONSOLE_Inst *pInst)
{
	U8_T	i, j, Value[13];

	argc = pInst->Argc;
    argv = pInst->Argv;

	if (argc == 1 && strlen(argv[0]) == 1) /* current setting */
    {
		if (clicmd_DecText2Char(argv[0], &j, 1) == 0xFF || j > 3)
		{
			return -1;
		}
        GCONFIG_GetWifiWep128Key(j, Value);
        CONSOLE_PutMessage(pInst, "WEP 128 Key %bu: ", j);
        for (i = 0; i < 13; i++)
        {
            CONSOLE_PutMessage(pInst, "%bx ", Value[i]);
        }
        CONSOLE_PutMessage(pInst, "\r\n");

        return 1;
	}
	else if (argc != 14)
	{
		return -1;
	}

	if (strlen(argv[0]) == 1)
	{
		if (clicmd_DecText2Char(argv[0], &j, 1) == 0xFF || j > 3)
		{
			return -1;
		}

		for (i = 0; i < 13; i++)
		{
			if (clicmd_HexText2Char(argv[i+1], &Value[i], 2) == 0xFF)
			{
				return -1;
			}
		}

        GCONFIG_SetWifiWep128Key(j, Value);
	IO_Wep128KeyRW (1, j, Value);	
	}
	else
	{
		return -1;
	}

	return 1;

} /* End of clicmd_SetWep128Key() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWep128KeyHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWep128KeyHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setw128k <index = 0 ~ 3> <HEX Byte 0> <HEX Byte 1> ... <HEX Byte 12>\r\n");
    return 1;

} /* End of clicmd_SetWep128KeyHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWepKeyIndex()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWepKeyIndex(CONSOLE_Inst *pInst)
{
	U8_T	Value;

	argc = pInst->Argc;
    argv = pInst->Argv;
	if (argc == 0) /* Read current WEP key index */
    {
        CONSOLE_PutMessage(pInst, "WEP Key Index: %bu\r\n", GCONFIG_GetWifiWepKeyIndex());
        return 1;
	}
	else if (argc != 1 || strlen(argv[0]) != 1) /* Write user's WEP key index */
        return -1;

	if (clicmd_DecText2Char(argv[0], &Value, 1) == 0xFF)
	{
		return -1;
	}
	if (Value < 4)
	{
        GCONFIG_SetWifiWepKeyIndex(Value);
	IO_WepKeyIndexRW(1, Value);
	}
	else
	{
		return -1;
	}

	return 1;

} /* End of clicmd_SetWepKeyIndex() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWepKeyIndexHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWepKeyIndexHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setwki <WEP key index = 0 ~ 3>\r\n");
    return 1;

} /* End of clicmd_SetWepKeyIndexHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWPAPassphrase()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWPAPassphrase(CONSOLE_Inst *pInst)
{
	U8_T	RegLen;

	argc = pInst->Argc;
    argv = pInst->Argv;

	if (argc == 0) /* current setting */
    {
        RegLen = GCONFIG_GetWifiPreShareKey(StrBuf);
        StrBuf[RegLen] = 0;
        CONSOLE_PutMessage(pInst, "Pre-Shared Key: %s\r\n", StrBuf);
        return 1;
	}
	else if (argc != 1)
	{
		return -1;
	}

	RegLen = strlen(argv[0]);
	if (RegLen >= 8 && RegLen <= 63)
	{
        GCONFIG_SetWifiPreShareKey(argv[0], RegLen);
	IO_TkipAesRW(1, RegLen, (U8_T *)argv[0]);
	}
	else
	{
		return -1;
	}

	return 1;

} /* End of clicmd_SetWPAPassphrase() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWPAPassphraseHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWPAPassphraseHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setwp <WPA Passphrase, 8 ~ 63 characters>\r\n");
    return 1;

} /* End of clicmd_SetWPAPassphraseHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEncryptionType()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEncryptionType(CONSOLE_Inst *pInst)
{
	U8_T	Value;

	argc = pInst->Argc;
    argv = pInst->Argv;

	if (argc == 0) /* current setting */
    {
        Value = GCONFIG_GetWifiEncryptMode();
        CONSOLE_PutMessage(pInst, "Encryption Type: ");
        switch(Value)
        {
            case 0:
                CONSOLE_PutMessage(pInst, "No Secuirty");
                break;
            case 1:
                CONSOLE_PutMessage(pInst, "WEP 64 bits");
                break;
            case 2:
                CONSOLE_PutMessage(pInst, "WEP 128 bits");
                break;
            case 3:
                CONSOLE_PutMessage(pInst, "TKIP");
                break;
            case 4:
                CONSOLE_PutMessage(pInst, "AES");
                break;

        }
        CONSOLE_PutMessage(pInst, "\r\n");
        return 1;
	}
	else if (argc != 1 || strlen(argv[0]) != 1)
		return -1;

	Value = *argv[0] - 0x30;
    if (Value > 4)
        return -1;
 
    GCONFIG_SetWifiEncryptMode(Value);
	if (Value == 2)
	{
		IO_OpenSysRW(1, 0); /* open system */
		IO_EncryptionRW(1, 5); /* wep128 */
	}
	else if (Value == 3)
	{
		IO_OpenSysRW(1, 3); /* mixed mode */
		IO_EncryptionRW(1, 2); /* tkip */
	}
	else if (Value == 1) /* do not know current encrypt mode */
	{
		IO_OpenSysRW(1, 0); /* open system */
		IO_EncryptionRW(1, 1); /* wep64 */
	}
	else if (Value == 4)
	{
		IO_OpenSysRW(1, 3); /* mixed mode */
		IO_EncryptionRW(1, 4);
	}
	else /* 0:disable, 1:wep64 */
	{
		IO_OpenSysRW(1, 0); /* open system */
		IO_EncryptionRW(1, Value);
	}
			
	return 1;

} /* End of clicmd_SetEncryptionType() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetEncryptionTypeHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetEncryptionTypeHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setet <Encryption Type>\r\n");
	CONSOLE_PutMessage(pInst, "       <Encryption Type> 0: No Security    3: TKIP\r\n");
	CONSOLE_PutMessage(pInst, "                         1: WEP64          4: AES\r\n");
	CONSOLE_PutMessage(pInst, "                         2: WEP128\r\n");

    return 1;

} /* End of clicmd_SetEncryptionTypeHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SiteSurvey()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SiteSurvey(CONSOLE_Inst *pInst)
{
	MWIFAPP_SITE_SURVEY_REPORT XDATA *pMsg;
	U8_T i, j;
	
	argc = pInst->Argc;

	if (pInst->State == CLI_STATE_COMMAND)
	{
		if (argc == 0) /* Do sitesurvey */
    	{
			pInst->CmdId = 0;			
			pInst->Cmd.Buf[0] = IO_SITE_SURVEY;
			pInst->Cmd.Buf[1] = ~IO_SITE_SURVEY;			
			pInst->Cmd.Buf[2] = 0;			
			MWIFAPP_TaskCmdEnQueue((MWIFAPP_LINK*)&(pInst->Cmd));
			CONSOLE_PutMessage(pInst, "Please wait...\r\n");
			pInst->State = CLI_STATE_COMMAND_WAIT;
			return 1;
		}
		else
			return -1;
	}
	else if (pInst->State == CLI_STATE_COMMAND_WAIT)
	{
		if (pInst->Cmd.ReplyLen && (pInst->Cmd.Buf[0] + pInst->Cmd.Buf[1]) == 0xFF &&
			pInst->Cmd.Buf[0] == IO_SITE_SURVEY_REPORT)		
		{
			if (!pInst->Cmd.Buf[2])
			{
				CONSOLE_PutMessage(pInst, "Can not found any BSS\r\n");
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				pInst->State = CLI_STATE_COMMAND;
				return 1;
			}
			if (!pInst->CmdId)
			{
				CONSOLE_PutMessage(pInst, "INDEX "							//6 digits //
										  "BSSID             "				//18 digits //
										  "SSID                          "	//30 digits //
										  "TYPE "	//5 digits //
										  "CH  "	//4 digits //
										  "RSSI "	//5 digits //
										  "SECURITY"//8 digits //
									  	  "\r\n");						
			}
							
			// Update site survey table //
			if (pInst->CmdId >= MWIFAPP_MAX_SITESURVEY_TABLE_MEMBER)
			{
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));		
				pInst->State = CLI_STATE_COMMAND;
				if (pInst->CmdId)
					CONSOLE_PutMessage(pInst, "Site survey table full!\r\n");
				return 1;			
			}
					
			pMsg = 	(MWIFAPP_SITE_SURVEY_REPORT XDATA*)&(pInst->Cmd.Buf[3]);
			MWIFAPP_SiteSurveyTable[pInst->CmdId].Index = pMsg->Index;				
			memcpy(MWIFAPP_SiteSurveyTable[pInst->CmdId].BSSID, pMsg->BSSID, 6);
			MWIFAPP_SiteSurveyTable[pInst->CmdId].Type = pMsg->Type;				
			MWIFAPP_SiteSurveyTable[pInst->CmdId].Channel = pMsg->Channel;
			MWIFAPP_SiteSurveyTable[pInst->CmdId].RSSI = pMsg->RSSI;				
			MWIFAPP_SiteSurveyTable[pInst->CmdId].Security = pMsg->Security;
			if ((pInst->Cmd.Buf[2] - 12) > (MWIFAPP_MAX_SSID_LENGTH - 1))
				i = MWIFAPP_MAX_SSID_LENGTH - 1;
			else
				i = pInst->Cmd.Buf[2] - 12;
			memcpy(MWIFAPP_SiteSurveyTable[pInst->CmdId].SSID, pMsg->SSID, i);
			MWIFAPP_SiteSurveyTable[pInst->CmdId].SSID[i] = '\0';
			
			// Display site survey table //
			CONSOLE_PutMessage(pInst, "%02bu    "	//6 digits //
									  "%02bx-%02bx-%02bx-%02bx-%02bx-%02bx ",//18 digits //
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].Index,
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].BSSID[0],
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].BSSID[1],
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].BSSID[2], 
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].BSSID[3], 
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].BSSID[4], 
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].BSSID[5]);
			for (j = 0; j < 30; j ++)						  
			{
				if (j < i)
					CONSOLE_PutMessage(pInst, "%c", MWIFAPP_SiteSurveyTable[pInst->CmdId].SSID[j]);//30 digits //
				else
					CONSOLE_PutMessage(pInst, " ");
			}
			CONSOLE_PutMessage(pInst, "%s  "		//5 digits //
									  "%03bu "		//4 digits //
									  "%02bx   "	//5 digits //
									  "%bu|%s"		//8 digits //
									  "\r\n",
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].Type ? "ap ":"sta",
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].Channel,
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].RSSI,
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].Security,
									   MWIFAPP_SiteSurveyTable[pInst->CmdId].Security ? "yes":"no ");
			pInst->CmdId ++;
            IO_SiteSurveyReportAck();
			pInst->Cmd.WaitTime = 5;// Set packet timeout = 5x50(250) ms//
			pInst->Cmd.ReplyLen = 0;											   
			return 1;
		}
		else if (pInst->Cmd.WaitTime == 0)
		{
			MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));		
			pInst->State = CLI_STATE_COMMAND;
			if (pInst->CmdId)
				return 1;
			else
			{
				CONSOLE_PutMessage(pInst, "Command Time out\r\n");		
				return -1;		
			}
		}
		else
			pInst->Cmd.ReplyLen = 0;
	}
	return 1;

} /* End of clicmd_SiteSurvey() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SiteSurveyHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SiteSurveyHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: sisrvy\r\n");
    return 1;

} /* End of clicmd_SiteSurveyHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_JoinBSS()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_JoinBSS(CONSOLE_Inst *pInst)
{
	U8_T	RegLen, Value, i;

	argc = pInst->Argc;
	argv = pInst->Argv;

	if (pInst->State == CLI_STATE_COMMAND)
	{
		if (argc != 1 || (RegLen = strlen(argv[0])) > 2) // Join the selected BSS //
        	return -1;
 
		if (clicmd_DecText2Char(argv[0], &Value, RegLen) == 0xFF)
		{
			return -1;
		}
		if (Value >= MWIFAPP_MAX_SITESURVEY_TABLE_MEMBER)
			return -1;
			
		if (MWIFAPP_SiteSurveyTable[Value].Security == 2)
			IO_OpenSysRW(1,3);
		else
			IO_OpenSysRW(1,0);
		
		if (MWIFAPP_SiteSurveyTable[Value].Security == 0)
			IO_EncryptionRW(1,0);
		else if (MWIFAPP_SiteSurveyTable[Value].Security == 1)
			i = GCONFIG_GetWifiEncryptMode();
			if (i == 2)
				IO_EncryptionRW(1,5);
			else if (i == 3)
				IO_EncryptionRW(1,2);			
			else
				IO_EncryptionRW(1,i);						
			
		IO_WepKeyIndexRW(1,GCONFIG_GetWifiWepKeyIndex());
	
		pInst->Cmd.Buf[0] = IO_JOIN_BSS;
		pInst->Cmd.Buf[1] = ~IO_JOIN_BSS;			
		pInst->Cmd.Buf[2] = 1;			
		pInst->Cmd.Buf[3] = Value;					
		pInst->CmdId = IO_MCPU_WCPU_CONNECT;// save index value //
		MWIFAPP_TaskCmdEnQueue((MWIFAPP_LINK*)&(pInst->Cmd));
		CONSOLE_PutMessage(pInst, "Please wait...\r\n");		
		pInst->State = CLI_STATE_COMMAND_WAIT;
	}
	else if (pInst->State == CLI_STATE_COMMAND_WAIT)
	{
		if (pInst->Cmd.ReplyLen && (pInst->Cmd.Buf[0] + pInst->Cmd.Buf[1]) == 0xFF)
		{
			if (pInst->CmdId == IO_MCPU_WCPU_CONNECT && pInst->Cmd.Buf[0] != IO_MCPU_WCPU_CONNECT)
			{
				pInst->Cmd.ReplyLen	= 0;
				return 1;
			}
			
			switch (pInst->Cmd.Buf[0])
			{
			case (IO_MCPU_WCPU_CONNECT):
				pInst->CmdId = 0;
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				CONSOLE_PutMessage(pInst, "Join BSS successful!\r\n");
				pInst->Cmd.Buf[0] = IO_OPERATE_MODE;
				goto CLICMD_JBSS_BY_READ;
				break;
			case IO_OPERATE_MODE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				CONSOLE_PutMessage(pInst, "Update base band|%bu= ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 1)
				{
					GCONFIG_SetWifiBasebandMode(1);
					CONSOLE_PutMessage(pInst, "802.11b/g mode\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 4)
				{
					GCONFIG_SetWifiBasebandMode(2);
					CONSOLE_PutMessage(pInst, "802.11a mode\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 5)
				{
					CONSOLE_PutMessage(pInst, "802.11a/b/g mode\r\n");		
				}
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");													
				pInst->Cmd.Buf[0] = IO_NETWORK_MODE;
				goto CLICMD_JBSS_BY_READ;				
				break;
			case IO_NETWORK_MODE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				CONSOLE_PutMessage(pInst, "Update network mode|%bu= ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 2)
				{
					GCONFIG_SetWifiNetworkMode(1);
					CONSOLE_PutMessage(pInst, "Ad-hoc\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 3)
				{
					GCONFIG_SetWifiNetworkMode(0);				
					CONSOLE_PutMessage(pInst, "infrastructure\r\n");
				}
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");						
				pInst->Cmd.Buf[0] = IO_CHANNEL;
				goto CLICMD_JBSS_BY_READ;
				break;
			case IO_CHANNEL_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				for (StrLen = 0; StrLen < IO_WiFiSupportedChBufSize; StrLen++)
				{
					if (IO_WiFiSupportedCh[StrLen] == pInst->Cmd.Buf[4])
						break;
				}
				if (StrLen != IO_WiFiSupportedChBufSize)
				{	
    				GCONFIG_SetChannel(StrLen);				
					CONSOLE_PutMessage(pInst, "Update channel= %bu\r\n",pInst->Cmd.Buf[4]);
				}
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
				pInst->Cmd.Buf[0] = IO_SSID_READ;
				goto CLICMD_JBSS_NO_OPTION;
				break;
			case IO_SSID_READ_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				CONSOLE_PutMessage(pInst, "Update ssid");				
				if (pInst->Cmd.Buf[2] > 32)
					CONSOLE_PutMessage(pInst, " length error\r\n");
				else
				{
#if (INCLUDE_DHCP_CLIENT)
					pInst->CmdId = 0xFF;
#endif									
					GCONFIG_SetSsid((U8_T *)&(pInst->Cmd.Buf[3]), pInst->Cmd.Buf[2]);
					pInst->Cmd.Buf[3+pInst->Cmd.Buf[2]] = '\0';
					CONSOLE_PutMessage(pInst, "= %s\r\n", (U8_T *)&(pInst->Cmd.Buf[3]));
					
				}
				pInst->Cmd.Buf[0] = IO_OPEN_SYSTEM;
				goto CLICMD_JBSS_BY_READ;
				break;
			case IO_OPEN_SYSTEM_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));	
				
				CONSOLE_PutMessage(pInst, "Authentication type|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0)
					CONSOLE_PutMessage(pInst, "open system\r\n");
				else if (pInst->Cmd.Buf[4] == 1)
					CONSOLE_PutMessage(pInst, "share key\r\n");
				else if (pInst->Cmd.Buf[4] == 2)
					CONSOLE_PutMessage(pInst, "WPA1_WPA2 only\r\n");
				else if (pInst->Cmd.Buf[4] == 3)
					CONSOLE_PutMessage(pInst, "WPA1_WPA2 mixed\r\n");
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
								
				pInst->Cmd.Buf[0] = IO_ENCRYPTION_TYPE;				
				goto CLICMD_JBSS_BY_READ;
				break;
			case IO_ENCRYPTION_TYPE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				CONSOLE_PutMessage(pInst, "Update encryption type|%bu=",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0x00)
				{
					GCONFIG_SetWifiEncryptMode(0);
					CONSOLE_PutMessage(pInst, "no security\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x01)
				{
					GCONFIG_SetWifiEncryptMode(1);				
					CONSOLE_PutMessage(pInst, "WEP64 bits\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x02)
				{
					GCONFIG_SetWifiEncryptMode(3);				
					CONSOLE_PutMessage(pInst, "TKIP\r\n");				
				}
				else if (pInst->Cmd.Buf[4] == 0x04)
				{
					GCONFIG_SetWifiEncryptMode(4);				
					CONSOLE_PutMessage(pInst, "AES\r\n");				
				}
				else if (pInst->Cmd.Buf[4] == 0x05)
				{
					GCONFIG_SetWifiEncryptMode(2);				
					CONSOLE_PutMessage(pInst, "WEP128 bits\r\n");
				}
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");					
				pInst->Cmd.Buf[0] = IO_TX_DATA_RATE;
				goto CLICMD_JBSS_BY_READ;
				break;
			case IO_TX_DATA_RATE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				CONSOLE_PutMessage(pInst, "Update Tx Rate|%bu= ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0x00)
				{
					GCONFIG_SetWifiTxRate(0);
					CONSOLE_PutMessage(pInst, "Auto\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x02)
				{
					GCONFIG_SetWifiTxRate(1);
					CONSOLE_PutMessage(pInst, "1 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x04)
				{
					GCONFIG_SetWifiTxRate(2);				
					CONSOLE_PutMessage(pInst, "2 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x0B)
				{
					GCONFIG_SetWifiTxRate(3);
					CONSOLE_PutMessage(pInst, "5.5 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x0C)
				{
					GCONFIG_SetWifiTxRate(4);				
					CONSOLE_PutMessage(pInst, "6 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x12)
				{
					GCONFIG_SetWifiTxRate(5);				
					CONSOLE_PutMessage(pInst, "9 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x16)
				{
					GCONFIG_SetWifiTxRate(6);				
					CONSOLE_PutMessage(pInst, "11 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x18)
				{
					GCONFIG_SetWifiTxRate(7);				
					CONSOLE_PutMessage(pInst, "12 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x24)
				{
					GCONFIG_SetWifiTxRate(8);				
					CONSOLE_PutMessage(pInst, "18 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x30)
				{
					GCONFIG_SetWifiTxRate(9);
					CONSOLE_PutMessage(pInst, "24 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x48)
				{
					GCONFIG_SetWifiTxRate(10);				
					CONSOLE_PutMessage(pInst, "36 Mbps\r\n");
				}					
				else if (pInst->Cmd.Buf[4] == 0x60)
				{
					GCONFIG_SetWifiTxRate(11);				
					CONSOLE_PutMessage(pInst, "48 Mbps\r\n");
				}
				else if (pInst->Cmd.Buf[4] == 0x6C)
				{
					GCONFIG_SetWifiTxRate(12);
					CONSOLE_PutMessage(pInst, "54 Mbps\r\n");
				}					
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
				
				GCONFIG_WriteConfigData();
				CONSOLE_PutMessage(pInst, "Save WiFi config to flash ROM\r\n");
                pInst->State = CLI_STATE_COMMAND;
#if (INCLUDE_DHCP_CLIENT)
				if (pInst->CmdId)	
					MAIN_RestartDhcpClient();
#endif				
				break;
			default:
				pInst->Cmd.ReplyLen	= 0;			
				break;
			}
			return 1;
		}
		else if (pInst->Cmd.WaitTime == 0)
		{
			MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));		
			pInst->State = CLI_STATE_COMMAND;
			CONSOLE_PutMessage(pInst, "Join fail!\r\n");		
			return -1;		
		}
		else
			pInst->Cmd.ReplyLen = 0;	
	}
	return 1;

CLICMD_JBSS_NO_OPTION:
	pInst->Cmd.Buf[1] = ~(pInst->Cmd.Buf[0]);
	pInst->Cmd.Buf[2] = 0;
	MWIFAPP_TaskCmdEnQueue((MWIFAPP_LINK*)&(pInst->Cmd));
	return 1;
CLICMD_JBSS_BY_READ:
	pInst->Cmd.Buf[1] = ~(pInst->Cmd.Buf[0]);
	pInst->Cmd.Buf[2] = 1;
	pInst->Cmd.Buf[3] = 0;	
	MWIFAPP_TaskCmdEnQueue((MWIFAPP_LINK*)&(pInst->Cmd));
	return 1;

} /* End of clicmd_JoinBSS() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_JoinBSSHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_JoinBSSHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: jbss <ID in sitesurvey table>\r\n");
    return 1;

} /* End of clicmd_JoinBSSHelp() */

#if 0 /* Support 802.11 b/g only */
/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetOperationMode()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetOperationMode(CONSOLE_Inst *pInst)
{
    U8_T value;

	argc = pInst->Argc;
	argv = pInst->Argv;

	if (argc == 0) /* Read current operation mode */
    {
        value = GCONFIG_GetWifiBasebandMode();
        CONSOLE_PutMessage(pInst, "Operation Mode: ");
        if (value == 1)
            CONSOLE_PutMessage(pInst, "802.11 b/g\r\n");
        else if (value == 2)
            CONSOLE_PutMessage(pInst, "802.11 a\r\n");
        return 1;
	}
	else if (argc != 1 || strlen(argv[0]) != 1)
        return -1;

    value = atoi(argv[0]);
    if (value == 1 || value == 2)
	{
        GCONFIG_SetWifiBasebandMode(value);
		IOCTLM2W_SendOperateMode(WIFI_WRITE, value);
	}
	else
	{
		return -1;
	}

	return 1;

} /* End of clicmd_SetOperationMode() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetOperationModeHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetOperationModeHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setopm <1 = 802.11 b/g mixde mode, 2 = 802.11 a mode>\r\n");
    return 1;

} /* End of clicmd_SetOperationModeHelp() */
#endif
/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWepKeyLength()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWepKeyLength(CONSOLE_Inst *pInst)
{
    U8_T value;

	argc = pInst->Argc;
	argv = pInst->Argv;

	if (argc == 0) /* Read current operation mode */
    {
        value = GCONFIG_GetWifiWepKeyLength();
        CONSOLE_PutMessage(pInst, "WEP key length: ");
        if (value == 0)
            CONSOLE_PutMessage(pInst, "64 bits\r\n");
        else if (value == 1)
            CONSOLE_PutMessage(pInst, "128 bits\r\n");
        return 1;
	}
	else if (argc != 1 || strlen(argv[0]) != 1)
        return -1;

    value = atoi(argv[0]);
    if (value > 1)
        return -1;

    GCONFIG_SetWifiWepKeyLength(value);

	return 1;

} /* End of clicmd_SetWepKeyLength() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetWepKeyLengthHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetWepKeyLengthHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: wepklen <0:64 bits, 1:128 bits>\r\n");
    return 1;

} /* End of clicmd_SetWepKeyLengthHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetPreamble()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetPreamble(CONSOLE_Inst *pInst)
{
    U8_T value;

	argc = pInst->Argc;
	argv = pInst->Argv;

	if (argc == 0) /* Read current preamble */
    {
        value = GCONFIG_GetWifiPreambleMode();
        CONSOLE_PutMessage(pInst, "Preamble: ");
        if (value)
            CONSOLE_PutMessage(pInst, "Auto\r\n");
        else
            CONSOLE_PutMessage(pInst, "Long\r\n");

        return 1;
	}
	else if (argc != 1 || strlen(argv[0]) != 1)
        return -1;

    value = atoi(argv[0]);
    if (value > 1)
        return -1;

    GCONFIG_SetWifiPreambleMode(value);
	IO_PreambleRW(1, value ? 20:0); /* 0/20: long/auto */
	return 1;

} /* End of clicmd_SetPreamble() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetPreambleHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetPreambleHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setpmb <0 = Long preamble, 1 = Auto>\r\n");
    return 1;

} /* End of clicmd_SetPreambleHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetRTSThreshold()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetRTSThreshold(CONSOLE_Inst *pInst)
{
	U8_T	RegLen;
	U16_T	Value;

	argc = pInst->Argc;
	argv = pInst->Argv;

	if (argc == 0) /* Read current RTS threshold */
    {
        Value = GCONFIG_GetWifiRtsThreshold();
        CONSOLE_PutMessage(pInst, "RTS Threahold: %u\r\n", Value);
        return 1;
	}
	else if (argc != 1 || (RegLen = strlen(argv[0])) > 4)
        return -1;

	if (clicmd_DecText2Short(argv[0], &Value, RegLen) == 0xFF)
	{
		return -1;
	}
	if (Value > 2432)
	{
		return -1;
	}

    GCONFIG_SetWifiRtsThreshold(Value);
	IO_RtsThresholdRW(1, (U8_T XDATA*)&Value);
	return 1;

} /* End of clicmd_SetRTSThreshold() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetRTSThresholdHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetRTSThresholdHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setrts <RTS threshold, 0 ~ 2432>\r\n");
    return 1;

} /* End of clicmd_SetRTSThresholdHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetTxPowerLevel()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetTxPowerLevel(CONSOLE_Inst *pInst)
{
	U8_T	Value;

	argc = pInst->Argc;
	argv = pInst->Argv;
	if (argc == 0) /* Read current power level */
    {
        Value = GCONFIG_GetWifiTxPowerLevel();
        CONSOLE_PutMessage(pInst, "Transmit Power Level: ");
        switch(Value)
        {
            case 0:
                CONSOLE_PutMessage(pInst, "100%%");
                break;
            case 1:
                CONSOLE_PutMessage(pInst, "50%%");
                break;
            case 2:
                CONSOLE_PutMessage(pInst, "25%%");
                break;
            case 3:
                CONSOLE_PutMessage(pInst, "12.5%%");
                break;
            case 4:
                CONSOLE_PutMessage(pInst, "6.25%%");
                break;
        }
        CONSOLE_PutMessage(pInst, "\r\n");
        return 1;
	}
	else if (argc != 1 || strlen(argv[0]) != 1)
        return -1;

    /* Write user's power level */
	if (clicmd_DecText2Char(argv[0], &Value, 1) == 0xFF)
	{
		return -1;
	}
	if (Value < 5)
	{
        GCONFIG_SetWifiTxPowerLevel(Value);
		IO_TxDefaultPowerLevelRW(1, Value);
	}
	else
	{
		return -1;
	}

	return 1;

} /* End of clicmd_SetTxPowerLevel() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetTxPowerLevelHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetTxPowerLevelHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: settpl <Power level>\r\n");
	CONSOLE_PutMessage(pInst, "       <Power level> 0: 100%%     3: 12.5%%\r\n");
	CONSOLE_PutMessage(pInst, "                     1: 50%%      4: 6.25%%\r\n");
	CONSOLE_PutMessage(pInst, "                     2: 25%%\r\n");

    return 1;

} /* End of clicmd_SetTxPowerLevelHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDataRate()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDataRate(CONSOLE_Inst *pInst)
{
	U8_T	RegLen, Value;

	argc = pInst->Argc;
	argv = pInst->Argv;
	if (argc == 0) /* Read current data rate */
	{
        Value = GCONFIG_GetWifiTxRate();
        CONSOLE_PutMessage(pInst, "Transmit Rate: ");
        switch(Value)
        {
            case 0:
                CONSOLE_PutMessage(pInst, "Auto");
                break;
            case 1:
                CONSOLE_PutMessage(pInst, "1 Mbps");
                break;
            case 2:
                CONSOLE_PutMessage(pInst, "2 Mbps");
                break;
            case 3:
                CONSOLE_PutMessage(pInst, "5.5 Mbps");
                break;
            case 4:
                CONSOLE_PutMessage(pInst, "6 Mbps");
                break;
            case 5:
                CONSOLE_PutMessage(pInst, "9 Mbps");
                break;
            case 6:
                CONSOLE_PutMessage(pInst, "11 Mbps");
                break;
            case 7:
                CONSOLE_PutMessage(pInst, "12 Mbps");
                break;
            case 8:
                CONSOLE_PutMessage(pInst, "18 Mbps");
                break;
            case 9:
                CONSOLE_PutMessage(pInst, "24 Mbps");
                break;
            case 10:
                CONSOLE_PutMessage(pInst, "36 Mbps");
                break;
            case 11:
                CONSOLE_PutMessage(pInst, "48 Mbps");
                break;
            case 12:
                CONSOLE_PutMessage(pInst, "54 Mbps");
                break;
        }
        CONSOLE_PutMessage(pInst, "\r\n");

        return 1;
	}
	else if (argc != 1 || ((RegLen = strlen(argv[0])) > 2)) /* Write user's data rate */
        return -1;

	if (clicmd_HexText2Char(argv[0], &Value, RegLen) == 0xFF)
	{
		return -1;
	}
	if (IO_TxDataRateRW(1, Value))
	{
		return -1;
	}

    GCONFIG_SetWifiTxRate(Value);

	return 1;

} /* End of clicmd_SetDataRate() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDataRateHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDataRateHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setdr <Data rate>\r\n");
	CONSOLE_PutMessage(pInst, "       <Data rate> 0: Auto        7: 12Mbps\r\n");
	CONSOLE_PutMessage(pInst, "                   1: 1Mbps       8: 18Mbps\r\n");
	CONSOLE_PutMessage(pInst, "                   2: 2Mbps       9: 24Mbps\r\n");
	CONSOLE_PutMessage(pInst, "                   3: 5.5Mbps    10: 36Mbps\r\n");
	CONSOLE_PutMessage(pInst, "                   4: 6Mbps      11: 48Mbps\r\n");
	CONSOLE_PutMessage(pInst, "                   5: 9Mbps      12: 54Mbps\r\n");
	CONSOLE_PutMessage(pInst, "                   6: 11Mbps\r\n");

    return 1;

} /* End of clicmd_SetDataRateHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetAutoPowerControl()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetAutoPowerControl(CONSOLE_Inst *pInst)
{
	U8_T	RegLen;

	argc = pInst->Argc;
	argv = pInst->Argv;
	if (argc == 0) /* Read current state of auto power control */
	{
        RegLen = GCONFIG_GetWifiAutoPowerCtrl();
        CONSOLE_PutMessage(pInst, "Auto Power Control: ");
        if (RegLen == 0)
            CONSOLE_PutMessage(pInst, "Disable\r\n");
        else
            CONSOLE_PutMessage(pInst, "Enable\r\n");

        return 1;
	}
	else if (argc != 1 || strlen(argv[0]) != 1) /* Write user's setting of auto power control */
        return -1;

    RegLen = atoi(argv[0]);
    if (RegLen < 2)
    {
        GCONFIG_SetWifiAutoPowerCtrl(RegLen);
	IO_AutoPowerCtrlRW(1, RegLen ? 1:0);
	}
    else
		return -1;

	return 1;

} /* End of clicmd_SetAutoPowerControl() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetAutoPowerControlHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetAutoPowerControlHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setapc <Auto power control>\r\n");
	CONSOLE_PutMessage(pInst, "       <Auto power control> 0: Disable   1: Enable\r\n");

    return 1;

} /* End of clicmd_SetAutoPowerControlHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetBeaconInterval()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetBeaconInterval(CONSOLE_Inst *pInst)
{
	U16_T	Value;

	argc = pInst->Argc;
	argv = pInst->Argv;
	if (argc == 0) /* Read current beacon interval */
	{
        Value = GCONFIG_GetWifiBcnInterval();
        CONSOLE_PutMessage(pInst, "Beacon: %u ms\r\n", Value);

        return 1;
	}
	else if (argc != 1) /* Write user's beacon interval */
        return -1;

    Value = atoi(argv[0]);
	if (Value < 20 || Value > 1000)
		return -1;

	GCONFIG_SetWifiBcnInterval(Value);
	IO_BeaconIntervalRW(1, (U8_T XDATA*)&Value);
	return 1;

} /* End of clicmd_SetBeaconInterval() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetBeaconIntervalHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetBeaconIntervalHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setbi <Beacon interval(ms)>\r\n");
	CONSOLE_PutMessage(pInst, "       <Beacon interval(ms)> 20 ~ 1000\r\n");
    return 1;

} /* End of clicmd_SetBeaconIntervalHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_GetWifiConfig()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_GetWifiConfig(CONSOLE_Inst *pInst)
{
    argc = pInst->Argc;
    argv = pInst->Argv;

	if (pInst->State == CLI_STATE_COMMAND)
	{
	    if (argc == 0)
		{
			CONSOLE_PutMessage(pInst, "Please wait...\r\n");		
            CONSOLE_PutMessage(pInst, "Current WiFi Settings\r\n");
			pInst->Cmd.Buf[0] = IO_MCPU_WCPU_READY;
			pInst->State = CLI_STATE_COMMAND_WAIT;
			goto CLICMD_GET_WIFI_CONFIG_NO_OPTION;
		}
		return -1;
    }
	else if (pInst->State == CLI_STATE_COMMAND_WAIT)
	{
		if  (pInst->Cmd.ReplyLen && (pInst->Cmd.Buf[0] + pInst->Cmd.Buf[1]) == 0xFF)
		{
			switch (pInst->Cmd.Buf[0])
			{
			case IO_MCPU_WCPU_READY_ACK:			
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				pInst->Cmd.Buf[0] = IO_WIFI_DRIVER_VER_READ;
				goto CLICMD_GET_WIFI_CONFIG_NO_OPTION;
				break;
				
			case IO_WIFI_DRIVER_VER_READ_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				if (pInst->Cmd.Buf[2] > 29)
					pInst->Cmd.Buf[2] = 29;
				pInst->Cmd.Buf[3+pInst->Cmd.Buf[2]] = '\0';					
				CONSOLE_PutMessage(pInst, "* WCPU F/W version: %s\r\n",&(pInst->Cmd.Buf[3]));
					
				pInst->Cmd.Buf[0] = IO_WIFI_MAC_ADDR_SET;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;
				break;
				
			case IO_WIFI_MAC_ADDR_SET_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				CONSOLE_PutMessage(pInst, "* WiFi MAC address: 0x%02bx%02bx%02bx%02bx%02bx%02bx\r\n"
											,pInst->Cmd.Buf[4]
											,pInst->Cmd.Buf[5]
											,pInst->Cmd.Buf[6]
											,pInst->Cmd.Buf[7]
											,pInst->Cmd.Buf[8]
											,pInst->Cmd.Buf[9]);
					
				pInst->Cmd.Buf[0] = IO_OPERATE_MODE;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;
				break;
			
			case IO_OPERATE_MODE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				
				CONSOLE_PutMessage(pInst, "* Baseband|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 1)
					CONSOLE_PutMessage(pInst, "802.11b/g mode\r\n");
				else if (pInst->Cmd.Buf[4] == 4)
					CONSOLE_PutMessage(pInst, "802.11a mode\r\n");
				else if (pInst->Cmd.Buf[4] == 5)
					CONSOLE_PutMessage(pInst, "802.11a/b/g mode\r\n");					
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
					
				pInst->Cmd.Buf[0] = IO_NETWORK_MODE;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;
				break;
				
			case IO_NETWORK_MODE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Network|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 2)
					CONSOLE_PutMessage(pInst, "Ad-hoc\r\n");
				else if (pInst->Cmd.Buf[4] == 3)
					CONSOLE_PutMessage(pInst, "infrastructure\r\n");
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
							
				pInst->Cmd.Buf[0] = IO_CHANNEL;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;
				break;
				
			case IO_CHANNEL_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Channel: %bu\r\n",pInst->Cmd.Buf[4]);
					
				pInst->Cmd.Buf[0] = IO_SSID_READ;
				goto CLICMD_GET_WIFI_CONFIG_NO_OPTION;
				break;
				
			case IO_SSID_READ_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* SSID: ");
				if (pInst->Cmd.Buf[2] > 32)
					CONSOLE_PutMessage(pInst, "\r\nLength error\r\n");
				pInst->Cmd.Buf[3+pInst->Cmd.Buf[2]] = '\0';
				CONSOLE_PutMessage(pInst, "%s\r\n", (U8_T *)&(pInst->Cmd.Buf[3]));
								
				pInst->Cmd.Buf[0] = IO_ENCRYPTION_TYPE;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;			
				break;
				
			case IO_ENCRYPTION_TYPE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Security|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0)
					CONSOLE_PutMessage(pInst, "no security\r\n");
				else if (pInst->Cmd.Buf[4] == 1)
					CONSOLE_PutMessage(pInst, "WEP64 bits\r\n");
				else if (pInst->Cmd.Buf[4] == 5)
					CONSOLE_PutMessage(pInst, "WEP128 bits\r\n");
				else if (pInst->Cmd.Buf[4] == 2)
					CONSOLE_PutMessage(pInst, "TKIP\r\n");
				else if (pInst->Cmd.Buf[4] == 4)
					CONSOLE_PutMessage(pInst, "AES\r\n");
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
								
				pInst->Cmd.Buf[0] = IO_WEP_KEY_INDEX;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;			
				break;
				
			case IO_WEP_KEY_INDEX_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* WEP Key Index: %bu\r\n", pInst->Cmd.Buf[4]);
				pInst->CmdId = 0;				
				pInst->Cmd.Buf[0] = IO_WEP64_KEY_READ;
				goto CLICMD_GET_WIFI_CONFIG_BY_INDEX_READ;				
				break;
				
			case IO_WEP64_KEY_READ_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* WEP64 bits key index %bu: ", pInst->CmdId);
				for (StrLen = 0; StrLen < 5; StrLen ++)
				{
					CONSOLE_PutMessage(pInst, "%02bx", pInst->Cmd.Buf[3+StrLen]);
				}
				CONSOLE_PutMessage(pInst, "\r\n");

				if (pInst->CmdId < 3)
				{
					pInst->CmdId ++;
					pInst->Cmd.Buf[0] = IO_WEP64_KEY_READ;
				}
				else
				{
					pInst->CmdId = 0;
					pInst->Cmd.Buf[0] = IO_WEP128_KEY_READ;
				}
				goto CLICMD_GET_WIFI_CONFIG_BY_INDEX_READ;
				break;
				
			case IO_WEP128_KEY_READ_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* WEP128 bits key index %bu: ", pInst->CmdId);
				for (StrLen = 0; StrLen < 13; StrLen ++)
				{
					CONSOLE_PutMessage(pInst, "%02bx", pInst->Cmd.Buf[3+StrLen]);
				}
				CONSOLE_PutMessage(pInst, "\r\n");

				if (pInst->CmdId < 3)
				{
					pInst->CmdId ++;
					pInst->Cmd.Buf[0] = IO_WEP128_KEY_READ;
					goto CLICMD_GET_WIFI_CONFIG_BY_INDEX_READ;					
				}
				else
				{
					pInst->CmdId = 0;
					pInst->Cmd.Buf[0] = IO_TKIP_AES_PASSPHASE_STRING_READ;
					goto CLICMD_GET_WIFI_CONFIG_NO_OPTION;
				}								
				break;
				
			case IO_TKIP_AES_PASSPHASE_STRING_READ_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* AES/TKIP Passphrase: ");
				for (StrLen = 0; StrLen < pInst->Cmd.Buf[2]; StrLen ++)
				{
					CONSOLE_PutMessage(pInst, "%c", pInst->Cmd.Buf[3+StrLen]);
				}
				CONSOLE_PutMessage(pInst, "\r\n");
								
				pInst->Cmd.Buf[0] = IO_TX_DATA_RATE;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;							
				break;
				
			case IO_TX_DATA_RATE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Tx Rate|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0x00)
					CONSOLE_PutMessage(pInst, "Auto\r\n");
				else if (pInst->Cmd.Buf[4] == 0x02)
					CONSOLE_PutMessage(pInst, "1 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x04)
					CONSOLE_PutMessage(pInst, "2 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x0B)
					CONSOLE_PutMessage(pInst, "5.5 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x0C)
					CONSOLE_PutMessage(pInst, "6 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x12)
					CONSOLE_PutMessage(pInst, "9 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x16)
					CONSOLE_PutMessage(pInst, "11 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x18)
					CONSOLE_PutMessage(pInst, "12 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x24)
					CONSOLE_PutMessage(pInst, "18 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x30)
					CONSOLE_PutMessage(pInst, "24 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x48)
					CONSOLE_PutMessage(pInst, "36 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x60)
					CONSOLE_PutMessage(pInst, "48 Mbps\r\n");
				else if (pInst->Cmd.Buf[4] == 0x6C)
					CONSOLE_PutMessage(pInst, "54 Mbps\r\n");																																													
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
								
				pInst->Cmd.Buf[0] = IO_TX_DEFAULT_POWER_LEVEL;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;				
				break;
				
			case IO_TX_DEFAULT_POWER_LEVEL_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Transmission Power Level|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0)
					CONSOLE_PutMessage(pInst, "100%%\r\n");
				else if (pInst->Cmd.Buf[4] == 1)
					CONSOLE_PutMessage(pInst, "50%%\r\n");
				else if (pInst->Cmd.Buf[4] == 2)
					CONSOLE_PutMessage(pInst, "25%%\r\n");
				else if (pInst->Cmd.Buf[4] == 3)
					CONSOLE_PutMessage(pInst, "12.5%%\r\n");
				else if (pInst->Cmd.Buf[4] == 4)
					CONSOLE_PutMessage(pInst, "6.25%%\r\n");
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
								
				pInst->Cmd.Buf[0] = IO_PREAMBLE;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;					
				break;
				
			case IO_PREAMBLE_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Preamble Mode|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0)
					CONSOLE_PutMessage(pInst, "long\r\n");
				else if (pInst->Cmd.Buf[4] == 0x20)
					CONSOLE_PutMessage(pInst, "auto\r\n");
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
								
				pInst->Cmd.Buf[0] = IO_BEACON_INTERVAL_READ;
				goto CLICMD_GET_WIFI_CONFIG_NO_OPTION;					
				break;
				
			case IO_BEACON_INTERVAL_READ_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Beacon Interval: %u ms\r\n", *((U16_T*)&(pInst->Cmd.Buf[3])));
								
				pInst->Cmd.Buf[0] = IO_RTS_THRESHOLD_READ;
				goto CLICMD_GET_WIFI_CONFIG_NO_OPTION;					
				break;
				
			case IO_RTS_THRESHOLD_READ_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* RTS Threshold: %u\r\n", *((U16_T*)&(pInst->Cmd.Buf[3])));
								
				pInst->Cmd.Buf[0] = IO_OPEN_SYSTEM;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;					
				break;

			case IO_OPEN_SYSTEM_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Authentication type|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0)
					CONSOLE_PutMessage(pInst, "open system\r\n");
				else if (pInst->Cmd.Buf[4] == 1)
					CONSOLE_PutMessage(pInst, "share key\r\n");
				else if (pInst->Cmd.Buf[4] == 2)
					CONSOLE_PutMessage(pInst, "WPA1_WPA2 only\r\n");
				else if (pInst->Cmd.Buf[4] == 3)
					CONSOLE_PutMessage(pInst, "WPA1_WPA2 mixed\r\n");
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
								
				pInst->Cmd.Buf[0] = IO_AUTO_POWER_CONTROL;
				goto CLICMD_GET_WIFI_CONFIG_BY_READ;					
				break;
				
			case IO_AUTO_POWER_CONTROL_RESP:
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));			
				
				CONSOLE_PutMessage(pInst, "* Auto Power Control|%bu: ",pInst->Cmd.Buf[4]);
				if (pInst->Cmd.Buf[4] == 0)
					CONSOLE_PutMessage(pInst, "disable\r\n");
				else if (pInst->Cmd.Buf[4] == 1)
					CONSOLE_PutMessage(pInst, "enable\r\n");
				else
					CONSOLE_PutMessage(pInst, "Unsupport\r\n");
							
				MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
				pInst->State = CLI_STATE_COMMAND;
				break;
				
			default:
				pInst->Cmd.ReplyLen	= 0;
				break;
			}
			return 1;
		}
		else if (pInst->Cmd.WaitTime == 0)
		{
			CONSOLE_PutMessage(pInst, "Command Time out\r\n");		
			MWIFAPP_TaskCmdDeQueue((MWIFAPP_LINK*)&(pInst->Cmd));
			pInst->State = CLI_STATE_COMMAND;
			return -1;		
		}
		else
			pInst->Cmd.ReplyLen = 0;	
	}
	return 1;
	
CLICMD_GET_WIFI_CONFIG_NO_OPTION:
	pInst->Cmd.Buf[1] = ~(pInst->Cmd.Buf[0]);
	pInst->Cmd.Buf[2] = 0;
	MWIFAPP_TaskCmdEnQueue((MWIFAPP_LINK*)&(pInst->Cmd));
	return 1;
CLICMD_GET_WIFI_CONFIG_BY_READ:
	pInst->Cmd.Buf[1] = ~(pInst->Cmd.Buf[0]);
	pInst->Cmd.Buf[2] = 1;
	pInst->Cmd.Buf[3] = 0;	
	MWIFAPP_TaskCmdEnQueue((MWIFAPP_LINK*)&(pInst->Cmd));
	return 1;
CLICMD_GET_WIFI_CONFIG_BY_INDEX_READ:
	pInst->Cmd.Buf[1] = ~(pInst->Cmd.Buf[0]);
	pInst->Cmd.Buf[2] = 1;
	pInst->Cmd.Buf[3] = pInst->CmdId;	
	MWIFAPP_TaskCmdEnQueue((MWIFAPP_LINK*)&(pInst->Cmd));
	return 1;	
} /* End of clicmd_GetWifiConfig() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_GetWifiConfigHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_GetWifiConfigHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: wificonfig\r\n");

    return 1;
} /* End of clicmd_GetWifiConfigHelp() */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDhcpServer()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDhcpServer(CONSOLE_Inst *pInst)
{
    U32_T addr, *pAddr;

    argc = pInst->Argc;
    argv = pInst->Argv;
    if (argc == 0) /* current setting */
    {
        addr = GCONFIG_GetDhcpSrvStartIp();
        CONSOLE_PutMessage(pInst, "Start IP address: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        addr = GCONFIG_GetDhcpSrvEndIp();
        CONSOLE_PutMessage(pInst, "End IP address: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        addr = GCONFIG_GetDhcpSrvNetmask();
        CONSOLE_PutMessage(pInst, "Netmask: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        addr = GCONFIG_GetDhcpSrvDefGateway();
        CONSOLE_PutMessage(pInst, "Gateway: %bu.%bu.%bu.%bu\r\n", (U8_T)((addr>>24)&0x000000FF),
                           (U8_T)((addr>>16)&0x000000FF), (U8_T)((addr>>8)&0x000000FF), (U8_T)(addr&0x000000FF));
        return 1;
    }
    else if (argc != 4)
        return -1;

    /* Check data validation */
    pAddr = (U32_T)StrBuf;
    if (clicmd_GetIp2Ulong(argv[0], strlen(argv[0]), &addr) < 0 || !clicmd_ValidIp(addr))
        return -1;
    pAddr[0] = addr;

    if (clicmd_GetIp2Ulong(argv[1], strlen(argv[1]), &addr) < 0 || !clicmd_ValidIp(addr))
        return -1;
    pAddr[1] = addr;

    if (clicmd_GetIp2Ulong(argv[2], strlen(argv[2]), &addr) < 0)
        return -1;
    pAddr[2] = addr;

    if (clicmd_GetIp2Ulong(argv[3], strlen(argv[3]), &addr) < 0 || !clicmd_ValidIp(addr))
        return -1;
    pAddr[3] = addr;

    GCONFIG_SetDhcpSrvStartIp(pAddr[0]);
    GCONFIG_SetDhcpSrvEndIp(pAddr[1]);
    GCONFIG_SetDhcpSrvNetmask(pAddr[2]);
    GCONFIG_SetDhcpSrvDefGateway(pAddr[3]);

    return 1;
} /* End of clicmd_SetDhcpServer() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_SetDhcpServerHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_SetDhcpServerHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: dhcpsrv <start addr> <end addr> <netmask> <gateway>\r\n");
    return 1;
} /* End of clicmd_SetDhcpServerHelp() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_EnableDhcpServer()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_EnableDhcpServer(CONSOLE_Inst *pInst)
{
    U8_T temp8, *argP;

    argc = pInst->Argc;
    argv = pInst->Argv;

    if (argc == 0) /* current setting */
    {
        temp8 = GCONFIG_GetDhcpSrvStatus();
        if (temp8)
            CONSOLE_PutMessage(pInst, "DHCP Server: enable\r\n");
        else
            CONSOLE_PutMessage(pInst, "DHCP Server: disable\r\n");
        return 1;
    }
    else if (argc > 1)
        return -1;
    
    argP = argv[0];
    if ((strlen(argP) == 1) && IsDigit(*argP))
        temp8 = *argP - 0x30;
    else
        return -1;
     
    if (temp8 > 1)
        return -1;

    GCONFIG_SetDhcpSrvStatus(temp8);

    return 1;

} /* End of clicmd_EnableDhcpServer() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: clicmd_EnableDhcpServerHelp()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T clicmd_EnableDhcpServerHelp(CONSOLE_Inst *pInst)
{
    CONSOLE_PutMessage(pInst, "Usage: setdhcpsrv <status>\r\n");
    CONSOLE_PutMessage(pInst, "       <status>: 0: disable     1: enable\r\n");

    return 1;

} /* End of clicmd_EnableDhcpServerHelp() */

/* End of clicmd.c */
