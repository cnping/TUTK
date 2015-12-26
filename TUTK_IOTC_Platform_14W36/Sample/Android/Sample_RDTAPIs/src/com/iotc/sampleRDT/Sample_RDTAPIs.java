/******************************************************************************
 *                                                                            *
 * Copyright (c) 2011 by TUTK Co.LTD. All Rights Reserved.                    *
 *                                                                            *
 * Author: joshua ju                                                          *
 * Date: 2011-05-14                                                           *

 ******************************************************************************/

package com.iotc.sampleRDT;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.nio.CharBuffer;

import com.iotc.sampleRDT.R;
import com.tutk.IOTC.IOTCAPIs;
import com.tutk.IOTC.RDTAPIs;
import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.TextView;

public class Sample_RDTAPIs extends Activity {
	public static final String THIS_APP_VER	="1.0.0.5";
	public static final String DEFAULT_FILENAME_CFG_INI	="IOTC.ini";
	public static final String strCLCF="\r\n";
	public static final int	   LINES_PER_PAGE	=16;
	
	public static final int SC_STOP		=0;
	public static final int SC_START	=1;
	
	protected String 		strSDPath =null;
	private EditText 		edt_uid;
	private RadioButton 	radio_device;
	private RadioButton 	radio_client;
	private Button	 btn_start;
	private Button	 btn_stop;
	private TextView txt_iotc_ver, txt_iotcrdt_ver;
	private TextView txt_log;
	private EditText edt_filename;
	private Button	 btn_send;
	
	protected CommApis m_commApis;
	public MainHandler mainHandler = new MainHandler();
	private boolean  m_bAsDevice=true;
	private boolean  m_bAutoMoveDown=true;
	
	private FileInputStream m_fis=null;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
                
        findView();
		setListenner();
		
		strSDPath = Environment.getExternalStorageDirectory().toString();
		m_commApis=new CommApis(this);
		initUI();
    }
    private void initUI(){
    	long[] ver = new long[1];
    	IOTCAPIs.IOTC_Get_Version(ver);
    	txt_iotc_ver.setText(verN2Str(ver[0]));
		txt_iotcrdt_ver.setText(verN2Str(RDTAPIs.RDT_GetRDTApiVer()));		
		uiSwitch(SC_STOP);		
		String strUID=loadIniFile();
		if(strUID!=null)edt_uid.setText(strUID);
		
		strUID=getTitle()+"(V"+THIS_APP_VER+")";
    	setTitle(strUID);
    }
    
    public static String verN2Str(long nVer)
    {
		String strVer=String.format("%d.%d.%d.%d", 
				(nVer>>24)&0xff, (nVer>>16)&0xff, (nVer>>8)&0xff, nVer&0xff);
		return strVer;
    }
    private void findView() {
    	edt_uid		=(EditText)findViewById(R.id.edt_uid);
    	radio_device=(RadioButton)findViewById(R.id.radio_device);
    	radio_client=(RadioButton)findViewById(R.id.radio_client);		
		btn_start	=(Button)findViewById(R.id.btn_start);
		btn_stop	=(Button)findViewById(R.id.btn_stop);
		txt_iotc_ver=(TextView)findViewById(R.id.txt_iotc_ver);
		txt_iotcrdt_ver =(TextView)findViewById(R.id.txt_iotcav_ver);
		txt_log		=(TextView)findViewById(R.id.txt_log);
		txt_log.setMovementMethod(ScrollingMovementMethod.getInstance());
		txt_log.setFocusable(true);
		edt_filename=(EditText)findViewById(R.id.edt_filename);
		btn_send	=(Button)findViewById(R.id.btn_send);
	}
	private void setListenner() {
		radio_device.setOnClickListener(radioListener);
		radio_client.setOnClickListener(radioListener);
		btn_start.setOnClickListener(btnStartListener);
		btn_stop.setOnClickListener(btnStopListener);
		txt_log.setOnTouchListener(txtLogTouchListener);
		btn_send.setOnClickListener(btnSedListener);
	}
	
	private View.OnClickListener radioListener=new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			RadioButton radio=(RadioButton)v;
			if(radio==radio_device){
				m_bAsDevice=true;
				btn_start.setText(getResources().getText(R.string.title_btn_start1));
			}else if(radio==radio_client){
				m_bAsDevice=false;
				btn_start.setText(getResources().getText(R.string.title_btn_start2));
			}
		}
	}; 
	private View.OnClickListener btnStartListener=new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			String strUID;
			strUID=edt_uid.getText().toString();
			if(strUID.length()<20){
				setLog("Length of UID <20");
				return;				
			}
			saveIniFile(strUID);
			txt_log.setText("");
			if(m_commApis.initIOTCRDT()>=0) {
				m_commApis.startSess(strUID, m_bAsDevice);
				uiSwitch(SC_START);
			}
		}
	};	
	private View.OnClickListener btnStopListener=new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			m_commApis.stopSess();
			if(m_bAsDevice) m_commApis.unInitIOTCRDT();
			uiSwitch(SC_STOP);			
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
	private View.OnClickListener btnSedListener=new View.OnClickListener(){
		@Override
		public void onClick(View v) {
			if(!m_commApis.isFinish_sendFile()) return;
			if(!m_commApis.hasClientConnect()){
				setLog("No client connected");
				return;
			}
			String strFile=edt_filename.getText().toString();
			if(strFile.length()==0) setLog("No file sended.");
			else{
				File tmpFile=null;
				tmpFile=new File(strFile);
		        if(!tmpFile.exists()) setLog("This file don't exit.");
		        else m_commApis.sendFile(strFile);
			}
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
	
	protected void uiSwitch(int nStatusCode)
	{
		if(nStatusCode==SC_START){		//start
			radio_device.setEnabled(false);
			radio_client.setEnabled(false);
			btn_start.setEnabled(false);
			btn_stop.setEnabled(true);
			btn_send.setEnabled(true);
			
		}else if(nStatusCode==SC_STOP){//stop
			radio_device.setEnabled(true);
			radio_client.setEnabled(true);
			btn_start.setEnabled(true);
			btn_stop.setEnabled(false);
			btn_send.setEnabled(false);
		}
	}
	
	protected void stopQuietly()
	{
    	if(m_commApis!=null) {
    		m_commApis.stopSess();
    		m_commApis.unInitIOTCRDT();
    	}
	}
	protected void sysDestroy()
    {
		stopQuietly();
    }

	@Override
	protected void onDestroy() {
		sysDestroy();
		super.onDestroy();
	}
	@Override
	protected void onStop() {
		stopQuietly();
		super.onStop();
	}
	
	public boolean openFile(String strFullFilename)
	{
		boolean bRet=false;
		try {
			m_fis=new FileInputStream(strFullFilename);
			if(m_fis!=null) bRet=true;
		}catch(FileNotFoundException e) {
			e.printStackTrace();			
		}
		return bRet;
	}
	public void closeFile()
	{
		if(m_fis!=null){
			try { m_fis.close(); } 
			catch (IOException e) {
				e.printStackTrace();
			}
			m_fis=null;
		}		
	}
	
	   //xx.ini
    protected String loadIniFile()
    {
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
		if(str==null || str.length()<=0) return false;		
		
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