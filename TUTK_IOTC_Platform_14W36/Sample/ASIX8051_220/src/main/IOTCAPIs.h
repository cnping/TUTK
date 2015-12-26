/**====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

IOTCAPIs.h

Copyright (c) 2010 by TUTK Co.LTD. All Rights Reserved.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/**===========================================================================

 Revisions of IOTCProto.h
 Version  		Name       	Date			Description
 0.1  			Charlie	  	11/15/2010		 Trial Version
 0.2  			Charlie	  	12/28/2010		 IOTC_Get_Version, Session Alive 
 0.2.1			Charlie	  	02/28/2011		 Header structur --> Flag , IOTC_Connect return IOTC_ER_DEVICE_NOT_LISTENING, if Target device is not listening
 0.3				Charlie		03/02/2011		 Support LAN Mode. 
 0.4				Kevin		04/22/2011		 Support ASIX110/220 8051 platform.
 0.4.0.1			Charlie		04/28/2011		 Fix bug of LanSearch.
 0.4.0.2			Kevin		04/28/2011		 Fix bug of gBuf multi-thread access currently.
 0.4.0.3			Charlie		05/06/2011		 Fix ReadBuf bug because multi-thread read/write competition
 0.4.0.4			Charlie		05/10/2011		 Hope 312 can't support fopen, so undefine ShowVersionInfoinLogFile() for Hope' _ARC_COMPILER
 0.5.0.0			Charlie/Kevin	06/30/2011	0.5.0.0 - A major revision change of IOTCAPIs, lead-in 																	multi-channel mechanism and including Using IOTC protocol 0.2
 1.0.0.0			Kevin		2011-07-12		Formal version, stability testing
 1.0.1.0			Kevin		2011-07-25		Fix Bug : __Search_DeviceInfo() and __Search_VPGServerList(), add threshold
 1.0.2.0			Kevin		2011-07-27		Fix Bug : _Check_P2P_Possible(), impove _Check_Nat_Type(), Modify IOTC_TIME_OUT_SECOND_P2P_REQUEST 60->10
 1.1.0.0			Kevin		2011-07-29		Fix Bug : Before login reset nat check status, client query device got unlicense respond not return error code IOTC_ER_UNLICENSE. Modify : judge precheck respond last login time 300->120, Got MSG_LAN_SEARCH_R start with _IOTC_Send_Knock() procedure. Add API : IOTC_Search_Device_XXX() and IOTC_Search_Stop()
 1.1.1.0			Kevin		2011-08-01		Fix Bug : _IOTC_Check_Master() and _IOTC_Check_Server(), check one server once -> check all server once. Modify : IOTC_TIME_OUT_SECOND_P2P_REQUEST 10->6
 1.1.2.0			Kevin		2011-08-08		Fix Bug : 1.Linux thread not reclaim so must detach thread 2.when only do search device done not release session index. Modify : not wait resolve master result until call IOTC_Device_Login() or IOTC_Connect_XXX(), it will improve LAN mode performance
 1.2.0.0			Kevin		2011-08-29		Improve : Alloc read buffer dynamically instead of declare static memory 
Fix Bug : 1.IOTC_Session_Channel_OFF() not clean read buffer 2.SessionInfo member HeaderPacketCount not separate with channel
 1.2.1.0			Kevin		2011-09-08		Add : 1.IOTC_Initialize() setup p2p socket buffer size 2.judge UID if valid function. Modify : remove query time threshold of __Search_DeviceInfo() and __Search_VPGServerList(). Fix Bug : in IOTC_Device_Login() give wrong argument when call _IOTC_SendQuryServerList() to query third and fourth master. Custom : reduce static memory usage for ARC
 1.3.0.0			Kevin		2011-11-15		Fix Bug : 1.ASIX not convert NAT and version of login info from little endian to big endian. 2.iPhone and Win32 Lan search 3.use worong VPG info to login. 4.Lan search send ARP to query will cause AP crash, send x.x.x.255 instead. Add : 1.use UID to bind lan search port. 2.UID toupper. 3.Network flow mechanism to know response and packet lost rate. Custom : replace malloc and free with p2p_malloc and p2p_free for ARC. Improve : 1.IOTC_DeInitialize() to use pthread_join like function to wait thread exit. 2. IOTC_Device_Login() return IOTC_ER_NoERROR when master address is "127.0.0.1" for only use LAN mode. Private : 1.search all device on LAN 2.force relay mode to connect.
 1.3.1.0			Kevin		2011-11-24		Fix Bug : _IOTC_SendAlive() via relay always give RLYSessionID 0 Improve : 1.When call IOTC_DeInitialize() to close all session. 2.When receive remote session close or keep alive timeout not clean read buf until user call IOTC_Session_Close().
 1.3.2.0			Kevin		2011-12-21		Fix Bug : 1.IOTC_Connect_ByUID duplicate call at the same time will return wrong SID. 2.IOTC_Connect_ByName() return IOTC_ER_UNLICENSE error and Lan search error. 3.IOTC_Connect_ByUID() use wrong VPG to send query to third and fourth Master. Add : 1.IOTC_Set_Max_Session_Number() to setup session number for control memory usage. 2._IOTC_thread_Device_Login also watch a alive timeout counter for avoiding _IOTC_thread_Session_Alive crash. Modify : int IOTC_Get_Login_Info(), if return > 0 means login failure count.
 1.4.0.0			Kevin		2012-02-02		Fix Bug : Mutex lock use error on WIN32 platform. Add : 1.IOTC_sListen() and IOTC_sConnect() create AES encryption session. 2.Big-endian SOC suport: device and client and AES encryption.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

#ifndef _IOTCAPIs_charlie_H_
#define _IOTCAPIs_charlie_H_

#include "platform_Config.h"

#ifdef IOTC_Win32
#ifdef P2PAPI_EXPORTS
#define P2PAPI_API __declspec(dllexport)
#else
#define P2PAPI_API __declspec(dllimport)
#endif
#endif ////#ifdef IOTC_Win32

#ifdef IOTC_ARC_HOPE312
#define P2PAPI_API 
#endif ////#ifdef IOTC_ARC_HOPE312

#ifdef IOTC_Linux
#define P2PAPI_API 
#endif ////#ifdef IOTC_Linux

#ifdef IOTC_ASIX8051
#define P2PAPI_API 
#endif ////#ifdef IOTC_Linux

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct st_SInfo
{
	char Mode; // 0:P2P mode,  1:Relay mode
	char CorD; // 0: As a Client, 1: As a Device
	char UID[21];
	char RemoteIP[17];
	unsigned short RemotePort;
	unsigned long TX_Packetcount;
	unsigned long RX_Packetcount;
	unsigned long IOTCVersion;
	unsigned short VID, PID,GID;
	unsigned char NatType; //remote NAT type
	char reserved2;
};

#define IOTC_MAX_PACKET_SIZE					1400
#ifdef IOTC_ASIX8051
#define MAX_CLIENT_NUMBER 4
#endif
#define MAX_CHANNEL_NUMBER  32

//// --------------------- P2P APIs -----------------------------------
P2PAPI_API int IOTC_Get_Login_Info(unsigned long *LoginInfo);
P2PAPI_API void IOTC_Get_Version(unsigned long *Version);
P2PAPI_API void IOTC_Set_Max_Session_Number(unsigned long num);
P2PAPI_API int  IOTC_Initialize(unsigned short UDPPort, char* P2PHostNamePrimary,char* P2PHostNameSecondary,
								char* P2PHostNameThird, char* P2PHostNameFourth);
P2PAPI_API int  IOTC_DeInitialize();
P2PAPI_API int  IOTC_Device_Login(char *UID, char *DeviceName, char *DevicePWD);
P2PAPI_API int  IOTC_Listen(unsigned long Timeout_ms);  // Timeout_ms is no use in 8051
P2PAPI_API int  IOTC_sListen(unsigned long Timeout_ms, char *AesKey);  // Timeout_ms is no use in 8051
#ifndef IOTC_ASIX8051
P2PAPI_API int  IOTC_Connect_ByName(char *DeviceName, char *DevicePWD);
P2PAPI_API int  IOTC_Connect_ByUID(char *UID);
P2PAPI_API int  IOTC_sConnect_ByName(char *DeviceName, char *DevicePWD, char *AesKey);
P2PAPI_API int  IOTC_sConnect_ByUID(char *UID, char *AesKey);
P2PAPI_API void IOTC_Connect_Stop();
P2PAPI_API int  IOTC_Search_Device_ByName(char *DeviceName, char *DevicePWD, unsigned long Timeout_sec);
P2PAPI_API int  IOTC_Search_Device_ByUID(char *UID, unsigned long Timeout_sec);
P2PAPI_API void IOTC_Search_Stop();
#endif
P2PAPI_API int  IOTC_Session_Check(int SID, struct st_SInfo *S_Info);
P2PAPI_API int  IOTC_Session_Read(int SID, char* Buf, int Buf_Size,unsigned long Timeout_ms, unsigned char ChID);
#ifndef IOTC_ASIX8051
P2PAPI_API int  IOTC_Session_Read_Check_Lost(int SID, char* Buf, int Buf_Size,unsigned long Timeout_ms, unsigned short *PacketSN,char *bFlagLost, unsigned char ChID);
#endif
P2PAPI_API int IOTC_Session_Write(int SID, char* Buf, int Size, unsigned char ChID);
P2PAPI_API void IOTC_Session_Close(int SID);
#ifdef IOTC_ASIX8051
void IOTC_PeriodicRun();  //--call it in main loop
#endif
P2PAPI_API int IOTC_Session_Channel_ON(int SID, unsigned char ChID);
P2PAPI_API int IOTC_Session_Channel_OFF(int SID, unsigned char ChID);
P2PAPI_API int IOTC_Get_Nat_Type();  //local NAT type
//// --------------------- Error Code ---------------------------------
#define		IOTC_ER_NoERROR						0	// No Error
#define		IOTC_ER_SERVER_NOT_RESPONSE			-1	// IOTC Server not response, probably caused by internet connection problem
#define		IOTC_ER_FAIL_RESOLVE_HOSTNAME		-2	// Can't Resolved P2P Server's Domain name, please check 1)network connection or 2) DNS setting 
#define		IOTC_ER_ALREADY_INITIALIZED			-3	// Already Initialized. Reinitialization is not necessary.
#define		IOTC_ER_FAIL_CREATE_MUTEX			-4	// Mutex Creation Failed. 
#define		IOTC_ER_FAIL_CREATE_THREAD			-5	// Thread Creation Failed. 
#define		IOTC_ER_FAIL_CREATE_SOCKET			-6	// Socket Creaktion Failed.
#define		IOTC_ER_FAIL_SOCKET_OPT				-7	// Socket option setting Failed.
#define		IOTC_ER_FAIL_SOCKET_BIND			-8	// Socket bind Failed
#define		IOTC_ER_UNLICENSE					-10	// The specified ID is not licensed.
#define		IOTC_ER_LOGIN_ALREADY_CALLED		-11	// The IOTC_Device_Login function is called already.
#define		IOTC_ER_NOT_INITIALIZED				-12	// The IOTC_Initialize function must be called.
#define		IOTC_ER_TIMEOUT						-13 // Timeout !!
#define		IOTC_ER_INVALID_SID					-14 // The specified Session ID is not correct!!
#define		IOTC_ER_UNKNOWN_DEVICE				-15 // The specified UID , or DeviceName/PWD is Unknown.
#define		IOTC_ER_FAIL_GET_LOCAL_IP			-16	// Can't Get local IP
#define		IOTC_ER_LISTEN_ALREADY_CALLED		-17	// IOTC_Listen already called
#define		IOTC_ER_EXCEED_MAX_SESSION			-18 // Exceed the max allowed session number
#define		IOTC_ER_CAN_NOT_FIND_DEVICE			-19 // No Server knows Where is the Device
#define		IOTC_ER_CONNECT_IS_CALLING			-20 // IOTC_Connect_ByXX() is on the way
#define		IOTC_ER_SENDTO_FAILED				-21 // the UDP sendto() function failed 
#define		IOTC_ER_SESSION_CLOSE_BY_REMOTE		-22 // the remote site already close this session, please call IOTC_Session_Close() to release session resource
#define		IOTC_ER_REMOTE_TIMEOUT_DISCONNECT	-23 // This session is disconnected due to remote site has no any response after a 'timrout' period of time. 15 sec.
#define		IOTC_ER_DEVICE_NOT_LISTENING		-24 // The target device is not on listening, so connection can't be setup.
#define		IOTC_ER_DEVICE_NOT_LOGIN			-25 // The target device not login in
#define		IOTC_ER_CH_NOT_ON					-26 // This channel not turn ON
#define		IOTC_ER_FAIL_CONNECT_SEARCH			-27 // User stop connect or search procedure
#define		IOTC_ER_MASTER_TOO_FEW				-28 // Must give at last two master
#define		IOTC_ER_AES_CERTIFY_FAIL			-29 // Client give error string to certify
#define		IOTC_ER_DEVICE_NOT_SECURE_LISTEN	-30 // Device not a AES security channel
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _IOTCAPIs_charlie_H_ */

