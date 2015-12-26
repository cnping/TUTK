/******************************************************************************
 *                                                                            *
 * Copyright (c) 2013 by TUTK Co.LTD. All Rights Reserved.                    *
 *                                                                            *
 * Author: Chun Chen                                                          *
 * Date: 2013-01-24                                                           *

 ******************************************************************************/

package com.iotc.sampleIOTC;

import java.io.UnsupportedEncodingException;

import android.os.Message;

import com.iotc.sampleIOTC.Sample_IOTCAPIs.MainHandler;
import com.tutk.IOTC.IOTCAPIs;
import com.tutk.IOTC.St_SInfo;

public class CommApis extends IOTCAPIs 
{
	public static final int	COMMAPIS_STOPPED		=-1001;
	private static final int CMD_REQUEST_DATA_CH = 1;
	private static final int CMD_DATA_CHANNEL_ON = 2;
	private static final int Default_Channel = 0;
	private static final int	MAXSIZE_RECVBUF	=1400;
	protected Sample_IOTCAPIs actMainView			=null;
	public static int 		  ms_nIOTCInit 			=IOTCAPIs.IOTC_ER_TIMEOUT;
	protected ThreadLogin	  m_threadLogin=null;
	protected ThreadListen 	  m_threadListen=null;

	protected String		  m_strUID;
	protected boolean		  m_bAsDevice=true;
	protected St_SInfo		  m_stSInfo=new St_SInfo();
	boolean m_bStoped=true;
	public static final int	STATUS_INIT_SEARCH_DEV	=10;
	
	int 	m_nSearchDev=STATUS_INIT_SEARCH_DEV;
	
	public CommApis(Sample_IOTCAPIs mainView)
	{
		actMainView = mainView;
	}
	
	public int initIOTC()
	{
		if(ms_nIOTCInit!=IOTCAPIs.IOTC_ER_NoERROR)
		{
			int port=(int)(10000+(System.currentTimeMillis()%10000));
			ms_nIOTCInit=IOTC_Initialize(port,"50.19.254.134", "122.248.234.207", 
										"m4.iotcplatform.com", "m5.iotcplatform.com");
			actMainView.setLog("IOTC_Initialize(.)="+ms_nIOTCInit+", udpPort="+port);
			return ms_nIOTCInit;
		}

		return 0;
	}
	public void unInitIOTC()
	{
		if(ms_nIOTCInit==IOTCAPIs.IOTC_ER_NoERROR){
			IOTC_DeInitialize();
			ms_nIOTCInit=IOTCAPIs.IOTC_ER_TIMEOUT;
			setLog(MainHandler.MSGTYPE_LOG, "IOTC_DeInitialize()");
		}
	}
	
	protected void setLog(int msgType, String strLog)
	{
		Message msg=new Message();
		msg.what=msgType;
		msg.obj =strLog;
		actMainView.mainHandler.sendMessageDelayed(msg, 0);
	}
	
	public int startSess(String strUID,boolean isControl){
		m_strUID=strUID;
		m_bStoped=false;
		
		if(m_threadLogin==null)
		{
			m_threadLogin=new ThreadLogin();
			m_threadLogin.start();
			
			try {Thread.sleep(100);}
			catch(InterruptedException e) { e.printStackTrace(); }
		}
		
		if(m_threadListen==null)
		{
			m_threadListen=new ThreadListen();
			m_threadListen.start();
			
			try {Thread.sleep(100);}
			catch(InterruptedException e) { e.printStackTrace(); }
		}
		return 0;
		
	}
	
	public void stopSess()
	{
		m_bStoped=true;		
		int nCount=0;	
		
		if(m_bAsDevice){//as device
			nCount=0;
			if(m_threadLogin!=null){
				do{
					if(m_threadLogin.mbStopedSure) break;
					try {Thread.sleep(40);}
					catch(InterruptedException e) { e.printStackTrace(); }
					nCount++;
				}while(nCount<50);
				if(nCount>=50) m_threadLogin.interrupt();
				m_threadLogin=null;
			}

			nCount=0;
			if(m_threadListen!=null){
				do{
					if(m_threadListen.mbStopedSure) break;
					try {Thread.sleep(40);}
					catch(InterruptedException e) { e.printStackTrace(); }
					nCount++;
				}while(nCount<50);
				if(nCount>=50) m_threadListen.interrupt();
				m_threadListen=null;
			}
		}
	}
	
	
	
	class ThreadLogin extends Thread
	{
		int nRet=-1;
		boolean mbStopedSure=false;
		
		public ThreadLogin(){
			mbStopedSure=false;
		}
		
		public void run() {
			System.out.println("ThreadLogin going...");
			int i=0;
			do{
				nRet=IOTC_Device_Login(m_strUID,null,null);
				System.out.println("IOTC_Device_Login(...)="+nRet);
				
				if(nRet==IOTCAPIs.IOTC_ER_NoERROR) 
				{
					String str="IOTC_Device_Login(.), success";
					setLog(MainHandler.MSGTYPE_LOG, str);
					break;
				}
				
				//2s=10*200ms
				for(i=0;i<10;i++)
				{
					try {Thread.sleep(200);}
					catch(InterruptedException e) { e.printStackTrace(); }					
					if(m_bStoped) break;
				}
			}while(!m_bStoped);
			
			System.out.println("===ThreadLogin exit.");
			mbStopedSure=true;
		}
	}
	
	class ThreadListen extends Thread
	{

		boolean mbStopedSure=false;
		int nSID = -1;

		public ThreadListen(){
			mbStopedSure=false;
		}
		
		public void run() 
		{

			do{
				
				setLog(MainHandler.MSGTYPE_LOG, "ThreadListen going...");

				nSID=IOTC_Listen(0);

				if(m_bStoped) break;
				
				if(clientConnectDev(nSID)<0) continue;

				synchronized(this){
					
					int nCheck = IOTC_Session_Check(nSID, m_stSInfo);
										
					if(nCheck >= 0)
					{
						new ThreadSession(nSID, true).start();
						
						try {Thread.sleep(5000);}
						catch(InterruptedException e) { e.printStackTrace(); }

					}
				}
				
			}while(!m_bStoped);
			
			mbStopedSure=true;
		}
		
		private int clientConnectDev(int SID)
		{
			nSID = SID;
			String str = "";	
			if(nSID<0){
				switch(nSID)
				{
					case IOTC_ER_NOT_INITIALIZED:
						str=String.format("Don't call IOTC_Initialize() when connecting.(%d)", m_nSearchDev);					
						break;

					case IOTC_ER_CONNECT_IS_CALLING:
						str=String.format("IOTC_Connect_ByXX() is calling when connecting.(%d)", nSID);
						break;

					case IOTC_ER_FAIL_RESOLVE_HOSTNAME:
						str=String.format("Can't resolved server's Domain name when connecting.(%d)", nSID);
						break;

					case IOTC_ER_SERVER_NOT_RESPONSE:
						str=String.format("Server not response when connecting.(%d)", nSID);
						break;

					case IOTC_ER_FAIL_GET_LOCAL_IP:
						str=String.format("Can't Get local IP when connecting.(%d)", nSID);
						break;

					case IOTC_ER_UNKNOWN_DEVICE:
						str=String.format("Wrong UID when connecting.(%d)", nSID);
						break;

					case IOTC_ER_UNLICENSE:
						str=String.format("UID is not registered when connecting.(%d)", nSID);
						break;

					case IOTC_ER_CAN_NOT_FIND_DEVICE:
						str=String.format("Device is NOT online when connecting.(%d)", nSID);
						break;

					case IOTC_ER_EXCEED_MAX_SESSION:
						str=String.format("Exceed the max session number when connecting.(%d)", nSID);
						break;

					case IOTC_ER_TIMEOUT:
						str=String.format("Timeout when connecting.(%d)", nSID);
						break;

					case IOTC_ER_DEVICE_NOT_LISTENING:
						str=String.format("The device is not on listening when connecting.(%d)", nSID);
						break;

					default:
						str=String.format("Failed to connect device when connecting.(%d)", nSID);
				}
				setLog(MainHandler.MSGTYPE_LOG, str);
				
				try {Thread.sleep(200);}
				catch(InterruptedException e) { e.printStackTrace(); }	
				
			}else{
				IOTC_Session_Check(nSID, m_stSInfo);
				str="  "+((m_stSInfo.Mode==0) ? "P2P" : "Relay")+", NAT=type"+IOTC_Get_Nat_Type();
				setLog(MainHandler.MSGTYPE_LOG, str);
			}
			
			if(m_bStoped) return COMMAPIS_STOPPED;
			else return nSID;
		}
	}
	
	class ThreadSession extends Thread
	{
		private int nSID;
		private boolean mbStopedSure=false;

		private boolean mbStopedGetReply = false;
		private byte[] mRecvBuf = new byte[MAXSIZE_RECVBUF];

		
		public ThreadSession(int SID,boolean live)
		{
			this.nSID = SID;
			this.mbStopedSure = live;
		}
		
		public void run() 
		{

			setLog(MainHandler.MSGTYPE_LOG, "ThreadSession Start");
			String strRecvMsg = null;
			int nRead=-1;

			do{
				nRead=IOTC_Session_Read(nSID, mRecvBuf, MAXSIZE_RECVBUF, 200, Default_Channel);
				
				try {
					strRecvMsg=new String(mRecvBuf,0, nRead,"utf-8");
					setLog(MainHandler.MSGTYPE_LOG, "got common = " + strRecvMsg);
					
				} catch (UnsupportedEncodingException e1) {
					e1.printStackTrace();
				}
				if(strRecvMsg.length() > 0)
				{
					setLog(MainHandler.MSGTYPE_LOG, "got free_channel strRecvMsg = " + strRecvMsg);
					setLog(MainHandler.MSGTYPE_LOG, "got free_channel nRead = " + nRead);
				}
				else
				{
					setLog(MainHandler.MSGTYPE_LOG, "NO got free_channel strRecvMsg.");
				}
					
				if(nRead>0)
				{
					try 
					{
						
						strRecvMsg=new String(mRecvBuf,0, nRead,"utf-8");

						setLog(MainHandler.MSGTYPE_LOG, "doClient got = " + strRecvMsg);
						try {Thread.sleep(1000);}
						catch(InterruptedException e) { e.printStackTrace(); }
						
					}catch(UnsupportedEncodingException e) {
						e.printStackTrace();
						setLog(MainHandler.MSGTYPE_LOG, "IOTC_Session_Read(.) received");
					}
					
					//common
					if(Integer.valueOf(strRecvMsg).equals(CMD_REQUEST_DATA_CH))
					{
						
						byte[] mRecvBuf_free_channel = new byte[MAXSIZE_RECVBUF];
						int nFree_channel = -1;
						
						do
						{
							//get new channel 1-31
							nFree_channel = IOTC_Session_Get_Free_Channel(nSID);
							
						}while(nFree_channel == -1 || nFree_channel == 0);
						
						int nErrorMessage = -1;
						nErrorMessage = IOTC_Session_Channel_ON(nSID,nFree_channel);
						setLog(MainHandler.MSGTYPE_LOG, "IOTC_Session Open Channel Message = " + nErrorMessage);
						if(nErrorMessage == 0)
						{
							setLog(MainHandler.MSGTYPE_LOG, "IOTC_Session_on_Open nFree_channel = " + nFree_channel);
							byte[] mRecvBufChannel = String.valueOf(nFree_channel).getBytes();
							IOTC_Session_Write(nSID, mRecvBufChannel, mRecvBufChannel.length, Default_Channel);
							
							do
							{
								nRead=IOTC_Session_Read(nSID, mRecvBuf_free_channel, MAXSIZE_RECVBUF, 0, nFree_channel);
		
								if(nRead > 0)
								{
									try
									{
										strRecvMsg=new String(mRecvBuf_free_channel,0, nRead,"utf-8");
									} catch (UnsupportedEncodingException e1) {
										e1.printStackTrace();
									}
	
									setLog(MainHandler.MSGTYPE_LOG, "free_channel got RecvMsg = " + strRecvMsg);
									setLog(MainHandler.MSGTYPE_LOG, "free_channel got nRead = " + nRead);
									break;
								}
							}while(!mbStopedGetReply);
								
							String sSendReplyMessage = "";
							if(mRecvBuf_free_channel.length > 0)
							{
								sSendReplyMessage = new StringBuffer(strRecvMsg).reverse().toString();
							}

							byte[] mRecvBufReplyMessage = String.valueOf(sSendReplyMessage).getBytes();
							int nRead_Reply = IOTC_Session_Write(nSID, mRecvBufReplyMessage, mRecvBufReplyMessage.length, nFree_channel);
							
							if(nRead_Reply > 0)
							{
								setLog(MainHandler.MSGTYPE_LOG, "free_channel Send Reply_nRead = " + nRead_Reply +"  End");
							}
							break;
						}
						else
						{
							setLog(MainHandler.MSGTYPE_LOG, "No Open Free_channel.");
						}
					}
					
				}else if(nRead==IOTCAPIs.IOTC_ER_SESSION_CLOSE_BY_REMOTE  ||
						nRead==IOTCAPIs.IOTC_ER_REMOTE_TIMEOUT_DISCONNECT ||
						nRead==IOTCAPIs.IOTC_ER_INVALID_SID)
				{
					IOTC_Session_Close(nSID);
					strRecvMsg="IOTC_Session_Read(.), Session close";
					setLog(MainHandler.MSGTYPE_LOG, strRecvMsg);
//					break;
				}

			}while(!mbStopedSure);
		}
	}
}
