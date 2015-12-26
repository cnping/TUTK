/******************************************************************************
 *                                                                            *
 * Copyright (c) 2013 by TUTK Co.LTD. All Rights Reserved.                    *
 *                                                                            *
 * Author: Chun Chen                                                          *
 * Date: 2013-02-7                                                           *

 ******************************************************************************/

package com.iotc.sample_AVServer;


import java.io.UnsupportedEncodingException;
import android.os.Message;
import android.util.Log;

import com.iotc.sample_AVServer.Sample.MainHandler;
//import com.tutk.IOTC.iotc_err;
//import com.tutk.IOTC.iotc2;
import com.tutk.IOTC.IOTCAPIs;
import com.tutk.IOTC.AVAPIs;
//import com.tutk.IOTC.st_LanSearchInfo;
//import com.tutk.IOTC.st_LanSearchInfo2;
//import com.tutk.IOTC.st_MasterName;
//import com.tutk.IOTC.st_SessionStatus;


public class CommApis extends IOTCAPIs
{
	private int rc=0;
	private String str="";
	
	protected Sample actMainView = null;
	
	public CommApis(Sample mainView)
	{
		actMainView = mainView;
	}
	
	public static String verN2Str(long nVer)
    {
		String strVer=String.format("%d.%d.%d.%d", 
				(nVer>>24)&0xff, (nVer>>16)&0xff, (nVer>>8)&0xff, nVer&0xff);
		return strVer;
    }
    
    public void SleepMS (int nMsec)
	{
		try {Thread.sleep (nMsec);} catch (InterruptedException ex) {Thread.currentThread().interrupt();} 
	}
	
	public static String ByteToString (byte[] buf) 
	{
	    String strRecvMsg = "";
	    try {strRecvMsg = new String (buf,0,buf.length, "utf-8");} catch (UnsupportedEncodingException e) {e.printStackTrace();}
	    return strRecvMsg;
	}
	
	public static String ByteToString (byte[] buf, int len)
	{
	    String strRecvMsg = "";
	    try {strRecvMsg = new String (buf,0,len, "utf-8");} catch (UnsupportedEncodingException e) {e.printStackTrace();}
	    return strRecvMsg;
	}
	
	public void logi(String _str)
	{
		Log.i("return", _str);
	}

}
