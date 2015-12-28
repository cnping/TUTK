
/******************************************************************************
 *                                                                            *
 * Copyright (c) 2011 by TUTK Co.LTD. All Rights Reserved.                    *
 *                                                                            *
 *                                                                            *
 * Class: St_RDT_Status.java                                                  *
 *                                                                            *
 * Author: cloud hsiao                                                        *
 *                                                                            *
 * Date: 2011-10-03                                                           *
 *                                                                            *
 ******************************************************************************/

package com.tutk.IOTC;

//base on struct st_SInfo of IOTCAPIs.h
public class St_RDT_Status {
	public short Timeout;  //Keepalive timeout for how many seconds
	public short TimeoutThreshold;  //when timeout reach this value will break RDT connection
	public long BufSizeInSendQueue; //byte
	public long BufSizeInRecvQueue; //byte
}
