#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>
#include "P2PTunnelAPIs.h"

// Enable auto launch PasswordServer.
#define LAUNCH_PASSWORD_PROC 0

char gUID[21];

int gSID_Used[20];
int gSID_StartTime[20];

// default password if no "passwd.txt" exist
char gPassword[24]="P2P Platform";

#if LAUNCH_PASSWORD_PROC
int gPassPID=0;
#endif

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
		gSID_Used[nSID] = 0;
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
		FILE *pfd = fopen("passwd.txt","r");
		if(pfd!=NULL)
		{
			int len=0;
			memset(gPassword, 0, sizeof(gPassword));
			//int ret = fread(gPassword, 1, sizeof(gPassword)-1, pfd);
			fgets(gPassword, sizeof(gPassword), pfd);
			len = strlen(gPassword);
			if(len>0 && gPassword[len-1]=='\n')
				gPassword[len-1] = '\0';
			fclose(pfd);
		}
		sAuthData *pAuthData = (sAuthData *)sSessionInfo->pAuthData;
		printf("  Auth data length = %d, username = %s, passwd = %s\n", sSessionInfo->nAuthDataLen, pAuthData->szUsername, pAuthData->szPassword);
		if(strcmp(pAuthData->szUsername, "Tutk.com") != 0 || strcmp(pAuthData->szPassword, gPassword) != 0)
			return -888;
	}
	gSID_Used[sSessionInfo->nSID] = 1;
	gSID_StartTime[sSessionInfo->nSID] = time(NULL);
	if(P2PTunnel_SetBufSize(sSessionInfo->nSID, 5120000) < 0)
		printf("P2PTunnel_SetBufSize error SID[%d]\n", sSessionInfo->nSID);
	
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

#if LAUNCH_PASSWORD_PROC
// Clean up Password setting process.
void clean_up_passd(int sig)
{
	printf("Clean up PasswordServer process.\n");
	waitpid(gPassPID, 0, WNOHANG);
}
#endif

int main(int argc, char *argv[])
{
	int errCode =0, retry=0;
	if(argc < 2)
	{
		printf("No UID input!!!\n");
		return 0;
	}
	
	strcpy(gUID, argv[1]);
	int i;
	for(i=0;i<20;i++)
		gSID_Used[i]=0;

#if LAUNCH_PASSWORD_PROC		
	// create password process
	// note: you can launch process manually, depend on your product.
	gPassPID = fork();
	if(gPassPID==0)	//child process
	{
		printf("PasswordServer Created!!\n");
		execlp("./PasswordServer", "PasswordServer", "9000",NULL);
		exit(0);
	}
	signal(SIGCHLD, clean_up_passd);
	printf("PasswordServer PID=%d\n", gPassPID);
#endif
	
	
	char *s = "My arg Pass to call back function";
	P2PTunnelServer_GetStatus(TunnelStatusCB, (void *)s);
	printf("Tunnel Version[%X]\n", P2PTunnel_Version());
	int ret = P2PTunnelServerInitialize(20);
	if(ret < 0)
	{
		printf("P2PTunnelServerInitialize error[%d]!\n", ret);
		return -1;
	}

	errCode = P2PTunnelServer_Start(gUID);
	if( errCode<0 )
	{
		// No Internet
		if( errCode == -41)
			printf("No Internet, error[%d]!! Reconnect after 15sec...\n", errCode);
		else
		{
			printf("P2PTunnelServer_Start error[%d]!\n", errCode);
			return -1;
		}
	}
	else
		printf("P2PTunnelServer_Start Success, I can connected by Internet.\n");
	
	/* If you don't want to use authentication mechanism, you can give NULL argument
	ret = P2PTunnelServer_GetSessionInfo(TunnelSessionInfoCB, NULL);
	*/
	ret = P2PTunnelServer_GetSessionInfo(TunnelSessionInfoCB, (void *)s);
	
	printf("Call P2PTunnelServer_GetSessionInfo ret[%d]\n", ret);
	
	while(1)
	{
		int end =0;
		
		// Retry connect to Internet every 15 sec.
		if( errCode<0 && ++retry%15==0)
		{
			errCode = P2PTunnelServer_Start(gUID);
			if( errCode<0 )
			{
				// No Internet
				if( errCode== -41)
					printf("No Internet, error[%d]!! Reconnect after 15sec...\n", errCode);
				else
				{
					printf("P2PTunnelServer_Start error[%d]!\n", errCode);
					return -1;
				}
			}
			else
				printf("P2PTunnelServer_Start Success, I can connected by Internet.\n");
			retry=0;
		}
		for(i=0; i<20; i++)
		{
			if(gSID_Used[i])
			{
				int access_time = P2PTunnel_LastIOTime(i);
				if(access_time < 0)
				{
					printf("P2PTunnel_LastIOTime Error Code %d\n", access_time);
					gSID_Used[i] = 0;
				}
				else
					printf("SID %d:%u, ", i, access_time);
				end =1;
			}
		}
		if(end)
			printf("\n");
		sleep(1);
	}
	
	return 0;
}

