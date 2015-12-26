#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <sys/timeb.h>
#include <wchar.h>
#include <io.h>
#pragma comment(lib, "ws2_32")
#define WSA_VERSION MAKEWORD(2, 2) // using winsock 2.2
#include "P2PTunnelAPIs.h"
#include "RDTAPIs.h"
#include "IOTCAPIs.h"

#define MAX_SERVER_CONNECT_NUM		4
#define WEB_MAPPING_LOCAL_PORT		10000
#define SSH_MAPPING_LOCAL_PORT		20000
#define TELNET_MAPPING_LOCAL_PORT	30000
#define WEB_MAPPING_REMOTE_PORT		8080
#define SSH_MAPPING_REMOTE_PORT		22
#define TELNET_MAPPING_REMOTE_PORT	23

char *gUID[MAX_SERVER_CONNECT_NUM];
int gSID[MAX_SERVER_CONNECT_NUM];
int gProcessRun = 1;
int gWebIndex[MAX_SERVER_CONNECT_NUM];
int gTelnetIndex[MAX_SERVER_CONNECT_NUM];
int gSshIndex[MAX_SERVER_CONNECT_NUM];
int gRetryConnectFailedCnt[MAX_SERVER_CONNECT_NUM];

typedef struct st_AuthData
{
	char szUsername[64];
	char szPassword[64];
} sAuthData;

int TunnelAgentStart(sAuthData *pAuthData)
{
	int ret = P2PTunnelAgentInitialize(MAX_SERVER_CONNECT_NUM);
	if(ret < 0)
	{
		printf("P2PTunnelAgentInitialize error[%d]!\n", ret);
		return -1;
	}
	
	int i;
	for(i=0;i<MAX_SERVER_CONNECT_NUM;i++)
	{
		if(gUID[i] == NULL) continue;
		
		int nErrFromDevice = 0;
		/* If you don't want to use authentication mechanism, you can give NULL argument
		gSID[i] = P2PTunnelAgent_Connect(gUID, NULL, 0, &nErrFromDevice);
		*/
		gSID[i] = P2PTunnelAgent_Connect(gUID[i], (void *)pAuthData, sizeof(sAuthData), &nErrFromDevice);
		
		if(gSID[i] < 0)
		{
			printf("P2PTunnelAgent_Connect failed[%d], device respond reject reason[%d]\n", gSID[i], nErrFromDevice);
			return -1;
		}
		else printf("P2PTunnelAgent_Connect OK\n");
		
		gWebIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[i], WEB_MAPPING_LOCAL_PORT+i, WEB_MAPPING_REMOTE_PORT);
		if(gWebIndex[gSID[i]] < 0)
		{
			printf("P2PTunnelAgent_PortMapping WEB error[%d]!\n", gWebIndex[gSID[i]]);
		}
		
		gTelnetIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[gSID[i]], TELNET_MAPPING_LOCAL_PORT+i, TELNET_MAPPING_REMOTE_PORT);
		if(gTelnetIndex[gSID[i]] < 0)
		{
			printf("P2PTunnelAgent_PortMapping Telnet error[%d]!\n", gTelnetIndex[gSID[i]]);
		}
		
		gSshIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[i], SSH_MAPPING_LOCAL_PORT+i, SSH_MAPPING_REMOTE_PORT);
		if(gSshIndex[gSID[i]] < 0)
		{
			printf("P2PTunnelAgent_PortMapping SSH error[%d]!\n", gSshIndex[gSID[i]]);
		}
	}
	
	return 0;
}

void __stdcall TunnelStatusCB(int nErrorCode, int nSID, void *pArg)
{
	if(nErrorCode == TUNNEL_ER_DISCONNECTED)
	{
		if(pArg != NULL)
			printf("MyArg = %s\n", (char *)pArg);
		
		P2PTunnelAgent_StopPortMapping(gWebIndex[nSID]);
		P2PTunnelAgent_StopPortMapping(gTelnetIndex[nSID]);
		P2PTunnelAgent_StopPortMapping(gSshIndex[nSID]);
		printf("StopPortMapping OK\n");

		P2PTunnelAgent_Disconnect(nSID);
		printf("P2PTunnelAgent_Disconnect OK SID[%d]\n", nSID);

		int i;
		for(i=0;i<MAX_SERVER_CONNECT_NUM;i++)
		{
			if(gSID[i] == nSID)
			{
				gSID[i] = -1;
				break;
			}
		}
	}
}

void InitGlobalArgument()
{
	int i;
	for(i=0;i<MAX_SERVER_CONNECT_NUM;i++)
	{
		gUID[i] = NULL;
		gSID[i] = -1;
		gWebIndex[i] = -1;
		gSshIndex[i] = -1;
		gTelnetIndex[i] = -1;
		gRetryConnectFailedCnt[i] = 0;
	}
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("No UID input!!!\n");
		return 0;
	}
	
	InitGlobalArgument();

	int i, j = 0;
	for(i=1;i<argc;i++)
	{
		gUID[j] = (char *)malloc(21);
		if(gUID[j] != NULL)
		{
			strcpy(gUID[j], argv[i]);
			printf("SERVER UID[%s]\n", gUID[j]);
			j++;
		}
	}
	
	char *s = "My arg Pass to call back function";
	P2PTunnelAgent_GetStatus(TunnelStatusCB, (void *)s);
	
	sAuthData authData;
	strcpy(authData.szUsername, "Tutk.com");
	strcpy(authData.szPassword, "P2P Platform");
	
	if(TunnelAgentStart(&authData) < 0)
	{
		printf("TunnelAgentStart failed\n");
		return -1;
	}
	
	while(gProcessRun)
	{
		int i;
		for(i=0;i<MAX_SERVER_CONNECT_NUM;i++)
		{
			if(gUID[i] != NULL && gSID[i] < 0)
			{
				printf("Reconnect to UID[%s]\n", gUID[i]);
				int nErrFromDevice;
				gSID[i] = P2PTunnelAgent_Connect(gUID[i], (void *)&authData, sizeof(sAuthData), &nErrFromDevice);
				if(gSID[i] < 0)
				{
					printf("P2PTunnelAgent_Connect failed[%d], UID[%s], device respond reject reason[%d]\n", gSID[i], gUID[i], nErrFromDevice);
					if(++gRetryConnectFailedCnt[i] > 3)
					{
						printf("Retry connection timeout UID[%s]\n", gUID[i]);
						free(gUID[i]);
						gUID[i] = NULL;
					}
				}
				else
				{
					gWebIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[i], WEB_MAPPING_LOCAL_PORT+i, WEB_MAPPING_REMOTE_PORT);
					if(gWebIndex[gSID[i]] < 0)
					{
						printf("P2PTunnelAgent_PortMapping WEB error[%d]!\n", gWebIndex[gSID[i]]);
					}
					
					gTelnetIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[gSID[i]], TELNET_MAPPING_LOCAL_PORT+i, TELNET_MAPPING_REMOTE_PORT);
					if(gTelnetIndex[gSID[i]] < 0)
					{
						printf("P2PTunnelAgent_PortMapping Telnet error[%d]!\n", gTelnetIndex[gSID[i]]);
					}
					
					gSshIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[i], SSH_MAPPING_LOCAL_PORT+i, SSH_MAPPING_REMOTE_PORT);
					if(gSshIndex[gSID[i]] < 0)
					{
						printf("P2PTunnelAgent_PortMapping SSH error[%d]!\n", gSshIndex[gSID[i]]);
					}
					gRetryConnectFailedCnt[i] = 0;
				}
			}
		}
		Sleep(2);
	}
	
	return 0;
}

