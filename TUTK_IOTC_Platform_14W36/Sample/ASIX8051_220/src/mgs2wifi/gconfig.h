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
 * Module Name:gconfig.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gconfig.h,v $
 *
 *=============================================================================
 */

#ifndef __GCONFIG_H__
#define __GCONFIG_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "stoe_cfg.h"

/* NAMING CONSTANT DECLARATIONS */
#define GCONFIG_DEFAULT_DATA_PORT		5000  // Default data port for both server and client modes

// Following UDP broadcast, multicast and unicast listening ports are used for communication
// with Device Server Manager program and are not configurable
#define GCONFIG_UDP_BCAST_SERVER_PORT	25122 // UDP broadcast listening ports
#define GCONFIG_UDP_MCAST_SERVER_PORT	25123 // UDP multicast listening ports
#define GCONFIG_UDP_UCAST_SERVER_PORT	25124 // UDP unicast listening ports

#define GCONFIG_CFG_DATA_FLASH_ADDR		0x4000

#define GCONFIG_OPCODE_OFFSET			8
#define GCONFIG_OPCODE_SEARCH_REQ		0x00
#define GCONFIG_OPCODE_SEARCH_ACK		0x01
#define GCONFIG_OPCODE_SET_REQ			0x02
#define GCONFIG_OPCODE_SET_ACK			0x03
#define GCONFIG_OPCODE_SET_DENY			0x04
#define GCONFIG_OPCODE_UPGRADE_REQ		0x05
#define GCONFIG_OPCODE_UPGRADE_ACK		0x06
#define GCONFIG_OPCODE_UPGRADE_DENY		0x07
#define GCONFIG_OPCODE_RESET_REQ		0x08
#define GCONFIG_OPCODE_RESET_ACK		0x09
#define GCONFIG_OPCODE_RESET_DENY		0x0A
#define GCONFIG_OPCODE_REBOOT_REQ		0x0B
#define GCONFIG_OPCODE_REBOOT_ACK		0x0C
#define GCONFIG_OPCODE_REBOOT_DENY		0x0D
#define GCONFIG_OPCODE_SEARCH_REQ1		0x0E
#define GCONFIG_OPCODE_MONITOR_REQ		0x10
#define GCONFIG_OPCODE_MONITOR_ACK		0x11
#define GCONFIG_OPCODE_NONE				0xFF

#define GCONFIG_OPTION_ENABLE_REBOOT			0x01
#define GCONFIG_OPTION_ENABLE_UDP_AUTO_CONNECT	0x02

#define GCONFIG_NETWORK_SERVER			0x0000
#define GCONFIG_NETWORK_CLIENT			0x8000
#define GCONFIG_NETWORK_DHCP_ENABLE		0x4000
#define GCONFIG_NETWORK_DHCP_DISABLE	0x0000
#define GCONFIG_NETWORK_PROTO_UDP		0x0800
#define GCONFIG_NETWORK_PROTO_TCP		0x0400
#define GCONFIG_NETWORK_PROTO_UDP_MCAST	0x0200
#define GCONFIG_NETWORK_PROTO_UDP_BCAST	0x0100
#define GCONFIG_NETWORK_TIMEOUT_0_MIN	0x0000
#define GCONFIG_NETWORK_TIMEOUT_10_MIN	0x000A
#define GCONFIG_NETWORK_ACC_IP_ENABLE	0x1000
#define GCONFIG_NETWORK_ACC_IP_DISABLE	0x0000

#define GCONFIG_SPORT_BRATE_115200		0x0800
#define GCONFIG_SPORT_DATABIT_8			0x0300
#define GCONFIG_SPORT_PARITY_NONE		0x0040
#define GCONFIG_SPORT_FLOWC_NONE		0x0008
#define GCONFIG_SPORT_STOPBIT_1			0x0000

#define GCONFIG_SMTP_EVENT_COLDSTART    0x0001
#define GCONFIG_SMTP_EVENT_WARMSTART    0x0002
#define GCONFIG_SMTP_EVENT_AUTH_FAIL    0x0004
#define GCONFIG_SMTP_EVENT_IP_CHANGED   0x0008
#define GCONFIG_SMTP_EVENT_PSW_CHANGED  0x0010

#define GCONFIG_ACCESSIBLE_IP_NUM       4
#define GCONFIG_VERSION_STR_LEN         12
#define GCONFIG_VERSION_STRING          "1.0.0"

/* TYPE DECLARATIONS */
typedef struct _GCONFIG_GID
{
	U8_T id[8]; 
} GCONFIG_GID;

typedef struct _GCONFIG_DEV_NAME
{
	U8_T DevName[16]; 
} GCONFIG_DEV_NAME;

typedef struct _GCONFIG_MAC_ADDR
{
	U8_T MacAddr[6]; 
} GCONFIG_MAC_ADDR;

typedef struct _GCONFIG_SMTP_CONFIG
{
    U8_T  DomainName[36];
    U8_T  FromAddr[36];
    U8_T  ToAddr1[36];
    U8_T  ToAddr2[36];
    U8_T  ToAddr3[36];
    U16_T EventEnableBits;
} GCONFIG_SMTP_CONFIG;

typedef struct
{
    U32_T IpPoolAddrStart;
    U32_T IpPoolAddrEnd;
    U32_T Netmask;
    U32_T DefaultGateway;
    U16_T LeaseTime; /* minutes */
    U8_T Status;    /* 0: disable   1: enable */
    U8_T Rsvd;    
} GCONFIG_DHCPSRV;

typedef struct
{
	U32_T DeviceIp;
	U32_T DeviceNetmask;
	U32_T DeviceGateway;
	U32_T TftpServerIpForBldr;
	U8_T FileNameBldr[64];
	U8_T FileNameLenBldr;
	U32_T TftpServerIpForMcpu;
	U8_T FileNameMcpu[64];
	U8_T FileNameLenMcpu;
	U32_T TftpServerIpForWcpu;
	U8_T FileNameWcpu[64];
	U8_T FileNameLenWcpu;
	U8_T Reserved;
} GCONFIG_UPGRADE;

#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
/* This is used for tool */
typedef struct
{
	U16_T WifiBcnInterval;	/* beacon interval, only for IBSS mode */
	U16_T WifiRtsThreshold; /* RTS threshold */
	U8_T WifiBasebandMode;	/* mixed mode, 802.11b+g mode, 802.11a mode */
	U8_T WifiNetworkMode;	/* ad hoc mode, infrastructure mode */
	U8_T WifiChannel;		/* channel */
	U8_T WifiSsidLen;		/* SSID length */
	U8_T WifiSsid[32];		/* SSID */
	U8_T WifiTxRate;		/* auto, 1, 2, .... 48, 54M */
	U8_T WifiTxPowerLevel;	/* tx power level */
	U8_T WifiPreambleMode;	/* auto, long preamble, short preamble */
	U8_T WifiAutoPowerCtrl;	/* auto power control enable or disable*/
	U8_T WifiEncryptMode;	/* 0:disable, 1:wep64, 2:wep128, 3:tkip, 4:aes */
	U8_T WifiWepKeyIndex;	/* wep key index */
	U8_T WifiWepKeyLength;	/* wep key length, 0:64 bits, 1:128 bits*/
	U8_T WifiPreShareKeyLen;	/* pre-share key length */
	U8_T WifiWep64Key[4][5];/* 64bit wep key */
	U8_T WifiWep128Key[4][13];	/* 128bit wep key */
	U8_T WifiPreShareKey[64];	/* pre-share key */
//    U8_T AuthenType;        /* open(0), shared key(1), wpa1/wpa2 only(2), wpa1/wpa2 mixed(3) */
} GCONFIG_PKT_WIFI;

/* This will be used in boot loader */
typedef struct
{
	U8_T WifiBasebandMode;	/* mixed mode, 802.11b+g mode, 802.11a mode */
	U8_T WifiNetworkMode;	/* ad hoc mode, infrastructure mode */
	U8_T WifiChannel;		/* channel */
	U8_T WifiSsid[32];		/* SSID */
	U8_T WifiSsidLen;		/* SSID length */
	U8_T WifiTxRate;		/* auto, 1, 2, .... 48, 54M */
	U8_T WifiTxPowerLevel;	/* tx power level */
	U8_T WifiPreambleMode;	/* auto, long preamble, short preamble */
	U16_T WifiBcnInterval;	/* beacon interval, only for IBSS mode */
	U16_T WifiRtsThreshold; /* RTS threshold */
	U8_T WifiAutoPowerCtrl;	/* auto power control enable or disable*/
	U8_T WifiEncryptMode;	/* 0:disable, 1:wep64, 2:wep128, 3:tkip, 4:aes */
	U8_T WifiWepKeyIndex;	/* wep key index */
	U8_T WifiWepKeyLength;	/* wep key length, 0:64 bits, 1:128 bits*/
	U8_T WifiWep64Key[4][5];/* 64bit wep key */
	U8_T WifiWep128Key[4][13];	/* 128bit wep key */
	U8_T WifiPreShareKey[64];	/* pre-share key */
	U8_T WifiPreShareKeyLen;	/* pre-share key length */
//    U8_T AuthenType;        /* open(0), shared key(1), wpa1/wpa2 only(2), wpa1/wpa2 mixed(3) */
//    U16_T FragThreshold;    /* 256~2346 */

} GCONFIG_WIFI;
#endif

typedef struct _GCONFIG_CFG_PKT
{
	GCONFIG_GID 		Gid;
	U8_T				Opcode;
	U8_T				Option;
 	GCONFIG_DEV_NAME 	DevName;
	GCONFIG_MAC_ADDR	MacAddr;
	U16_T				Network;
	U32_T				ServerDynamicIP;
	U32_T				ServerStaticIP;
	U16_T				ServerDataPktListenPort;
	U16_T				ServerMulticastListenPort;
	U16_T				ServerBroadcastListenPort;
	U32_T				ClientDestIP;
	U16_T				ClientDestPort;
	U32_T				Netmask;
	U32_T				Gateway;
	U32_T				Dns;
	U16_T				SerialPort;
	U8_T				Rs485Mode;
	U8_T				DeviceStatus;
	U16_T				EthernetTxTimer;
    U32_T               AccessibleIP[GCONFIG_ACCESSIBLE_IP_NUM];
    GCONFIG_SMTP_CONFIG Smtp;
	U8_T				DestHostName[36];
    U32_T               TftpServerIp;
    U8_T                Filename[64];
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
    GCONFIG_PKT_WIFI    WifiConfig;
#endif
    GCONFIG_DHCPSRV     DhcpSrv;
} GCONFIG_CFG_PKT;

typedef struct
{
    U8_T Username[16];
    U8_T Passwd[16];
    U8_T Level;
    U8_T Rsvd;
} GCONFIG_ADMIN;

typedef struct _GCONFIG_CFG_DATA
{
	U8_T			Signature[4];
	U16_T			Checksum;
	U16_T			SwCfgLength;
	GCONFIG_UPGRADE	UpgradeCfg;
#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
	GCONFIG_WIFI	WifiConfig;
#endif
	U32_T				UdpAutoConnectClientIp;
	U16_T				UdpAutoConnectClientPort;
 	GCONFIG_DEV_NAME 	DevName;
	U16_T				Network;
	U32_T				ServerStaticIP;
	U16_T				ServerDataPktListenPort;
	U16_T				ServerMulticastListenPort;
	U16_T				ServerBroadcastListenPort;
	U32_T				ClientDestIP;
	U16_T				ClientDestPort;
	U32_T				Netmask;
	U32_T				Gateway;
	U32_T				Dns;
	U16_T				SerialPort;
	U8_T				Rs485Mode;
	U8_T				Option;
	U16_T				EthernetTxTimer;
    U32_T               AccessibleIP[GCONFIG_ACCESSIBLE_IP_NUM];
    GCONFIG_SMTP_CONFIG Smtp;
	U8_T				DestHostName[36];
    GCONFIG_ADMIN       Admin;
    GCONFIG_DHCPSRV     DhcpSrv;
} GCONFIG_CFG_DATA;

typedef struct _GCONFIG_MONITOR_PKT
{
	GCONFIG_GID 		Gid;
	U8_T				Opcode;
    U8_T                ModemStatus;
    U8_T                VerStr[GCONFIG_VERSION_STR_LEN];
	U32_T				TxBytes;
    U32_T               RxBytes;
} GCONFIG_MONITOR_PKT;

typedef struct _GCONFIG_UPGRADE_PKT
{
	GCONFIG_GID 		Gid;
	U8_T				Opcode;
	U8_T				Option;
 	GCONFIG_DEV_NAME 	DevName;
	GCONFIG_MAC_ADDR	MacAddr;
    U32_T               TftpSrvIp;
    U8_T                Filename[64];
} GCONFIG_UPGRADE_PKT;

/* GLOBAL VARIABLES */
extern GCONFIG_GID XDATA GCONFIG_Gid;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void GCONFIG_Task(void);
void GCONFIG_Init(void);
void GCONFIG_SetConfigPacket(GCONFIG_CFG_PKT*);
void GCONFIG_GetConfigPacket(GCONFIG_CFG_PKT*);
void GCONFIG_ReadConfigData(void);
void GCONFIG_WriteConfigData(void);
void GCONFIG_ReadDefaultConfigData(void);
U32_T GCONFIG_IpAddr2Ulong(U8_T*, U8_T);

U8_T GCONFIG_EnableUdpAutoConnectAfterReboot(void);
void GCONFIG_EnableFirmwareUpgrade(void);
void GCONFIG_SetDeviceName(GCONFIG_DEV_NAME*);
void GCONFIG_SetNetwork(U16_T);
void GCONFIG_SetServerStaticIP(U32_T);
void GCONFIG_SetServerDataPktListenPort(U16_T);
void GCONFIG_SetServerMulticastListenPort(U16_T);
void GCONFIG_SetServerBroadcastListenPort(U16_T);
void GCONFIG_SetClientDestIP(U32_T);
void GCONFIG_SetClientDestPort(U16_T);
void GCONFIG_SetNetmask(U32_T);
void GCONFIG_SetGateway(U32_T);
void GCONFIG_SetDNS(U32_T);
void GCONFIG_SetSerialPort(U16_T);
void GCONFIG_SetEthernetTxTimer(U16_T);
void GCONFIG_SetUdpAutoConnectClientIp(U32_T);
void GCONFIG_SetUdpAutoConnectClientPort(U16_T);
void GCONFIG_SetTftpServerIp(U32_T ip);
U8_T GCONFIG_SetFilename(U8_T *filename, U8_T len);

GCONFIG_DEV_NAME* GCONFIG_GetDeviceName(void);
GCONFIG_MAC_ADDR GCONFIG_GetMacAddress(void);
U16_T GCONFIG_GetNetwork(void);
U32_T GCONFIG_GetServerStaticIP(void);
U16_T GCONFIG_GetServerDataPktListenPort(void);
U16_T GCONFIG_GetServerMulticastListenPort(void);
U16_T GCONFIG_GetServerBroadcastListenPort(void);
U32_T GCONFIG_GetClientDestIP(void);
U16_T GCONFIG_GetClientDestPort(void);
U32_T GCONFIG_GetNetmask(void);
U32_T GCONFIG_GetGateway(void);
U32_T GCONFIG_GetDNS(void);
U16_T GCONFIG_GetSerialPort(void);
U16_T GCONFIG_GetEthernetTxTimer(void);
U32_T GCONFIG_GetUdpAutoConnectClientIp(void);
U16_T GCONFIG_GetUdpAutoConnectClientPort(void);
U16_T GCONFIG_GetBaudRate(U8_T index);

S16_T GCONFIG_GetAdmin(U8_T *username, U8_T *passwd, U8_T *level);
S16_T GCONFIG_SetAdmin(U8_T *username, U8_T *passwd);

S16_T GCONFIG_GetAccessibleIP(U8_T idx, U32_T *ip);
S16_T GCONFIG_SetAccessibleIP(U8_T idx, U32_T ip);

void GCONFIG_SetSMTPDomainName(U8_T*, U8_T);
U8_T GCONFIG_GetSMTPDomainName(U8_T*);
void GCONFIG_SetSMTPFrom(U8_T*, U8_T);
U8_T GCONFIG_GetSMTPFrom(U8_T*);
void GCONFIG_SetSMTPTo1(U8_T*, U8_T);
U8_T GCONFIG_GetSMTPTo1(U8_T*);
void GCONFIG_SetSMTPTo2(U8_T*, U8_T);
U8_T GCONFIG_GetSMTPTo2(U8_T*);
void GCONFIG_SetSMTPTo3(U8_T*, U8_T);
U8_T GCONFIG_GetSMTPTo3(U8_T*);
void GCONFIG_SetAutoWarning(U16_T);
U16_T GCONFIG_GetAutoWarning(void);
void GCONFIG_SetRs485Mode(U8_T mode);
U8_T GCONFIG_GetRs485Mode(void);

void GCONFIG_SetDestHostName(U8_T*, U8_T);
U8_T GCONFIG_GetDestHostName(U8_T*);
U32_T GCONFIG_GetTftpServerIp(void);
U8_T GCONFIG_GetFilename(U8_T *filename, U8_T len);

#if (MAC_ARBIT_MODE & MAC_ARBIT_WIFI)
void GCONFIG_SetSsid(U8_T *pIn, U8_T len);
U8_T GCONFIG_GetSsid(U8_T *pOut);
U8_T GCONFIG_GetChannel(void);
void GCONFIG_SetChannel(U8_T channel);
U8_T GCONFIG_GetWifiBasebandMode(void);
U8_T GCONFIG_SetWifiBasebandMode(U8_T value);
U8_T GCONFIG_GetWifiNetworkMode(void);
void GCONFIG_SetWifiNetworkMode(U8_T value);
U8_T GCONFIG_GetWifiTxRate(void);
void GCONFIG_SetWifiTxRate(U8_T index);
U8_T GCONFIG_GetWifiTxPowerLevel(void);
void GCONFIG_SetWifiTxPowerLevel(U8_T value);
U8_T GCONFIG_GetWifiPreambleMode(void);
U8_T GCONFIG_SetWifiPreambleMode(U8_T value);
U16_T GCONFIG_GetWifiBcnInterval(void);
void GCONFIG_SetWifiBcnInterval(U16_T value);
U16_T GCONFIG_GetWifiRtsThreshold(void);
void GCONFIG_SetWifiRtsThreshold(U16_T value);
U8_T GCONFIG_GetWifiAutoPowerCtrl(void);
void GCONFIG_SetWifiAutoPowerCtrl(U8_T value);
U8_T GCONFIG_GetWifiEncryptMode(void);
U8_T GCONFIG_SetWifiEncryptMode(U8_T value);
U8_T GCONFIG_GetWifiWepKeyIndex(void);
U8_T GCONFIG_SetWifiWepKeyIndex(U8_T value);
U8_T GCONFIG_GetWifiWepKeyLength(void);
void GCONFIG_SetWifiWepKeyLength(U8_T value);
U8_T GCONFIG_GetWifiWep64Key(U8_T index, U8_T *pBuf);
void GCONFIG_SetWifiWep64Key(U8_T index, U8_T *pBuf);
U8_T GCONFIG_GetWifiWep128Key(U8_T index, U8_T *pBuf);
void GCONFIG_SetWifiWep128Key(U8_T index, U8_T *pBuf);
U8_T GCONFIG_GetWifiPreShareKey(U8_T *pBuf);
void GCONFIG_SetWifiPreShareKey(U8_T *pBuf, U8_T len);

#endif /* (MAC_ARBIT_MODE & MAC_ARBIT_WIFI) */

U32_T GCONFIG_GetDhcpSrvStartIp(void);
void GCONFIG_SetDhcpSrvStartIp(U32_T startIp);
U32_T GCONFIG_GetDhcpSrvEndIp(void);
void GCONFIG_SetDhcpSrvEndIp(U32_T endIp);
U32_T GCONFIG_GetDhcpSrvNetmask(void);
void GCONFIG_SetDhcpSrvNetmask(U32_T mask);
U32_T GCONFIG_GetDhcpSrvDefGateway(void);
void GCONFIG_SetDhcpSrvDefGateway(U32_T gateway);
U16_T GCONFIG_GetDhcpSrvLeaseTime(void);
void GCONFIG_SetDhcpSrvLeaseTime(U16_T lease);
U8_T GCONFIG_GetDhcpSrvStatus(void);
void GCONFIG_SetDhcpSrvStatus(U8_T status);

#endif /* End of __GCONFIG_H__ */

/* End of gconfig.h */
