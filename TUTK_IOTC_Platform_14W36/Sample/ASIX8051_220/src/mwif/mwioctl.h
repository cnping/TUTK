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
 * Module Name: wioctl.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: wioctl.h,v $
 * no message
 *
 *=============================================================================
 */

#ifndef __WIOCTL_H__
#define __WIOCTL_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "mwif.h"

/* NAMING CONSTANT DECLARATIONS */
#define IO_COMMAND_READ			0
#define IO_COMMAND_WRITE		1
#define IO_COMMAND_ERROR		0xff
#define IO_COMMAND_DONE			0

#if IO_CPU_TYPE
#define MASS_PRODUCTION			0
#define MAX_IO_TX_BUFFER_SIZE	128
#else
#include "userdef.h"
#define MAX_IO_TX_BUFFER_SIZE	128
#endif
#define MAX_IO_RX_BUFFER_SIZE	512

#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
/* MIB register definitions */
#define MIB_TXMIC_KEY		0x00
#define MIB_TXMIC_PRIORITY	0x08
#define MIB_TX_THRESHOLD	0x09
#define MIB_TX_START_ADDR	0x0B
#define MIB_TX_END_ADDR		0x0D
#define MIB_TX_CONTROL		0x0F
#define MIB_RXMIC_KEY		0x10
#define MIB_RXMIC_PRIORITY	0x18
#define MIB_RXMIC_FAIL_CNT	0x19
#define MIB_BOUNDARY_SIZE	0x1A
#define MIB_LLC_PATTERN		0x1B

/* PCIB register definitions */
#define PCIB_RFD_BASE_ADDR		0x00
#define PCIB_RFD_END_ADDR		0x02
#define PCIB_RFD_BNDRY_SIZE		0x04
#define PCIB_FILTER_CONTROL		0x05
#define PCIB_FILTER_MANG_FRAME	0x06
  #define RCV_FILTER_INFR_NORMAL	0xffcf
  #define RCV_FILTER_IBSS_NORMAL	0xfecf
  #define RCV_FILTER_SCAN			0xfecf
#define PCIB_FILTER_BSSID		0x08
/* WCPU indirect register */
#define WCPU_MAIN_CTL_REG			0x00
#define WCPU_WIFI_MAIN_CTL_REG		0x01
#define WCPU_DMA_SOUR_ADDR_REG		0x02
#define WCPU_DMA_TARGET_ADDR_REG	0x04
#define WCPU_DMA_BYTE_COUNT_LOW_REG	0x06
#define WCPU_DMA_BYTE_COUNT_CMD_REG	0x07
#define WCPU_DMA_CONTROL_REG		0x08
#define WCPU_DMA_INTR_MASK			0x09
#define WCPU_SWTIMER_TIMER_REG		0x0A
#define WCPU_SWTIMER_TIMER_CTRL		0x0C
#define WCPU_SYSTEM_SETTING_REG		0x0D
#endif			/***** End of IO_CPU_TYPE macro *****/

/* Interface Command Definition */
// System Command
#define IO_MCPU_WCPU_READY				0x00
#define IO_MCPU_WCPU_READY_ACK			0x80
#define IO_WCPU_RESET					0x01
#define IO_WCPU_RESET_ACK				0x81
#define IO_MCPU_WCPU_CONNECT			0x02
#define IO_MCPU_WCPU_CONNECT_ACK		0x82
#define IO_MCPU_WCPU_DISCONNECT			0x03
#define IO_MCPU_WCPU_DISCONNECT_RESP	0x83
#define IO_SCAN							0x04
#define IO_SCAN_ACK						0x84
#define IO_SCAN_REPORT					0x05
#define IO_SCAN_REPORT_ACK				0x85
#define IO_SITE_SURVEY					0x06
#define IO_SITE_SURVEY_ACK				0x86
#define IO_SITE_SURVEY_REPORT			0x07
#define IO_SITE_SURVEY_REPORT_ACK		0x87
#define IO_JOIN_BSS						0x08
#define IO_JOIN_BSS_ACK					0x88
#define IO_SSID_READ					0x09
#define IO_SSID_READ_RESP				0x89
#define IO_SSID_WRITE					0x0A
#define IO_SSID_WRITE_ACK				0x8A
#define IO_OPERATE_MODE					0x0B
#define IO_OPERATE_MODE_RESP			0x8B
#define IO_CHANNEL						0x0C
#define IO_CHANNEL_RESP					0x8C
#define IO_NETWORK_MODE					0x0D
#define IO_NETWORK_MODE_RESP			0x8D
#define IO_MULTICAST_SET				0x0E
#define IO_MULTICAST_SET_ACK			0x8E
#define IO_WIFI_MAC_ADDR_SET			0x0F
#define IO_WIFI_MAC_ADDR_SET_RESP		0x8F
#define IO_WIFI_RADIO_ON_OFF			0x10
#define IO_WIFI_RADIO_ON_OFF_ACK		0x90
#define IO_WIFI_B_G_PROTECTION			0x11
#define IO_WIFI_B_G_PROTECTION_RESP		0x91
#define IO_GPIO_ZERO					0x12
#define IO_GPIO_ZERO_RESP				0x92
#define IO_GPIO_ONE						0x13
#define IO_GPIO_ONE_RESP				0x93
#define IO_GPIO_TWO						0x14
#define IO_GPIO_TWO_RESP				0x94
#define IO_GPIO_THREE					0x15
#define IO_GPIO_THREE_RESP				0x95
// System Command -- Security
#define IO_ENCRYPTION_TYPE				0x16
#define IO_ENCRYPTION_TYPE_RESP			0x96
#define IO_OPEN_SYSTEM					0x17
#define IO_OPEN_SYSTEM_RESP				0x97
#define IO_WEP64_KEY_READ				0x18
#define IO_WEP64_KEY_READ_RESP			0x98
#define IO_WEP64_KEY_WRITE				0x19
#define IO_WEP64_KEY_WRITE_ACK			0x99
#define IO_WEP128_KEY_READ				0x1A
#define IO_WEP128_KEY_READ_RESP			0x9A
#define IO_WEP128_KEY_WRITE				0x1B
#define IO_WEP128_KEY_WRITE_ACK			0x9B
#define IO_WEP_KEY_INDEX				0x1C
#define IO_WEP_KEY_INDEX_RESP			0x9C
#define IO_TKIP_AES_PASSPHASE_STRING_READ		0x1D
#define IO_TKIP_AES_PASSPHASE_STRING_READ_RESP	0x9D
#define IO_TKIP_AES_PASSPHASE_STRING_WRITE		0x1E
#define IO_TKIP_AES_PASSPHASE_STRING_WRITE_ACK	0x9E
#define IO_ERROR_COMMAND				0x1F
// Wireless Advance Setting 1 Command
#define IO_FRAGMENT_THRESHOLD_READ		0x20
#define IO_FRAGMENT_THRESHOLD_READ_RESP	0xA0
#define IO_FRAGMENT_THRESHOLD_WRITE		0x21
#define IO_FRAGMENT_THRESHOLD_WRITE_ACK	0xA1
#define	IO_RTS_THRESHOLD_READ			0x22
#define	IO_RTS_THRESHOLD_READ_RESP		0xA2
#define IO_RTS_THRESHOLD_WRITE			0x23
#define IO_RTS_THRESHOLD_WRITE_ACK		0xA3
#define IO_PREAMBLE						0x24
#define IO_PREAMBLE_RESP				0xA4
#define IO_TX_POWER_LEVEL_READ			0x25
#define IO_TX_POWER_LEVEL_READ_RESP		0xA5
#define IO_TX_POWER_LEVEL_WRITE			0x26
#define IO_TX_POWER_LEVEL_WRITE_ACK		0xA6
#define IO_BEACON_INTERVAL_READ			0x27
#define IO_BEACON_INTERVAL_READ_RESP	0xA7
#define IO_BEACON_INTERVAL_WRITE		0x28
#define IO_BEACON_INTERVAL_WRITE_ACK	0xA8
#define IO_COUNTRY_REGION				0x29
#define IO_COUNTRY_REGION_RESP			0xA9
#define IO_ATIM_INTERVAL_READ			0x2A
#define IO_ATIM_INTERVAL_READ_RESP		0xAA
#define IO_ATIM_INTERVAL_WRITE			0x2B
#define IO_ATIM_INTERVAL_WRITE_ACK		0xAB
#define IO_TX_DATA_RATE					0x2C
#define IO_TX_DATA_RATE_RESP			0xAC
#define IO_AUTO_POWER_CONTROL			0x2D
#define IO_AUTO_POWER_CONTROL_RESP		0xAD
#define IO_WIFI_ROAMING					0x2E	// not use yet
#define IO_WIFI_ROAMING_RESP			0xAE	// not use yet
#define IO_WIFI_MULTIMEDIA_SET			0x2F
#define IO_WIFI_MULTIMEDIA_SET_RESP		0xAF
#define IO_LISTEN_INTERVAL				0x30
#define IO_LISTEN_INTERVAL_RESP			0xB0
// Wireless Advance Setting 2 Command
#define IO_MINIMUM_CONTENTION_WINDOW		0x31
#define IO_MINIMUM_CONTENTION_WINDOW_RESP	0xB1
#define IO_MAXIMUM_CONTENTION_WINDOW		0x32
#define IO_MAXIMUM_CONTENTION_WINDOW_RESP	0xB2
#define IO_SHOW_RSSI						0x33
#define IO_SHOW_RSSI_RESP					0xB3
#define IO_SHOW_TSSI						0x34
#define IO_SHOW_TSSI_RESP					0xB4
// Wireless Advance Setting 3 Command
#define IO_PAYLOAD_LENGTH_ATTACH_IN_WIFI_PACKET			0x35
#define IO_PAYLOAD_LENGTH_ATTACH_IN_WIFI_PACKET_RESP	0xB5
#define IO_RETAIN_LLC_IN_WIFI_PACKET		0x36
#define IO_RETAIN_LLC_IN_WIFI_PACKET_RESP	0xB6
#define IO_2_LLC_IN_WIFI_VLAN_PACKET		0x37
#define IO_2_LLC_IN_WIFI_VLAN_PACKET_RESP	0xB7
#define IO_TRAFFIC_PROFILE					0x38
#define IO_TRAFFIC_PROFILE_RESP				0xB8
#define IO_TX_DEFAULT_POWER_LEVEL			0x39
#define IO_TX_DEFAULT_POWER_LEVEL_RESP		0xB9
#define IO_QUEUE_PRIORITY_SET				0x3A
#define IO_QUEUE_PRIORITY_SET_RESP			0xBA
#define IO_SELF_CTS							0x3B
#define IO_SELF_CTS_RESP					0xBB
#define IO_BSSID							0x3C
#define IO_BSSID_RESP						0xBC
#define IO_WIFI_IP_READ						0x3D
#define IO_WIFI_IP_READ_RESP				0xBD
#define IO_WIFI_DRIVER_VER_READ				0x3E
#define IO_WIFI_DRIVER_VER_READ_RESP		0xBE
// Mass Production Command
#define IO_CONTINUOUS_TX					0x40
#define IO_CONTINUOUS_TX_ACK				0xC0
#define IO_SENSITIVITY_TEST					0x41
#define IO_SENSITIVITY_TEST_ACK				0xC1
#define IO_SENSITIVITY_TEST_REPORT			0x42
#define IO_SENSITIVITY_TEST_REPORT_ACK		0xC2
// Debug Command
#define IO_DBG_WCPU_SFR_READ				0x50
#define IO_DBG_WCPU_SFR_READ_RESP			0xD0
#define IO_DBG_WCPU_SFR_WRITE				0x51
#define IO_DBG_WCPU_SFR_WRITE_ACK			0xD1
#define IO_DBG_WCPU_SUB_SYS_REG_READ		0x52
#define IO_DBG_WCPU_SUB_SYS_REG_READ_RESP	0xD2
#define IO_DBG_WCPU_SUB_SYS_REG_WRITE		0x53
#define IO_DBG_WCPU_SUB_SYS_REG_WRITE_RESP	0xD3
#define IO_DBG_WCPU_MIIB_REG_READ			0x54
#define IO_DBG_WCPU_MIIB_REG_READ_RESP		0xD4
#define IO_DBG_WCPU_MIIB_REG_WRITE			0x55
#define IO_DBG_WCPU_MIIB_REG_WRITE_ACK		0xD5
#define IO_DBG_WCPU_PCIB_REG_READ			0x56
#define IO_DBG_WCPU_PCIB_REG_READ_RESP		0xD6
#define IO_DBG_WCPU_PCIB_REG_WRITE			0x57
#define IO_DBG_WCPU_PCIB_REG_WRITE_ACK		0xD7
#define IO_DBG_PCI_CONFIGURATION_READ		0x58
#define IO_DBG_PCI_CONFIGURATION_READ_RESP	0xD8
#define IO_DBG_PCI_CONFIGURATION_WRITE		0x59
#define IO_DBG_PCI_CONFIGURATION_WRITE_ACK	0xD9
#define IO_DBG_WIFI_REG_READ				0x5A
#define IO_DBG_WIFI_REG_READ_RESP			0xDA
#define IO_DBG_WIFI_REG_WRITE				0x5B
#define IO_DBG_WIFI_REG_WRITE_ACK			0xDB
#define IO_DBG_WIFI_PACKET_FILTERING_WRITE		0x5C
#define IO_DBG_WIFI_PACKET_FILTERING_WRITE_ACK	0xDC
#define IO_DBG_WIFI_PACKET_FILTERING_READ		0x5D
#define IO_DBG_WIFI_PACKET_FILTERING_READ_RESP	0xDD
#define IO_DBG_SYNTHESIZER_SET				0x5E
#define IO_DBG_SYNTHESIZER_SET_ACK			0xDE
#define IO_DBG_RETRY_NUMBER					0x5F
#define IO_DBG_RETRY_NUMBER_RESP			0xDF
#define IO_DBG_AUTO_POWER_CONTROL			0x64
#define IO_DBG_AUTO_POWER_CONTROL_RESP		0x64

#define IO_WLAN_DEBUG1						0x65
#define IO_WLAN_DEBUG1_RESP					0xE5

#define IO_REMAINDER_BUF_IS_TOO_SMALL		0xFE

/* TYPE DECLARATIONS */
//---------------------------------------------------------------
#if IO_CPU_TYPE
typedef struct _SITE_SURVEY_TABLE
{
	U8_T	Bssid[6];
	U8_T	SsidLen;
	U8_T	Ssid[32];
	U8_T	InfraMode;
	U8_T	Channel;
	U8_T	Rssi;
	U8_T	SecurFlag;

} SISRVY_T;
#endif

/* GLOBAL VARIABLES */
extern U8_T XDATA	IO_RxBuf[MAX_IO_RX_BUFFER_SIZE];
extern U16_T XDATA	IO_RxHead;
extern U16_T XDATA	IO_RxTail;

//extern U8_T XDATA	IO_CmdFlag;
#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
//extern U8_T XDATA	IO_BssCount;
extern U8_T	XDATA	IO_WifiRFEnable;
extern U8_T XDATA	IO_WifiBasebandMode;
extern U8_T XDATA	IO_WifiNetworkMode;
extern U8_T XDATA	IO_WifiChannel;
extern U8_T XDATA	IO_WiFiSsid[];
extern U8_T XDATA	IO_WifiSsidLen;
extern U8_T XDATA	IO_WiFiTxRate;
extern U8_T XDATA	IO_WiFiTxPowerLevel;
extern U8_T XDATA	IO_GBProtection;
extern U8_T XDATA	IO_PreambleMode;
extern U16_T XDATA	IO_BcnInterval;
extern U16_T XDATA	IO_RtsThreshold;
extern U8_T XDATA	IO_AutoPowerCtrl;
extern U8_T XDATA	IO_EncryptMode;
extern U8_T XDATA	IO_WepKeyLength;
extern U8_T XDATA	IO_WepKeyIndex;
extern U8_T XDATA	IO_Wep64Key[4][5];
extern U8_T XDATA	IO_Wep128Key[4][13];
extern U8_T XDATA	IO_PreShareKey[64];
extern U8_T XDATA	IO_PreShareKeyLen;
extern SISRVY_T XDATA	IO_SiteSurveyTable[];
extern U8_T XDATA	IO_FoundBssNum;
extern U8_T XDATA	IO_JbssIndex;
extern U8_T XDATA	IO_SiteSurveyStatusFlag;
extern U8_T const FAR	IO_WiFiSupportedCh[];
extern U8_T XDATA	IO_WiFiSupportedChBufSize;
extern U8_T const FAR	IO_WiFiSupportTxRate[];
#else			/***** Middle of IO_CPU_TYPE, Below are for WCPU use *****/
extern U8_T XDATA	IO_SiteSurveyFromMcpu;
extern U8_T XDATA	IO_ScanFromMcpu;
#endif			/***** End of IO_CPU_TYPE macro *****/

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void IO_Init(void);
U8_T IO_CmdParsing(U8_T*);
void IO_CmdError(void);

#if IO_CPU_TYPE	/***** Begin of IO_CPU_TYPE, Below are for MCPU use *****/
void IO_NoticeWcpu(U8_T, U8_T*, U8_T);
void IO_Reset(void);
void IO_Scan(void);
void IO_SiteSurvey(void);
void IO_JoinBss(U8_T value);
void IO_SsidRW(U8_T type, U8_T len, U8_T XDATA* pReg);
void IO_OperateModeRW(U8_T type, U8_T value);
void IO_ChannelRW(U8_T type, U8_T value);
void IO_NetworkModeRW(U8_T type, U8_T value);
void IO_SetMulticastMac(U8_T XDATA* pMulticastMac);
void IO_WiFiMacRW(U8_T type, U8_T XDATA* pReg);
U8_T IO_RadioOnOff(U8_T state);
void IO_BgProtectionRW(U8_T type, U8_T state);
void IO_EncryptionRW(U8_T type, U8_T value);
void IO_OpenSysRW(U8_T type, U8_T value);
void IO_Wep64KeyRW(U8_T type, U8_T index, U8_T XDATA* pReg);
void IO_Wep128KeyRW(U8_T type, U8_T index, U8_T XDATA* pReg);
void IO_WepKeyIndexRW(U8_T type, U8_T value);
void IO_TkipAesRW(U8_T type, U8_T len, U8_T XDATA* pReg);
void IO_FragmentThresholdRW(U8_T type, U8_T XDATA* pValue);
void IO_RtsThresholdRW(U8_T type, U8_T XDATA* pValue);
void IO_PreambleRW(U8_T type, U8_T value);
void IO_TxGainRW(U8_T rw, U8_T txGain);
void IO_BeaconIntervalRW(U8_T type, U8_T* pValue);
void IO_CountryRegionRW(U8_T type, U16_T value);
void IO_AtimIntervalRW(U8_T type, U8_T XDATA* pValue);
U8_T IO_TxDataRateRW(U8_T rw, U8_T txRate);
void IO_AutoPowerCtrlRW(U8_T type, U8_T OnOff);
void IO_AutoPowerCtrlDetailW(U8_T value);
void IO_Roaming(U8_T type, U8_T value);
void IO_WifiMultimediaSet(U8_T type, U8_T value);
void IO_MinContentWindowRW(U8_T type, U16_T value);
void IO_MaxContentWindowRW(U8_T type, U16_T value);
void IO_ShowRssi(void);
void IO_PayloadLenAddInWifi(U8_T type, U8_T value);
void IO_RetainLlcInWifi(U8_T type, U8_T value);
void IO_2LlcInWifiVlan(U8_T type, U8_T value);
void IO_TrafficProfile(U8_T type, U8_T value);
void IO_TxDefaultPowerLevelRW(U8_T rw, U8_T level);
void IO_SelfCtsRW(U8_T rw, U8_T value);
void IO_BssidR(void);
void IO_WifiIpR(void);
#if MASS_PRODUCTION
U8_T IO_ContinuousTx(U8_T mode);
void IO_SensitivityTest(U16_T counts, U8_T source);
void IO_SensitivityTestReport(void);
#endif
void IO_SfrRW(U8_T type, U8_T index, U8_T XDATA value);
U8_T IO_WcpuIndirectRW(U8_T type, U8_T index, U8_T XDATA* pReg);
U8_T IO_MiibIndirectRW(U8_T type, U8_T index, U8_T XDATA* pReg);
U8_T IO_PcibIndirectRegRW(U8_T type, U8_T index, U8_T XDATA* pReg);
void IO_PciConfigRW(U8_T type, U8_T index, U8_T XDATA* pReg);
void IO_WiFiRegRW(U8_T type, U8_T XDATA* pValue, U8_T XDATA* pReg);
void IO_WlanPktFilterRW(U8_T rw, U8_T type, U16_T subtype, U8_T XDATA* pBssid);
void IO_SynthesizerSet(U8_T XDATA* pValue);
void IO_RetryRW(U8_T type, U8_T value);
void IO_Gpio0RW(U8_T type, U8_T value);
void IO_Gpio1RW(U8_T type, U8_T value);
void IO_Gpio2RW(U8_T type, U8_T value);
void IO_Gpio3RW(U8_T type, U8_T value);
void IO_ZPHYTestW(void);
void IO_WMIBTestW(U8_T XDATA enable);
void IO_APNFTW(void);
void IO_ENCTW(void);
void IO_WifiDriverVer(void);
void IO_SiteSurveyReportAck(void);
#else			/***** Middle of IO_CPU_TYPE, Below are for WCPU use *****/
void IO_NoticeMcpu(U8_T, U8_T*, U8_T);
void IO_ScanReport(U8_T result);
void IO_SiteSurveyReport(U8_T BssNum, U8_T IsTheLast);
#if MASS_PRODUCTION
void IO_SensitivityTestReport(U8_T XDATA* pValue);
#endif
void IO_WlanDebug1(U8_T value);
#endif			/***** End of IO_CPU_TYPE macro *****/

#endif /* End of __WIOCTL_H__ */

/* End of wioctrl.h */
