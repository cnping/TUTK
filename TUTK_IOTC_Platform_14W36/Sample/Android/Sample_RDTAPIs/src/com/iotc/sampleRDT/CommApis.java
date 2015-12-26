/******************************************************************************
 *                                                                            *
 * Copyright (c) 2011 by TUTK Co.LTD. All Rights Reserved.                    *
 *                                                                            *
 * Author: joshua ju                                                          *
 * Date: 2011-05-14                                                           *

 ******************************************************************************/

package com.iotc.sampleRDT;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import android.os.Message;

import com.iotc.sampleRDT.Sample_RDTAPIs.MainHandler;
import com.tutk.IOTC.IOTCAPIs;
import com.tutk.IOTC.RDTAPIs;
import com.tutk.IOTC.St_RDT_Status;
import com.tutk.IOTC.St_SInfo;

public class CommApis {
	public static final int COMMAPIS_INVALID_PARA = -1000;
	public static final int COMMAPIS_STOPPED = -1001;
	public static final int MAXNUM_DO_INDEX = 8;
	public static int ms_nIOTCInit = IOTCAPIs.IOTC_ER_TIMEOUT;
	public static final int STATUS_INIT_SEARCH_DEV = 10;
	public static final int TIMEOUT_sec_SEARCH_DEV = 20; // second

	protected Sample_RDTAPIs actMainView = null;
	protected String m_strUID;
	// as device,MAXNUM is MAXNUM_DO_SID; as client m_arrSID[0]
	protected CommItem[] m_arrRDT_ID = new CommItem[MAXNUM_DO_INDEX];
	protected boolean m_bAsDevice = true, m_bHasClientConn = false;
	protected St_SInfo m_stSInfo = new St_SInfo();
	protected ThreadLogin m_threadLogin = null;
	protected ThreadListen m_threadListen = null;
	protected ThreadSession m_threadSession = null;
	protected ThreadSend m_threadSend = null;
	boolean m_bStoped = true;
	int m_nSearchDev = STATUS_INIT_SEARCH_DEV;
	long m_nTimeCount = 0; // for search device countdown

	public CommApis(Sample_RDTAPIs mainView) {
		actMainView = mainView;
		for (int i = 0; i < MAXNUM_DO_INDEX; i++)
			m_arrRDT_ID[i] = new CommItem();
	}

	public int initIOTCRDT() {
		if (ms_nIOTCInit != IOTCAPIs.IOTC_ER_NoERROR) {
			int port = (int) (10000 + (System.currentTimeMillis() % 10000));
			// ms_nIOTCInit=IOTC_Initialize(port,"m1.iotcplatform.com",
			// "m2.iotcplatform.com", "m4.iotcplatform.com",
			// "m5.iotcplatform.com");
			ms_nIOTCInit = IOTCAPIs.IOTC_Initialize(port, "50.19.254.134", "122.248.234.207", null, null);
			actMainView.setLog("IOTC_Initialize(.)=" + ms_nIOTCInit + ", udpPort=" + port);

			if (ms_nIOTCInit >= 0)
				RDTAPIs.RDT_Initialize();
			return ms_nIOTCInit;
		}
		return 0;
	}

	public void unInitIOTCRDT() {
		if (ms_nIOTCInit == IOTCAPIs.IOTC_ER_NoERROR) {
			RDTAPIs.RDT_DeInitialize();
			IOTCAPIs.IOTC_DeInitialize();
			ms_nIOTCInit = IOTCAPIs.IOTC_ER_TIMEOUT;
			setLog(MainHandler.MSGTYPE_LOG, "IOTC_DeInitialize()");
		}
	}

	protected void setLog(int msgType, String strLog) {
		Message msg = new Message();
		msg.what = msgType;
		msg.obj = strLog;
		actMainView.mainHandler.sendMessageDelayed(msg, 0);
	}

	public int startSess(String strUID, boolean bAsDevice) {
		m_strUID = strUID;
		m_bStoped = false;
		m_bHasClientConn = false;
		m_bAsDevice = bAsDevice;

		// as Device, total: 3 thread
		if (bAsDevice) {
			if (m_threadLogin == null) {
				m_threadLogin = new ThreadLogin();
				m_threadLogin.start();

				try {
					Thread.sleep(40);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			if (m_threadListen == null) {
				m_threadListen = new ThreadListen();
				m_threadListen.start();

				try {
					Thread.sleep(40);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}

		if (m_threadSession == null) {
			m_threadSession = new ThreadSession();
			m_threadSession.start();
		}
		return 0;
	}

	public void stopSess() {
		m_bStoped = true;
		int nCount = 0, i = 0;

		if (m_bAsDevice) {
			nCount = 0;
			if (m_threadLogin != null) {
				do {
					if (m_threadLogin.mbStopedSure)
						break;
					try {
						Thread.sleep(40);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					nCount++;
				} while (nCount < 50);
				if (nCount >= 50)
					m_threadLogin.interrupt();
				m_threadLogin = null;
			}

			nCount = 0;
			if (m_threadListen != null) {
				do {
					if (m_threadListen.mbStopedSure)
						break;
					try {
						Thread.sleep(40);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					nCount++;
				} while (nCount < 50);
				if (nCount >= 50)
					m_threadListen.interrupt();
				m_threadListen = null;
			}

			unInitIOTCRDT();
		} else {
			// as client
			IOTCAPIs.IOTC_Connect_Stop();

			nCount = 0;
		}

		// wait threadSession exit
		nCount = 0;
		if (m_threadSession != null) {
			do {
				if (m_threadSession.mbStopedSure)
					break;
				try {
					Thread.sleep(40);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				nCount++;
			} while (nCount < 50);
			if (nCount >= 50)
				m_threadSession.interrupt();
			m_threadSession = null;
		}

		nCount = 0;
		if (m_threadSend != null) {
			do {
				if (m_threadSend.mbStopedSure)
					break;
				try {
					Thread.sleep(40);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				nCount++;
			} while (nCount < 50);
			if (nCount >= 50)
				m_threadSend.interrupt();
			m_threadSend = null;
		}

		synchronized (this) {
			for (i = 0; i < MAXNUM_DO_INDEX; i++) {
				destroyRDT_ID(m_arrRDT_ID[i]);
			}// end for--loop
		}
	}

	public void sendFile(String fileSended) {
		m_threadSend = new ThreadSend(fileSended);
		m_threadSend.start();
	}

	public boolean isFinish_sendFile() {
		if (m_threadSend == null)
			return true;
		else
			return m_threadSend.mbStopedSure;
	}

	public boolean hasClientConnect() {
		return m_bHasClientConn;
	}

	protected void destroyRDT_ID(CommItem refCommItem) {
		if (refCommItem == null)
			return;
		if (refCommItem.mSID > -1) {
			IOTCAPIs.IOTC_Session_Close(refCommItem.mSID);
			refCommItem.mSID = -1;
		}
		if (refCommItem.mRDT_ID > -1) {
			RDTAPIs.RDT_Destroy(refCommItem.mRDT_ID);
			refCommItem.mRDT_ID = -1;
		}
		refCommItem.closeFile();
	}

	class ThreadLogin extends Thread {
		int nRet = -1;
		boolean mbStopedSure = false;

		public ThreadLogin() {
			mbStopedSure = false;
		}

		public void run() {
			System.out.println("ThreadLogin going...");
			int i = 0;
			do {
				nRet = IOTCAPIs.IOTC_Device_Login(m_strUID, null, null);
				if (nRet == IOTCAPIs.IOTC_ER_NoERROR) {
					String str = "IOTC_Device_Login(.), success";
					setLog(MainHandler.MSGTYPE_LOG, str);
					break;
				}

				// 2s=10*200ms
				for (i = 0; i < 10; i++) {
					try {
						Thread.sleep(200);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					if (m_bStoped)
						break;
				}
			} while (!m_bStoped);

			System.out.println("===ThreadLogin exit.");
			mbStopedSure = true;
		}
	}

	class ThreadListen extends Thread {
		int nRet = -1;
		boolean mbStopedSure = false;

		public ThreadListen() {
			mbStopedSure = false;
		}

		public void run() {
			System.out.println("ThreadListen going...");
			int nSID = -1, nRDTIndex = -1, i = 0;
			do {
				nSID = IOTCAPIs.IOTC_Listen(500);
				if (m_bStoped)
					break;
				if (nSID < 0)
					continue;

				nRDTIndex = RDTAPIs.RDT_Create(nSID, 3000, 0);
				if (nRDTIndex < 0) {
					IOTCAPIs.IOTC_Session_Close(nSID);
					continue;
				}

				synchronized (this) {
					for (i = 0; i < MAXNUM_DO_INDEX; i++) {
						if (m_arrRDT_ID[i].mRDT_ID < 0) {
							m_arrRDT_ID[i].mRDT_ID = nRDTIndex;
							m_arrRDT_ID[i].mSID = nSID;
							m_arrRDT_ID[i].openFile();

							m_bHasClientConn = true;
							St_RDT_Status status = new St_RDT_Status();
							RDTAPIs.RDT_Status_Check(nRDTIndex, status);
							String str = ("RDT_Create(...), nRDTIndex=" + nRDTIndex + " arrived. BufSizeInRecvQueue=" + status.BufSizeInRecvQueue
									+ ", BufSizeInSendQueue=" + status.BufSizeInSendQueue);
							setLog(MainHandler.MSGTYPE_LOG, str);
							break;
						}
					}
				}
				if (i >= MAXNUM_DO_INDEX) {
					RDTAPIs.RDT_Destroy(nRDTIndex);
					IOTCAPIs.IOTC_Session_Close(nSID);
					String str = ("RDT_Create(.), exceed MAXNUM_DO_INDEX");
					setLog(MainHandler.MSGTYPE_LOG, str);
				}
			} while (!m_bStoped);

			System.out.println("===ThreadListen exit.");
			mbStopedSure = true;
		}
	}

	class ThreadSession extends Thread {
		public static final int MAXSIZE_RECVBUF = 1024;
		byte[] mRecvBuf = new byte[MAXSIZE_RECVBUF];
		boolean mbStopedSure = false;

		public ThreadSession() {
		}

		public void run() {
			System.out.println("ThreadSession going...");
			if (m_bAsDevice)
				doDevice();
			else
				doClient();

			mbStopedSure = true;
			System.out.println("===ThreadSession exit.");
		}

		private void doDevice() {
			boolean bDataArrived = true;
			int nRDT_IDConned = 0, nRead = -1;
			boolean iFlag = false;
			int refBufMaxSize;
			String strRecvMsg;
			do {
				if (!bDataArrived)
				{
					try 
					{
						Thread.sleep(40);
					} 
					
					catch (InterruptedException e) 
					{
						e.printStackTrace();
					}
				}

				bDataArrived = false;
				for (nRDT_IDConned = 0; nRDT_IDConned < MAXNUM_DO_INDEX; nRDT_IDConned++) 
				{
					if (m_bStoped)
						break;
					
					if (m_arrRDT_ID[nRDT_IDConned].mRDT_ID < 0)
						continue;

					bDataArrived = true;
					
					refBufMaxSize = MAXSIZE_RECVBUF;
					nRead = RDTAPIs.RDT_Read(m_arrRDT_ID[nRDT_IDConned].mRDT_ID, mRecvBuf, refBufMaxSize, 1000);
					
					if (nRead > 0)
					{
						iFlag = true;
						m_arrRDT_ID[nRDT_IDConned].appendBytToFile(mRecvBuf, nRead);
						setLog(MainHandler.MSGTYPE_LOG, "The file is being read!");
					} 
					else if (nRead == RDTAPIs.RDT_ER_TIMEOUT) 
					{
						System.out.println("IOTCRDTApis.RDT_ER_TIMEOUT");
						
					}
					else if (nRead < 0)
					{
						if(iFlag)
						{
							setLog(MainHandler.MSGTYPE_LOG, "The file is Stored!");
							iFlag = false;
						}
						
						System.out.println("IOTCRDTApis.xxx,Session Close");

						synchronized (this)
						{
							destroyRDT_ID(m_arrRDT_ID[nRDT_IDConned]);
						}
						
						strRecvMsg = "RDT_Read(.), Session close,=" + nRead;
						setLog(MainHandler.MSGTYPE_LOG, strRecvMsg);
					}
				}// end for--loop
			} while (!m_bStoped);
		}

		private int clientConnectDev() {
			// connect this device
			int nSID = -1;
			setLog(MainHandler.MSGTYPE_LOG, "Connecting...");
			
			nSID = IOTCAPIs.IOTC_Connect_ByUID(m_strUID);
			String str = ("IOTC_Connect_ByUID(.)=" + nSID);
			setLog(MainHandler.MSGTYPE_LOG, str);
			
			if (nSID < 0)
			{
				switch (nSID) {
				case IOTCAPIs.IOTC_ER_NOT_INITIALIZED:
					str = String.format("Don't call IOTC_Initialize() when connecting.(%d)", m_nSearchDev);
					break;

				case IOTCAPIs.IOTC_ER_CONNECT_IS_CALLING:
					str = String.format("IOTC_Connect_ByXX() is calling when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_FAIL_RESOLVE_HOSTNAME:
					str = String.format("Can't resolved server's Domain name when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_SERVER_NOT_RESPONSE:
					str = String.format("Server not response when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_FAIL_GET_LOCAL_IP:
					str = String.format("Can't Get local IP when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_UNKNOWN_DEVICE:
					str = String.format("Wrong UID when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_UNLICENSE:
					str = String.format("UID is not registered when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_CAN_NOT_FIND_DEVICE:
					str = String.format("Device is NOT online when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_EXCEED_MAX_SESSION:
					str = String.format("Exceed the max session number when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_TIMEOUT:
					str = String.format("Timeout when connecting.(%d)", nSID);
					break;

				case IOTCAPIs.IOTC_ER_DEVICE_NOT_LISTENING:
					str = String.format("The device is not on listening when connecting.(%d)", nSID);
					break;

				default:
					str = String.format("Failed to connect device when connecting.(%d)", nSID);
				}
				setLog(MainHandler.MSGTYPE_LOG, str);
			} 
			
			/*else {
				IOTCAPIs.IOTC_Session_Check(nSID, m_stSInfo);
				str = "  " + ((m_stSInfo.Mode == 0) ? "P2P" : "Relay") + ", localNAT=type" + IOTCAPIs.IOTC_Get_Nat_Type() + ",remoteNAT="
						+ m_stSInfo.NatType;
				setLog(MainHandler.MSGTYPE_LOG, str);
			}*/
			

			if (m_bStoped)
				return COMMAPIS_STOPPED;
			else
				return nSID;
		}

		private void doClient() {
			String strRecvMsg;
			int nRead = -1, nSID = -1;
			boolean iFlag = false;

			nSID = clientConnectDev();
			
			if (nSID < 0)
				return;// return--------------------------
			else {
					int nRDT_ID = RDTAPIs.RDT_Create(nSID, 3000, 0);
					if (nRDT_ID < 0) 
					{
						IOTCAPIs.IOTC_Session_Close(nSID);
						return;
					} 
					else 
					{
						m_arrRDT_ID[0].mRDT_ID = nRDT_ID;
						m_arrRDT_ID[0].mSID = nSID;
						m_arrRDT_ID[0].openFile();
						m_bHasClientConn = true;
	
						St_RDT_Status status = new St_RDT_Status();
						RDTAPIs.RDT_Status_Check(nRDT_ID, status);
						strRecvMsg = "doClient(), BufSizeInRecvQueue=" + status.BufSizeInRecvQueue + ", BufSizeInSendQueue=" + status.BufSizeInSendQueue;
						setLog(MainHandler.MSGTYPE_LOG, strRecvMsg);
					}
				}

			int bufMaxSize = MAXSIZE_RECVBUF;
			do {
					nRead = RDTAPIs.RDT_Read(m_arrRDT_ID[0].mRDT_ID, mRecvBuf, bufMaxSize, 1000);
					 System.out.println("doClient(): RDT_Read(.)="+nRead);
					 
					if (nRead > 0) 
					{
						iFlag = true;
						m_arrRDT_ID[0].appendBytToFile(mRecvBuf, nRead);
						setLog(MainHandler.MSGTYPE_LOG, "The file is being read!");
	
					}
					else if (nRead == RDTAPIs.RDT_ER_TIMEOUT) 
					{
						System.out.println("IOTCRDTApis.RDT_ER_TIMEOUT");
	
					}
					else if (nRead < 0)
					{
						if(iFlag)
						{
							setLog(MainHandler.MSGTYPE_LOG, "The file is Stored!");
						}
						synchronized (this) 
						{
							destroyRDT_ID(m_arrRDT_ID[0]);
						}
						
						strRecvMsg = "RDT_Read(.), Session close,=" + nRead;
						setLog(MainHandler.MSGTYPE_LOG, strRecvMsg);
						break;
					}
				} while (!m_bStoped);
		}
	}

	class ThreadSend extends Thread {
		public static final int MAXSIZE_SENDBUF = 1024;
		byte[] mSendBuf = new byte[MAXSIZE_SENDBUF];
		boolean mbStopedSure = false;
		int nRDT_IDConned = 0;

		private String mStrFileSend = null;
		FileInputStream mFis;

		public ThreadSend(String strFileSend) {
			mStrFileSend = strFileSend;
		}

		public void run() {
			System.out.println("ThreadSend going...");
			if (mStrFileSend != null && !m_bStoped) 
			{
				if (openFile(mStrFileSend))
				{
					if (m_bAsDevice)
						doDeviceSend();
					else
						doClientSend();
				}
				closeFile(mFis);
			}
			
			mbStopedSure = true;
			System.out.println("===ThreadSend exit.");
		}

		private void doDeviceSend() {
			int nRetWrite = -1, nReadSize = 0;
			boolean iFlag = false;
			do {
				try {
				//	nReadSize = mFis.read(mSendBuf, 0, MAXSIZE_SENDBUF);
					nReadSize = mFis.read(mSendBuf);
					if (nReadSize < 0)
					{
						closeFile(mFis);
						System.out.println("doDeviceSend(), file reached end");
						setLog(MainHandler.MSGTYPE_LOG, "The file tail is arrived.");
						
						if(iFlag)
						{
							setLog(MainHandler.MSGTYPE_LOG, "The file sent successfully!");
						}
						
						break;// break----------------------
					}

					for (nRDT_IDConned = 0; nRDT_IDConned < MAXNUM_DO_INDEX; nRDT_IDConned++)
					{
						if (m_bStoped)
							break;
						if (m_arrRDT_ID[nRDT_IDConned].mRDT_ID < 0)
							continue;

						nRetWrite = RDTAPIs.RDT_Write(m_arrRDT_ID[nRDT_IDConned].mRDT_ID, mSendBuf, nReadSize);
						
						if (nRetWrite < 0)
						{
							System.out.println("SESSION_CLOSE_BY_REMOTE, REMOTE_TIMEOUT_DISCONNECT");
							String str = "RDT_ID=" + m_arrRDT_ID[nRDT_IDConned].mRDT_ID + " is over";
							setLog(MainHandler.MSGTYPE_LOG, str);
							
						//	synchronized (this)
						//	{
							//	destroyRDT_ID(m_arrRDT_ID[nRDT_IDConned]);
						//	}
							
							RDTAPIs.RDT_Destroy(m_arrRDT_ID[nRDT_IDConned].mRDT_ID);
							m_arrRDT_ID[0].mRDT_ID = -1;
							IOTCAPIs.IOTC_Session_Close(m_arrRDT_ID[0].mSID);
							m_arrRDT_ID[0].mSID = -1;
							break; // break---------------------
						}
						else 
						{
							iFlag = true;
							setLog(MainHandler.MSGTYPE_LOG, "File is sending!");
							
							System.out.println("RDT_Write(" + nRetWrite + ")");
						}
					}// end for--loop
					
				} 
				catch (IOException e1)
				{
					e1.printStackTrace();
				}

			} while (!m_bStoped);
		}

		private void doClientSend() {
			int nRetWrite = -1, nReadSize = 0;
			boolean iFlag = false;
			do {
				try {
						//nReadSize = mFis.read(mSendBuf, 0, MAXSIZE_SENDBUF);
						nReadSize = mFis.read(mSendBuf);
						if (nReadSize < 0)
						{
							closeFile(mFis);
							System.out.println("doClientSend(), file reached end");
							setLog(MainHandler.MSGTYPE_LOG, "The file tail is arrived.");
							
							if(iFlag)
							{
								setLog(MainHandler.MSGTYPE_LOG, "The file sent successfully!");
								iFlag = false;
							}
							
							break;// break----------------------
						}
	
						nRetWrite = RDTAPIs.RDT_Write(m_arrRDT_ID[0].mRDT_ID, mSendBuf, nReadSize);
						if (nRetWrite < 0)
						{
							System.out.println("SESSION_CLOSE_BY_REMOTE, REMOTE_TIMEOUT_DISCONNECT");
							String str = "RDT_ID=" + m_arrRDT_ID[0].mRDT_ID + " is over";
							setLog(MainHandler.MSGTYPE_LOG, str);
							
							RDTAPIs.RDT_Destroy(m_arrRDT_ID[0].mRDT_ID);
							m_arrRDT_ID[0].mRDT_ID = -1;
							IOTCAPIs.IOTC_Session_Close(m_arrRDT_ID[0].mSID);
							m_arrRDT_ID[0].mSID = -1;
							break; // break---------------------
						}
						else
						{
							iFlag = true;
							setLog(MainHandler.MSGTYPE_LOG, "The file is sending!");
						}
					}
				catch (IOException e1) 
				{
					e1.printStackTrace();
				}
			} while (!m_bStoped);
		}

		protected boolean openFile(String strFile) 
		{
			boolean bOpen = false;
			try {
					mFis = new FileInputStream(strFile);
					if (mFis != null)
						bOpen = true;
				} 
			catch (FileNotFoundException e) 
				{
					e.printStackTrace();
				}

			return bOpen;
		}

		protected void closeFile(FileInputStream fis) 
		{
			if (fis != null)
			{
				try {
						fis.close();
					} 
				catch (IOException e) 
					{
						e.printStackTrace();
					}
				fis = null;
			}
		}
	}
}
