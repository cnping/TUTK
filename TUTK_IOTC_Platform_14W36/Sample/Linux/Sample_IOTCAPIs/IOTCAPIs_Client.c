#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "IOTCAPIs.h"

static int gSID = -1;

void *thread_ForSessionRead(void *arg)
{
	int ChID = *(int *)arg;
	int SID = gSID;
	char buf[IOTC_MAX_PACKET_SIZE];
	int ret;
	printf("[thread_ForSessionRead] started channel[%d]\n", ChID);
	
	while(1)
	{
		ret = IOTC_Session_Read(SID, buf, IOTC_MAX_PACKET_SIZE, 6000, ChID);
		
		if(ret > -1)
		{
			buf[ret] = 0;
			if(strcasecmp(buf, "end") == 0)
				break;
			printf("[IOTC_Session_Read] SID[%d] ChID[%d] Size[%d] Data=> %s\n", SID, ChID, ret, buf);
		}
		// check error code
		else if(ret == IOTC_ER_SESSION_CLOSE_BY_REMOTE)
		{
			printf("[thread_ForSessionRead] remote site close this session, SID[%d]\n", SID);
			break;
		}
		else if(ret == IOTC_ER_REMOTE_TIMEOUT_DISCONNECT)
		{
			printf("[thread_ForSessionRead] disconnected due to remote site no response for a while SID[%d]\n", SID);
			break;
		}
		else if(ret == IOTC_ER_INVALID_SID)
		{
			printf("Session already closed by other thread\n");
			break;
		}
		else if(ret == IOTC_ER_CH_NOT_ON)
			IOTC_Session_Channel_ON(SID, ChID);
	}
	
	printf("[thread_ForSessionRead] exit channel[%d]\n", ChID);
	
	pthread_exit(0);
}

void *thread_ForSessionWrite(void *arg)
{
	int ChID = *(int *)arg;
	int SID = gSID;
	char buf[IOTC_MAX_PACKET_SIZE];
	int ret;
	int i;
	
	printf("[thread_ForSessionWrite] started channel[%d]\n", ChID);
	
	// wait a minute to send for avoid packet lost
	sleep(3);
	
	for(i = 0; i < 3; i++)
	{
		sprintf(buf, "Hello World CH[%d] NO[%d]", ChID, i);
		ret = IOTC_Session_Write(SID, buf, strlen(buf), ChID);
		if(ret == IOTC_ER_CH_NOT_ON)
		{
			printf("IOTC_ER_CH_NOT_ON!!!!!!!!!!1\n");
			IOTC_Session_Channel_ON(SID, ChID);
		}
		else if(ret < 0) // other error < 0 means this session cant use anymore
		{
			printf("IOTC_Session_Write error, code[%d]\n", ret);
			pthread_exit(0);
		}
	}
	
	sprintf(buf, "end");
	IOTC_Session_Write(SID, buf, strlen(buf), ChID);
	
	printf("[thread_ForSessionWrite] exit channel[%d]\n", ChID);
	
	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	int ret;
	
	if(argc < 2)
	{
		printf("No UID input!!!\n");
		printf("IOTCAPIs_Client [UID]\n");
		return 0;
	}
	
	// Must be first call before IOTC_Initialize(), but not a must
	IOTC_Set_Max_Session_Number(32);
	
	// use which Master base on location, port 0 means to get a random port
	ret = IOTC_Initialize(0, "61.188.37.216", "50.19.254.134", "m2.iotcplatform.com", "m4.iotcplatform.com");
	printf("Step 1: call IOTC_Initialize(), ret = %d\n", ret);
	if(ret != IOTC_ER_NoERROR)
	{
		printf("IOTCAPIs_Client exit...!!\n");
		return 0;
	}
	
	char *UID = (char *)argv[1];
	
	int i;
	// alloc 12 struct st_LanSearchInfo maybe enough to contain device number on LAN
	// or you have more than 12 device on LAN, please modify this value
	struct st_LanSearchInfo *psLanSearchInfo = (struct st_LanSearchInfo *)malloc(sizeof(struct st_LanSearchInfo)*12);
	if(psLanSearchInfo != NULL)
	{
		// wait time 2000 ms to get result, if result is 0 you can extend to 3000 ms
		int nDeviceNum = IOTC_Lan_Search(psLanSearchInfo, 12, 2000);
		printf("IOTC_Lan_Search ret[%d]\n", nDeviceNum);
		for(i=0;i<nDeviceNum;i++)
		{
			printf("UID[%s] Addr[%s:%d]\n", psLanSearchInfo[i].UID, psLanSearchInfo[i].IP, psLanSearchInfo[i].port);
		}
		free(psLanSearchInfo);
	}
	printf("LAN search done...\n");
	
	#if 1 // normal
	gSID = IOTC_Connect_ByUID(UID);
	#else // secure
	gSID = IOTC_Connect_ByUID2(UID, "1234567890aaabbb", IOTC_SECURE_MODE);
	#endif
	printf("Step 2: call IOTC_Connect_ByUID(%s).......\n", UID);
	
	struct st_SInfo Sinfo;
	IOTC_Session_Check(gSID, &Sinfo);
	if(Sinfo.Mode ==0)
		printf("Device info %s:%d[%s] Mode=P2P\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	else if(Sinfo.Mode == 1)
		printf("Device info %s:%d[%s] Mode=RLY\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	else if(Sinfo.Mode == 2)
		printf("Device info %s:%d[%s] Mode=LAN\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	
	if(gSID >= 0)
	{
		printf("IOTC Connect OK SID[%d]\n", gSID);
		int ChID1 = 0, ChID2 = 1;
		// channel 0 always on, so just turn on channel 1
		IOTC_Session_Channel_ON(gSID, 1);
		
		pthread_t ThreadRead_ID1, ThreadRead_ID2;
		pthread_t ThreadWrt_ID1, ThreadWrt_ID2;
		pthread_create(&ThreadRead_ID1, NULL, &thread_ForSessionRead, (void*)&ChID1);
		pthread_create(&ThreadRead_ID2, NULL, &thread_ForSessionRead, (void*)&ChID2);
		pthread_create(&ThreadWrt_ID1, NULL, &thread_ForSessionWrite, (void*)&ChID1);
		pthread_create(&ThreadWrt_ID2, NULL, &thread_ForSessionWrite, (void*)&ChID2);
		// wait all thread exit for confirm all data already read/write
		pthread_join(ThreadWrt_ID1, NULL);
		pthread_join(ThreadWrt_ID2, NULL);
		pthread_join(ThreadRead_ID1, NULL);
		pthread_join(ThreadRead_ID2, NULL);
		IOTC_Session_Close(gSID);
	}
	else
		printf("IOTC_Connect error code[%d]\n", gSID);

	IOTC_DeInitialize();

	return 0;
}
