/*
 ******************************************************************************
 *     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name: TELNET
 * Purpose: The purpose of this package provides the services to TELNET
 * Author:
 * Date:
 * Notes:
 * $Log$
*=============================================================================
*/
/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "mstimer.h"
#include "tcpip.h"
#include "console.h"
#include "telnet.h"
#include "printd.h"
#include "gconfig.h"

#if (INCLUDE_TELNET_SERVER)
/* NAMING CONSTANT DECLARATIONS */
#define UF_ECHO 0x01
#define UF_SGA  0x02
#define CHM_TRY  0
#define CHM_ON   1
#define CHM_OFF  2
#define TELNET_SERVER_PORT 23
#define TELNET_RX_BUF_SZIE DATABUFSIZE
#define TELNET_TX_BUF_SZIE (DATABUFSIZE*26)

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static TELNET_SERVER_CONN TELNET_Connect;
static U8_T far telnet_RxBuf[TELNET_RX_BUF_SZIE];
static U8_T far telnet_TxBuf[TELNET_TX_BUF_SZIE];
static U16_T telnet_RxHeadPtr, telnet_RxTailPtr;
static U16_T telnet_TxHeadPtr, telnet_TxTailPtr;
static U8_T telnet_InterfaceId, telnet_InitConsole=0;
static U8_T telnet_TxReady=1; /* ready to transmit data to tcp layer */
static U8_T far txPktBuf[8*DATABUFSIZE];

/* LOCAL SUBPROGRAM DECLARATIONS */
static void telnet_PutRxChar(U8_T c);
static U8_T telnet_GetTxChar(void);
static void telnet_SendData(TELNET_SERVER_CONN *pTnConn);
static int telnet_Subneg(TELNET_SERVER_CONN *pTnConn, U8_T c);
static void telnet_Telopt(TELNET_SERVER_CONN *pTnConn, U8_T c);
static void telnet_ToEcho(TELNET_SERVER_CONN *pTnConn);
static void telnet_ToSGA(TELNET_SERVER_CONN *pTnConn);
static void telnet_ToNotSup(TELNET_SERVER_CONN *pTnConn, U8_T c);
static void telnet_IACFlush(TELNET_SERVER_CONN *pTnConn);
static void telnet_PutIAC(TELNET_SERVER_CONN *pTnConn, U8_T c);
static void telnet_PutIAC2(TELNET_SERVER_CONN *pTnConn, U8_T wwdd, U8_T c);
static void telnet_InitInstance(void);
static void telnet_RcvdData(TELNET_SERVER_CONN *pTnConn, U8_T *pData, U16_T len);
static void telnet_SendInitOptions(TELNET_SERVER_CONN *pTnConn);
static void telnet_TcpSend(U8_T tcpSocket, U8_T *pBuf, U16_T len);

/* LOCAL SUBPROGRAM BODIES */
/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_PutRxChar()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_PutRxChar(U8_T c)
{
    U16_T tmp16;

    tmp16 = telnet_RxTailPtr+1;
    if (tmp16 >= TELNET_RX_BUF_SZIE)
        tmp16 = 0;

    /* is buffer full ? */
    if (telnet_RxHeadPtr == tmp16)
        return;

    telnet_RxBuf[telnet_RxTailPtr] = c;
    telnet_RxTailPtr = tmp16;

} /* End of telnet_PutRxChar() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: TELNET_GetChar()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S8_T TELNET_GetChar(void)
{
    S8_T c;

    if ((TELNET_Connect.State != TELNET_STATE_CONNECTED) ||
        (telnet_RxTailPtr == telnet_RxHeadPtr))
        return 0;

    c = telnet_RxBuf[telnet_RxHeadPtr];
    telnet_RxHeadPtr++;
    if (telnet_RxHeadPtr >= TELNET_RX_BUF_SZIE)
        telnet_RxHeadPtr = 0;

    return c;
} /* End of TELNET_GetChar() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: TELNET_PutChar()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S8_T TELNET_PutChar(S8_T c)
{
    U16_T tmp16;

    tmp16 = telnet_TxTailPtr+1;
    if (tmp16 >= TELNET_TX_BUF_SZIE)
        tmp16 = 0;
    
    /* is buffer full ? */
    if (telnet_TxHeadPtr == tmp16)
    {
        if (telnet_TxReady && TELNET_Connect.State == TELNET_STATE_CONNECTED)
            telnet_SendData(&TELNET_Connect);
        else
        return -1;
    }

    telnet_TxBuf[telnet_TxTailPtr] = c;
    telnet_TxTailPtr = tmp16;

    return 1;
} /* End of TELNET_PutChar() */ 

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_SendInitOptions()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_SendInitOptions(TELNET_SERVER_CONN *pTnConn)
{
    /* char mode */
    telnet_PutIAC2(pTnConn, WILL, TELOPT_ECHO);
    telnet_PutIAC2(pTnConn, WILL, TELOPT_SGA);
    pTnConn->TnServer.TelFlags |= (UF_ECHO | UF_SGA);
    telnet_IACFlush(pTnConn);
          
} /* End of telnet_SendInitOptions() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_GetTxChar()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static U8_T telnet_GetTxChar(void)
{
    S8_T c;

    if (telnet_TxTailPtr == telnet_TxHeadPtr)
        return 0;
   
    c = telnet_TxBuf[telnet_TxHeadPtr];
    telnet_TxHeadPtr++;
    if (telnet_TxHeadPtr >= TELNET_TX_BUF_SZIE)
        telnet_TxHeadPtr = 0;

    return c;
} /* End of telnet_GetTxChar() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_SendData()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_SendData(TELNET_SERVER_CONN *pTnConn)
{
    U8_T c;
    U16_T len=0;

    /* send data */
    for (len= 0; len < (8*DATABUFSIZE);)
    {
        if ((c = telnet_GetTxChar()) == 0)
            break;

        txPktBuf[len++] = c;
        if (c == 0xFF)
            txPktBuf[len++] = 0xFF;
        else if (c == 0x0D)
            txPktBuf[len++] = 0x00;
    }

    if (len)
        telnet_TcpSend(pTnConn->TcpSocket, txPktBuf, len);

} /* End of telnet_SendData() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_SendData()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_TcpSend(U8_T tcpSocket, U8_T *pBuf, U16_T len)
{
    telnet_TxReady = 0;
    TCPIP_TcpSend(tcpSocket, pBuf, len, TCPIP_SEND_NOT_FINAL);

} /* End of telnet_TcpSend() */
 

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_IACFlush()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_IACFlush(TELNET_SERVER_CONN *pTnConn)
{
    if (pTnConn->TnServer.IACLen > 0)
    {
        if (telnet_TxReady)
            telnet_TcpSend(pTnConn->TcpSocket, pTnConn->TnServer.IACBuf, pTnConn->TnServer.IACLen);
        else
            return;
        pTnConn->TnServer.IACLen = 0;
    }

} /* End of telnet_IACFlush() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_PutIAC()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_PutIAC(TELNET_SERVER_CONN *pTnConn, U8_T c)
{
    pTnConn->TnServer.IACBuf[pTnConn->TnServer.IACLen++] = c;

} /* End of telnet_PutIAC() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_PutIAC2()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_PutIAC2(TELNET_SERVER_CONN *pTnConn, U8_T wwdd, U8_T c)
{
    if (pTnConn->TnServer.IACLen + 3 > IACBUFSIZE)
        telnet_IACFlush(pTnConn);

    telnet_PutIAC(pTnConn, IAC);
    telnet_PutIAC(pTnConn, wwdd);
    telnet_PutIAC(pTnConn, c);

} /* End of telnet_PutIAC2() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_ToNotSup()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_ToNotSup(TELNET_SERVER_CONN *pTnConn, U8_T c)
{
    if (pTnConn->TnServer.TelWish == WILL)
        telnet_PutIAC2(pTnConn, DONT, c);
    else if (pTnConn->TnServer.TelWish == DO)
        telnet_PutIAC2(pTnConn, WONT, c);

} /* End of telnet_ToNotSup */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_ToEcho()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_ToEcho(TELNET_SERVER_CONN *pTnConn)
{
    if (pTnConn->TnServer.TelFlags & UF_ECHO)
    {
        pTnConn->TnServer.TelFlags ^= UF_ECHO;
        return;
    }

    if (pTnConn->TnServer.TelWish == DO)
    { 
    	telnet_PutIAC2(pTnConn, WILL, TELOPT_ECHO);
    }
    else if (pTnConn->TnServer.TelWish == WILL)
    {
        telnet_PutIAC2(pTnConn, WONT, TELOPT_ECHO);
    }

} /* End of telnet_ToEcho() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_ToSGA()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_ToSGA(TELNET_SERVER_CONN *pTnConn)
{
    if (pTnConn->TnServer.TelFlags & UF_SGA)
    {
        pTnConn->TnServer.TelFlags ^= UF_SGA;
        return;
    }

    if (pTnConn->TnServer.TelWish == DO)
    { 
    	telnet_PutIAC2(pTnConn, WILL, TELOPT_SGA);
    }
    else if (pTnConn->TnServer.TelWish == WILL)
    {
        telnet_PutIAC2(pTnConn, DO, TELOPT_SGA);
    }

} /* End of telnet_ToSGA() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_Telopt()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_Telopt(TELNET_SERVER_CONN *pTnConn, U8_T c)
{
    switch (c)
    {
        case TELOPT_ECHO:
            telnet_ToEcho(pTnConn);
            break;

        case TELOPT_SGA:
            telnet_ToSGA(pTnConn);
            break;

        default:
            telnet_ToNotSup(pTnConn, c);
            break;
    }

} /* End of telnet_Telopt() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_Subneg()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static int telnet_Subneg(TELNET_SERVER_CONN *pTnConn, U8_T c)
{
    switch (pTnConn->TnServer.TelState)
    {
        case TS_SUB1:
            if (c == IAC)
                pTnConn->TnServer.TelState = TS_SUB2;
            break;
        case TS_SUB2:
            if (c == SE)
                return 1;
            pTnConn->TnServer.TelState = TS_SUB1;
            break;
    }
    return 0;
} /* End of telnet_Subneg() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_RcvdData()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_RcvdData(TELNET_SERVER_CONN *pTnConn, U8_T *pData, U16_T len)
{
    U16_T i;

    for (i = 0; i < len; i++)
    {
        U8_T c = pData[i];
        switch (pTnConn->TnServer.TelState)
        {
            case TS_CR:
                pTnConn->TnServer.TelState = TS_DATA;
                if ((c == 0) || (c == '\n'))
                    break;

            case TS_DATA:
                if (c == IAC)
                    pTnConn->TnServer.TelState = TS_IAC;
                else
                {
                    if (c == '\r')
                        pTnConn->TnServer.TelState = TS_CR;
                    telnet_PutRxChar(c);
                }
                break;

            case TS_IAC:
                if (c == IAC) /* IAC IAC -> 0xFF */
                {
                    telnet_PutRxChar(c);
                    pTnConn->TnServer.TelState = TS_DATA;
                    break;
                }
                /* else */
                switch (c)
                {
                    case SB:
                        pTnConn->TnServer.TelState = TS_SUB1;
                        break;

                    case DO:
                    case DONT:
                    case WILL:
                    case WONT:
                        pTnConn->TnServer.TelWish =  c;
                        pTnConn->TnServer.TelState = TS_OPT;
                        break;
                    default:
                        pTnConn->TnServer.TelState = TS_DATA;     /* DATA MARK must be added later */
                        break;
                }
                break;
            case TS_OPT: /* WILL, WONT, DO, DONT */
                telnet_Telopt(pTnConn, c);
                pTnConn->TnServer.TelState = TS_DATA;
                break;
            case TS_SUB1: /* Subnegotiation */
            case TS_SUB2: /* Subnegotiation */
                if (telnet_Subneg(pTnConn, c))
                    pTnConn->TnServer.TelState = TS_DATA;
                break;
        }
    }

    if (pTnConn->TnServer.IACLen)
        telnet_IACFlush(pTnConn);

} /* End of telnet_RcvdData() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: TELNET_NotifyClose()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TELNET_NotifyClose(void)
{
    TELNET_Connect.State = TELNET_STATE_CLOSING;
    
} /* End of TELNET_NotifyClose() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: TELNET_Event()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TELNET_Event(U8_T id, U8_T event)
{
    TELNET_SERVER_CONN *pTnConn = &TELNET_Connect;

    if (event == TCPIP_CONNECT_CANCEL)
    {
        CONSOLE_Inst *pInst;

        pTnConn->State = TELNET_STATE_FREE;
        /* notify the console that telnet close connectiion */
        pInst = CONSOLE_GetInstance(LL_TELNET);
        pInst->State = CLI_STATE_BLOCK;
    }
    else if (event == TCPIP_CONNECT_ACTIVE)
    {
        telnet_InitConsole = 1;
        telnet_RxHeadPtr = 0;
        telnet_RxTailPtr = 0;
        telnet_TxHeadPtr = 0;
        telnet_TxTailPtr = 0;;
        
        pTnConn->State = TELNET_STATE_CONNECTING;
        pTnConn->TnServer.TelState = TS_DATA;
        pTnConn->TnServer.TelFlags = 0;
        pTnConn->TnServer.IACLen = 0;
        telnet_SendInitOptions(pTnConn);
    }
	else if (event == TCPIP_CONNECT_XMIT_COMPLETE)
	{
        telnet_TxReady = 1;
        if (pTnConn->TnServer.IACLen)
            telnet_IACFlush(pTnConn);
        else if (pTnConn->State == TELNET_STATE_CONNECTED)
            telnet_SendData(pTnConn);
    }

} /* End of TELNET_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: TELNET_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T TELNET_NewConn(U32_T XDATA *pip, U16_T remotePort, U8_T socket)
{
    U8_T i, found;
    U16_T temp16;
    U32_T addr;

    if (TELNET_Connect.State == TELNET_STATE_FREE)
	{
        found = 0;
        temp16 = GCONFIG_GetNetwork();
        if (temp16 & GCONFIG_NETWORK_ACC_IP_ENABLE)
        {
            for (i=0; i < GCONFIG_ACCESSIBLE_IP_NUM; i++)
            {
                GCONFIG_GetAccessibleIP(i, &addr);
                if (addr == *pip)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
                return TCPIP_NO_NEW_CONN;
        }

        TELNET_Connect.State = TELNET_STATE_ACTIVE;
        TELNET_Connect.Ip = *pip;
        TELNET_Connect.Port = remotePort;
        TELNET_Connect.TcpSocket = socket;

        return 0;
    }

    return TCPIP_NO_NEW_CONN;
} /* End of TELNET_NewConn() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: telnet_InitInstance
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void telnet_InitInstance(void)
{
    CONSOLE_Inst *pInst;

    pInst = CONSOLE_GetInstance(LL_TELNET);
    pInst->State = CLI_STATE_LOGIN;
    pInst->PromptEnable = 1;
    pInst->Privilege = 5;
    pInst->BufIndex = 0;
    memset(pInst->UserName, 0, CLI_MAX_USERNAME_LEN);
    memset(pInst->Passwd, 0, CLI_MAX_PASSWD_LEN);
    memset(pInst->CmdBuf, 0, CLI_MAX_COMMAND_LEN);

} /* End of telnet_InitInstance() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: TELNET_Receive()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TELNET_Receive(U8_T XDATA *pData, U16_T length, U8_T conn_id)
{
    TELNET_SERVER_CONN *pTnConn = &TELNET_Connect;

    if (pTnConn->State == TELNET_STATE_CONNECTING ||
        pTnConn->State == TELNET_STATE_CONNECTED)
    {
        telnet_RcvdData(pTnConn, pData, length); 
    }

} /* End of TELNET_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: TELNET_Init()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TELNET_Init(void)
{
    TELNET_Connect.State = TELNET_STATE_FREE;
    telnet_RxHeadPtr = 0;
    telnet_RxTailPtr = 0;
    telnet_TxHeadPtr = 0;
    telnet_TxTailPtr = 0;;

	telnet_InterfaceId = TCPIP_Bind(TELNET_NewConn, TELNET_Event, TELNET_Receive);
	TCPIP_TcpListen(TELNET_SERVER_PORT ,telnet_InterfaceId);

} /* End of TELNET_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: TELNET_Task
 * Purpose: Main function
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TELNET_Task(void)
{
    TELNET_SERVER_CONN *pTnConn = &TELNET_Connect;

    switch (pTnConn->State)
    {
        case TELNET_STATE_CONNECTING:
            if (pTnConn->TnServer.TelFlags == 0)
                pTnConn->State = TELNET_STATE_CONNECTED;
            break;

        case TELNET_STATE_CONNECTED:
            if (telnet_InitConsole)
            {
                /* Initailize the console state to login */
                telnet_InitInstance();
                telnet_InitConsole = 0;
            }

            if (telnet_TxReady)
                telnet_SendData(pTnConn);

            break;

        case TELNET_STATE_CLOSING:
            if (telnet_TxReady)
            {
                /* no more data to be transmitted */
                if (telnet_TxTailPtr == telnet_TxHeadPtr)
                {
                    TCPIP_TcpClose(TELNET_Connect.TcpSocket);
                    pTnConn->State = TELNET_STATE_FREE;
                }
                else
                    telnet_SendData(pTnConn);
            }

            break;

        default:
            break;
    }

} /* End of TELNET_Task() */
#endif /* #if (INCLUDE_TELNET_SERVER) */

/* End of telnet.c */