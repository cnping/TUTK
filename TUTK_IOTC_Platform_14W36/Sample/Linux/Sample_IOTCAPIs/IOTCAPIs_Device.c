#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "IOTCAPIs.h"

static int gSID;

void *thread_ForSessionReadWrite(void *arg)
{
	unsigned char ChID = *(int *)arg;
	int SID = gSID;
	int i;
	int nRead;
	char buf[IOTC_MAX_PACKET_SIZE];
	struct st_SInfo Sinfo;
	
	printf("[thread_ForSessionReadWrite] Thread started...[%d]\n", ChID);
	
	IOTC_Session_Check(SID, &Sinfo);
	if(Sinfo.Mode ==0)
		printf("Client is from %s:%d[%s] Mode=P2P\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	else if(Sinfo.Mode == 1)
		printf("Client is from %s:%d[%s] Mode=RLY\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	else if(Sinfo.Mode == 2)
		printf("Client is from %s:%d[%s] Mode=LAN\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);

	printf("VPG [%d:%d:%d] Remote NAT[%d]\n", Sinfo.VID, Sinfo.PID, Sinfo.GID, Sinfo.NatType);
	
	IOTC_Session_Channel_ON(SID, ChID);
	sleep(3);
	
	for(i = 0; i < 3; i++)
	{
		sprintf(buf, "Hello World[%d][%d]", ChID, i);
		int ret = IOTC_Session_Write(SID,buf,strlen(buf), ChID);
		if(ret < 0)
			printf("Write error[%d]->%d\n", ChID, ret);
	}
	
	sprintf(buf, "end");
	IOTC_Session_Write(SID, buf, strlen(buf), ChID);
	
	while((nRead = IOTC_Session_Read(SID, buf, IOTC_MAX_PACKET_SIZE, 6000, ChID)) > -1)
	{
		buf[nRead] = 0;
		printf("[IOTC_Session_Read] SID=%d, ChID = %d, Size=%d, Data:%s\n",SID,ChID,nRead,buf);
	}
	// check if remote site close this session
	if(nRead == IOTC_ER_SESSION_CLOSE_BY_REMOTE)
	{
		printf("[thread_ForSessionReadWrite] remote site close this session, SID = %d\n", SID);
		IOTC_Session_Close(SID);
	}
	else if(nRead == IOTC_ER_REMOTE_TIMEOUT_DISCONNECT)
	{
		printf("[thread_ForSessionReadWrite] disconnected due to remote site has no any response after a 'timeout' period of time., SID = %d\n", SID);
		IOTC_Session_Close(SID);
	}
	
	printf("[thread_ForSessionRead] Thread exit...[%d]\n", ChID);
	
	pthread_exit(0);
}

void *_thread_Login(void *arg)
{
	int ret;
	char *UID = (char *)arg;
	while(1)
	{
		ret = IOTC_Device_Login(UID, "AAAA0009", "12345678");
		printf("     Calling IOTC_Device_Login() ret = %d\n", ret);
		if(ret == IOTC_ER_NoERROR)
			break;
		else
			sleep(5);
	}
	
	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	int ret;
	
	if(argc < 2)
	{
		printf("No UID input!!!\n");
		printf("IOTCAPIs_Device [UID]\n");
		return 0;
	}
	
	// Must be first call before IOTC_Initialize(), but not a must
	IOTC_Set_Max_Session_Number(8);
	
	printf("IOTCAPIs_Device start...\n");
	//ret = IOTC_Initialize(0, "61.188.37.216", "50.19.254.134", "m2.iotcplatform.com", "m4.iotcplatform.com");
	ret = IOTC_Initialize2(0);
	printf("Step 1: IOTC_Initialize() ret = %d\n", ret);
	if(ret != IOTC_ER_NoERROR)
	{
		printf("IOTCAPIs_Device exit...!!\n");
		return 0;
	}
	
	unsigned char ChID = 0, ChID2 = 1;
	
	char *UID = (char *)argv[1];
	pthread_t threadID_Login;
	pthread_create(&threadID_Login, NULL, &_thread_Login, (void *)UID);
	pthread_detach(threadID_Login);
	
	while(1)
	{
		pthread_t Thread_ID, Thread_ID2;
		#if 1 // normal
		gSID = IOTC_Listen(0);
		#else // secure
		gSID = IOTC_Listen2(0, "1234567890aaabbb", IOTC_SECURE_MODE);
		#endif
		
		if(gSID > -1)
		{
			printf("Step 3: IOTC_Listen() ret = %d ", gSID);
			printf("Step4: Create a Thread to handle data read / write from this session\n");
			//IOTC_Session_Channel_ON(gSID, ChID);
			IOTC_Session_Channel_ON(gSID, ChID2);
			pthread_create(&Thread_ID, NULL, &thread_ForSessionReadWrite, (void*)&ChID);
			pthread_create(&Thread_ID2, NULL, &thread_ForSessionReadWrite, (void*)&ChID2);
			pthread_detach(Thread_ID);
			pthread_detach(Thread_ID2);
		}
		else if(gSID == IOTC_ER_EXCEED_MAX_SESSION)
		{
			sleep(5); // wait a minute for getting free seesion
		}
		else if(gSID == IOTC_ER_EXCEED_MAX_SESSION)
		{
			printf("IOTC_ER_EXCEED_MAX_SESSION\n");
			sleep(1);
		}
		else
		{
			printf("IOTC_Listen--Error[%d]\n", gSID);
			printf("IOTCAPIs_Device exit...!!\n");
			break;
		}
	}

	IOTC_DeInitialize();

	return 0;
}

