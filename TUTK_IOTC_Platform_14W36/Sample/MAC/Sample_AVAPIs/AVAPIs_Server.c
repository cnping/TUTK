#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"

#define SERVTYPE_STREAM_SERVER 16
#define MAX_CLIENT_NUMBER	128
#define MAX_SIZE_IOCTRL_BUF		1024
#define MAX_AV_CHANNEL_NUMBER	16

//#define MAX_BITRATE 600000
#define FPS 5	// Video FPS

// *** AV server ID and password, set here ***
char avID[]="admin";
char avPass[]="888888";


#define AUDIO_FORMAT_PCM  // modify this define for test which audio format

#ifdef AUDIO_FORMAT_PCM
#define AUDIO_FRAME_SIZE 640
#define AUDIO_FPS 25
#define AUDIO_CODEC 0x8C

#elif defined (AUDIO_FORMAT_ADPCM)
#define AUDIO_FRAME_SIZE 160
#define AUDIO_FPS 25
#define AUDIO_CODEC 0x8B

#elif defined (AUDIO_FORMAT_SPEEX)
#define AUDIO_FRAME_SIZE 38
#define AUDIO_FPS 56
#define AUDIO_CODEC 0x8D

#elif defined (AUDIO_FORMAT_MP3)
#define AUDIO_FRAME_SIZE 380
#define AUDIO_FPS 32
#define AUDIO_CODEC 0x8E

#elif defined (AUDIO_FORMAT_SPEEX_ENC)
#define AUDIO_FRAME_SIZE 160
#define AUDIO_ENCODED_SIZE 160
#define AUDIO_FPS 56
#define AUDIO_CODEC 0x8D

#elif defined (AUDIO_FORMAT_G726_ENC)
#define AUDIO_FRAME_SIZE 320
#define AUDIO_ENCODED_SIZE 40
#define AUDIO_FPS 50
#define AUDIO_CODEC 0x8F

#elif defined (AUDIO_FORMAT_G726)
#define AUDIO_FRAME_SIZE 40
#define AUDIO_FPS 50
#define AUDIO_CODEC 0x8F
#endif


#define VIDEO_BUF_SIZE	(1024 * 300)
#define AUDIO_BUF_SIZE	1024

typedef struct _AV_Client
{
	int avIndex;
	unsigned char bEnableAudio;
	unsigned char bEnableVideo;
	unsigned char bEnableSpeaker;
	unsigned char bStopPlayBack;
	unsigned char bPausePlayBack;
	int speakerCh;
	int playBackCh;
	SMsgAVIoctrlPlayRecord playRecord;
	pthread_rwlock_t sLock;
}AV_Client;

static AV_Client gClientInfo[MAX_CLIENT_NUMBER];

static char gVideoFn[128];
static char gAudioFn[128];
static int gProcessRun = 1;
static int gOnlineNum = 0;
static int gbSearchEvent = 0;
SMsgAVIoctrlListEventResp *gEventList;
static char gUID[21];
struct sockaddr_in gPushMsgSrvAddr;


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

FILE *pFileLog;

#if 0
static char *GetPushMessageString(char *UID, int eventType)
{
	static char msgBuf[2048];

	sprintf(msgBuf, "GET /apns/apns.php?cmd=raise_event&uid=%s&event_type=%d&event_time=%lu HTTP/1.1\r\n"
	"Host: %s\r\n"
	"Connection: keep-alive\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
	"Accept: */*\r\n"
	"Accept-Encoding: gzip,deflate,sdch\r\n"
	"Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
	"Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
	"Pragma: no-cache\r\n"
	"Cache-Control: no-cache\r\n"
	"\r\n", UID, eventType, time((time_t *)NULL), inet_ntoa(gPushMsgSrvAddr.sin_addr));

	return msgBuf;
}

static void SendPushMessage(int eventType)
{
	int skt;
	if(gPushMsgSrvAddr.sin_addr.s_addr == 0)
	{
		printf("No push message server\n");
		return;
	}
	if((skt =(int) socket(AF_INET, SOCK_STREAM, 0)) >= 0)
	{
		if(connect(skt, (struct sockaddr *)&gPushMsgSrvAddr, sizeof(struct sockaddr_in)) == 0)
		{
			printf("connect HTTP OK\n");
			char *msg = GetPushMessageString(gUID, eventType);
			send(skt, msg, strlen(msg), 0);
		}

		close(skt);
	}
}

static char *GetRegMessageString(char *UID)
{
	static char msgBuf[2048];
	sprintf(msgBuf, "GET /apns/apns.php?cmd=reg_server&uid=%s HTTP/1.1\r\n"
	"Host: %s\r\n"
	"Connection: keep-alive\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
	"Accept: */*\r\n"
	"Accept-Encoding: gzip,deflate,sdch\r\n"
	"Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
	"Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
	"Pragma: no-cache\r\n"
	"Cache-Control: no-cache\r\n"
	"\r\n", UID, inet_ntoa(gPushMsgSrvAddr.sin_addr));

	return msgBuf;
}

static void SendRegister()
{
	struct hostent *host = gethostbyname("push.iotcplatform.com");
	if(host != NULL)
	{
		memcpy(&gPushMsgSrvAddr.sin_addr, host->h_addr_list[0], host->h_length);
		gPushMsgSrvAddr.sin_port = htons(80);
		gPushMsgSrvAddr.sin_family = AF_INET;
	}
	else
	{
		printf("faile to resolve\n");
		memset(&gPushMsgSrvAddr, 0, sizeof(gPushMsgSrvAddr));
		return;
	}

	int skt;
	if((skt =(int) socket(AF_INET, SOCK_STREAM, 0)) >= 0)
	{
		if(connect(skt, (struct sockaddr *)&gPushMsgSrvAddr, sizeof(struct sockaddr_in)) == 0)
		{
			char *msg = GetRegMessageString(gUID);
			//printf("%s", msg);
			send(skt, msg, strlen(msg), 0);
			//recv(skt, buf, 1024, 0);
			//printf("Reg = %s\n", buf);
			printf("Register OK\n");
		}

		close(skt);
	}
}
#endif


int AuthCallBackFn(char *viewAcc,char *viewPwd)
{
	if(strcmp(viewAcc, avID) == 0 && strcmp(viewPwd, avPass) == 0)
		return 1;

	return 0;
}

void regedit_client_to_video(int SID, int avIndex)
{
	AV_Client *p = &gClientInfo[SID];
	p->avIndex = avIndex;
	p->bEnableVideo = 1;
}

void unregedit_client_from_video(int SID)
{
	AV_Client *p = &gClientInfo[SID];
	p->bEnableVideo = 0;
}

void regedit_client_to_audio(int SID, int avIndex)
{
	AV_Client *p = &gClientInfo[SID];
	p->bEnableAudio = 1;
}

void unregedit_client_from_audio(int SID)
{
	AV_Client *p = &gClientInfo[SID];
	p->bEnableAudio = 0;
}

void close_dsp(int fd)
{
	close(fd);
}

int open_dsp()
{
	unlink("audio.in");
	return open("audio.in", O_WRONLY | O_CREAT, 644);
}

void audio_playback(int fd, char *buf, int size)
{
	int ret = write(fd, buf, size);
	if (ret < 0)
	{
		printf("audio_playback::write , ret=[%d]\n", ret);
	}
}

void *thread_ReceiveAudio(void *arg)
{
	int SID = *((int *)arg);
	free(arg);
	int recordCnt = 0, recordFlag = 1;
	AV_Client *p = &gClientInfo[SID];
	unsigned long servType;
	int avIndex = avClientStart(SID, NULL, NULL, 30, &servType, p->speakerCh);
	printf("[thread_ReceiveAudio] start ok idx[%d]\n", avIndex);

	if(avIndex > -1)
	{
		char buf[AUDIO_BUF_SIZE];
		FRAMEINFO_t frameInfo;
		unsigned int frmNo = 0;
		int dspFd = open_dsp();

		if(dspFd < 0)
		{
			printf("open_dsp error[%d]!\n", dspFd);
			avClientStop(avIndex);
			pthread_exit(0);
		}

		while(p->bEnableSpeaker)
		{
			int ret = avCheckAudioBuf(avIndex);
			if(ret < 0) break;
			if(ret < 25) // determined by audio frame rate
			{
				usleep(500000);
				continue;
			}

			ret = avRecvAudioData(avIndex, buf, AUDIO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
			printf("avRecvAudioData[%d]\n", ret);
			if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				printf("avRecvAudioData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				break;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				printf("avRecvAudioData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				break;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				printf("avRecvAudioData Session[%d] cant be used anymore\n", SID);
				break;
			}
			else if(ret == AV_ER_LOSED_THIS_FRAME)
			{
				printf("Audio LOST[%d] ", frmNo);fflush(stdout);
				continue;
			}
			else if(ret < 0)
			{
				printf("Other error[%d]!!!\n", ret);
				continue;
			}

			if(recordFlag)
			{
				if(recordCnt++ > 2000)
				{
					recordFlag = 0;
					printf("Record finish\n");
					close(dspFd);
					break;
				}
				audio_playback(dspFd, buf, ret);
			}
		}
	}

	printf("[thread_ReceiveAudio] exit\n");
	avClientStop(avIndex);
	pthread_exit(0);
}

void *thread_PlayBack(void *arg)
{
	int SID = *((int *)arg);
	free(arg);
	AV_Client *p = &gClientInfo[SID];
	//int avIndex = avServStart2(SID, AuthCallBackFn, 0, SERVTYPE_STREAM_SERVER, p->playBackCh);
	int avIndex = avServStart(SID, avID, avPass, 0, SERVTYPE_STREAM_SERVER, p->playBackCh);
	if(avIndex < 0)
	{
		printf("avServStart2 failed SID[%d] code[%d]!!!\n", SID, avIndex);
		pthread_exit(0);
	}
	printf("thread_PlayBack start OK[%d]\n", avIndex);

	/*
		search play back by p->playRecord.stTimeDay
	*/
	FILE *vFp = fopen("play.IVGA", "rb");
	if(vFp == NULL)
	{
		printf("fopen play.IVGA error\n");
		pthread_exit(0);
	}
	FILE *aFp = fopen("play.audio", "rb");
	if(aFp == NULL)
	{
		printf("fopen play.audio error\n");
		pthread_exit(0);
	}
	//fseek(aFp, 160000, SEEK_SET);
	char videoBuf[20480];
	char audioBuf[256];
	int videoSize = fread(videoBuf, 1, 20480, vFp);
	int sendCnt = 0, ret;
	FRAMEINFO_t videoInfo, audioInfo;
	memset(&videoInfo, 0, sizeof(FRAMEINFO_t));
	memset(&audioInfo, 0, sizeof(FRAMEINFO_t));
	videoInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
	videoInfo.flags = IPC_FRAME_FLAG_IFRAME;
	audioInfo.codec_id = MEDIA_CODEC_AUDIO_SPEEX;
	audioInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
	SMsgAVIoctrlPlayRecordResp resp;
	resp.command = AVIOCTRL_RECORD_PLAY_START;
	int lock_ret;
	
	while(1)
	{
		//get reader lock
		lock_ret = pthread_rwlock_rdlock(&gClientInfo[SID].sLock);
		if(lock_ret)
			printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
		if(p->bStopPlayBack == 1)
		{
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
			printf("SID %d:%d bStopPlayBack bStopPlayBack\n", SID, avIndex );
			break;
		}
		if(gClientInfo[SID].bPausePlayBack)
		{
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
			usleep(50000);
			continue;
		}
		if(sendCnt++ % 20 == 0) // by fps
		{
			ret = avSendFrameData(avIndex, videoBuf, videoSize, (void *)&videoInfo, sizeof(FRAMEINFO_t));
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
			if(ret < 0 && ret != AV_ER_EXCEED_MAX_SIZE)
			{
				printf("avSendFrameData error\n");
				break;
			}
		}
		else
		{
			int size = fread(audioBuf, 1, 38, aFp);
			if(size <= 0)
			{
				printf("fread end\n");
				//release reader lock
				lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
				if(lock_ret)
					printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
				break;
			}
			ret = avSendAudioData(avIndex, audioBuf, size, (void *)&audioInfo, sizeof(FRAMEINFO_t));
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
			if(ret < 0)
			{
				printf("avSendAudioData error\n");
				break;
			}
		}

		if(sendCnt == 300)
		{
			printf("%d play back end\n", avIndex );
			memset(&resp, 0, sizeof(SMsgAVIoctrlPlayRecordResp));
			resp.command = AVIOCTRL_RECORD_PLAY_END;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp));
			//get write lock
			lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
				gClientInfo[SID].bStopPlayBack = 1;
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			break;
		}

		usleep(17000); // by fps
	}

	avServStop(avIndex);
	fclose(vFp);
	fclose(aFp);
	lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
	if(lock_ret)
		printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
	gClientInfo[SID].playBackCh = -1;
	lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
	printf("thread_PlayBack exit\n");
	pthread_exit(0);
}

void Handle_IOCTRL_Cmd(int SID, int avIndex, char *buf, int type)
{
	printf("Handle CMD: ");
	switch(type)
	{
		case IOTYPE_USER_IPCAM_START:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_START, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			//get writer lock
			int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
			regedit_client_to_video(SID, avIndex);
			//release lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);

			printf("regedit_client_to_video OK\n");
		}
		break;
		case IOTYPE_USER_IPCAM_STOP:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_STOP, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			//get writer lock
			int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
			unregedit_client_from_video(SID);
			//release lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
			printf("unregedit_client_from_video OK\n");
		}
		break;
		case IOTYPE_USER_IPCAM_AUDIOSTART:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_AUDIOSTART, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			//get writer lock
			int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
			regedit_client_to_audio(SID, avIndex);
			//release lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
			printf("regedit_client_to_audio OK\n");
		}
		break;
		case IOTYPE_USER_IPCAM_AUDIOSTOP:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_AUDIOSTOP, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			//get writer lock
			int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
			unregedit_client_from_audio(SID);
			//release lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
			printf("unregedit_client_from_audio OK\n");
		}
		break;
		case IOTYPE_USER_IPCAM_SPEAKERSTART:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_SPEAKERSTART, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			//get writer lock
			int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
			gClientInfo[SID].speakerCh = p->channel;
			gClientInfo[SID].bEnableSpeaker = 1;
			//release lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
			// use which channel decided by client
			int *sid = (int *)malloc(sizeof(int));
			*sid = SID;
			pthread_t Thread_ID;
			int ret;
			if((ret = pthread_create(&Thread_ID, NULL, &thread_ReceiveAudio, (void *)sid)))
			{
				printf("pthread_create ret=%d\n", ret);
				exit(-1);
			}
			pthread_detach(Thread_ID);
		}
		break;
		case IOTYPE_USER_IPCAM_SPEAKERSTOP:
		{
			printf("IOTYPE_USER_IPCAM_SPEAKERSTOP\n\n");
			//get writer lock
			int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
			gClientInfo[SID].bEnableSpeaker = 0;
			//release lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
		}
		break;
		case IOTYPE_USER_IPCAM_LISTEVENT_REQ:
		{
			printf("IOTYPE_USER_IPCAM_LISTEVENT_REQ\n\n");
			/*
				SMsgAVIoctrlListEventReq *p = (SMsgAVIoctrlListEventReq *)buf;
				if(p->event == AVIOCTRL_EVENT_MOTIONDECT)
				Handle search event(motion) list
				from p->stStartTime to p->stEndTime
				and get list to respond to App
			*/
			#if 1
			//SendPushMessage(AVIOCTRL_EVENT_MOTIONDECT);

			if(gbSearchEvent == 0) //sample code just do search event list once, actually must renew when got this cmd
			{
				gEventList = (SMsgAVIoctrlListEventResp *)malloc(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*3);
				memset(gEventList, 0, sizeof(SMsgAVIoctrlListEventResp));
				gEventList->total = 1;
				gEventList->index = 0;
				gEventList->endflag = 1;
				gEventList->count = 3;
				int i;
				for(i=0;i<gEventList->count;i++)
				{
					gEventList->stEvent[i].stTime.year = 2012;
					gEventList->stEvent[i].stTime.month = 6;
					gEventList->stEvent[i].stTime.day = 20;
					gEventList->stEvent[i].stTime.wday = 5;
					gEventList->stEvent[i].stTime.hour = 11;
					gEventList->stEvent[i].stTime.minute = i;
					gEventList->stEvent[i].stTime.second = 0;
					gEventList->stEvent[i].event = AVIOCTRL_EVENT_MOTIONDECT;
					gEventList->stEvent[i].status = 0;
				}
				gbSearchEvent = 1;
			}
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)gEventList, sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*gEventList->count);
			#endif
		}
		break;
		case IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL:
		{
			SMsgAVIoctrlPlayRecord *p = (SMsgAVIoctrlPlayRecord *)buf;
			SMsgAVIoctrlPlayRecordResp res;
			printf("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL cmd[%d]\n\n", p->command);
			if(p->command == AVIOCTRL_RECORD_PLAY_START)
			{
				memcpy(&gClientInfo[SID].playRecord, p, sizeof(SMsgAVIoctrlPlayRecord));
				//get writer lock
				int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
				if(lock_ret)
					printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
				res.command = AVIOCTRL_RECORD_PLAY_START;
				printf("playback now %d\n",gClientInfo[SID].playBackCh);
				if( gClientInfo[SID].playBackCh < 0 )
				{
					gClientInfo[SID].bPausePlayBack = 0;
					gClientInfo[SID].bStopPlayBack = 0;
					gClientInfo[SID].playBackCh = IOTC_Session_Get_Free_Channel(SID);
					res.result = gClientInfo[SID].playBackCh;
				}
				else
					res.result = -1;
				//release lock
				lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
				if(lock_ret)
					printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
				if( res.result >= 0 )
				{
					printf("%d start playback\n", res.result);
					int i;
					for(i=0;i<gEventList->count;i++)
					{
						if(p->stTimeDay.minute == gEventList->stEvent[i].stTime.minute)
							gEventList->stEvent[i].status = 1;
					}
					int *sid = (int *)malloc(sizeof(int));
					*sid = SID;
					pthread_t ThreadID;
					int ret;
					if((ret = pthread_create(&ThreadID, NULL, &thread_PlayBack, (void *)sid)))
					{
						printf("pthread_create ret=%d\n", ret);
						exit(-1);
					}
					pthread_detach(ThreadID);
				}else
					printf("Playback on SID %d is still functioning\n", SID );
				
				printf("Sending res [%d]\n",res.result);
				if(avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&res, sizeof(SMsgAVIoctrlPlayRecordResp)) < 0)
					break;
			}
			else if(p->command == AVIOCTRL_RECORD_PLAY_PAUSE)
			{
				res.command = AVIOCTRL_RECORD_PLAY_PAUSE;
				res.result = 0;
				if(avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&res, sizeof(SMsgAVIoctrlPlayRecordResp)) < 0)
					break;
				//get writer lock
				int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
				if(lock_ret)
					printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
				gClientInfo[SID].bPausePlayBack = !gClientInfo[SID].bPausePlayBack;
				//release lock
				lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
				if(lock_ret)
					printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
			}
			else if(p->command == AVIOCTRL_RECORD_PLAY_STOP)
			{
				//get writer lock
				int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
				if(lock_ret)
					printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
				gClientInfo[SID].bStopPlayBack = 1;
				//release lock
				lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
				if(lock_ret)
					printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
			}
		}
		break;
		default:
		printf("avIndex %d: non-handle type[%X]\n", avIndex, type);
		break;
	}
}

/****
Thread - Start AV server and recv IOCtrl cmd for every new av idx
*/
void *thread_ForAVServerStart(void *arg)
{
	int SID = *(int *)arg;
	free(arg);
	int ret;
	unsigned int ioType;
	char ioCtrlBuf[MAX_SIZE_IOCTRL_BUF];
	struct st_SInfo Sinfo;

	printf("SID[%d], thread_ForAVServerStart, OK\n", SID);

	int nResend=-1;
	int avIndex = avServStart3(SID, AuthCallBackFn, 0, SERVTYPE_STREAM_SERVER, 0, &nResend);
	//int avIndex = avServStart2(SID, AuthCallBackFn, 0, SERVTYPE_STREAM_SERVER, 0);
	//int avIndex = avServStart(SID, "admin", "888888", 0, SERVTYPE_STREAM_SERVER, 0);
	if(avIndex < 0)
	{
		printf("avServStart3 failed!! SID[%d] code[%d]!!!\n", SID, avIndex);
		printf("thread_ForAVServerStart: exit!! SID[%d]\n", SID);
		IOTC_Session_Close(SID);
		gOnlineNum--;
		pthread_exit(0);
	}
	if(IOTC_Session_Check(SID, &Sinfo) == IOTC_ER_NoERROR)
	{
		char *mode[3] = {"P2P", "RLY", "LAN"};
		// print session information(not a must)
		if( isdigit( Sinfo.RemoteIP[0] ))
			printf("Client is from[IP:%s, Port:%d] Mode[%s] VPG[%d:%d:%d] VER[%lX] NAT[%d] AES[%d]\n", Sinfo.RemoteIP, Sinfo.RemotePort, mode[(int)Sinfo.Mode], Sinfo.VID, Sinfo.PID, Sinfo.GID, Sinfo.IOTCVersion, Sinfo.NatType, Sinfo.isSecure);
	}
	printf("avServStart3 OK, avIndex[%d], Resend[%d]\n\n", avIndex, nResend);

	while(1)
	{
		ret = avRecvIOCtrl(avIndex, &ioType, (char *)&ioCtrlBuf, MAX_SIZE_IOCTRL_BUF, 1000);
		if(ret >= 0)
		{
			Handle_IOCTRL_Cmd(SID, avIndex, ioCtrlBuf, ioType);
		}
		else if(ret != AV_ER_TIMEOUT)
		{
			printf("avIndex[%d], avRecvIOCtrl error, code[%d]\n",avIndex, ret);
			break;
		}
	}

	unregedit_client_from_video(SID);
	unregedit_client_from_audio(SID);
	avServStop(avIndex);
	printf("SID[%d], avIndex[%d], thread_ForAVServerStart exit!!\n", SID, avIndex);

/*
	pFileLog = fopen("log.txt", "a+");
	fprintf(pFileLog, "[thread_ForAVServerStart] exit, SID=[%d], avIndex=[%d]....\n", SID, avIndex);
	fclose(pFileLog);
*/

	IOTC_Session_Close(SID);
	gOnlineNum--;

	pthread_exit(0);
}

void *thread_Login(void *arg)
{
	int ret;

	while(gProcessRun)
	{
		ret = IOTC_Device_Login((char *)arg, NULL, NULL);
		printf("IOTC_Device_Login() ret = %d\n", ret);
		if(ret == IOTC_ER_NoERROR)
		{
			//SendRegister();
			break;
		}
		else
		{
			PrintErrHandling (ret);
			sleep(30);
		}
	}

	pthread_exit(0);
}

unsigned int getTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}


int32_t g_nAudioPreSample=0;
int32_t g_nAudioIndex=0;

static int gs_index_adjust[8]= {-1,-1,-1,-1,2,4,6,8};
static int gs_step_table[89] =
{
	7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,
	50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,
	408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,
	2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,
	10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767
};

void AdpcmEncode(unsigned char *pRaw, int nLenRaw, unsigned char *pBufEncoded)
{
	short *pcm = (short *)pRaw;
	int cur_sample;
	int i;
	int delta;
	int sb;
	int code;
	nLenRaw >>= 1;

	for(i = 0; i<nLenRaw; i++)
	{
		cur_sample = pcm[i];
		delta = cur_sample - g_nAudioPreSample;
		if (delta < 0){
			delta = -delta;
			sb = 8;
		}else sb = 0;

		code = 4 * delta / gs_step_table[g_nAudioIndex];
		if (code>7)	code=7;

		delta = (gs_step_table[g_nAudioIndex] * code) / 4 + gs_step_table[g_nAudioIndex] / 8;
		if(sb) delta = -delta;

		g_nAudioPreSample += delta;
		if (g_nAudioPreSample > 32767) g_nAudioPreSample = 32767;
		else if (g_nAudioPreSample < -32768) g_nAudioPreSample = -32768;

		g_nAudioIndex += gs_index_adjust[code];
		if(g_nAudioIndex < 0) g_nAudioIndex = 0;
		else if(g_nAudioIndex > 88) g_nAudioIndex = 88;

		if(i & 0x01) pBufEncoded[i>>1] |= code | sb;
		else pBufEncoded[i>>1] = (code | sb) << 4;
	}
}

/********
Thread - Send Audio frames to all AV-idx
*/
void *thread_AudioFrameData(void *arg)
{
	FILE *fp=NULL;
	char buf[AUDIO_BUF_SIZE];
	int frameRate = AUDIO_FPS;
	int sleepTick = 1000000/frameRate;
	FRAMEINFO_t frameInfo;
		
	if( gAudioFn[0]=='\0')
	{
		printf("[Audio] is DISABLED!!\n");
		printf("thread_AudioFrameData: exit\n");
		pthread_exit(0);
	}
	fp = fopen(gAudioFn, "rb");
	if(fp == NULL)
	{
		printf("thread_AudioFrameData: Audio File \'%s\' open error!!\n", gAudioFn);
		printf("[Audio] is DISABLED!!\n");
		printf("thread_AudioFrameData: exit\n");
		pthread_exit(0);
	}

	// *** set audio frame info here ***
	memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
	frameInfo.codec_id = AUDIO_CODEC;
	frameInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;

	printf("thread_AudioFrameData start OK\n");
	printf("[Audio] is ENABLED!!\n");

	while(gProcessRun)
	{
		int i;
		int ret;
		int size = fread(buf, 1, AUDIO_FRAME_SIZE, fp);
		if(size <= 0)
		{
			printf("rewind audio\n");
			rewind(fp);
			continue;
		}
		//AdpcmEncode(rawBuf, size, buf);
		frameInfo.timestamp = getTimeStamp();

		for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
		{
			//get reader lock
			int lock_ret = pthread_rwlock_rdlock(&gClientInfo[i].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);
			if(gClientInfo[i].avIndex < 0 || gClientInfo[i].bEnableAudio == 0)
			{
				//release reader lock
				lock_ret = pthread_rwlock_unlock(&gClientInfo[i].sLock);
				if(lock_ret)
					printf("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);
				continue;
			}
			
			// send audio data to av-idx
			ret = avSendAudioData(gClientInfo[i].avIndex, buf, size, &frameInfo, sizeof(FRAMEINFO_t));
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[i].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);

			//printf("avIndex[%d] size[%d]\n", gClientInfo[i].avIndex, size);
			if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				printf("thread_AudioFrameData: AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				unregedit_client_from_audio(i);
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				printf("thread_AudioFrameData: AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				unregedit_client_from_audio(i);
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				printf("Session cant be used anymore\n");
				unregedit_client_from_audio(i);
			}
			else if(ret < 0)
			{
				printf("avSendAudioData error[%d]\n", ret);
				unregedit_client_from_audio(i);
			}
		}

		usleep(sleepTick);
	}

	fclose(fp);

	printf("[thread_AudioFrameData] exit\n");

	pthread_exit(0);
}

/********
Thread - Send Video frames to all AV-idx
*/
void *thread_VideoFrameData(void *arg)
{
	unsigned int totalCnt = 0;
	float hF = 0.0, lF= 0.0;
	float totalFps = 0;
	int fpsCnt = 0, round = 0;
	struct timeval tv, tv2;
	int bSendFrameOut = 0;
	
	
	FILE *fp = fopen(gVideoFn, "rb");
	if(fp == NULL)
	{
		printf("thread_VideoFrameData: Video File \'%s\' open error!!\n", gVideoFn);
		printf("[Vidio] is DISABLED!!\n");
		printf("thread_VideoFrameData: exit\n");
		pthread_exit(0);
	}
	int frameRate = FPS;
	//int frameSize = MAX_BITRATE / frameRate / 8;
	char buf[VIDEO_BUF_SIZE];
	int i;
	int sleepTick = 1000000/frameRate;
	
	// input file only one I frame for test
	int size = fread(buf, 1, VIDEO_BUF_SIZE, fp);
	fclose(fp);
	if(size <= 0)
	{
		printf("thread_VideoFrameData: Video File \'%s\' read error!!\n", gVideoFn);
		printf("[Vidio] is DISABLED!!\n");
		printf("thread_VideoFrameData: exit\n");
		pthread_exit(0);
	}	
	
	// *** set Video Frame info here *** 
	FRAMEINFO_t frameInfo;
	memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
	frameInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
	frameInfo.flags = IPC_FRAME_FLAG_IFRAME;

	printf("thread_VideoFrameData start OK\n");
	printf("[Video] is ENABLED!!\n");

	while(gProcessRun)
	{
		int ret;
		frameInfo.timestamp = getTimeStamp();
		bSendFrameOut = 0;
		struct timeval tStart, tEnd;
		long takeSec=0, takeUSec=0, sendFrameRoundTick=0;
		
		if(fpsCnt == 0)
			gettimeofday(&tv, NULL);

		for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
		{
			//get reader lock
			int lock_ret = pthread_rwlock_rdlock(&gClientInfo[i].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);
			if(gClientInfo[i].avIndex < 0 || gClientInfo[i].bEnableVideo == 0)
			{
				//release reader lock
				lock_ret = pthread_rwlock_unlock(&gClientInfo[i].sLock);
				if(lock_ret)
					printf("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);
				continue;
			}

			// Send Video Frame to av-idx and know how many time it takes
			frameInfo.onlineNum = gOnlineNum;
			gettimeofday(&tStart, NULL);
			ret = avSendFrameData(gClientInfo[i].avIndex, buf, size, &frameInfo, sizeof(FRAMEINFO_t));
			gettimeofday(&tEnd, NULL);
			
			takeSec = tEnd.tv_sec-tStart.tv_sec, takeUSec = tEnd.tv_usec-tStart.tv_usec;
			if(takeUSec < 0)
			{
				takeSec--;
				takeUSec += 1000000;
			}
			sendFrameRoundTick += takeUSec;
			
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[i].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);

			totalCnt++;

			if(ret == AV_ER_EXCEED_MAX_SIZE) // means data not write to queue, send too slow, I want to skip it
			{
				usleep(10000);
				continue;
			}
			else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				printf("thread_VideoFrameData AV_ER_SESSION_CLOSE_BY_REMOTE SID[%d]\n", i);
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				printf("thread_VideoFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT SID[%d]\n", i);
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				printf("Session cant be used anymore\n");
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret < 0)
				printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
			
			bSendFrameOut = 1;
		}

		if(bSendFrameOut && fpsCnt++ == 30)
		{
			round++;
			gettimeofday(&tv2, NULL);
			long sec = tv2.tv_sec-tv.tv_sec, usec = tv2.tv_usec-tv.tv_usec;
			if(usec < 0)
			{
				sec--;
				usec += 1000000;
			}
			usec += (sec*1000000);

			long oneFrameUseTime = usec / fpsCnt;
			float fps = (float)1000000/oneFrameUseTime;
			if(fps > hF) hF = fps;
			if(lF == 0.0) lF = fps;
			else if(fps < lF) lF = fps;
			printf("Fps = %f R[%d]\n", fps, round);
			fpsCnt = 0;
			totalFps += fps;
		}

		// notice the frames sending time for more specific frame rate control
		if( sleepTick > sendFrameRoundTick )
			usleep(sleepTick-sendFrameRoundTick);
	}

	printf("[thread_VideoFrameData] exit High/Low [%f/%f] AVG[%f] totalCnt[%d]\n", hF, lF, (float)totalFps/round, totalCnt);

	pthread_exit(0);
}

void InitAVInfo()
{
	int i;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].avIndex = -1;
		gClientInfo[i].playBackCh = -1;
		pthread_rwlock_init(&(gClientInfo[i].sLock), NULL);
	}
}

void DeInitAVInfo()
{
	int i;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].avIndex = -1;
		pthread_rwlock_destroy(&gClientInfo[i].sLock);
	}
}

void create_streamout_thread()
{
	int ret;
	pthread_t ThreadVideoFrameData_ID;
	pthread_t ThreadAudioFrameData_ID;

	if((ret = pthread_create(&ThreadVideoFrameData_ID, NULL, &thread_VideoFrameData, NULL)))
	{
		printf("pthread_create ret=%d\n", ret);
		exit(-1);
	}
	pthread_detach(ThreadVideoFrameData_ID);

	if((ret = pthread_create(&ThreadAudioFrameData_ID, NULL, &thread_AudioFrameData, NULL)))
	{
		printf("pthread_create ret=%d\n", ret);
		exit(-1);
	}
	pthread_detach(ThreadAudioFrameData_ID);
}

void LoginInfoCB(unsigned long nLoginInfo)
{
	if((nLoginInfo & 0x04))
		printf("I can be connected via Internet\n");
	else if((nLoginInfo & 0x08))
		printf("I am be banned by IOTC Server because UID multi-login\n");
}

int main(int argc, char *argv[])
{
	int ret , SID;
	pthread_t ThreadLogin_ID;

	if(argc < 3 || argc > 4)
	{
		printf("Argument Error!!!\n");
		printf("Usage: ./AVAPIs_Server UID VideoFile [AudioFile]\n");
		return -1;
	}
	
	strcpy(gUID, argv[1]);
	strcpy(gVideoFn, argv[2]);
	if(argc == 4)
		strcpy(gAudioFn, argv[3]);
	else
		memset(gAudioFn, 0, sizeof(gAudioFn));

	InitAVInfo();

	create_streamout_thread();

	IOTC_Set_Max_Session_Number(MAX_CLIENT_NUMBER);
	// use which Master base on location, port 0 means to get a random port
	//ret = IOTC_Initialize(0, "46.137.188.54", "122.226.84.253", "m2.iotcplatform.com", "m5.iotcplatform.com");
	ret = IOTC_Initialize2(0);
	if(ret != IOTC_ER_NoERROR)
	{
		printf("  [] IOTC_Initialize2(), ret=[%d]\n", ret);
		PrintErrHandling (ret);
		DeInitAVInfo();
		return -1;
	}

	// Versoin of IOTCAPIs & AVAPIs
	unsigned long iotcVer;
	IOTC_Get_Version(&iotcVer);
	int avVer = avGetAVApiVer();
	unsigned char *p = (unsigned char *)&iotcVer;
	unsigned char *p2 = (unsigned char *)&avVer;
	char szIOTCVer[16], szAVVer[16];
	sprintf(szIOTCVer, "%d.%d.%d.%d", p[3], p[2], p[1], p[0]);
	sprintf(szAVVer, "%d.%d.%d.%d", p2[3], p2[2], p2[1], p2[0]);
	printf("IOTCAPI version[%s] AVAPI version[%s]\n", szIOTCVer, szAVVer);

	IOTC_Get_Login_Info_ByCallBackFn(LoginInfoCB);
	// alloc MAX_CLIENT_NUMBER*3 for every session av data/speaker/play back
	avInitialize(MAX_CLIENT_NUMBER*3);

	// create thread to login because without WAN still can work on LAN
	if((ret = pthread_create(&ThreadLogin_ID, NULL, &thread_Login, (void *)gUID)))
	{
		printf("Login Thread create fail, ret=[%d]\n", ret);
		return -1;
	}
	pthread_detach(ThreadLogin_ID);

	while(gProcessRun)
	{
		int *sid = NULL;
		
		// Accept connection only when IOTC_Listen() calling
		SID = IOTC_Listen(0);
		if(SID < 0)
		{
			PrintErrHandling (SID);
			if(SID == IOTC_ER_EXCEED_MAX_SESSION)
			{
				sleep(5);
			}
			continue;
		}
		
		sid = (int *)malloc(sizeof(int));
		*sid = SID;
		pthread_t Thread_ID;
		int ret = pthread_create(&Thread_ID, NULL, &thread_ForAVServerStart, (void *)sid);
		if(ret < 0)
			printf("pthread_create failed ret[%d]\n", ret);
		else
		{
			pthread_detach(Thread_ID);
			gOnlineNum++;
		}
		
		
	}
	DeInitAVInfo();
	avDeInitialize();
	IOTC_DeInitialize();


	return 0;
}


