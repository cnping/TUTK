//
//  Client.m
//  Sample_AVAPIs
//
//  Created by tutk on 3/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "Client.h"
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <unistd.h>
#import <time.h>
#import <pthread.h>
#import "IOTCAPIs.h"
#import "RDTAPIs.h"




@implementation Client




#define _UID "K0000000000000000001"
#define DUMP_FILE 0
#define FileNum 2

#define START_STRING "Start"
#define RDT_WAIT_TIME_MS 30000
#define MAX_BUF_SIZE 102400
#define MAX_MEM_USE 128000




- (void)start:(NSString *)UID {
    
    NSLog(@"RDT Client Start");
    
	printf("RDT Version[%X]\n", RDT_GetRDTApiVer());
    
    
	int ret = IOTC_Initialize(0, "61.188.37.216", "50.19.254.134", "m2.iotcplatform.com", "m4.iotcplatform.com");
	if(ret != IOTC_ER_NoERROR)
	{
		printf("IOTC_Initialize error!!\n");
		return ;
	}
    
	int rdtCh = RDT_Initialize();
	if(rdtCh <= 0)
	{
		printf("RDT_Initialize error!!\n");
		return ;
	}
    
	int SID, i = 0;
    
	while(1)
	{
		SID = IOTC_Connect_ByUID(_UID);
		printf("Step 2: call IOTC_Connect_ByUID(%s) ret = %d\n", _UID, SID);
		if(SID < 0)
		{
			printf("p2pAPIs_Client connect failed...!!\n");
			return ;
		}
		else if(SID > -1)
			break;
	}
    
	int nRDTIndex = RDT_Create(SID, RDT_WAIT_TIME_MS, 0);
	if(nRDTIndex < 0)
	{
		printf("RDT_Create failed[%d]!!\n", nRDTIndex);
		IOTC_Session_Close(SID);
		exit(0);
	}
	printf("RDT_Create OK[%d]\n", nRDTIndex);
	char buf[MAX_BUF_SIZE];
	sprintf(buf, "%d", FileNum);
	ret = RDT_Write(nRDTIndex, buf, strlen(buf));
	if(ret < 0)
	{
		printf("RDT send cmd failed[%d]!!\n", ret);
		IOTC_Session_Close(SID);
		exit(0);
	}
    
	ret = RDT_Read(nRDTIndex, buf, 1024, RDT_WAIT_TIME_MS);
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
#if DUMP_FILE
	FILE *fp;
#endif
	char fn[32];
	struct st_RDT_Status rdtStatus;
	for(i=0;i<FileNum;i++)
	{
		if(RDT_Status_Check(nRDTIndex, &rdtStatus) < 0)
		{
			printf("RDT_Status_Check error!\n");
			break;
		}
		remainReadSize = fileSize;
		sprintf(fn, "%d.jpg", i);
#if DUMP_FILE
		FILE *fp = fopen(fn, "w+");
		if(fp == NULL)
		{
			printf("Open file [%s] error!!\n", fn);
			break;
		}
#endif
		while(1)
		{
			if(remainReadSize < MAX_BUF_SIZE)
				ret = RDT_Read(nRDTIndex, buf, remainReadSize, RDT_WAIT_TIME_MS);
			else
				ret = RDT_Read(nRDTIndex, buf, MAX_BUF_SIZE, RDT_WAIT_TIME_MS);
            
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
#if DUMP_FILE
			fwrite(buf, 1, ret, fp);
			fflush(fp);
#endif
			if(remainReadSize == 0) break;
		}
#if DUMP_FILE
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
    
	return ;
    
    
    
	NSLog(@"RDT Client exit...");
}

@end