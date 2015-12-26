/******************************************************************************
 *                                                                            *
 * Copyright (c) 2011 by TUTK Co.LTD. All Rights Reserved.                    *
 * Class: IOTCAVApis.java                                                     *
 *                                                                            *
 * Author: joshua ju                                                          *
 * Date: 2011-05-14                                                           *

	Revisions of IOTCAVApis
	IOTCApis Version joined		Name		Date
	-------						----		----------
	0.5.0.0						Joshua		2011-06-30
	1.1.0.0						Joshua		2011-08-01
	1.2.0.0						Joshua		2011-08-30
	1.3.0.0						Kevin		2013-02-08
 ******************************************************************************/

package com.tutk.IOTC;


public class AVAPIs {
	
	public static final int TIME_DELAY_DELTA				=1;		//ms
	public static final int TIME_DELAY_MIN					=4;		//ms
	public static final int TIME_DELAY_MAX					=500;	//ms
	public static final int TIME_DELAY_INITIAL				=0;	//ms
	
	public static final int TIME_SPAN_LOSED					=1000;	//ms
	
	//--{{inner iotype-----------------------------------------------------
	public static final int IOTYPE_INNER_SND_DATA_DELAY		=0xFF;	//C--->D: avClient(AP) change time interval of sending packets by avSendFrameData(avServer)
	
	//--}}inner iotype-----------------------------------------------------
	
	public static final int API_ER_ANDROID_NULL					= -10000;
	//AVApis error code	===================================================================================
	/** The function is performed successfully. */
	public static final int		AV_ER_NoERROR						 =0;
	
	/** The passed-in arguments for the function are incorrect */
	public static final int		AV_ER_INVALID_ARG					=-20000;
	
	/** The buffer to receive frame is too small to store one frame */
	public static final int		AV_ER_BUFPARA_MAXSIZE_INSUFF		=-20001;
	
	/** The number of AV channels has reached maximum.
	* The maximum number of AV channels is determined by the passed-in
	* argument of avInitialize() */
	public static final int		AV_ER_EXCEED_MAX_CHANNEL			=-20002;
	
	/** Insufficient memory for allocation */
	public static final int		AV_ER_MEM_INSUFF					=-20003;
	
	/** AV fails to create threads. Please check if OS has ability to create threads for AV. */
	public static final int		AV_ER_FAIL_CREATE_THREAD			=-20004;
	
	/** A warning error code to indicate that the sending queue of video frame of an AV server
	* is almost full, probably caused by slow handling of an AV client or network
	* issue. Please note that this is just a warning, the video frame is actually
	* put in the queue. */
	public static final int		AV_ER_EXCEED_MAX_ALARM				=-20005;
	
	/** The frame to be sent exceeds the currently remaining video frame buffer.
	* The maximum of video frame buffer is controlled by avServSetMaxBufSize() */
	public static final int		AV_ER_EXCEED_MAX_SIZE				=-20006;
	
	/** The specified AV server has no response */
	public static final int		AV_ER_SERV_NO_RESPONSE				=-20007;
	
	/** An AV client does not call avClientStart() yet */
	public static final int		AV_ER_CLIENT_NO_AVLOGIN				=-20008;
	
	/** The client fails in authentication due to incorrect view account or password */
	public static final int		AV_ER_WRONG_VIEWACCorPWD			=-20009;
	
	/** The IOTC session of specified AV channel is not valid */
	public static final int		AV_ER_INVALID_SID					=-20010;
	
	/** The specified timeout has expired during some operation */
	public static final int		AV_ER_TIMEOUT						=-20011;
	
	/** The data is not ready for receiving yet. */
	public static final int		AV_ER_DATA_NOREADY					=-20012;
	
	/** Some parts of a frame are lost during receiving */
	public static final int		AV_ER_INCOMPLETE_FRAME				=-20013;
	
	/** The whole frame is lost during receiving */
	public static final int		AV_ER_LOSED_THIS_FRAME				=-20014;
	
	/** The remote site already closes the IOTC session.
	* Please call IOTC_Session_Close() to release local IOTC session resource */
	public static final int		AV_ER_SESSION_CLOSE_BY_REMOTE		=-20015;
	
	/** This IOTC session is disconnected because remote site has no any response
	* after a specified timeout expires. */
	public static final int		AV_ER_REMOTE_TIMEOUT_DISCONNECT		=-20016;
	
	/** Users exit starting AV server process */
	public static final int		AV_ER_SERVER_EXIT		    		=-20017;
	
	/** Users exit starting AV client process */
	public static final int		AV_ER_CLIENT_EXIT		    		=-20018;
	
	/** AV module has not been initialized */
	public static final int		AV_ER_NOT_INITIALIZED	    		=-20019;
	
	/** By design, an AV client cannot send frame and audio data to an AV server */
	public static final int		AV_ER_CLIENT_NOT_SUPPORT	   		=-20020;
	
	/** The AV channel of specified AV channel ID is already in sending IO control process */
	public static final int		AV_ER_SENDIOCTRL_ALREADY_CALLED	   	=-20021;
	
	/** The sending IO control process is terminated by avSendIOCtrlExit() */
	public static final int		AV_ER_SENDIOCTRL_EXIT		    	=-20022;
	
	/** The UID is a lite UID */
	public static final int		AV_ER_NO_PERMISSION					=-20023;
	
	

	public native static int  avGetAVApiVer();	//save as Little endian
	public native static int  avInitialize(int nMaxNumAllowed);
	public native static int  avDeInitialize();
	public native static int  avSendIOCtrl(int avIndex, int ioType, byte[] ioCtrlBuf, int ioCtrlBufSize); //block thread,wait ack
	public native static int  avRecvIOCtrl(int avIndex, int[] pioType, byte[] ioCtrlBuf, int ioCtrlBufMaxSize, int timeout_ms);
	public native static int  avSendIOCtrlExit(int avIndex);

	//Device Side
	public native static int  avServStart(int nSID, byte[] viewAcc, byte[] viewPwd, long timeout_sec, long servType, int ChID);
	public native static void avServStop(int avIndex);
	public native static void avServExit(int nSID, int ChID);
	public native static int  avSendFrameData(int avIndex, byte[]data,int dataSize, byte[] pFrmInfo,int FrmInfoSize);
	public native static int  avSendAudioData(int avIndex, byte[]data,int dataSize, byte[] pFrmInfo,int FrmInfoSize);
	
	//Client Side
	public native static int  avClientStart(int nSID,String viewAcc,String viewPwd, long timeout_sec,long[]pservType,int ChID);
	public native static int  avClientStart2(int nSID,String viewAcc,String viewPwd, long timeout_sec,long[]pservType,int ChID, int[] bResend);
	public native static void avClientStop(int avIndex);
	public native static void avClientExit(int nSID, int ChID);
	public native static int  avRecvFrameData(int avIndex,byte[] buf, int bufMaxSize,
									   byte[] pFrmInfo, int FrmInfoMaxSize, int[] pFrmNo);
	public native static int  avRecvFrameData2(int avIndex,  byte[] buf, int inBufSize, int[] outBufSize, int[] outFrmSize,
					  			 byte[] pFrmInfoBuf, int inFrmInfoBufSize, int[] outFrmInfoBufSize, int[] pFrmNo);
	public native static int  avRecvAudioData(int avIndex,byte[] buf, int bufMaxSize,
									   byte[] pFrmInfo, int FrmInfoMaxSize, int[] pFrmNo);
	public native static int  avCheckAudioBuf(int avIndex); //return buf count
	public native static void avClientSetMaxBufSize(int nMaxBufSize);
	public native static int  avClientCleanBuf(int avIndex);
	public native static int  avClientCleanVideoBuf(int avIndex);
	public native static int  avClientCleanAudioBuf(int avIndex);

	public native static int  avServStart2(int nSID, String viewAcc, String viewPwd, long timeout_sec, long servType, int ChID);
	public native static int  avServStart3(int nSID, String viewAcc, String viewPwd, long timeout_sec, long servType, int ChID, int[] bResend);
	public native static void avServSetResendSize(int avIndex, long nSize);
	public native static int  avServSetDelayInterval(int avIndex, int nPacketNum, int nDelayMs);
	public native static float avResendBufUsageRate(int nAVChannelID);
	
	static { try {System.loadLibrary("AVAPIs");}
			 catch(UnsatisfiedLinkError ule){
			   System.out.println("loadLibrary(AVAPIs),"+ule.getMessage());
			 }
	}
}
