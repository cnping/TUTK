#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "IOTCAPIs.h"
#include "RDTAPIs.h"
#include "common.h"

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("No UID or File Number!!!\n");
		printf("RDTClient [UID] [FileNum]\n");
		return 0;
	}

	printf("RDT Version[%X]\n", RDT_GetRDTApiVer());
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

	int nRDTIndex = RDT_Create(SID, RDT_WAIT_TIMEMS, 0);
	if(nRDTIndex < 0)
	{
		printf("RDT_Create failed[%d]!!\n", nRDTIndex);
		IOTC_Session_Close(SID);
		exit(0);
	}
	printf("RDT_Create OK[%d]\n", nRDTIndex);
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
	//FILE *fp;
	char fn[32];
	struct st_RDT_Status rdtStatus;
	for(i=0;i<fileNum;i++)
	{
		if(RDT_Status_Check(nRDTIndex, &rdtStatus) < 0)
		{
			printf("RDT_Status_Check error!\n");
			break;
		}
		remainReadSize = fileSize;
		sprintf(fn, "%d.jpg", i);
		#if 0
		FILE *fp = fopen(fn, "w+");
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

			if(ret > 0)
			{
				printf("RDT_Read[%d]\n", ret);
				remainReadSize -= ret;
			}
			//fwrite(buf, 1, ret, fp);
			//fflush(fp);
			if(remainReadSize == 0) break;
		}
		//fclose(fp);
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

