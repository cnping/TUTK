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
#include "IOTCAPIs.h"
#include "RDTAPIs.h"

#include "common.h"

//#define OUTPUT_FILE

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("No UID or File Number!!!\n");
		printf("RDTClient [UID] [FileNum]\n");
		return 0;
	}
	
	int fileNum = atoi(argv[2]);
	
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
	
	int SID, i = 0;
	char *UID = (char *)argv[1];
	
	while(1)
	{
		SID = IOTC_Connect_ByUID(UID);
		printf("Step 2: call IOTC_Connect_ByUID(%s) ret = %d\n", UID, SID);
		if(SID < 0)
		{
			printf("p2pAPIs_Client connect failed...!!\n");
			return 0;
		}
		else if(SID > -1)
			break;
	}
	
	struct st_SInfo Sinfo;
	IOTC_Session_Check(SID, &Sinfo);
	char *mode[3] = {"P2P", "RLY", "LAN"};
	printf("Device from %s:%d Mode=%s\n", Sinfo.RemoteIP, Sinfo.RemotePort, mode[(int)Sinfo.Mode]);
	
	int nRDTIndex = RDT_Create(SID, RDT_WAIT_TIMEMS, 0);
	if(nRDTIndex < 0)
	{
		printf("RDT_Create failed[%d]!!\n", nRDTIndex);
		IOTC_Session_Close(SID);
		exit(0);
	}
	
	char buf[MAX_BUF_SIZE];
	sprintf(buf, "%d", fileNum);
	ret = RDT_Write(nRDTIndex, buf, strlen(buf));
	if(ret < 0)
	{
		printf("RDT send cmd failed[%d]!!\n", ret);
		IOTC_Session_Close(SID);
		exit(0);
	}
	
	ret = RDT_Read(nRDTIndex, buf, 1024, RDT_WAIT_TIMEMS);
	if(ret < 0)
	{
		printf("RDT rcv file size failed[%d]!!\n", ret);
		IOTC_Session_Close(SID);
		exit(0);
	}
	int fileSize = atoi(buf), remainReadSize;
	
	strcpy(buf, START_STRING);
	ret = RDT_Write(nRDTIndex, buf, strlen(buf));
	if(ret < 0)
	{
		printf("RDT send start failed[%d]!!\n", ret);
		IOTC_Session_Close(SID);
		exit(0);
	}
	
	printf("fileSize[%d]\n", fileSize);
	struct st_RDT_Status rdtStatus;
	unsigned int round = 0;
	for(i=0;i<fileNum;i++)
	{
		if(RDT_Status_Check(nRDTIndex, &rdtStatus) < 0)
		{
			printf("RDT_Status_Check error!\n");
			break;
		}
		remainReadSize = fileSize;
		
		#ifdef OUTPUT_FILE
		char fn[32];
		sprintf(fn, "%d.rcv", i);
		FILE *fp = fopen(fn, "wb+");
		if(fp == NULL)
		{
			printf("Open file[%s] error!!\n", fn);
			break;
		}
		#endif
		while(1)
		{
			if(remainReadSize < MAX_BUF_SIZE)
				ret = RDT_Read(nRDTIndex, buf, remainReadSize, RDT_WAIT_TIMEMS);
			else
				ret = RDT_Read(nRDTIndex, buf, MAX_BUF_SIZE, RDT_WAIT_TIMEMS);
			
			if(ret < 0 && ret != RDT_ER_TIMEOUT)
			{
				printf("RDT_Read data failed[%d]!!", ret);
				break;
			}
			remainReadSize -= ret;
			if(round++ % 5 == 0)
			{
				printf(".");
				fflush(stdout);
			}
			#ifdef OUTPUT_FILE
			fwrite(buf, 1, ret, fp);
			#endif
			if(remainReadSize == 0) break;
		}
		#ifdef OUTPUT_FILE
		fclose(fp);
		#endif
		printf("rcv file OK [%d]\n", i);
	}
	
	//printf("RDT_Destroy calling....\n");
	RDT_Destroy(nRDTIndex);
	printf("RDT_Destroy calling....OK!\n");
	IOTC_Session_Close(SID);
	printf("IOTC_Session_Close OK!\n");
	RDT_DeInitialize();
	printf("RDT_DeInitialize OK!\n");
	IOTC_DeInitialize();
	printf("*****clien exit*****\n");
	
	return 0;
}

