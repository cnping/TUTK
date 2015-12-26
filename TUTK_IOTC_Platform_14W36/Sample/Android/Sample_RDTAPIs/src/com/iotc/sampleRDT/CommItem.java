package com.iotc.sampleRDT;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;

public class CommItem {
	public static final String DEFAULT_DIR	  ="/sdcard/IOTCRDT";
	public static final String PREF_FILE_NAME ="RDT";
	public static final String EXT_FILE_NAME  =".jpg";
	
	int mSID	=-1;
	int mRDT_ID	=-1;	
	FileOutputStream mFos=null;
	
	private boolean createDir(String strPath)
    {
    	boolean bResult=true;
    	File dir=new File(strPath);
		if(!dir.exists()){
			try { dir.mkdir(); }
			catch(SecurityException se){
				System.out.println(se.getMessage());
				bResult=false;
			}
		}
		return bResult;
    }
	//xxxhh_mm_ss.ms.jpg
	static public String getFileNameWithTime()
	{
    	Calendar c = Calendar.getInstance();
        int mHour 	= c.get(Calendar.HOUR_OF_DAY);
        int mMinute = c.get(Calendar.MINUTE);
        int mSec	= c.get(Calendar.SECOND);
        int mMSec	= c.get(Calendar.MILLISECOND);
        
        StringBuffer sb=new StringBuffer(DEFAULT_DIR);
        sb.append('/'); sb.append(PREF_FILE_NAME);
        
        if(mHour<10) 		sb.append('0');
        sb.append(mHour);	sb.append('_');
        if(mMinute<10) 		sb.append('0');
        sb.append(mMinute);	sb.append('_');
        if(mSec<10) 		sb.append('0');
        sb.append(mSec);	sb.append('.');
        sb.append(mMSec);
        sb.append(EXT_FILE_NAME);
        
        return sb.toString();
	}
	public boolean openFile(){
		boolean bRet=false;
		if(createDir(DEFAULT_DIR)){
			try {
				mFos = new FileOutputStream(getFileNameWithTime());
				bRet=true;
			}catch (FileNotFoundException e) {
				e.printStackTrace();
				bRet=false;
			}
		}
		return bRet;
	}
	
	public void closeFile(){
		if(mFos!=null){
			try { mFos.close(); } 
			catch (IOException e) {
				e.printStackTrace();
			}
			mFos=null;
		}
	}
	
	public void appendBytToFile(byte[] buffer, int byteCount){
		if(mFos==null) return;
		try {
			mFos.write(buffer, 0, byteCount);
		} catch (IOException e) {
			e.printStackTrace();
		}		
	}
	
}
