/******************************************************************************
 *                                                                            *
 * Copyright (c) 2013 by TUTK Co.LTD. All Rights Reserved.                    *
 *                                                                            *
 * Author: Chun Chen                                                          *
 * Date: 2013-01-29                                                           *

 ******************************************************************************/

package com.iotc.sampleIOTC;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.nio.CharBuffer;

import com.tutk.IOTC.IOTCAPIs;


import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class Sample_IOTCAPIs extends Activity {
	public static final String THIS_APP_VER	="1.0.0.4";
	public static final String DEFAULT_FILENAME_CFG_INI	="IOTC.ini";
	public static final String strCLCF="\r\n";
	public static final int	   LINES_PER_PAGE	=18;
	
	protected String 		strSDPath =null;
	private EditText 		edt_uid;
	private Button	 btn_start;
	private Button	 btn_stop;
	private TextView txt_api_ver;
	private TextView txt_log;
	public static Context content;
	
	protected CommApis m_commApis;
	public MainHandler mainHandler = new MainHandler();
	private boolean  m_bAsDevice=true;
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
		m_commApis=new CommApis(this);
		initUI();
    }
    private void initUI(){
    	long[] ver = new long[1];
    	IOTCAPIs.IOTC_Get_Version(ver);
    	txt_api_ver.setText(verN2Str(ver[0]));
		String strUID=loadIniFile();
		if(strUID!=null)edt_uid.setText(strUID);
		strUID=getTitle()+"(V"+THIS_APP_VER+")";
    	setTitle(strUID);
    	
    	setLog("device");
    }
    public static String verN2Str(long nVer)
    {
		String strVer=String.format("%d.%d.%d.%d", 
				(nVer>>24)&0xff, (nVer>>16)&0xff, (nVer>>8)&0xff, nVer&0xff);
		return strVer;
    }
    private void findView() {
    	edt_uid		=(EditText)findViewById(R.id.edt_uid);	
		btn_start	=(Button)findViewById(R.id.btn_start);
		btn_stop	=(Button)findViewById(R.id.btn_stop);
		txt_api_ver =(TextView)findViewById(R.id.txt_api_ver);
		txt_log		=(TextView)findViewById(R.id.txt_log);
		txt_log.setMovementMethod(ScrollingMovementMethod.getInstance());
		txt_log.setFocusable(true);
	}
	private void setListenner() {
		btn_start.setOnClickListener(btnStartListener);
		btn_stop.setOnClickListener(btnStopListener);
		txt_log.setOnTouchListener(txtLogTouchListener);
	}	

	private View.OnClickListener btnStartListener=new View.OnClickListener() 
	{		
		@Override
		public void onClick(View v) {
			Toast.makeText(v.getContext(), "start", Toast.LENGTH_LONG).show();
			String strUID;
			strUID=edt_uid.getText().toString();
			if(strUID.length()<20){
				setLog("Length of UID <20");
				return;				
			}
			saveIniFile(strUID);
			txt_log.setText("");
			
			if(m_commApis.initIOTC()>=0) {
				m_commApis.startSess(strUID, true);
			}
		}
	};	
	private View.OnClickListener btnStopListener=new View.OnClickListener() 
	{
		@Override
		public void onClick(View v) {
			m_commApis.stopSess();
			m_commApis.unInitIOTC();		
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
	
	public void setLog(String str)
	{
		txt_log.append(str+"\n");
		
		if(!m_bAutoMoveDown) return;		
		int nTotal=txt_log.getLineCount();
		if(nTotal>LINES_PER_PAGE){
			int y=txt_log.getLayout().getLineBottom(nTotal-LINES_PER_PAGE);
			txt_log.scrollTo(0, y);
		}
	}
	
	protected void stopQuietly()
	{
    	if(m_commApis!=null) {
    		m_commApis.stopSess();
    		m_commApis.unInitIOTC();
    	}
	}
	protected void sysDestroy()
    {
		stopQuietly();
    }

	@Override
	protected void onDestroy() {
	    android.os.Process.killProcess(android.os.Process.myPid());
		sysDestroy();
		super.onDestroy();
	}
	@Override
	protected void onStop() {
		stopQuietly();
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
					setLog("Disconnected("+IOTCAPIs.IOTC_ER_REMOTE_TIMEOUT_DISCONNECT+")");
					break;
					
				case MSGTYPE_LOG:
					setLog((String)msg.obj);
					break;
					
				default:;
			}
		}
	}
}