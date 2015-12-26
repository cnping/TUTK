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
#include <tchar.h>
#pragma comment(lib, "ws2_32")
#define WSA_VERSION MAKEWORD(2, 2) // using winsock 2.2
#include "IOTCAPIs.h"
#include "RDTAPIs.h"

#include "common.h"

static char gFn[64];

DWORD WINAPI SendData(void* arg)
{
	FILE *fp;
	if(fopen_s(&fp, gFn, "rb") != 0)
	{
		printf("FOpen file[%s] errno\n", gFn);
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	unsigned long fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	int SID = *((int *)arg);
	free(arg);
	int RDT_ID = RDT_Create(SID, RDT_WAIT_TIMEMS, 0);
	printf("RDT_ID = %d\n", RDT_ID);
	if(RDT_ID < 0)
	{
		printf("RDT_Create failed[%d]!!\n", RDT_ID);
		IOTC_Session_Close(SID);
		return 0;
	}
	
	struct st_SInfo sInfo;
	IOTC_Session_Check(SID, &sInfo);
	struct st_RDT_Status rdtStatus;
	char buf[MAX_BUF_SIZE];
	
	int ret = RDT_Read(RDT_ID, buf, 1024, RDT_WAIT_TIMEMS);
	if(ret < 0)
	{
		printf("RDT rcv cmd error[%d]!\n", ret);
		return 0;
	}
	int requestCnt = atoi(buf), i;
	
	printf("requestCnt[%d] file size[%lu]\n", requestCnt, fileSize);
	sprintf(buf, "%lu", fileSize);
	ret =  RDT_Write(RDT_ID, buf, strlen(buf));
	if(ret < 0)
	{
		printf("RDT send file size error[%d]!\n", ret);
		return 0;
	}
	ret = RDT_Read(RDT_ID, buf, 1024, RDT_WAIT_TIMEMS);
	if(ret < 0 && strcmp(buf, START_STRING) != 0)
	{
		printf("RDT rcv start to send file error[%d]!\n", ret);
		return 0;
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
				return 0;
			}
			
			if((ret = RDT_Status_Check(RDT_ID, &rdtStatus)) == RDT_ER_NoERROR)
			{
				if(sInfo.Mode == 2)
				{
					if(rdtStatus.BufSizeInSendQueue > 1024000)
						Sleep(50);
				}
				else
				{
					if(rdtStatus.BufSizeInSendQueue > 1024000)
						Sleep(1000);
				}
			}
			else
			{
				printf("RDT status check error[%d]!\n", ret);
				return 0;
			}
		}
		
		fseek(fp, 0, SEEK_SET);
		//printf("file send ok[%d]\n", i);
	}

	RDT_Destroy(RDT_ID);
	fclose(fp);

	while(1)
	{
		struct st_SInfo info;
		if( IOTC_Session_Check(SID, &info) < 0)
		{
			IOTC_Session_Close(SID);
			break;
		}
		Sleep(1000);
	}
	return 0;
}

DWORD WINAPI _thread_Login(void *arg)
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
			Sleep(5000);
	}
	
	return 0;
}

#define MAX_CLIENT_NUM 8

int main(int argc, TCHAR *argv[])
{
	if(argc < 4)
	{
		printf("No UID or input file!!!\n");
		printf("RDTServer [UID] [Filename] [MaxClientNum]\n");
		return 0;
	}
	
	strcpy(gFn, (char *)argv[2]);
	int maxClientNum = atoi((char *)argv[3]);
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
	DWORD ThreadLogin_ID;
	HANDLE hThread1 = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)_thread_Login, (LPVOID)UID, 0, &ThreadLogin_ID);
	if(NULL == ThreadLogin_ID || hThread1 == NULL)
	{
		printf("Failed to create thread1!!!!\n");
		return 0;
	}
	
	int SID = -1, clientCnt = 0;
	struct st_SInfo Sinfo;
	unsigned long t;
	DWORD ThreadId[MAX_CLIENT_NUM];
	HANDLE ThreadHandler[MAX_CLIENT_NUM];
	printf("start IOTC_Listen...\n");
	do {
		SID = IOTC_Listen(300000);
		if(SID > -1)
		{
			int *sid = (int *)malloc(sizeof(int));
			*sid = SID;
			IOTC_Session_Check(SID, &Sinfo);
			char *mode[3] = {"P2P", "RLY", "LAN"};
			printf("Client from %s:%d Mode=%s\n",Sinfo.RemoteIP, Sinfo.RemotePort, mode[(int)Sinfo.Mode]);
	
			ThreadHandler[clientCnt] = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)SendData, (LPVOID)sid, 0, &ThreadId[clientCnt]);
			if(clientCnt == 0) t = GetTickCount();
			clientCnt++;
		}
		if(clientCnt == maxClientNum)
			break;
	}while(1);
	
	int i;
	for(i=0;i<maxClientNum;i++)
		WaitForSingleObject(ThreadHandler[i], INFINITE);
	
	printf("RDT Send Data Cost[%lu ms]....\n", GetTickCount()-t);
	RDT_DeInitialize();
	printf("RDT_DeInitialize OK\n");
	IOTC_DeInitialize();
	
	printf("Server exit!\n");
	
	return 0;
}


