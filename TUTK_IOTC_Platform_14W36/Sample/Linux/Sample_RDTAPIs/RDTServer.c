#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include "IOTCAPIs.h"
#include "RDTAPIs.h"
#include "common.h"

int gFd;
static char gFn[64];

void *thread_Send(void *arg)
{
	FILE *fp = fopen(gFn, "rb");
	if(fp == NULL)
	{
		printf("FOpen file[%s] errno[%d]!\n", gFn, errno);
		pthread_exit(0);
	}

	int SID = *(int *)arg;
	struct st_SInfo sInfo;
	IOTC_Session_Check(SID, &sInfo);
	char buf[MAX_BUF_SIZE];
	int RDT_ID = RDT_Create(SID, RDT_WAIT_TIMEMS, 0);
	if(RDT_ID < 0)
	{
		printf("RDT_Create failed[%d]!!\n", RDT_ID);
		IOTC_Session_Close(SID);
		fclose(fp);
		pthread_exit(0);
	}

	printf("RDT_ID = %d\n", RDT_ID);

	int ret = RDT_Read(RDT_ID, buf, 1024, RDT_WAIT_TIMEMS);
	if(ret < 0)
	{
		printf("RDT rcv cmd error[%d]!\n", ret);
		fclose(fp);
		pthread_exit(0);
	}
	int requestCnt = atoi(buf), i;
	struct stat fStat;
	fstat(gFd, &fStat);
	struct st_RDT_Status rdtStatus;
	printf("requestCnt[%d] file size[%lu]\n", requestCnt, fStat.st_size);
	sprintf(buf, "%lu", fStat.st_size);
	ret =  RDT_Write(RDT_ID, buf, strlen(buf));
	if(ret < 0)
	{
		printf("RDT send file size error[%d]!\n", ret);
		fclose(fp);
		pthread_exit(0);
	}
	ret = RDT_Read(RDT_ID, buf, 1024, RDT_WAIT_TIMEMS);
	if(ret < 0 && strcmp(buf, START_STRING) != 0)
	{
		printf("RDT rcv start to send file error[%d]!\n", ret);
		fclose(fp);
		pthread_exit(0);
	}

	for(i=0;i<requestCnt;i++)
	{
		while(1)
		{
			ret = fread(buf, 1, MAX_BUF_SIZE, fp);
			if(ret <= 0) break;
			ret = RDT_Write(RDT_ID, buf, ret);
			if(ret < 0)
			{
				printf("RDT send file data error[%d]!\n", ret);
				pthread_exit(0);
			}

			if((ret = RDT_Status_Check(RDT_ID, &rdtStatus)) == RDT_ER_NoERROR)
			{
				if(sInfo.Mode == 2)
				{
					if(rdtStatus.BufSizeInSendQueue > 1024000)
						usleep(50000);
				}
				else
				{
					if(rdtStatus.BufSizeInSendQueue > 512000)
						usleep(100000);
						//sleep(1);
				}
			}
			else
			{
				printf("RDT status check error[%d]!\n", ret);
				pthread_exit(0);
			}
		}

		fseek(fp, 0, SEEK_SET);
	}

	printf("***RDT_Destroy enter\n");
	RDT_Destroy(RDT_ID);
	printf("thread_Send exit[%d]\n", RDT_ID);
	fclose(fp);

	while(1)
	{
		struct st_SInfo info;
		if( IOTC_Session_Check(SID, &info) < 0)
		{
			IOTC_Session_Close(SID);
			break;
		}
		sleep(1);
	}

	pthread_exit(0);
}

void *_thread_Login(void *arg)
{
	int ret;
	char *UID = (char *)arg;
	while(1)
	{
		ret = IOTC_Device_Login(UID, "TUTK", "1234");
		printf("     Calling IOTC_Device_Login() ret = %d\n", ret);
		if(ret == IOTC_ER_NoERROR)
			break;
		else
			sleep(60);
	}

	pthread_exit(0);
}

#define MAX_CLIENT_NUM 8

int main(int argc, char *argv[])
{
	if(argc < 4)
	{
		printf("Arg wrong!!!\n");
		printf("  --RDTServer [UID] [Filename] [MaxClientNum]\n");
		return 0;
	}
	printf("RDT Ver[%X]\n", RDT_GetRDTApiVer());

	int maxClientNum = atoi(argv[3]);
	strcpy(gFn, argv[2]);
	printf("%s\n", gFn);
	gFd = open(gFn, O_RDONLY);
	if(gFd < 0)
	{
		printf("Open file[%s] error code[%d], %d!!\n", gFn, gFd, errno);
		exit(0);
	}

	int ret = IOTC_Initialize(0, "61.188.37.216", "50.19.254.134", "m2.iotcplatform.com", "m4.iotcplatform.com");
	if(ret != IOTC_ER_NoERROR)
	{
		printf("IOTC_Initialize error!!\n");
		return 0;
	}

	int rdtCh = RDT_Initialize();
	if(rdtCh <= 0)
	{
		printf("RDT_Initialize error!!\n");
		return 0;
	}

	char *UID = (char *)argv[1];
	pthread_t threadID_Login;
	pthread_create(&threadID_Login, NULL, &_thread_Login, (void *)UID);
	pthread_detach(threadID_Login);

	int SID = -1, clientCnt = 0;
	struct st_SInfo Sinfo;
	printf("start IOTC_Listen...\n");
	//unsigned long t;
	pthread_t Thread_ID[MAX_CLIENT_NUM];
	do {
		SID = IOTC_Listen(300000);
		if(SID > -1)
		{
			IOTC_Session_Check(SID, &Sinfo);
			char *mode[3] = {"P2P", "RLY", "LAN"};
			printf("Client from %s:%d Mode=%s\n",Sinfo.RemoteIP, Sinfo.RemotePort, mode[(int)Sinfo.Mode]);
			pthread_create(&Thread_ID[clientCnt], NULL, &thread_Send, (void *)&SID);
			//if(clientCnt == 0) t = IOTC_GetTickCount();
			clientCnt++;
		}
		if(clientCnt == maxClientNum)
			break;
	}while(1);

	int i;
	for(i=0;i<maxClientNum;i++)
		pthread_join(Thread_ID[i], NULL);

	//printf("RDT Send Data Cost[%lu ms]....\n", IOTC_GetTickCount()-t);
	RDT_DeInitialize();
	printf("RDT_DeInitialize OK\n");
	IOTC_DeInitialize();

	close(gFd);
	printf("Server exit!\n");

	return 0;
}


