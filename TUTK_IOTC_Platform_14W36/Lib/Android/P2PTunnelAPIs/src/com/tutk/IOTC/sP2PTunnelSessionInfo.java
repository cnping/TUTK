package com.tutk.IOTC;

public class sP2PTunnelSessionInfo {
	int  nSID=0;
	byte nMode=0;
	byte nNatType=0;
	byte[] szRemoteIP=null; //char szRemoteIP[16]
	int    nRemotePort=0;
	int    nVersion=0;
	int    nAuthDataLen=0; //How many byte of the authentication data.
	byte[] pAuthData=null; //The authentication data buffer, it will be freed by API automatically.
	
	public int getMode(){
		int nMode=0;
		nMode=(int)(nMode&0xFF);
		return nMode;
	}
	
	public int getNatType(){
		int nNatType=0;
		nNatType=(int)(nNatType&0xFF);
		return nNatType;
	}
	
	public String getRemoteIP()
    {
    	String str="";
		int iLen=0;
		for(iLen=0; iLen<szRemoteIP.length; iLen++){
			if(szRemoteIP[iLen]==(byte)0) break;
		}		
		if(iLen==0) str="";
		else str=new String(szRemoteIP,0,iLen);
		return str;
    }
	
	public int getSID()			{ return nSID;			}
	public int getRemotePort()	{ return nRemotePort;	}
	public int getVersion()		{ return nVersion;		}
	public int getAuthDataLen()	{ return nAuthDataLen;	}
	
}
