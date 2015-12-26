/*
 *  AVFRAMEINFO.h
 *	Define AVFRAME Info
 *  Created on:2011-08-12
 *  Last update on:2014-07-02
 *  Author: TUTK
 *
 *  2012-08-07 - Add video width and height to FRAMEINFO_t;
 */

#ifndef _AVFRAME_INFO_H_
#define _AVFRAME_INFO_H_

/* CODEC ID */
typedef enum 
{
	MEDIA_CODEC_UNKNOWN			= 0x00,
	MEDIA_CODEC_VIDEO_MPEG4		= 0x4C,
	MEDIA_CODEC_VIDEO_H263		= 0x4D,
	MEDIA_CODEC_VIDEO_H264		= 0x4E,
	MEDIA_CODEC_VIDEO_MJPEG		= 0x4F,
	
    MEDIA_CODEC_AUDIO_AAC       = 0x88,   // 2014-07-02 add AAC audio codec definition
    MEDIA_CODEC_AUDIO_G711U     = 0x89,   //g711 u-law
    MEDIA_CODEC_AUDIO_G711A     = 0x8A,   //g711 a-law	
    MEDIA_CODEC_AUDIO_ADPCM     = 0X8B,
	MEDIA_CODEC_AUDIO_PCM		= 0x8C,
	MEDIA_CODEC_AUDIO_SPEEX		= 0x8D,
	MEDIA_CODEC_AUDIO_MP3		= 0x8E,
    MEDIA_CODEC_AUDIO_G726      = 0x8F,

}ENUM_CODECID;

/* FRAME Flag */
typedef enum 
{
	IPC_FRAME_FLAG_PBFRAME	= 0x00,	// A/V P/B frame..
	IPC_FRAME_FLAG_IFRAME	= 0x01,	// A/V I frame.
	IPC_FRAME_FLAG_MD		= 0x02,	// For motion detection.
	IPC_FRAME_FLAG_IO		= 0x03,	// For Alarm IO detection.
}ENUM_FRAMEFLAG;

typedef enum
{
	AUDIO_SAMPLE_8K			= 0x00,
	AUDIO_SAMPLE_11K		= 0x01,
	AUDIO_SAMPLE_12K		= 0x02,
	AUDIO_SAMPLE_16K		= 0x03,
	AUDIO_SAMPLE_22K		= 0x04,
	AUDIO_SAMPLE_24K		= 0x05,
	AUDIO_SAMPLE_32K		= 0x06,
	AUDIO_SAMPLE_44K		= 0x07,
	AUDIO_SAMPLE_48K		= 0x08,
}ENUM_AUDIO_SAMPLERATE;

typedef enum
{
	AUDIO_DATABITS_8		= 0,
	AUDIO_DATABITS_16		= 1,
}ENUM_AUDIO_DATABITS;

typedef enum
{
	AUDIO_CHANNEL_MONO		= 0,
	AUDIO_CHANNEL_STERO		= 1,
}ENUM_AUDIO_CHANNEL;

/* Audio Frame: flags =  (samplerate << 2) | (databits << 1) | (channel) */


/* Audio/Video Frame Header Info */
typedef struct _FRAMEINFO
{
	unsigned short codec_id;	// Media codec type defined in sys_mmdef.h,
								// MEDIA_CODEC_AUDIO_PCMLE16 for audio,
								// MEDIA_CODEC_VIDEO_H264 for video.
	unsigned char flags;		// Combined with IPC_FRAME_xxx.
	unsigned char cam_index;	// 0 - n

	unsigned char onlineNum;	// number of client connected this device
	unsigned char reserve1[3];

	unsigned int reserve2;	//
	unsigned int timestamp;	// Timestamp of the frame, in milliseconds

    // unsigned int videoWidth;
    // unsigned int videoHeight;
    
}FRAMEINFO_t;
#endif

