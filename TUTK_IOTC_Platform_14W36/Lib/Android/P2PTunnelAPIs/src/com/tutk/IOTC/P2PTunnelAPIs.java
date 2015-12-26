package com.tutk.IOTC;

public class P2PTunnelAPIs {

	public IP2PTunnelCallback mSelf = null;
	//======================================================================
	public static final int API_ER_ANDROID_NULL				=-10000;
	
	//P2PTunnelAPIs error code======================================================================
	/** The function is performed successfully. */
	public static final int	TUNNEL_ER_NoERROR				=0;

	/** P2PTunnel module is not initialized yet. Please use P2PTunnelServerInitialize() or
	* P2PTunnelAgentInitialize() for initialization. */
	public static final int	TUNNEL_ER_NOT_INITIALIZED		=-30000;
	
	/** The number of P2PTunnel port mapping service has reached maximum.
	* The maximum number of P2PTunnel port mapping service is determined by
	* #TUNNEL_MAX_PORT_MAPPING_SERVICE_NUMBER */
	public static final int	TUNNEL_ER_EXCEED_MAX_SERVICE	=-30001;
	
	/** Failed to start port mapping when binding because local port had been used by
	* the other service in agent side */
	public static final int	TUNNEL_ER_BIND_LOCAL_SERVICE	=-30002;
	
	/** Failed to start port mapping when listening because local port has been used
	* by the other service in agent side */
	public static final int	TUNNEL_ER_LISTEN_LOCAL_SERVICE	=-30003;
	
	/** P2PTunnel module fails to create threads. Please check if OS has ability
	* to create threads for P2PTunnel module. */
	public static final int	TUNNEL_ER_FAIL_CREATE_THREAD	=-30004;
	
	/** Tunnel agent has already connected to a tunnel server, therefore, it failed
	* connect to another tunnel server */
	public static final int	TUNNEL_ER_ALREADY_CONNECTED		=-30005;
	
	/** The tunnel between tunnel agent and tunnel server has been disconnected.
	* Used in tunnel status callback function for notifying tunnel connection status */
	public static final int	TUNNEL_ER_DISCONNECTED			=-30006;
	
	/** The P2PTunnel module has been initialized in a tunnel server or a tunnel agent */
	public static final int	TUNNEL_ER_ALREADY_INITIALIZED	=-30007;
	
	/** Failed to connect to tunnel server because authentication data is wrong. */
	public static final int	TUNNEL_ER_AUTH_FAILED			=-30008;
	
	/** Authentication data length exceed #MAX_AUTH_DATA_LEN. */
	public static final int	TUNNEL_ER_EXCEED_MAX_LEN		=-30009;
	
	/** The specified P2PTunnel session ID is not valid */
	public static final int	TUNNEL_ER_INVALID_SID			=-30010;
	
	/** This UID is illegal. */
	public static final int	TUNNEL_ER_UID_UNLICENSE			=-30011;
	
	/** The specified device does not support advance function
	*(TCP relay and P2PTunnel module) */
	public static final int	TUNNEL_ER_UID_NO_PERMISSION		=-30012;
	
	/** This UID can't setup connection through relay. */
	public static final int	TUNNEL_ER_UID_NOT_SUPPORT_RELAY	=-30013;
	
	/** Tunnel server not login to IOTC server. */
	public static final int	TUNNEL_ER_DEVICE_NOT_ONLINE		=-30014;
		
	/** Tunner server is not listening for connections. */
	public static final int	TUNNEL_ER_DEVICE_NOT_LISTENING	=-30015;
		
	/** Internet not available or firewall block. */
	public static final int	TUNNEL_ER_NETWORK_UNREACHABLE	=-30016;
		
	/** Tunnel agent failed to connect to tunnel server maybe network unstable. */
	public static final int	TUNNEL_ER_FAILED_SETUP_CONNECTION	=-30017;
		
	/** Tunnel server failed to login to IOTC Server but still can be connected by tunnel agent on LAN. */
	public static final int	TUNNEL_ER_LOGIN_FAILED				=-30018;
		
	/** Notify tunnel server session connection alreay reached maximum through tunnelStatusCB,
	* can't be connected anymore until anyone session release. */
	public static final int	TUNNEL_ER_EXCEED_MAX_SESSION		=-30019;
	
	/** Tunnel agent can't call P2PTunnelServer_GetSessionInfo() */
	public static final int	TUNNEL_ER_AGENT_NOT_SUPPORT			=-30020;
	
	/** The arguments passed to a function is invalid. */
	public static final int	TUNNEL_ER_INVALID_ARG               =-30021;
	
	/** OS resource not enough to malloc memory or open socket. */
	public static final int	TUNNEL_ER_OS_RESOURCE_LACK          =-30022;
	 		
	public native static int P2PTunnel_Version();

	public native int  P2PTunnelServerInitialize(int maxConnectionAllowed); //define Server max connection number
	public native void P2PTunnelServerDeInitialize();
	public native int  P2PTunnelServer_Start(String UID);
	public native void P2PTunnelServer_Stop();
	public native int  P2PTunnelServer_Disconnect(int SID);

	public native int  P2PTunnelAgentInitialize(int maxConnectionAllowed); // only support one sesseion connection
	public native void P2PTunnelAgentDeInitialize();
	public native int  P2PTunnelAgent_Connect(String UID, byte[] authData, int authDataLength, int[] errFromDeviceCB);
	public native int  P2PTunnelAgent_Disconnect(int SID);
	public native int  P2PTunnelAgent_PortMapping(int SID, int localPort, int remotePort);
	public native void P2PTunnelAgent_StopPortMapping(int index);
	
	public native int P2PTunnel_LastIOTime(int SID);
	public native int P2PTunnel_SetBufSize(int SID, int size);

	public P2PTunnelAPIs(IP2PTunnelCallback obj){
		mSelf = obj; 
	}
	
	public void tunnelStatusCB(int nErrorCode, int nSID) { //callback function
		if(mSelf != null) 
			mSelf.onTunnelStatusChanged(nErrorCode, nSID);
	}	
	
	public void tunnelSessionInfoCB(sP2PTunnelSessionInfo sSessionInfo) {
		if (mSelf != null) {
			mSelf.onTunnelSessionInfoChanged(sSessionInfo);
		}
	}

	public interface IP2PTunnelCallback {
		public void onTunnelStatusChanged(int nErrCode, int nSID);
		public void onTunnelSessionInfoChanged(sP2PTunnelSessionInfo object);
	}
	
	static { try {
		System.loadLibrary("P2PTunnelAPIs");
	}catch(UnsatisfiedLinkError ule){
		System.out.println("loadLibrary(P2PTunnelAPIs),"+ule.getMessage());
	}
}
}
