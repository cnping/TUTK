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
#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"

#define SERVTYPE_STREAM_SERVER	1
#define AUDIO_STREAM_OUT_CH	1
#define MAX_SIZE_IOCTRL_BUF		1024

#define VIDEO_RECORD_FRAMES 120
#define AUDIO_RECORD_FRAMES 250

#define AUDIO_BUF_SIZE	1024

int gSID;

DWORD WINAPI thread_ReceiveAudio(void *arg)
{
	printf("[thread_ReceiveAudio] Starting....\n");
	
	int avIndex = *(int *)arg;
	char buf[AUDIO_BUF_SIZE];
	
	FRAMEINFO_t frameInfo;
	unsigned int frmNo;
	int recordCnt = 0;
	int ret;
	printf("Start IPCAM audio stream OK![%d]\n", avIndex);
	
	while(1)
	{
		ret = avCheckAudioBuf(avIndex);
		if(ret < 0) break;
		if(ret < 30) // determined by audio frame rate
		{
			Sleep(100);
			continue;
		}
		
		ret = avRecvAudioData(avIndex, buf, AUDIO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);		
		
		if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
		{
			printf("[thread_ReceiveAudio] AV_ER_SESSION_CLOSE_BY_REMOTE\n");
			break;
		}
		else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
		{
			printf("[thread_ReceiveAudio] AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
			break;
		}
		else if(ret == IOTC_ER_INVALID_SID)
		{
			printf("[thread_ReceiveAudio] Session cant be used anymore\n");
			break;
		}
		else if(ret == AV_ER_LOSED_THIS_FRAME)
		{
			printf("AV_ER_LOSED_THIS_FRAME[%d]\n", frmNo);
			continue;
		}
		else if(ret < 0)
		{
			printf("Other error[%d]!!!\n", ret);
			continue;
		}
		
		//audio_playback(fd, buf, ret);
		//printf("[%d]", ret); //fflush(stdout);
		if(recordCnt++ > AUDIO_RECORD_FRAMES) break;
	}
	
	printf("[thread_ReceiveAudio] thread exit\n");
	
	return 0;
}

#define VIDEO_BUF_SIZE	48000

DWORD WINAPI thread_ReceiveVideo(void *arg)
{
	printf("[thread_ReceiveVideo] Starting....\n");
	
	int avIndex = *(int *)arg;
	char buf[VIDEO_BUF_SIZE];
	int ret;
	
	FRAMEINFO_t frameInfo;
	unsigned int frmNo;
	printf("Start IPCAM video stream OK![%d]\n", avIndex);
	int flag = 0, cnt = 0;
	//char fn[32];
	while(1)
	{
		ret = avRecvFrameData(avIndex, buf, VIDEO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
			
		if(ret == AV_ER_DATA_NOREADY)
		{
			//printf("AV_ER_DATA_NOREADY[%d]\n", frmNo);
			Sleep(300);
			continue;
		}
		else if(ret == AV_ER_LOSED_THIS_FRAME)
		{
			printf("Lost video frame NO[%d]\n", frmNo);
			continue;
		}
		else if(ret == AV_ER_INCOMPLETE_FRAME)
		{
			printf("Incomplete video frame NO[%d]\n", frmNo);
			continue;
		}
		else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
		{
			printf("[thread_ReceiveVideo] AV_ER_SESSION_CLOSE_BY_REMOTE\n");
			break;
		}
		else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
		{
			printf("[thread_ReceiveVideo] AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
			break;
		}
		else if(ret == IOTC_ER_INVALID_SID)
		{
			printf("[thread_ReceiveVideo] Session cant be used anymore\n");
			break;
		}
		
		if(flag)
		{
			//video_draw(fd, buf, ret);
			//break;
			#if 0
			sprintf(fn, "%d.VGA", cnt);
			FILE *fp = fopen(fn, "w+");
			fwrite(buf, 1, ret, fp);
			fclose(fp);
			#endif
			cnt++;
		}
		else if(frameInfo.flags == IPC_FRAME_FLAG_IFRAME)
		{
			#if 0
			sprintf(fn, "%d.IVGA", cnt);
			FILE *fp = fopen(fn, "w+");
			fwrite(buf, 1, ret, fp);
			fclose(fp);
			#endif
			//flag = 1;
			//video_draw(fd, buf, ret);
			//printf("I frame[%d]\n", cnt);
			cnt++;
			//break;
		}
		
		if(cnt > VIDEO_RECORD_FRAMES) break;
	}
	
	printf("[thread_ReceiveVideo] thread exit\n");
	
	return 0;
}

int start_ipcam_stream(int avIndex)
{
	int ret;
	unsigned short val = 0;
	if((ret = avSendIOCtrl(avIndex, IOTYPE_INNER_SND_DATA_DELAY, (char *)&val, sizeof(unsigned short)) < 0))
	{
		printf("start_ipcam_stream failed[%d]\n", ret);
		return 0;
	}
	
	SMsgAVIoctrlAVStream ioMsg;
	memset(&ioMsg, 0, sizeof(SMsgAVIoctrlAVStream));
	if((ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_START, (char *)&ioMsg, sizeof(SMsgAVIoctrlAVStream)) < 0))
	{
		printf("start_ipcam_stream failed[%d]\n", ret);
		return 0;
	}
	
	if((ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_AUDIOSTART, (char *)&ioMsg, sizeof(SMsgAVIoctrlAVStream)) < 0))
	{
		printf("start_ipcam_stream failed[%d]\n", ret);
		return 0;
	}
	
	return 1;
}

int main(int argc, char *argv[])
{
	int ret;
	
	if(argc < 2)
	{
		printf("No UID!!!\n");
		exit(0);
	}
	
	DWORD t1 = GetTickCount();
	
	char *UID = argv[1];
	printf("StreamClient start...\n");
	// use which Master base on location, port 0 means to get a random port
	ret = IOTC_Initialize(0, "61.188.37.216", "50.19.254.134", "m4.iotcplatform.com", "m2.iotcplatform.com");
	printf("IOTC_Initialize() ret = %d\n", ret);
	if(ret != IOTC_ER_NoERROR) 
	{
		printf("IOTCAPIs_Device exit...!!\n");
		return 0;
	}
	
	// alloc 3 sessions for video and two-way audio
	avInitialize(8);
	
	// use IOTC_Connect_ByUID or IOTC_Connect_ByName to connect with device
	
	gSID = IOTC_Connect_ByUID(UID);
	printf("Step 2: call IOTC_Connect_ByUID(%s) ret(%d).......\n", UID, gSID);
	struct st_SInfo Sinfo;	
	IOTC_Session_Check(gSID, &Sinfo);
	if(Sinfo.Mode == 0)
		printf("Device is from %s:%d[%s] Mode=P2P\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	else if(Sinfo.Mode == 1)
		printf("Device is from %s:%d[%s] Mode=RLY\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	else if(Sinfo.Mode == 2)
		printf("Device is from %s:%d[%s] Mode=LAN\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	printf("Device info VER[%X] NAT[%d]\n", Sinfo.IOTCVersion, Sinfo.NatType);
	//printf("My NAT Type[%d]\n", IOTC_Get_Nat_Type());
	printf("Cost time = %ld ms\n", GetTickCount() - t1);
	
	unsigned long srvType;
	int avIndex = avClientStart(gSID, "admin", "888888", 20000, &srvType, 0);
	printf("Step 2: call avClientStart OK(%d).......\n", avIndex);	
	if(avIndex < 0)
	{
		printf("avClientStart failed[%d]\n", avIndex);
		return 0;
	}
	
	if(start_ipcam_stream(avIndex))
	{
		DWORD ThreadVideo_ID, ThreadAudio_ID;
		
		HANDLE hThread1 = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)thread_ReceiveVideo, (LPVOID)&avIndex, 0, &ThreadVideo_ID);
		if(NULL == ThreadVideo_ID || hThread1 == NULL)
		{
			printf("Failed to create thread1!!!!\n");
			return 0;
			
		}
		
		HANDLE hThread2 = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)thread_ReceiveAudio, (LPVOID)&avIndex, 0, &ThreadAudio_ID);
		if(NULL == ThreadAudio_ID || hThread2 == NULL)
		{
			printf("Failed to create thread2!!!!\n");
			return 0;
			
		}
		
		//Sleep(1000000000);
		#if 1
		WaitForSingleObject(hThread1, INFINITE);
		WaitForSingleObject(hThread2, INFINITE);
		//WaitForSingleObject(hThread3, INFINITE);
		#endif
	}
	
	printf("avClientStop calling....\n");
	avClientStop(avIndex);
	printf("avClientStop OK\n");
	IOTC_Session_Close(gSID);
	printf("IOTC_Session_Close OK\n");
	avDeInitialize();
	IOTC_DeInitialize();
	
	printf("StreamClient exit...\n");
	
	return 0;
}

