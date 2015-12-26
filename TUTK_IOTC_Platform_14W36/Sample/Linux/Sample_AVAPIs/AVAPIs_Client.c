#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"

// *** set to 1 to enable audio or speaker function *** 
#define ENABLE_AUDIO 0
#define ENABLE_SPEAKER 0

#define SERVTYPE_STREAM_SERVER	1
#define AUDIO_STREAM_OUT_CH	1
#define MAX_SIZE_IOCTRL_BUF		1024

#define AUDIO_BUF_SIZE	1024

#define AUDIO_SPEAKER_CHANNEL 5

// *** AV server ID and password, set here ***
char avID[]="admin";
char avPass[]="888888";


void PrintErrHandling (int nErr)
{
	switch (nErr)
	{
	case IOTC_ER_SERVER_NOT_RESPONSE :
		//-1 IOTC_ER_SERVER_NOT_RESPONSE
		printf ("[Error code : %d]\n", IOTC_ER_SERVER_NOT_RESPONSE );
		printf ("Master doesn't respond.\n");
		printf ("Please check the network wheather it could connect to the Internet.\n");
		break;
	case IOTC_ER_FAIL_RESOLVE_HOSTNAME :
		//-2 IOTC_ER_FAIL_RESOLVE_HOSTNAME
		printf ("[Error code : %d]\n", IOTC_ER_FAIL_RESOLVE_HOSTNAME);
		printf ("Can't resolve hostname.\n");
		break;
	case IOTC_ER_ALREADY_INITIALIZED :
		//-3 IOTC_ER_ALREADY_INITIALIZED
		printf ("[Error code : %d]\n", IOTC_ER_ALREADY_INITIALIZED);
		printf ("Already initialized.\n");
		break;
	case IOTC_ER_FAIL_CREATE_MUTEX :
		//-4 IOTC_ER_FAIL_CREATE_MUTEX
		printf ("[Error code : %d]\n", IOTC_ER_FAIL_CREATE_MUTEX);
		printf ("Can't create mutex.\n");
		break;
	case IOTC_ER_FAIL_CREATE_THREAD :
		//-5 IOTC_ER_FAIL_CREATE_THREAD
		printf ("[Error code : %d]\n", IOTC_ER_FAIL_CREATE_THREAD);
		printf ("Can't create thread.\n");
		break;
	case IOTC_ER_UNLICENSE :
		//-10 IOTC_ER_UNLICENSE
		printf ("[Error code : %d]\n", IOTC_ER_UNLICENSE);
		printf ("This UID is unlicense.\n");
		printf ("Check your UID.\n");
		break;
	case IOTC_ER_NOT_INITIALIZED :
		//-12 IOTC_ER_NOT_INITIALIZED
		printf ("[Error code : %d]\n", IOTC_ER_NOT_INITIALIZED);
		printf ("Please initialize the IOTCAPI first.\n");
		break;
	case IOTC_ER_TIMEOUT :
		//-13 IOTC_ER_TIMEOUT
		break;
	case IOTC_ER_INVALID_SID :
		//-14 IOTC_ER_INVALID_SID
		printf ("[Error code : %d]\n", IOTC_ER_INVALID_SID);
		printf ("This SID is invalid.\n");
		printf ("Please check it again.\n");
		break;
	case IOTC_ER_EXCEED_MAX_SESSION :
		//-18 IOTC_ER_EXCEED_MAX_SESSION
		printf ("[Error code : %d]\n", IOTC_ER_EXCEED_MAX_SESSION);
		printf ("[Warning]\n");
		printf ("The amount of session reach to the maximum.\n");
		printf ("It cannot be connected unless the session is released.\n");
		break;
	case IOTC_ER_CAN_NOT_FIND_DEVICE :
		//-19 IOTC_ER_CAN_NOT_FIND_DEVICE
		printf ("[Error code : %d]\n", IOTC_ER_CAN_NOT_FIND_DEVICE);
		printf ("Device didn't register on server, so we can't find device.\n");
		printf ("Please check the device again.\n");
		printf ("Retry...\n");
		break;
	case IOTC_ER_SESSION_CLOSE_BY_REMOTE :
		//-22 IOTC_ER_SESSION_CLOSE_BY_REMOTE
		printf ("[Error code : %d]\n", IOTC_ER_SESSION_CLOSE_BY_REMOTE);
		printf ("Session is closed by remote so we can't access.\n");
		printf ("Please close it or establish session again.\n");
		break;
	case IOTC_ER_REMOTE_TIMEOUT_DISCONNECT :
		//-23 IOTC_ER_REMOTE_TIMEOUT_DISCONNECT
		printf ("[Error code : %d]\n", IOTC_ER_REMOTE_TIMEOUT_DISCONNECT);
		printf ("We can't receive an acknowledgement character within a TIMEOUT.\n");
		printf ("It might that the session is disconnected by remote.\n");
		printf ("Please check the network wheather it is busy or not.\n");
		printf ("And check the device and user equipment work well.\n");
		break;
	case IOTC_ER_DEVICE_NOT_LISTENING :
		//-24 IOTC_ER_DEVICE_NOT_LISTENING
		printf ("[Error code : %d]\n", IOTC_ER_DEVICE_NOT_LISTENING);
		printf ("Device doesn't listen or the sessions of device reach to maximum.\n");
		printf ("Please release the session and check the device wheather it listen or not.\n");
		break;
	case IOTC_ER_CH_NOT_ON :
		//-26 IOTC_ER_CH_NOT_ON
		printf ("[Error code : %d]\n", IOTC_ER_CH_NOT_ON);
		printf ("Channel isn't on.\n");
		printf ("Please open it by IOTC_Session_Channel_ON() or IOTC_Session_Get_Free_Channel()\n");
		printf ("Retry...\n");
		break;
	case IOTC_ER_SESSION_NO_FREE_CHANNEL :
		//-31 IOTC_ER_SESSION_NO_FREE_CHANNEL
		printf ("[Error code : %d]\n", IOTC_ER_SESSION_NO_FREE_CHANNEL);
		printf ("All channels are occupied.\n");
		printf ("Please release some channel.\n");
		break;
	case IOTC_ER_TCP_TRAVEL_FAILED :
		//-32 IOTC_ER_TCP_TRAVEL_FAILED
		printf ("[Error code : %d]\n", IOTC_ER_TCP_TRAVEL_FAILED);
		printf ("Device can't connect to Master.\n");
		printf ("Don't let device use proxy.\n");
		printf ("Close firewall of device.\n");
		printf ("Or open device's TCP port 80, 443, 8080, 8000, 21047.\n");
		break;
	case IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED :
		//-33 IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED
		printf ("[Error code : %d]\n", IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED);
		printf ("Device can't connect to server by TCP.\n");
		printf ("Don't let server use proxy.\n");
		printf ("Close firewall of server.\n");
		printf ("Or open server's TCP port 80, 443, 8080, 8000, 21047.\n");
		printf ("Retry...\n");
		break;
	case IOTC_ER_NO_PERMISSION :
		//-40 IOTC_ER_NO_PERMISSION
		printf ("[Error code : %d]\n", IOTC_ER_NO_PERMISSION);
		printf ("This UID's license doesn't support TCP.\n");
		break;
	case IOTC_ER_NETWORK_UNREACHABLE :
		//-41 IOTC_ER_NETWORK_UNREACHABLE
		printf ("[Error code : %d]\n", IOTC_ER_NETWORK_UNREACHABLE);
		printf ("Network is unreachable.\n");
		printf ("Please check your network.\n");
		printf ("Retry...\n");
		break;
	case IOTC_ER_FAIL_SETUP_RELAY :
		//-42 IOTC_ER_FAIL_SETUP_RELAY
		printf ("[Error code : %d]\n", IOTC_ER_FAIL_SETUP_RELAY);
		printf ("Client can't connect to a device via Lan, P2P, and Relay mode\n");
		break;
	case IOTC_ER_NOT_SUPPORT_RELAY :
		//-43 IOTC_ER_NOT_SUPPORT_RELAY
		printf ("[Error code : %d]\n", IOTC_ER_NOT_SUPPORT_RELAY);
		printf ("Server doesn't support UDP relay mode.\n");
		printf ("So client can't use UDP relay to connect to a device.\n");
		break;

	default :
		break;
	}
}

void close_dsp(int fd)
{
	close(fd);
}

int open_dsp()
{
	unlink("audio.in");
	return open("audio.in", O_RDWR | O_CREAT, 644);
}

void audio_playback(int fd, char *buf, int size)
{
	int ret = write(fd, buf, size);
	if (ret < 0)
	{
		printf("audio_playback::write , ret=[%d]\n", ret);
	}
}

void *thread_Speaker(void *arg)
{
	int SID = *(int *)arg;
	char buf[AUDIO_BUF_SIZE];
	int frameRate = 20;
	int sleepTick = 1000000/frameRate;
	//int sendCnt = 0;
	FILE *fp = fopen("audio.out", "rb");
	if(fp == NULL)
	{
		printf("fopen error!!!\n");
		return 0;
	}

	FRAMEINFO_t frameInfo;
	memset(&frameInfo, 0, sizeof(frameInfo));
	printf("Free CH[%d]\n", IOTC_Session_Get_Free_Channel(SID));
	int avIndex = avServStart(SID, NULL, NULL, 50, 0, AUDIO_SPEAKER_CHANNEL);
	if(avIndex < 0)
	{
		printf("avServStart failed[%d]\n", avIndex);
		return 0;
	}
	printf("[thread_Speaker] Starting avIndex[%d]....\n", avIndex);

	frameInfo.codec_id = MEDIA_CODEC_AUDIO_SPEEX;
	frameInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;

		while(1)
		{
			int size = fread(buf, 1, 38, fp);
			if(size <= 0)
			{
				printf("rewind audio\n");
				rewind(fp);
				continue;
			}

			int ret = avSendAudioData(avIndex, buf, 38, &frameInfo, sizeof(FRAMEINFO_t));
			if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				printf("thread_AudioFrameData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				break;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				printf("thread_AudioFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				break;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				printf("Session cant be used anymore\n");
				break;
			}
			else if(ret < 0)
			{
				printf("avSendAudioData error[%d]\n", ret);
				break;
			}
			usleep(sleepTick);
	
			//*** Speaker thread stop condition if necessary ***
/*			
			if(sendCnt++ > 1500) 
				break;
*/
		}

	fclose(fp);
	printf("[thread_Speaker] exit...\n");
	avServStop(avIndex);

	return 0;
}

void *thread_ReceiveAudio(void *arg)
{
	printf("[thread_ReceiveAudio] Starting....\n");

	int avIndex = *(int *)arg;
	char buf[AUDIO_BUF_SIZE]={0};
	int fd = open_dsp();
	if(fd < 0)
	{
		printf("open_dsp failed[%d]\n", fd);
		return 0;
	}

	FRAMEINFO_t frameInfo;
	unsigned int frmNo;
	//int recordCnt = 0;
	int ret;
	printf("Start IPCAM audio stream OK!\n");

	while(1)
	{
		ret = avCheckAudioBuf(avIndex);
		if(ret < 0) 
			break;
		if(ret < 25) // determined by audio frame rate
		{
			// waiting for audio buf...
			usleep(500000);
			continue;
		}

		ret = avRecvAudioData(avIndex, buf, AUDIO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
	
		// show Frame Info at 1st frame
		if(frmNo==1)
		{
			char *format[] = {"ADPCM","PCM","SPEEX","MP3","G726","UNKNOWN"};
			int idx = 0;
			if(frameInfo.codec_id == MEDIA_CODEC_AUDIO_ADPCM)
				idx = 0;
			else if(frameInfo.codec_id == MEDIA_CODEC_AUDIO_PCM)
				idx = 1;
			else if(frameInfo.codec_id == MEDIA_CODEC_AUDIO_SPEEX)
				idx = 2;
			else if(frameInfo.codec_id == MEDIA_CODEC_AUDIO_MP3)
				idx = 3;
			else if(frameInfo.codec_id == MEDIA_CODEC_AUDIO_G726)
				idx = 4;
			else
				idx = 5;
			printf("--- Audio Formate: %s ---\n", format[idx]);
		}
		
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
			//printf("Audio AV_ER_LOSED_THIS_FRAME[%d]\n", frmNo);
			continue;
		}
		else if(ret < 0)
		{
			printf("Other error[%d]!!!\n", ret);
			continue;
		}

		//audio_playback(fd, buf, ret);

		//*** audio recv thread stop condition if necessary ***
/*		
		if(recordCnt++ > 800) 
			break;
*/
	}

	close_dsp(fd);
	printf("[thread_ReceiveAudio] thread exit\n");

	return 0;
}

int open_videoX()
{
	unlink("video.in");
	return open("video.in", O_RDWR | O_CREAT, 644);
}

void close_videoX(int fd)
{
	close(fd);
}

void video_draw(int fd, char *buf, int size)
{
	int ret=write(fd, buf, size);
	if (ret < 0)
	{
		printf("video_draw::write , ret=[%d]\n", ret);
	}
}

#define VIDEO_BUF_SIZE	128000
int gSleepMs = 10;

void *thread_ReceiveVideo(void *arg)
{
	printf("[thread_ReceiveVideo] Starting....\n");
	int avIndex = *(int *)arg;
	char buf[VIDEO_BUF_SIZE]={0};
	int ret;
	//int fd = open_videoX();
	//if(fd < 0) return 0;

	FRAMEINFO_t frameInfo;
	unsigned int frmNo;
	struct timeval tv, tv2;
	printf("Start IPCAM video stream OK!\n");
	int /*flag = 0,*/ cnt = 0, fpsCnt = 0, round = 0, lostCnt = 0;
	int outBufSize = 0;
	int outFrmSize = 0;
	int outFrmInfoSize = 0;
	//int bCheckBufWrong;
	int bps = 0;
	gettimeofday(&tv, NULL);


	while(1)
	{
		//ret = avRecvFrameData(avIndex, buf, VIDEO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
		ret = avRecvFrameData2(avIndex, buf, VIDEO_BUF_SIZE, &outBufSize, &outFrmSize, (char *)&frameInfo, sizeof(FRAMEINFO_t), &outFrmInfoSize, &frmNo);
		
		// show Frame Info at 1st frame
		if(frmNo==1)
		{
			char *format[] = {"MPEG4","H263","H264","MJPEG","UNKNOWN"};
			int idx = 0;
			if(frameInfo.codec_id == MEDIA_CODEC_VIDEO_MPEG4)
				idx = 0;
			else if(frameInfo.codec_id == MEDIA_CODEC_VIDEO_H263)
				idx = 1;
			else if(frameInfo.codec_id == MEDIA_CODEC_VIDEO_H264)
				idx = 2;
			else if(frameInfo.codec_id == MEDIA_CODEC_VIDEO_MJPEG)
				idx = 3;
			else
				idx = 4;
			printf("--- Video Formate: %s ---\n", format[idx]);
		}

		if(ret == AV_ER_DATA_NOREADY)
		{
			//printf("AV_ER_DATA_NOREADY[%d]\n", avIndex);
			usleep(gSleepMs * 1000);
			continue;
		}
		else if(ret == AV_ER_LOSED_THIS_FRAME)
		{
			printf("Lost video frame NO[%d]\n", frmNo);
			lostCnt++;
			//continue;
		}
		else if(ret == AV_ER_INCOMPLETE_FRAME)
		{
			#if 1
			if(outFrmInfoSize > 0)
			printf("Incomplete video frame NO[%d] ReadSize[%d] FrmSize[%d] FrmInfoSize[%u] Codec[%d] Flag[%d]\n", frmNo, outBufSize, outFrmSize, outFrmInfoSize, frameInfo.codec_id, frameInfo.flags);
			else
			printf("Incomplete video frame NO[%d] ReadSize[%d] FrmSize[%d] FrmInfoSize[%u]\n", frmNo, outBufSize, outFrmSize, outFrmInfoSize);
			#endif
			lostCnt++;
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
		else
		{
			bps += outBufSize;
			#if 0
			static int frmCnt = 0;
			char fn[32];
			if(frameInfo.flags == IPC_FRAME_FLAG_IFRAME)
				sprintf(fn, "I_%03d.bin", frmCnt);
			else
				sprintf(fn, "P_%03d.bin", frmCnt);
			frmCnt++;
			FILE *fp = fopen(fn, "wb+");
			fwrite(buf, 1, outBufSize, fp);
			fclose(fp);
			#endif
		}

		cnt++;
		#if 0
		if(flag)
		{
			//video_draw(fd, buf, ret);
		}
		else if(frameInfo.flags == IPC_FRAME_FLAG_IFRAME)
		{
			flag = 1;
			//video_draw(fd, buf, ret);
			//printf("I frame[%d] frmNo[%d]\n", cnt, frmNo);
			cnt++;
		}
		#endif

		fpsCnt++;
		gettimeofday(&tv2, NULL);
		long sec = tv2.tv_sec-tv.tv_sec, usec = tv2.tv_usec-tv.tv_usec;
		if(usec < 0)
		{
			sec--;
			usec += 1000000;
		}
		usec += (sec*1000000);

		if(usec > 1000000)
		{
			round++;
			printf("[avIndex:%d] FPS=%d, LostFrmCnt:%d, TotalCnt:%d, LastFrameSize:%d Byte, Codec:%d, Flag:%d, bps:%d Kbps\n", \
					avIndex, fpsCnt, lostCnt, cnt, outFrmSize, frameInfo.codec_id, frameInfo.flags, (bps/1024)*8);
			gettimeofday(&tv, NULL);
			fpsCnt = 0;
			bps = 0;
		}

		// *** AV recv video thread stop condition if necessary. ***
/*
		if(cnt > 400)
		{
			printf ("[thread_ReceiveVideo] recv Video frame cnt=[%d] > 400.\n", cnt);
			break;
		}
*/
	}

	//close_videoX(fd);
	printf("[thread_ReceiveVideo] thread exit\n");

	return 0;
}

/*********
Send IOCtrl CMD to device
***/
int start_ipcam_stream(int avIndex)
{
	int ret;
	unsigned short val = 0;
	if((ret = avSendIOCtrl(avIndex, IOTYPE_INNER_SND_DATA_DELAY, (char *)&val, sizeof(unsigned short)) < 0))
	{
		printf("start_ipcam_stream failed[%d]\n", ret);
		return 0;
	}
	printf("send Cmd: IOTYPE_INNER_SND_DATA_DELAY, OK\n");

	SMsgAVIoctrlAVStream ioMsg;
	memset(&ioMsg, 0, sizeof(SMsgAVIoctrlAVStream));
	if((ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_START, (char *)&ioMsg, sizeof(SMsgAVIoctrlAVStream))) < 0)
	{
		printf("start_ipcam_stream failed[%d]\n", ret);
		return 0;
	}
	printf("send Cmd: IOTYPE_USER_IPCAM_START, OK\n");

#if ENABLE_AUDIO
	if((ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_AUDIOSTART, (char *)&ioMsg, sizeof(SMsgAVIoctrlAVStream))) < 0)
	{
		printf("start_ipcam_stream failed[%d]\n", ret);
		return 0;
	}
	printf("send Cmd: IOTYPE_USER_IPCAM_AUDIOSTART, OK\n");
#endif

#if ENABLE_SPEAKER
	ioMsg.channel = AUDIO_SPEAKER_CHANNEL;
	if((ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SPEAKERSTART, (char *)&ioMsg, sizeof(SMsgAVIoctrlAVStream))) < 0)
	{
		printf("start_ipcam_stream failed[%d]\n", ret);
		return 0;
	}
	printf("send Cmd: IOTYPE_USER_IPCAM_SPEAKERSTART, OK\n");
#endif
/*
	if((ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ, (char *)&ioMsg, sizeof(SMsgAVIoctrlAVStream))) < 0)
	{
		printf("start_ipcam_stream failed[%d]\n", ret);
		return 0;
	}
*/
	return 1;
}


struct timeval tv;

void *thread_ConnectCCR(void *arg)
{
	int ret;
	int SID;
	char *UID=(char *)arg;
	int tmpSID = IOTC_Get_SessionID();
	printf("  [] thread_ConnectCCR::IOTC_Get_SessionID, ret=[%d]\n", tmpSID);
	if(tmpSID < 0)
	{
		printf("IOTC_Get_SessionID failed[%d]\n", tmpSID);
		return 0;
	}

	//SID = IOTC_Connect_ByUID(UID);
	SID = IOTC_Connect_ByUID_Parallel(UID, tmpSID);
	printf("  [] thread_ConnectCCR::IOTC_Connect_ByUID_Parallel, ret=[%d]\n", SID);
	if(SID < 0)
	{
		printf("IOTC_Connect_ByUID_Parallel failed[%d]\n", SID);
		return 0;
	}

	struct timeval tv2;
	gettimeofday(&tv2, NULL);
	long sec = tv2.tv_sec-tv.tv_sec, usec = tv2.tv_usec-tv.tv_usec;
	if(usec < 0)
	{
		sec--;
		usec += 1000000;
	}
	printf("SID[%d] Cost time = %ld sec, %ld ms\n", SID, sec, usec);

	printf("Step 2: call IOTC_Connect_ByUID(%s) ret(%d).......\n", UID, SID);
	struct st_SInfo Sinfo;
	memset(&Sinfo, 0, sizeof(struct st_SInfo));
	
	char *mode[] = {"P2P", "RLY", "LAN"};

	int nResend=-1;
	unsigned long srvType;
	int avIndex = avClientStart2(SID, avID, avPass, 20, &srvType, 0, &nResend);
	printf("Step 2: call avClientStart2(%d).......\n", avIndex);
	if(avIndex < 0)
	{
		printf("avClientStart2 failed[%d]\n", avIndex);
		return 0;
	}
	if(IOTC_Session_Check(SID, &Sinfo) == IOTC_ER_NoERROR)
	{
		if( isdigit( Sinfo.RemoteIP[0] ))
			printf("Device is from %s:%d[%s] Mode=%s NAT[%d] IOTCVersion[%lX]\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID, mode[(int)Sinfo.Mode], Sinfo.NatType, Sinfo.IOTCVersion);
	}
	printf("avClientStart2 OK[%d], Resend[%d]\n", avIndex, nResend);


	if(start_ipcam_stream(avIndex))
	{
		pthread_t ThreadVideo_ID = 0, ThreadAudio_ID = 0, ThreadSpeaker_ID = 0;
		
		printf("  [] thread_ConnectCCR::start_ipcam_stream \n");
		
		// Create Video Recv thread
		if ( (ret=pthread_create(&ThreadVideo_ID, NULL, &thread_ReceiveVideo, (void *)&avIndex)) )
		{
			printf("Create Video Receive thread failed\n");
			exit(-1);
		}
#if ENABLE_AUDIO
		// Create Audio Recv thread
		if ( (ret=pthread_create(&ThreadAudio_ID, NULL, &thread_ReceiveAudio, (void *)&avIndex)) )
		{
			printf("Create Audio Receive thread failed\n");
			exit(-1);
		}
#endif
#if ENABLE_SPEAKER
		// Create Speaker thread
		if ( (ret=pthread_create(&ThreadSpeaker_ID, NULL, &thread_Speaker, (void *)&SID)) )
		{
			printf("Create Speaker thread failed\n");
			exit(-1);
		}
#endif
		if( ThreadVideo_ID!=0)
			pthread_join(ThreadVideo_ID, NULL);
		if( ThreadAudio_ID!=0)
			pthread_join(ThreadAudio_ID, NULL);
		if( ThreadSpeaker_ID!=0)
			pthread_join(ThreadSpeaker_ID, NULL);
	}

	avClientStop(avIndex);
	printf("avClientStop OK\n");
	IOTC_Session_Close(SID);
	printf("SID[%d] IOTC_Session_Close, OK\n", SID);

	return NULL;
}


int main(int argc, char *argv[])
{
	srand(time(NULL));

	int ret;

	char *UID[4];
	int nNumUID;
	int j=0;

	if(argc < 2 || argc > 5)
	{
		printf("Argument Error!!!\n");
		printf("Usage: ./AVAPIs_Client UID1 [UID2 UID3 UID4]\n");
		return -1;
	}
	else
	{
		nNumUID = argc-1;
		for (j=0; j<nNumUID; j++)
			UID[j] = argv[j+1];
	}


	//char *UID = argv[1];
	//struct timeval tv, tv2;
	gettimeofday(&tv, NULL);
	ret = IOTC_Initialize2(0);
	//printf("IOTC_Initialize() ret = %d\n", ret);
	if(ret != IOTC_ER_NoERROR)
	{
		printf("IOTCAPIs_Device exit...!!\n");
		PrintErrHandling (ret);
		return 0;
	}

	// alloc 3 sessions for video and two-way audio
	avInitialize(32);
	unsigned long iotcVer;
	IOTC_Get_Version(&iotcVer);
	int avVer = avGetAVApiVer();
	unsigned char *p = (unsigned char *)&iotcVer;
	unsigned char *p2 = (unsigned char *)&avVer;
	char szIOTCVer[16], szAVVer[16];
	sprintf(szIOTCVer, "%d.%d.%d.%d", p[3], p[2], p[1], p[0]);
	sprintf(szAVVer, "%d.%d.%d.%d", p2[3], p2[2], p2[1], p2[0]);
	printf("IOTCAPI version[%s] AVAPI version[%s]\n", szIOTCVer, szAVVer);

	int i;
	// alloc 12 struct st_LanSearchInfo maybe enough to contain device number on LAN
	// or you have more than 12 device on LAN, please modify this value
	struct st_LanSearchInfo *psLanSearchInfo = (struct st_LanSearchInfo *)malloc(sizeof(struct st_LanSearchInfo)*12);
	if(psLanSearchInfo != NULL)
	{
		// wait time 1000 ms to get result, if result is 0 you can extend to 2000 ms
		int nDeviceNum = IOTC_Lan_Search(psLanSearchInfo, 12, 1000);
		printf("IOTC_Lan_Search ret[%d]\n", nDeviceNum);
		for(i=0;i<nDeviceNum;i++)
		{
			printf("UID[%s] Addr[%s:%d]\n", psLanSearchInfo[i].UID, psLanSearchInfo[i].IP, psLanSearchInfo[i].port);
		}
		free(psLanSearchInfo);
	}
	printf("LAN search done...\n");

	pthread_t ConnectThread_ID[4];
	for(j=0; j<nNumUID; j++)
	{

		if((ret = pthread_create(&ConnectThread_ID[j], NULL, &thread_ConnectCCR, (void *)UID[j])))
		{
			printf("pthread_create(ConnectThread_ID), ret=[%d]\n", ret);
			exit(-1);
		}
	}

	for(j=0; j<nNumUID; j++)
	{
		pthread_join(ConnectThread_ID[j], NULL);
	}

	avDeInitialize();
	IOTC_DeInitialize();

	printf("StreamClient exit...\n");

	return 0;
}


