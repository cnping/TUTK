#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include "P2PTunnelAPIs.h"

#define MAX_SERVER_CONNECT_NUM		4
#define WEB_MAPPING_LOCAL_PORT		10000
#define SSH_MAPPING_LOCAL_PORT		20000
#define TELNET_MAPPING_LOCAL_PORT	30000
#define PASSWORD_MAPPING_LOCAL_PORT	40000
#define WEB_MAPPING_REMOTE_PORT		80
#define SSH_MAPPING_REMOTE_PORT		22
#define TELNET_MAPPING_REMOTE_PORT	23
#define PASSWORD_MAPPING_REMOTE_PORT	9000

char *gUID[MAX_SERVER_CONNECT_NUM];
int gSID[MAX_SERVER_CONNECT_NUM];
int gProcessRun = 1;
int gWebIndex[MAX_SERVER_CONNECT_NUM];
int gTelnetIndex[MAX_SERVER_CONNECT_NUM];
int gSshIndex[MAX_SERVER_CONNECT_NUM];
int gPasswordIndex[MAX_SERVER_CONNECT_NUM];
int gRetryConnectFailedCnt[MAX_SERVER_CONNECT_NUM];

int gWebPort = WEB_MAPPING_LOCAL_PORT;

// default password if no "passwd.txt" exist
char gPassword[24]="P2P Platform";

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
		gSID[i] = P2PTunnelAgent_Connect(gUID[i], NULL, 0, &nErrFromDevice);
		*/
		gSID[i] = P2PTunnelAgent_Connect(gUID[i], (void *)pAuthData, sizeof(sAuthData), &nErrFromDevice);
		
		if(gSID[i] < 0)
		{
			printf("P2PTunnelAgent_Connect failed[%d], device respond reject reason[%d]\n", gSID[i], nErrFromDevice);
			return -1;
		}
		else printf("P2PTunnelAgent_Connect OK\n");

		if(P2PTunnel_SetBufSize(gSID[i], 5120000) < 0)
			printf("P2PTunnel_SetBufSize error SID[%d]\n", gSID[i]);
		
		gWebIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[i], gWebPort+i, WEB_MAPPING_REMOTE_PORT);
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
		
		gPasswordIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[i], PASSWORD_MAPPING_LOCAL_PORT+i, PASSWORD_MAPPING_REMOTE_PORT);
		if(gPasswordIndex[gSID[i]] < 0)
		{
			printf("P2PTunnelAgent_PortMapping Password error[%d]!\n", gPasswordIndex[gSID[i]]);
		}
	}
	
	return 0;
}

void TunnelStatusCB(int nErrorCode, int nSID, void *pArg)
{
	if(nErrorCode == TUNNEL_ER_DISCONNECTED)
	{
		if(pArg != NULL)
			printf("MyArg = %s\n", (char *)pArg);
		
		P2PTunnelAgent_StopPortMapping(gWebIndex[nSID]);
		P2PTunnelAgent_StopPortMapping(gTelnetIndex[nSID]);
		P2PTunnelAgent_StopPortMapping(gSshIndex[nSID]);
		P2PTunnelAgent_StopPortMapping(gPasswordIndex[nSID]);
		printf("StopPortMapping OK\n");

		P2PTunnelAgent_Disconnect(nSID);
		printf("P2PTunnelAgent_Disconnect OK SID[%d] nErrorCode[%d]\n", nSID, nErrorCode);
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
		gPasswordIndex[i] = -1;
		gRetryConnectFailedCnt[i] = 0;
	}
}

int main(int argc, char *argv[])
{
	FILE *pfd = NULL;
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
	
	if( (pfd=fopen("passwd.txt","r")) !=NULL )
	{
		int len=0;
		memset(gPassword, 0, sizeof(gPassword));
		if ( fgets(gPassword, sizeof(gPassword), pfd) != NULL )
		{
		}
		
			len = strlen(gPassword);
		if(len>0 && gPassword[len-1]=='\n')
			gPassword[len-1] = '\0';
		fclose(pfd);
	}
	strcpy(authData.szPassword, gPassword);
	
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
			if(gSID[i] >= 0)
			{
				int access_time = P2PTunnel_LastIOTime(gSID[i]);
				if(access_time >= 0)
					printf("SID %d:%u\n", gSID[i], access_time);
			}
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
					if(P2PTunnel_SetBufSize(gSID[i], 5120000) < 0)
						printf("P2PTunnel_SetBufSize error SID[%d]\n", gSID[i]);
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
					gPasswordIndex[gSID[i]] = P2PTunnelAgent_PortMapping(gSID[i], PASSWORD_MAPPING_LOCAL_PORT+i, PASSWORD_MAPPING_REMOTE_PORT);
					if(gPasswordIndex[gSID[i]] < 0)
					{
						printf("P2PTunnelAgent_PortMapping Password error[%d]!\n", gPasswordIndex[gSID[i]]);
					}
					gRetryConnectFailedCnt[i] = 0;
				}
			}
		}
		
		sleep(2);
	}
	
	return 0;
}

