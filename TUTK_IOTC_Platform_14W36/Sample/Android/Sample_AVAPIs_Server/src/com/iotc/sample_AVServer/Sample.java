/******************************************************************************
 *                                                                            *
 * Copyright (c) 2013 by TUTK Co.LTD. All Rights Reserved.                    *
 *                                                                            *
 * Author: Chun Chen                                                          *
 * Date: 2013-02-7                                                          *

 ******************************************************************************/

package com.iotc.sample_AVServer;

import com.iotc.sampleIOTC2.R;

import com.tutk.IOTC.IOTCAPIs;
import com.tutk.IOTC.St_SInfo;

import com.tutk.IOTC.AVAPIs;
import com.tutk.IOTC.AVIOCTRLDEFs;
import com.tutk.IOTC.AVFrame;
import com.tutk.IOTC.Packet;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.ByteArrayInputStream;
import java.nio.CharBuffer;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;



public class Sample extends Activity {
	public static final String THIS_APP_VER	="1.0.0.4";
	public static final String DEFAULT_FILENAME_CFG_INI	="IOTC.ini";
	public static final String strCLCF="\r\n";
	public static final int	   LINES_PER_PAGE	=18;

	protected String 		strSDPath =null;
	private EditText 		edt_uid;
	private Button	 		btn_listen;
	private TextView 		txt_api_ver;
	private TextView 		txt_log;
	public static Context 	content;
	

	protected CommApis main;
	public MainHandler mainHandler = new MainHandler();
	private boolean  m_bAutoMoveDown=true;

	
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
                
        content = this.getApplicationContext();
        findView();
		setListenner();
		
		strSDPath = Environment.getExternalStorageDirectory().toString();
    	setLog("Device");

		main=new CommApis(this);
		initUI();
    }

	
	/*****************************/
	/*****************************/
	/*****************************/
	/***                       ***/
	/***         main          ***/
	/***                       ***/
	/*****************************/
	/*****************************/
	/*****************************/
	
	private String UID;
	private String gAcc="admin";
	private String gPwd="888888";
	private int gOnlineNum=0;
	
	Thread t_Main;
	Thread t_Login;
	Thread t_Session;
	Thread t_Video;
	
	private static final int MAX_CLIENT = 12;
	private static final int MAX_BUF_SIZE = 1024;
	private static final long SERVTYPE_STREAM_SERVER=16;
	
	private boolean bInProgress = false;
	private boolean bInitialized = false;	
	private int bState = 1;
    
	public class FrameInfo
	{
		public short codec_id;
		public byte flags;
		public byte cam_index;
		public byte onlineNum;
		public byte[] reserve1 = new byte[3];
		public int reserve2;
		public int timestamp;
		
		public byte[] parseContent(short _codec_id, byte _flags) {

			byte[] result = new byte[16];
			byte[] arg1 = Packet.shortToByteArray_Little(_codec_id);
			byte[] arg2 = new byte[1];
			arg2[0] = _flags;

			System.arraycopy(arg1, 0, result, 0, 2);
			System.arraycopy(arg2, 0, result, 2, 1);
			return result;
		}
	}

	
	public class AV_Client
	{
		public int avIndex;
		public boolean bEnableVideo = false;
		public boolean bEnableAudio = false;
		
		public int speakerCH;
		public int playBackCH;
		
		public Object lock = new Object();
		
		public AV_Client ()
		{
			avIndex=-1;
			bEnableVideo=false;
			bEnableAudio=false;
			speakerCH=0;
			playBackCH=0;
		}
	}
	
	AV_Client[] gClientInfo = new AV_Client[MAX_CLIENT]; 
	
	public void DebugL()
	{
		//logi(getLineInfo());
	}
	public void Create()
	{
		logi ("=== Create ===");
		int i=0;

		for (i=0; i<MAX_CLIENT; i++)
		{
			gClientInfo[i] = new AV_Client();
		}
	}
	public void Initialize()
	{
		logi ("=== Initialize ===");
		int i=0;

    	for (i=0; i<MAX_CLIENT; i++)
    	{
    		gClientInfo[i].avIndex = -1;
    		gClientInfo[i].bEnableAudio = false;
    		gClientInfo[i].bEnableVideo = false;
    	}
	}
	public void DeInitialize()
	{
		logi ("=== DeInitialize ===");
		int rc=0;
		
		rc = AVAPIs.avDeInitialize ();
		logi ("avDeInitialize, rc=["+rc+"]");
		
		rc = IOTCAPIs.IOTC_DeInitialize ();
		logi ("IOTC_DeInitialize, rc=["+rc+"]");
	}
	
	
    public byte[] getBytesFromFile(String fileName) throws IOException {
    	if(strSDPath==null) return null;

		//StringBuffer sb=new StringBuffer(strSDPath);
		StringBuffer sb=new StringBuffer("/sdcard");
		sb.append("/z_jpg/");
		sb.append(fileName);
    	File tmpFile=null;
		tmpFile=new File(sb.toString());
		
        FileInputStream is = new FileInputStream(tmpFile);
    
        //��o�ɮפj�p
        long length = tmpFile.length();
        byte[] bytes = new byte[(int)length];
    
        int offset = 0;
        int numRead = 0;
        while (offset < bytes.length
               && (numRead=is.read(bytes, offset, bytes.length-offset)) >= 0) {
            offset += numRead;
        }
 
        is.close();
        return bytes;
    }
    
    
	public static String getLineInfo()  
    {  
        StackTraceElement ste = new Throwable().getStackTrace()[1];  
        return ste.getFileName() + ": Line " + ste.getLineNumber();  
    }
	
	class ThreadVideo extends Thread {
		@Override
		public void run() {
    		super.run();
    		int rc=0;
    		int i=0;
    		int count=0;
    		int second=3;
    		final int page=4;
    		int pageNumber=0;
			
    		try {
    			logi ("=== Video ===");
    			byte[] buf_frame=null;
    			byte[] buf_info=new byte[1024];
    			
    			byte[][] buf_file=new byte[page][];
    			for (i=0; i<page; i++)
    			{
    				buf_file[i] = getBytesFromFile("pp_"+i+".jpg");
    			}


    			while (bInProgress)
    			{
    				// change a photo every 3 sec.
    				pageNumber = ((int)(count/second))%page;
    				buf_frame = buf_file[pageNumber];
    				
    				
    				for (i=0; i<MAX_CLIENT; i++)
    				{
    					// Search a valid avIndex.
    					if (gClientInfo[i].avIndex <0 || gClientInfo[i].bEnableVideo == false)
    					{
    						continue;
    					}
    					
    					FrameInfo frame = new FrameInfo();
    					frame.codec_id = AVFrame.MEDIA_CODEC_VIDEO_MJPEG;
    					frame.flags = AVFrame.IPC_FRAME_FLAG_IFRAME;
    					buf_info = frame.parseContent(frame.codec_id, frame.flags);

    					// Send a frame every 3 sec.
    					if ( (count%second)==0)
    					{
        					rc = AVAPIs.avSendFrameData(gClientInfo[i].avIndex, buf_frame, buf_frame.length, buf_info, buf_info.length);
        					logi ("avSendFrameData(), rc=["+rc+"], avIndex=["+gClientInfo[i].avIndex+"]");
        					if (rc == AVAPIs.AV_ER_REMOTE_TIMEOUT_DISCONNECT) {
        						unregedit_client_from_video (i);
        					} else if (rc == AVAPIs.AV_ER_SESSION_CLOSE_BY_REMOTE){
        						unregedit_client_from_video (i);
        					} else if (rc == IOTCAPIs.IOTC_ER_INVALID_SID) {
        						unregedit_client_from_video (i);
        					} else if (rc<0) {
        						logi ("avSendFrameData(), XXXXXXXXXX");
        					}
    					}
    				}
    				
    				count++;
    				main.SleepMS(1000);
    			}
    			
    			logi ("End!");
    			
    		} catch (Exception e) {
    			e.printStackTrace();
    		}
		}
	}
	
	private void create_streamout_thread()
	{
		t_Video = new ThreadVideo();
		t_Video.start();
	}
	
	class ThreadLogin extends Thread {
		@Override
		public void run() {
    		super.run();
    		int rc=0;
    		try {
    			logi ("=== Login ===");
    			while (true)
    			{
        			rc = IOTCAPIs.IOTC_Device_Login(UID, null, null);
        			logi ("IOTC_Device_Login, rc=["+rc+"], UID=["+UID+"]");
        			if (rc==IOTCAPIs.IOTC_ER_NoERROR) {
        				break;
        			} else {
        				main.SleepMS(1000);
        			}
    			}
    		} catch (Exception e) {
    			e.printStackTrace();
    		}
		}
	}

	
	private int regedit_client_to_video (int sid, int avIndex)
	{
		gClientInfo[sid].avIndex = avIndex;
		gClientInfo[sid].bEnableVideo = true;
		return 0;
	}
	
	private int unregedit_client_from_video (int sid)
	{
		gClientInfo[sid].avIndex = -1;
		gClientInfo[sid].bEnableVideo = false;
		return 0;
	}
	
	private void Handle_IOCTRL_Cmd (int sid, int avIndex, byte[] buf, int type)
	{
		logi ("Handle_IOCTRL_Cmd:");
		ByteArrayInputStream in = null;
		switch (type)
		{
		case AVIOCTRLDEFs.IOTYPE_USER_IPCAM_START:
			logi (" - IOTYPE_USER_IPCAM_START");
	        try {
	            in = new ByteArrayInputStream(buf,0,1);
	            int data = in.read();
	            while (data != -1)
	            {
	            	data = in.read();
	            }
	        } catch (Exception e) {
	        	e.printStackTrace();
	        }
	        
	        synchronized (gClientInfo[sid].lock) {
	        	regedit_client_to_video (sid, avIndex);
	        }
	        logi ("regedit_client_to_video, ok !!!");
	        break;
	
		
		case AVIOCTRLDEFs.IOTYPE_USER_IPCAM_STOP:
			logi (" - IOTYPE_USER_IPCAM_STOP");
	        try {
	            in = new ByteArrayInputStream(buf,0,1);
	            int data = in.read();
	            while (data != -1)
	            {
	            	data = in.read();
	            }
	        } catch (Exception e) {
	        	e.printStackTrace();
	        }
	        
	        synchronized (gClientInfo[sid].lock) {
	        	unregedit_client_from_video (sid);
	        }
	        logi ("unregedit_client_from_video, ok !!!");
	        break;
		
		}  // end of switch
	}
	
	class Thread_ForAVServerStart extends Thread {
		int sid;
		Thread_ForAVServerStart (int _sid) {
			sid = _sid;
		}
		
		@Override
		public void run() {
    		super.run();
    		int rc=0;
    		try {
    			logi ("=== Session ===");
    			logi ("SID=["+sid+"]");
    			
    			int avIndex=0;
    			int[] bResend = new int[1];
    			avIndex = AVAPIs.avServStart3(sid, gAcc, gPwd, 2, SERVTYPE_STREAM_SERVER, 0, bResend);
    			logi ("avServStart3(), avIndex=["+avIndex+"], sid=["+sid+"], bResend=["+bResend[0]+"]");
    			if (avIndex<0)
    			{
    				logi ("Fail...");
    				IOTCAPIs.IOTC_Session_Close(sid);
    				gOnlineNum--;
    			}
    			
    			AVAPIs.avServSetResendSize(avIndex,4096);
    			St_SInfo info = new St_SInfo(); 
    			String [] mode={"P2P mode", "Relay mode", "Lan mode"};

    			rc=IOTCAPIs.IOTC_Session_Check(sid, info);
    			if (rc==IOTCAPIs.IOTC_ER_NoERROR)
    			{
    				logi ("   -> IP:Port=["+main.ByteToString(info.RemoteIP)+":"+info.RemotePort+"]");
    				if (info.Mode>=0 && info.Mode<=2) {
    					logi ("   -> Mode=["+mode[info.Mode]+"]");
    				}
    				logi ("   -> NatType=["+info.NatType+"]");
    				logi ("   -> Version=["+info.IOTCVersion+"]");
    			}
    			
    			 
    			byte[] ioCtrlBuf=new byte[MAX_BUF_SIZE];
    			int[] ioType=new int[1];
    			
    			while (true)
    			{
    				rc = AVAPIs.avRecvIOCtrl(avIndex, ioType, ioCtrlBuf, MAX_BUF_SIZE, 1000);
    				if (rc>=0) {
    					logi ("avRecvIOCtrl(), rc=["+rc+"]");
    					Handle_IOCTRL_Cmd (sid, avIndex, ioCtrlBuf, ioType[0]);
    				} else if (rc != AVAPIs.AV_ER_TIMEOUT) {
    					logi ("avRecvIOCtrl(), rc=["+rc+"]");
    					break;
    				}
    			}
    			
    			unregedit_client_from_video (sid);
    			AVAPIs.avServStop (avIndex);
    			logi ("SID=["+sid+"], avIndex=["+avIndex+"], thread_ForAVServerStart Exit !!");
    			
    		} catch (Exception e) {
    			e.printStackTrace();
    		}
		}
	}
	
	class ThreadMain extends Thread {
		
		@Override
		public void run() {
    		super.run();
    		int rc=0;
    		try {
    			logi ("=== main ===");
    			
    			// version //-----
    			long [] VersionIOTC=new long [1];
    			IOTCAPIs.IOTC_Get_Version (VersionIOTC);
    			logi ("IOTC ver:" + verN2Str (VersionIOTC[0]));

    			long VersionAV=0;
    			VersionAV = AVAPIs.avGetAVApiVer();
    			logi ("AV ver:" + verN2Str (VersionAV));
    			
    			// initialize //-----
    			Create();
    			Initialize();
    			
    			rc = IOTCAPIs.IOTC_Initialize2 (0);
    			logi ("IOTC_Initialize2, rc=["+rc+"]");
    			
    			rc = AVAPIs.avInitialize (MAX_CLIENT);
    			logi ("avInitialize, rc=["+rc+"]");
    			
    			create_streamout_thread ();
    			
    			// device login //-----
    			t_Login = new ThreadLogin();
				t_Login.start();
				
				// listen //-----
    			while (bInProgress == true)
    			{
    				rc = IOTCAPIs.IOTC_Listen(0);
    				if ( rc>=0 ) {
    					gOnlineNum++;
    					t_Session = new Thread_ForAVServerStart (rc);
    					t_Session.start();
    				} else if (rc==IOTCAPIs.IOTC_ER_EXCEED_MAX_SESSION) {
    					main.SleepMS (5000);
    				} else {
    					logi ("IOTC_Listen, rc=["+rc+"]");
    					break;
    				}
    				continue;
    			}
    			


    		} catch (Exception e) {
    			e.printStackTrace();
    		}
		}
	}
	
    private void initUI(){
    	String strTitle=loadIniFile();
		if(strTitle!=null) {
			edt_uid.setText(strTitle);
		}
		strTitle=getTitle()+"";
    	setTitle(strTitle);
		long [] VersionIOTC=new long [1];
		IOTCAPIs.IOTC_Get_Version (VersionIOTC);
    	txt_api_ver.setText(" (IOTC) "+verN2Str(VersionIOTC[0]));
    }
    public static String verN2Str(long nVer)
    {
		String strVer=String.format("%d.%d.%d.%d", 
				(nVer>>24)&0xff, (nVer>>16)&0xff, (nVer>>8)&0xff, nVer&0xff);
		return strVer;
    }
    private void findView() {
    	edt_uid		=(EditText)findViewById(R.id.edt_uid);
    	btn_listen	=(Button)findViewById(R.id.btn_listen);
		txt_api_ver =(TextView)findViewById(R.id.txt_api_ver);
		txt_log		=(TextView)findViewById(R.id.txt_log);
		txt_log.setMovementMethod(ScrollingMovementMethod.getInstance());
		txt_log.setFocusable(true);
	}
	private void setListenner() {
		btn_listen.setOnClickListener(btnStartListener);
		txt_log.setOnTouchListener(txtLogTouchListener);
	}	
	private View.OnClickListener btnStartListener=new View.OnClickListener() 
	{		
		@Override
		public void onClick(View v) {
			
			if(bState==1){
					btn_listen.setText(R.string.title_btn_exit);
					Toast.makeText(v.getContext(), "Listen", Toast.LENGTH_LONG).show();
					if (bInitialized == false)
					{
						txt_log.setText("");
						bInProgress = true;
						bInitialized = true;
					}
					
					String strUID;
					strUID=edt_uid.getText().toString();
					saveIniFile(strUID);
					if(strUID.length()<20 || strUID.length()>20) {
						setLog("The length of UID must be 20 characters.");
						return;				
					}
					UID = strUID;
					
					t_Main = new ThreadMain();
					t_Main.start();
					bState=2;

			}else if (bState==2){
				bInitialized = false;
				bInProgress = false;

				main.SleepMS(500);
				DeInitialize();
				t_Main.interrupt();
				t_Main = null;

				int pid = android.os.Process.myPid();
				android.os.Process.killProcess(pid);
			}
		}
	};	

	private View.OnTouchListener txtLogTouchListener=new View.OnTouchListener() {
		float yLast=0.0f;
		@Override
		public boolean onTouch(View v, MotionEvent event) {
			if(v==txt_log){
				switch(event.getAction()){
					case MotionEvent.ACTION_DOWN:
						yLast=event.getY();
						break;
						
					case MotionEvent.ACTION_UP:{
							float offset=(event.getY()-yLast);
							if(offset>0) m_bAutoMoveDown=false;
							else{
								int lineHided=txt_log.getScrollY()/txt_log.getLineHeight();
								if((lineHided+LINES_PER_PAGE)>=txt_log.getLineCount()-1) m_bAutoMoveDown=true;
								else m_bAutoMoveDown=false;
							}
						}
						break;
					default:;						
				}
			}
			return false;
		}
	};
	
	public void logi(String _str)
	{
		Log.i("return", _str);
	}
	
	public void setLog(String str)
	{
		txt_log.append(str+"\n");
		
		if(!m_bAutoMoveDown) return;		
		int nTotal=txt_log.getLineCount();
		if(nTotal>LINES_PER_PAGE){
			int y=txt_log.getLayout().getLineBottom(nTotal-LINES_PER_PAGE);
			txt_log.scrollTo(0, y);
		}
		
		Log.i("return",str);
	}
	
	@Override
	protected void onDestroy() {
	    android.os.Process.killProcess(android.os.Process.myPid());
		super.onDestroy();
	}
	@Override
	protected void onStop() {
		super.onStop();
	}

	//xx.ini
    protected String loadIniFile()
    {
		if(strSDPath==null) return null;

    	String strUID=null;
		StringBuffer sb=new StringBuffer(strSDPath);
		sb.append('/');
		sb.append(DEFAULT_FILENAME_CFG_INI);
		
		File tmpFile=null;
		tmpFile=new File(sb.toString());
        if(!tmpFile.exists()) return null;

    	try {
    		FileInputStream fis =new FileInputStream(sb.toString());
    		CharBuffer cb=null;
            cb = CharBuffer.allocate(fis.available());     

    		InputStreamReader inReader = new InputStreamReader(fis, "utf-8");
    		if(cb!=null) inReader.read(cb);
    		String strTmp = new String(cb.array());
    		inReader.close();
    		fis.close();
			
    		int nPos=strTmp.indexOf(strCLCF);			
    		strUID=strTmp.substring(0, nPos);
    		
		}catch(FileNotFoundException e) {
			e.printStackTrace();
			strUID=null;
			
		}catch(NullPointerException npe){
			npe.printStackTrace();
			strUID=null;
			
		}catch(UnsupportedEncodingException uee){
			uee.printStackTrace();
			strUID=null;
			
		}catch(IndexOutOfBoundsException iobe){
			iobe.printStackTrace();
			strUID=null;
			
		}catch(IOException e){
			e.printStackTrace();
			strUID=null;
		}
    	return strUID;
    }
    //xx.ini
    protected boolean saveIniFile(String str)
    {
		if(strSDPath==null || str==null || str.length()<=0) return false;		
		
		StringBuffer sb=new StringBuffer(strSDPath);
		sb.append('/');
		sb.append(DEFAULT_FILENAME_CFG_INI);
		System.out.println("saveIniFile("+sb.toString()+")");
		
		boolean bErr=false;
		FileOutputStream fos=null;
		try{
			fos = new FileOutputStream(sb.toString());
			OutputStreamWriter outWriter = new OutputStreamWriter(fos, "utf-8");
			outWriter.write(str, 0, str.length());
			outWriter.write(strCLCF, 0, strCLCF.length());
			outWriter.flush();
			outWriter.close();
			fos.close();
			
		}catch(FileNotFoundException fe){
			fe.printStackTrace();
			bErr=true;
			
		}catch(NullPointerException npe){
			npe.printStackTrace();
			bErr=true;
			
		}catch(UnsupportedEncodingException uee){
			uee.printStackTrace();
			bErr=true;
			
		}catch(SecurityException se){
			se.printStackTrace();
			bErr=true;
			
		}catch(IOException ioe){
			ioe.printStackTrace();
			bErr=true;
			
		}finally{
			if(bErr){
				if(fos!=null){
					try { fos.close(); }
					catch(IOException e){ e.printStackTrace(); }
				}
				return false;
			}
		}
		return true;    	
    }

	class MainHandler extends Handler{
		static final int MSGTYPE_IOTC_ABORTDISCONN	=1;
		static final int MSGTYPE_LOG				=2;
		
		@Override
		public void handleMessage(Message msg) {		
			
			handleMsg(msg);
			
			super.handleMessage(msg);
		}
		
		void handleMsg(Message msg)
		{
			switch(msg.what){
				case MSGTYPE_IOTC_ABORTDISCONN:
					break;
				case MSGTYPE_LOG:
					setLog((String)msg.obj);
					break;
				default:;
			}
		}
	}
}