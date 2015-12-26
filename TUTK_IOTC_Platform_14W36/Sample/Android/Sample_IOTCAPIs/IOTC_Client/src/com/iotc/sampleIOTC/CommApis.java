/******************************************************************************
 *                                                                            *
 * Copyright (c) 2013 by TUTK Co.LTD. All Rights Reserved.                    *
 *                                                                            *
 * Author: Chun Chen                                                          *
 * Date: 2013-02-7                                                           *

 ******************************************************************************/

package com.iotc.sampleIOTC;

import java.io.UnsupportedEncodingException;

import android.os.Message;

import com.iotc.sampleIOTC.Sample_IOTCAPIs.MainHandler;
import com.tutk.IOTC.IOTCAPIs;
import com.tutk.IOTC.St_SInfo;
import com.tutk.IOTC.st_LanSearchInfo;

public class CommApis extends IOTCAPIs 
{
	public static final int	COMMAPIS_STOPPED		=-1001;
	private static final int CMD_REQUEST_DATA_CH = 1;
	private static final int CMD_DATA_CHANNEL_ON = 2;
	
	protected Sample_IOTCAPIs actMainView			=null;
	public static int 		  ms_nIOTCInit 			=IOTCAPIs.IOTC_ER_TIMEOUT;
	private int nFristGet = 0;
	private int nSID = -1;
	protected String		  m_strUID;
	private String s_Text;
	protected boolean		  m_bAsDevice=true;
	protected St_SInfo		  m_stSInfo=new St_SInfo();
	
	boolean m_bStoped=true;
	boolean m_bStopedSearch = true;
	boolean mbStopedSure = true;
	boolean mb_bStopedGetDate = true;
	
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
			mLanSearch();	
			
			return ms_nIOTCInit;
		}

		return 0;
	}
	public void unInitIOTC()
	{
		if(ms_nIOTCInit==IOTCAPIs.IOTC_ER_NoERROR){
			IOTC_DeInitialize();
			nFristGet = 0;
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
	
	private void mLanSearch() 
	{
		int[] nArray = new int[1];
		
		st_LanSearchInfo[] ab_LanSearchInfo = IOTC_Lan_Search(nArray,2000);
		
		setLog(MainHandler.MSGTYPE_LOG, "st_LanSearchInfo  === " + nArray[0]);
		
		for(int i = 0; i < nArray[0];i++){
			
			try 
			{
				setLog(MainHandler.MSGTYPE_LOG, "UID = " + i +  " = " + new String(ab_LanSearchInfo[i].UID,0, ab_LanSearchInfo[i].UID.length,"utf-8"));
				setLog(MainHandler.MSGTYPE_LOG, "IP " + i +  " = " + new String(ab_LanSearchInfo[i].IP,0, ab_LanSearchInfo[i].IP.length,"utf-8"));
			} catch (UnsupportedEncodingException e) {

				e.printStackTrace();
			}
			setLog(MainHandler.MSGTYPE_LOG, "Port " + i +  " = " + String.valueOf(ab_LanSearchInfo[i].port));
			setLog(MainHandler.MSGTYPE_LOG, "****************************");			
		}
	}

	public void stopSess()
	{
		mbStopedSure=true;
		m_bStoped=true;		
		m_bStopedSearch=true;

		IOTC_Connect_Stop();

	}
	
	public void start(String strUID,String strString, boolean isControl)
	{
		m_strUID=strUID;
		s_Text=strString;
		m_bStoped=false;
		mbStopedSure=false;
		m_bStopedSearch= false;
		
		int nRead=-1;

		int nCount = 0;
		final	int	MAXSIZE_RECVBUF	=1400;
		byte[] 	mRecvBufGetRelpy = new byte[MAXSIZE_RECVBUF];
		byte[]  mRecvBuf=new byte[MAXSIZE_RECVBUF];
			
		nSID = clientConnectDev();

		setLog(MainHandler.MSGTYPE_LOG, "doClient got nSID = " + nSID);
		
		if(nSID<0) return;
		
		byte[] sendBuf=String.valueOf(CMD_REQUEST_DATA_CH).toString().getBytes();
		
		
		IOTC_Session_Write(nSID,sendBuf , sendBuf.length, 0);
	
		do{
			nRead =IOTC_Session_Read(nSID, mRecvBuf, MAXSIZE_RECVBUF, 0, 0);
			setLog(MainHandler.MSGTYPE_LOG, "doClient got nSID 3 nRead= " + nRead);
			
			String strRecvMsg = "";
			
			if(nRead != 0)setLog(MainHandler.MSGTYPE_LOG, "doClient got Read = " + nRead);
			
			int nStrRecvMsg = -1;
			
			if(nRead > 0)
			{
			
				try
				{
					strRecvMsg=new String(mRecvBuf,0, nRead,"utf-8");
					nStrRecvMsg = Integer.parseInt(strRecvMsg);
					setLog(MainHandler.MSGTYPE_LOG, "doClient got strRecvMsg = " + nStrRecvMsg);
					
					try {Thread.sleep(200);}
					catch(InterruptedException e) { e.printStackTrace(); }
					
				}catch(UnsupportedEncodingException e) {
					e.printStackTrace();
					setLog(MainHandler.MSGTYPE_LOG, "IOTC_Session_Read(.) received");
				}

				if( nStrRecvMsg > 0)
				{
					//get new channel
					do
					{
						nStrRecvMsg = Integer.valueOf(strRecvMsg);
						int n_erMessage = -1;
						n_erMessage = IOTC_Session_Channel_ON(nSID,nStrRecvMsg);
						if(n_erMessage == 0)
						{
							setLog(MainHandler.MSGTYPE_LOG, "IOTC_Session_on_Open nFree_channel = " + nStrRecvMsg);
							String sSendMessage = s_Text;
							byte[] mRecvBufMessage = String.valueOf(sSendMessage).getBytes();
							IOTC_Session_Write(nSID, mRecvBufMessage, mRecvBufMessage.length, nStrRecvMsg);
							setLog(MainHandler.MSGTYPE_LOG, "IOTC_Session_sSendMessage = " + sSendMessage);
							
							int nRead2 = -1;

							do{
								nRead2 =IOTC_Session_Read(nSID, mRecvBufGetRelpy, MAXSIZE_RECVBUF, 200, nStrRecvMsg);//def == 1000 sec
								
								setLog(MainHandler.MSGTYPE_LOG, "doClient got Reply_Read = " + nRead2);

								String strRecvMsg2="";
								if(nRead2 > 0)
								{
									try 
									{
										strRecvMsg2=new String(mRecvBufGetRelpy,0, nRead2,"utf-8");
										
										setLog(MainHandler.MSGTYPE_LOG, "doClient got MessageData = " + strRecvMsg2);

										try {Thread.sleep(40);}
										catch(InterruptedException e) { e.printStackTrace(); }

									}catch(UnsupportedEncodingException e) {
										e.printStackTrace();
										setLog(MainHandler.MSGTYPE_LOG, "IOTC_Session_Read(.) received");
									}
									break;
									
								}
							}while(!mb_bStopedGetDate);
							
							mbStopedSure = true;
							m_bStopedSearch=true;
						}
						else
						{
							setLog(MainHandler.MSGTYPE_LOG, "IOTC_Session_open_nFree_channel_Error");
						}

					}while(!mbStopedSure);
				}
			}
			else
			{
			
				nCount ++;
				try {Thread.sleep(100);}
				catch(InterruptedException e) { e.printStackTrace(); }
				
				if(nCount >= 50)
				{	
					m_bStopedSearch = true;
				}
			}
		}while(!m_bStopedSearch);
		nFristGet++;
	}
	
	public int clientConnectDev()
	{
			//connect this device
//			int nSID = -1;
			setLog(MainHandler.MSGTYPE_LOG, "Connecting...");
			if(nFristGet == 0)
			{
				nSID=IOTC_Connect_ByUID(m_strUID);
			}
			String str = ("IOTC_Connect_ByUID(.)="+nSID);
			setLog(MainHandler.MSGTYPE_LOG, str);			
			if(nSID<0)
			{
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
			}
			else
			{
				IOTC_Session_Check(nSID, m_stSInfo);
				str="  "+((m_stSInfo.Mode==0) ? "P2P" : "Relay")+", NAT=type"+IOTC_Get_Nat_Type();
				setLog(MainHandler.MSGTYPE_LOG, str);
			}
			
			if(m_bStoped) return COMMAPIS_STOPPED;
			else return nSID;
	}
}
