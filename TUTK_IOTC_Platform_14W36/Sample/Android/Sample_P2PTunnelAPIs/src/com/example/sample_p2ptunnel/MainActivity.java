package com.example.sample_p2ptunnel;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.nio.CharBuffer;
import com.tutk.IOTC.P2PTunnelAPIs;
import com.tutk.IOTC.P2PTunnelAPIs.IP2PTunnelCallback;
import com.tutk.IOTC.sP2PTunnelSessionInfo;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.TextView;
import android.text.method.ScrollingMovementMethod;

public class MainActivity extends Activity implements IP2PTunnelCallback{
	public static final String THIS_APP_VER	="1.0.0.1";
	public static final String DEFAULT_FILENAME_CFG_INI	="P2PTunnel.ini";
	public static final String strCLCF="\r\n";
	public static final int	   LINES_PER_PAGE	=18;
	
	private static final int OPT_MENU_ITEM_EXIT = Menu.FIRST;
	
	public static final int SC_STOP		=0;
	public static final int SC_START	=1;
	
	protected String 		strSDPath =null;
	private EditText 		edt_uid;
	private EditText 		edt_bufSize;
	private RadioButton 	radio_device;
	private RadioButton 	radio_client;
	private Button	 btn_start;
	private Button	 btn_stop;
	private TextView txt_api_ver;
	private TextView txt_log;

	private P2PTunnelAPIs m_commApis=null;
	private int m_nInit=-1;
	private int m_nMapIndex=-1;
	private int nStart=-1;
	public  MainHandler mainHandler = new MainHandler();
	private boolean  m_bAsServer=true;
	private boolean  m_bAutoMoveDown=true;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
                
        findView();
		setListenner();
		
		strSDPath = Environment.getExternalStorageDirectory().toString();
		m_commApis=new P2PTunnelAPIs(this);
		initUI();
    }
    private void initUI(){
    	int ver = P2PTunnelAPIs.P2PTunnel_Version();
    	txt_api_ver.setText(verN2Str(ver));
		uiSwitch(SC_STOP);
		String strUID=loadIniFile();
		if(strUID==null || strUID=="")
            strUID = "ERPAA154KFVWTNPGUHCJ";
        else
            edt_uid.setText(strUID);
		strUID=getTitle()+"(V"+THIS_APP_VER+")";
    	setTitle(strUID);
    	
    	startOnGoingNotification("");
    }
    public static String verN2Str(long nVer)
    {
		String strVer=String.format("%d.%d.%d.%d", 
				(nVer>>24)&0xff, (nVer>>16)&0xff, (nVer>>8)&0xff, nVer&0xff);
		return strVer;
    }
    private void findView() {
    	edt_uid		=(EditText)findViewById(R.id.edt_uid);
    	edt_bufSize =(EditText)findViewById(R.id.edt_bufSize);
    	radio_device=(RadioButton)findViewById(R.id.radio_device);
    	radio_client=(RadioButton)findViewById(R.id.radio_client);		
		btn_start	=(Button)findViewById(R.id.btn_start);
		btn_stop	=(Button)findViewById(R.id.btn_stop);
		txt_api_ver =(TextView)findViewById(R.id.txt_api_ver);
		txt_log		=(TextView)findViewById(R.id.txt_log);
		txt_log.setMovementMethod(ScrollingMovementMethod.getInstance());
		txt_log.setFocusable(true);
	}
	private void setListenner() {
		radio_device.setOnClickListener(radioListener);
		radio_client.setOnClickListener(radioListener);
		btn_start.setOnClickListener(btnStartListener);
		btn_stop.setOnClickListener(btnStopListener);
		txt_log.setOnTouchListener(txtLogTouchListener);
	}
	
	private View.OnClickListener radioListener=new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			RadioButton radio=(RadioButton)v;
			if(radio==radio_device){
				m_bAsServer=true;
				btn_start.setText(getResources().getText(R.string.title_btn_start1));
			}else if(radio==radio_client){
				m_bAsServer=false;
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
			
	    	if(m_bAsServer) m_nInit=m_commApis.P2PTunnelServerInitialize(8);
	    	else m_nInit=m_commApis.P2PTunnelAgentInitialize(4);
	    	setLog(" P2PTunnel m_nInit="+m_nInit);
			

			if(m_bAsServer) {
				nStart=m_commApis.P2PTunnelServer_Start(strUID);
				setLog("P2PTunnelServer_Start(.)="+nStart);
			}else {
				//////////////////// Chun //////////////////////////
				String username = "Tutk.com", password = "P2P Platform";
				//P2P Platform
				if(username.length() < 64)
				{
					for(int i = 0 ;username.length()<64;i++)
					{
						username += "\0";
					}
				}
				setLog(username);
				if(password.length() < 64)
				{
					for(int i = 0 ;password.length()<64;i++)
					{
						password += "\0";
						
					}
				}
				setLog(password);
				byte[] baAuthData = (username+password).getBytes();
				int[] pnErrFromDeviceCB = new int[1];
				////////////////////// Chun //////////////////////////
				nStart=m_commApis.P2PTunnelAgent_Connect(strUID,baAuthData,baAuthData.length,pnErrFromDeviceCB);
				setLog("P2PTunnelAgent_Connect(.)="+nStart);
				setLog("P2PTunnelAgent_Connect(.) Error Message = "+pnErrFromDeviceCB[0]);
				
				if(nStart>=0){
					int nLocalPort=8080;
					int size = Integer.parseInt(edt_bufSize.getText().toString());
					int ret = m_commApis.P2PTunnel_SetBufSize(nStart, size);
					setLog("P2PTunnel_SetBufSize SID[" + nStart + "], result=>" + ret);
					
					m_nMapIndex = m_commApis.P2PTunnelAgent_PortMapping(nStart,nLocalPort, 80);
					setLog("P2PTunnelAgent_PortMapping("+nLocalPort+", 80)="+m_nMapIndex);
					setLog("vist: http://127.0.0.1:"+nLocalPort);
				}
			}
			if(nStart>=0) uiSwitch(SC_START);
		}
	};	
	private View.OnClickListener btnStopListener=new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			if(m_bAsServer) m_commApis.P2PTunnelServer_Stop();
			else {
				if(m_nMapIndex>=0) m_commApis.P2PTunnelAgent_StopPortMapping(m_nMapIndex);
				m_nMapIndex=-1;
				int nErrorDisconnect = m_commApis.P2PTunnelAgent_Disconnect(nStart);
				setLog("P2PTunnel Error Message = "+ nErrorDisconnect);

			}
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
	
	protected void setLog(int msgType, int nErrCode,int nSID)
	{
		Message msg=new Message();
		msg.what=msgType;
		msg.arg1=nErrCode;
		msg.arg2=nSID;
		mainHandler.sendMessageDelayed(msg, 0);
	}
	
	public void setLog(String str)
	{
		Message msg=new Message();
		msg.what=MainHandler.MSGTYPE_LOG;
		msg.obj = str;
		mainHandler.sendMessageDelayed(msg, 0);
	}
	
	protected void uiSwitch(int nStatusCode)
	{
		if(nStatusCode==SC_START){		//start
			radio_device.setEnabled(false);
			radio_client.setEnabled(false);
			btn_start.setEnabled(false);
			btn_stop.setEnabled(true);
			
		}else if(nStatusCode==SC_STOP){//stop
			radio_device.setEnabled(true);
			radio_client.setEnabled(true);
			btn_start.setEnabled(true);
			btn_stop.setEnabled(false);
		}
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.clear();
		menu.add(Menu.NONE, OPT_MENU_ITEM_EXIT, 3, getText(R.string.optExit).toString());
		
		return super.onCreateOptionsMenu(menu);
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		int id = item.getItemId();
		switch (id) {
			case OPT_MENU_ITEM_EXIT:
			sysDestroy();
			break;
			
		default:
			break;
		}
		return super.onOptionsItemSelected(item);
	}
	
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if(keyCode == KeyEvent.KEYCODE_BACK) {
			moveTaskToBack(true);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
	
	private void startOnGoingNotification(String Text) {
		NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
		try {
			Intent intent = new Intent(this, MainActivity.class);
			intent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
			PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);

			Notification notification = new Notification(R.drawable.ic_launcher, getText(R.string.app_name).toString(), 0);
			notification.setLatestEventInfo(this, getText(R.string.app_name), Text, pendingIntent);
			notification.flags |= Notification.FLAG_ONGOING_EVENT;
			manager.notify(0, notification);
			
		}catch(Exception e) { e.printStackTrace(); }
	}

	private void stopOnGoingNotification() {
		NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
		manager.cancel(0);
	}
	
	protected void stopQuietly()
	{
    	if(m_commApis!=null) {
    		if(m_nMapIndex>=0) {
    			m_commApis.P2PTunnelAgent_StopPortMapping(m_nMapIndex);
    			m_nMapIndex=-1;
    		}

    		if(m_bAsServer) {
    			m_commApis.P2PTunnelServer_Stop();
    			m_commApis.P2PTunnelServerDeInitialize();
    		}else {
    			m_commApis.P2PTunnelAgent_Disconnect(nStart);
    			m_commApis.P2PTunnelAgentDeInitialize();
    		}
    		m_nInit=-1;
    	}
	}
	protected void sysDestroy()
    {
		System.out.println("---sysDestroy");
		stopQuietly();
		stopOnGoingNotification();
		finish();
    }

	@Override
	protected void onDestroy() {
		//sysDestroy();
		super.onDestroy();
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

	void _printLog(String str) {
		txt_log.append(str+"\n");
		
		if(!m_bAutoMoveDown) return;		
		int nTotal=txt_log.getLineCount();
		if(nTotal>LINES_PER_PAGE){
			int y=txt_log.getLayout().getLineBottom(nTotal-LINES_PER_PAGE);
			txt_log.scrollTo(0, y);
		}
	}
	
	class MainHandler extends Handler {
		static final int MSGTYPE_LOG =1;
		static final int MSGTYPE_P2PTUNNEL_LOG=2;
		
		@Override
		public void handleMessage(Message msg) {		
			handleMsg(msg);
			super.handleMessage(msg);
		}
		
		void handleMsg(Message msg)
		{
			switch(msg.what){
				case MSGTYPE_LOG:
					_printLog((String)msg.obj);
					break;
				
				case MSGTYPE_P2PTUNNEL_LOG:
					{
						_printLog("recv: nErrorCode="+msg.arg1);
						if(msg.arg1==P2PTunnelAPIs.TUNNEL_ER_DISCONNECTED){
							_printLog("recv: TUNNEL_ER_DISCONNECTED");
							if(m_bAsServer){
								m_commApis.P2PTunnelServer_Stop();
							}else {
								if(m_nMapIndex>=0) {
									m_commApis.P2PTunnelAgent_StopPortMapping(m_nMapIndex);
									m_nMapIndex=-1;
								}
								int nRetunDisconnect = m_commApis.P2PTunnelAgent_Disconnect(msg.arg2);
								_printLog("recv: msg.arg2(nSID)"+msg.arg2);
								_printLog("recv: msg.arg2(nRetunDisconnect)"+ nRetunDisconnect);
							}							
						}
					}
					break;
				default:;
			}
		}
	}

	@Override
	public void onTunnelStatusChanged(int nErrCode, int nSID) {
		setLog(MainHandler.MSGTYPE_P2PTUNNEL_LOG, nErrCode, nSID);		
		Log.i("Sample_P2PTunnel", "TunnelStatusCB: SID[ " + nSID + "] ErrorCode[" + nErrCode + "]");
	}
	@Override
	public void onTunnelSessionInfoChanged(sP2PTunnelSessionInfo object) {
		setLog("sessionInfo: SID[" + object.getSID() + "] IP[" + object.getRemoteIP() + "] NAT[" + object.getNatType() + "]");
		Log.i("Sample_P2PTunnel", "TunnelSessionInfoCB: SID[" + object.getSID() + "] IP[" + object.getRemoteIP() + "] NAT[" + object.getNatType() + "]");
		int size = Integer.parseInt(edt_bufSize.getText().toString());
		int ret = m_commApis.P2PTunnel_SetBufSize(object.getSID(), size);
		setLog("P2PTunnel_SetBufSize SID[" + nStart + "], result=>" + ret);
		Log.i("Sample_P2PTunnel","P2PTunnel_SetBufSize SID[" + object.getSID() + "], result=>" + ret);
	}
}
