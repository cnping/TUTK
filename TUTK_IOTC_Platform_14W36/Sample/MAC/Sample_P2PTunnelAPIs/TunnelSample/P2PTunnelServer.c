#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "P2PTunnelAPIs.h"

char gUID[21];

typedef struct st_AuthData
{
	char szUsername[64];
	char szPassword[64];
} sAuthData;

void TunnelStatusCB(int nErrorCode, int nSID, void *pArg)
{
	if(nErrorCode == TUNNEL_ER_DISCONNECTED)
	{
		printf("SID[%d] TUNNEL_ER_DISCONNECTED Log file here!\n", nSID);
		if(pArg != NULL)
			printf("MyArg = %s\n", (char *)pArg);
	}
}

void *Thread_TestCloseSession(void *arg)
{
	pthread_detach(pthread_self());
	
	int SID = *((int *)arg);
	free(arg);
	srand(time(NULL));
    int nDelayTime = rand() % 60;
    printf("nDelayTime to close[%d]\n", nDelayTime);
	sleep(nDelayTime);
	printf("P2PTunnelServer_Disconnect SID[%d] ret[%d]\n", SID, P2PTunnelServer_Disconnect(SID));
	
	return 0;
}

int TunnelSessionInfoCB(sP2PTunnelSessionInfo *sSessionInfo, void *pArg)
{
	printf("TunnelSessionInfoCB trigger\n");
	if(pArg != NULL) printf("pArg = %s\n", (char *)pArg);
	printf("[Client Session Info]\n");
	printf("  Connection Mode = %d, NAT type = %d\n", sSessionInfo->nMode, sSessionInfo->nNatType);
	printf("  P2PTunnel Version = %X, SID = %d\n", (unsigned int)sSessionInfo->nVersion, sSessionInfo->nSID);
	printf("  IP Address = %s:%d\n", sSessionInfo->szRemoteIP, sSessionInfo->nRemotePort);
	
	if(sSessionInfo->nAuthDataLen == 0 || sSessionInfo->pAuthData == NULL)
		return -777;
	else if(sSessionInfo->nAuthDataLen > 0)
	{
		sAuthData *pAuthData = (sAuthData *)sSessionInfo->pAuthData;
		printf("  Auth data length = %d, username = %s, passwd = %s\n", sSessionInfo->nAuthDataLen, pAuthData->szUsername, pAuthData->szPassword);
		if(strcmp(pAuthData->szUsername, "Tutk.com") != 0 || strcmp(pAuthData->szPassword, "P2P Platform") != 0)
			return -888;
	}
	
	#if 0 // test random time to close session
	int *SID = (int *)malloc(sizeof(int));
	if(SID != NULL)
	{
		*SID = sSessionInfo->nSID;
		pthread_t threadID;
		pthread_create(&threadID, NULL, &Thread_TestCloseSession, (void *)SID);
	}
	#endif
	
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("No UID input!!!\n");
		return 0;
	}
	
	strcpy(gUID, argv[1]);
	
	char *s = "My arg Pass to call back function";
	P2PTunnelServer_GetStatus(TunnelStatusCB, (void *)s);
	printf("Tunnel Version[%X]\n", P2PTunnel_Version());
	int ret = P2PTunnelServerInitialize(20);
	if(ret < 0)
	{
		printf("P2PTunnelServerInitialize error[%d]!\n", ret);
		return -1;
	}
	ret = P2PTunnelServer_Start(gUID);
	if(ret < 0)
	{
		printf("P2PTunnelServer_Start error[%d]!\n", ret);
		return -1;
	}
	
	/* If you don't want to use authentication mechanism, you can give NULL argument
	ret = P2PTunnelServer_GetSessionInfo(TunnelSessionInfoCB, NULL);
	*/
	ret = P2PTunnelServer_GetSessionInfo(TunnelSessionInfoCB, (void *)s);
	
	printf("Call P2PTunnelServer_GetSessionInfo ret[%d]\n", ret);
	
	while(1)
	{
		sleep(3600);
	}
	
	return 0;
}

