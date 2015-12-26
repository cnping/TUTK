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
 * Module Name: CONSOLE
 * Purpose: The purpose of this package provides the services to CONSOLE
 * Author:
 * Date:
 * Notes:
 * $Log$
*=============================================================================
*/

/* INCLUDE FILE DECLARATIONS */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "gconfig.h"
#include "ax22000.h"
#include "mcpu.h"
#include "console.h"
#include "uart0.h"
#include "clicmd.h"
#if (INCLUDE_TELNET_SERVER)
#include "telnet.h"
#endif
#if (INCLUDE_EVENT_DETECT)
#include "gevent.h"
#endif
/* NAMING CONSTANT DECLARATIONS */
#if (INCLUDE_TELNET_SERVER)
#define MAX_INST_NUM 2
#else
#define MAX_INST_NUM 1
#endif
#define USERNAME_STR "username: "
#define PASSWD_STR   "password: "
#define CONSOLE_QUIT "quit"
#define CONSOLE_HELP "help"
#define CONSOLE_PASSWD "passwd"
#define CONSOLE_REBOOT "reboot"
#define CONSOLE_PROMPT "uart> "

#define CTRL_C  0x03
#define BACKSP  0x08
#define DELCHAR 0x7F
#define SPACE   0x20
#define BELL    0x07
#define CLI_CR  0x0d
#define CLI_LF  0x0a
#define CLI_ECHO_ON "\010 \010"
#define CLI_MAX_ARGS 16
#define CLI_CLOSED -1
#define CLI_COMMAND_DONE 10
/* MACRO DECLARATIONS */
#define IsSpace(x) ((x == SPACE) ? 1 : 0)

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static CONSOLE_Inst far console_Instance[MAX_INST_NUM];
static S8_T far pBuf[CLI_MAX_MESSAGE_LEN];
static U16_T far console_CmdTableSize=0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static S16_T console_PutString(CONSOLE_Inst *pInst, S8_T *strData, S16_T len);
static void console_ClearLine(CONSOLE_Inst *pInst);
static void console_HelpMessage(CONSOLE_Inst *pInst, U8_T privilege);
static void console_ParseLine(char *pLine, int numArgvs, int *pArgc, char *argv[]);
static int console_CollectCommand(CONSOLE_Inst *pInst, S8_T buf[], S16_T size, S16_T echo);
static void console_InitInstance(CONSOLE_Inst *pInst);
static int console_Authenticate(CONSOLE_Inst *pInst);
static int console_CommandProcess(CONSOLE_Inst *pInst, S8_T *pLine, U8_T privilege);
static void console_CmdExecute(CONSOLE_Inst *pInst, S16_T argc, S8_T *argv[], U8_T privilege);
static void console_LoadUserAccount(void);
static S16_T console_ChangePasswd(CONSOLE_Inst *pInst);

/* LOCAL SUBPROGRAM BODIES */
/*
 * ----------------------------------------------------------------------------
 * Function Name: CONSOLE_ChangeUsername()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S16_T CONSOLE_ChangeUsername(CONSOLE_Inst *pInst, U8_T *username)
{
    CONSOLE_Account *account;
    U8_T ulen, plen, i;

    ulen = strlen(pInst->UserName);
    plen = strlen(username);
    
    for (i=0, account=&CLICMD_userTable[0]; i < MAX_USER_ACCOUNT; i++, account++)
    {
        if ((ulen == strlen(account->Name)) && (memcmp(account->Name, pInst->UserName, ulen)==0))
        {
            strcpy(account->Name, username);
            strcpy(pInst->UserName, username);
            /* Update the configuration */
            GCONFIG_SetAdmin(account->Name, account->Passwd);

            return 1;
        }
    }

    return -1;
} /* End of CONSOLE_ChangeUsername() */


/*
 * ----------------------------------------------------------------------------
 * Function Name: console_ChangePasswd()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T console_ChangePasswd(CONSOLE_Inst *pInst)
{
    CONSOLE_Account *account;
    U8_T ulen, plen, i;

    ulen = strlen(pInst->UserName);
    plen = strlen(pInst->Passwd);
    
    for (i=0, account=&CLICMD_userTable[0]; i < MAX_USER_ACCOUNT; i++, account++)
    {
        if ((ulen == strlen(account->Name)) &&
            (memcmp(account->Name, pInst->UserName, ulen)==0) &&
            (plen == strlen(account->Passwd)) &&
            (memcmp(account->Passwd, pInst->Passwd, plen)==0))
        {
            ulen = strlen(pInst->PasswdNew);
            plen = strlen(pInst->CmdBuf);
       	    if ((ulen == plen) && (memcmp(pInst->PasswdNew, pInst->CmdBuf, ulen)==0))
            {
                strcpy(account->Passwd, pInst->PasswdNew);
                /* Update the configuration */
                GCONFIG_SetAdmin(account->Name, account->Passwd);

                return 1;
            }
            else
                return -1;
        }
    }

    return -1;
} /* End of console_ChangePasswd() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_LoadUserAccount()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void console_LoadUserAccount(void)
{
    GCONFIG_GetAdmin(CLICMD_userTable[0].Name, CLICMD_userTable[0].Passwd, &CLICMD_userTable[0].Level);
} /* End of console_LoadUserAccount() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_PutString()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T console_PutString(CONSOLE_Inst *pInst, S8_T *strData, S16_T len)
{
    S8_T *pStr=strData;
    
    if (!pStr)
        return -1;

    while (len-->0)
    {
        pInst->PutChar(*pStr);
        pStr++;
    }
    
    return 1;	
} /* End of console_PutString() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_ParseLine()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void console_ParseLine(S8_T *pLine, S16_T numArgvs, S16_T *pArgc, S8_T *argv[])
{
    S16_T argc;

    /* for each argument */
    for (argc = 0; argc < (numArgvs - 1); argc++)
    {
        /* Skip leading white space */
        while (IsSpace(*pLine))
        {
            pLine++;
        }

        /* if end of line. */
        if( *pLine == '\0')
        {
            break;
        }

        /* record the start of the argument */
        argv[argc] = pLine;

        /* find the end of the argument */
        while (*pLine != '\0' && !IsSpace(*pLine))
        {
            pLine++;
        }

        /* null terminate argument */
        if (*pLine != '\0')
        {
            *pLine = '\0';
            pLine++;
        }
    }

    /* null terminate list of arguments */
    argv[argc] = 0;
    *pArgc = argc;
} /* End of console_ParseLine() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_HelpMessage()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void console_HelpMessage(CONSOLE_Inst *pInst, U8_T privilege)
{
    CONSOLE_CmdEntry *pCmd;
    U8_T i;

    CONSOLE_PutMessage(pInst, "help\r\n");
    CONSOLE_PutMessage(pInst, "quit\r\n");
    CONSOLE_PutMessage(pInst, "reboot\r\n");
    CONSOLE_PutMessage(pInst, "Usage: passwd\r\n");
    CONSOLE_PutMessage(pInst, "       Old Password: \r\n");
    CONSOLE_PutMessage(pInst, "       New Password: \r\n");
    CONSOLE_PutMessage(pInst, "       Re-enter New Password: \r\n");
    for (i = 0, pCmd = &CLICMD_userCmdTable[0]; i < console_CmdTableSize; i++, pCmd++)
    {
        if (pCmd->Level >= privilege && pCmd->Help)
        {
            pCmd->Help(pInst);
        }
    }
} /* End of console_HelpMessage() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_Authenticate()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T console_Authenticate(CONSOLE_Inst *pInst)
{
    CONSOLE_Account *account;
    U8_T ulen, plen, i;

    ulen = strlen(pInst->UserName);
    plen = strlen(pInst->Passwd);
    
    for (i=0, account=&CLICMD_userTable[0]; i < MAX_USER_ACCOUNT; i++, account++)
    {
        if ((ulen == strlen(account->Name)) &&
            (memcmp(account->Name, pInst->UserName, ulen)==0) &&
            (plen == strlen(account->Passwd)) &&
            (memcmp(account->Passwd, pInst->Passwd, plen)==0))
        {
#if (INCLUDE_TELNET_SERVER)
       	    if (pInst->LowLayerId == LL_TELNET )
    	        CONSOLE_PutMessage(pInst, "Successful login through telnet\r\n");
#endif
      	    pInst->Privilege = account->Level;
            return 1;
        }
    }
    
    return -1;
} /* End of console_Authenticate() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_CommandProcess()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T console_CommandProcess(CONSOLE_Inst *pInst, S8_T *pLine, U8_T privilege)
{
    S8_T *argv[CLI_MAX_ARGS+1];
    S16_T argc;
   
    if (!strncmp(pLine, CONSOLE_HELP, strlen(CONSOLE_HELP)))
    {
        console_HelpMessage(pInst, privilege);
        return 1;
    }
    else if (!strncmp(pLine, CONSOLE_PASSWD, strlen(CONSOLE_PASSWD)))
    {
        pInst->State = CLI_STATE_PASSWD_CHANGE1;
        return 1;
    }
    else if (!strncmp(pLine, CONSOLE_QUIT, strlen(CONSOLE_QUIT)))
    {
        return -1;
    }
    else if (!strncmp(pLine, CONSOLE_REBOOT, strlen(CONSOLE_REBOOT)))
    {
        MCPU_SoftReboot();
        return 1;
    }

    console_ParseLine(pLine, (sizeof(argv)/sizeof(argv[0])), &argc, argv);
    console_CmdExecute(pInst, argc, argv, privilege);
    
    return 1;
} /* End of console_CommandProcess() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_CmdExecute()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void console_CmdExecute(CONSOLE_Inst *pInst, S16_T argc, S8_T* argv[], U8_T privilege)
{
    U8_T i;
    CONSOLE_CmdEntry *pCmd;

    if (argc < 1)
    {
        return;
    }

    /* check the command in command list */
    for (i = 0, pCmd = &CLICMD_userCmdTable[0]; i < console_CmdTableSize; i++, pCmd++)
    {
        if (strcmp(argv[0], pCmd->Cmd) == 0)
        {
            if (pCmd->Level >= privilege)
            {
                pInst->Argc = argc - 1;
                pInst->Argv = (S8_T **)&argv[1];
                if (pCmd->CmdFunc(pInst) > 0)
                {
					if (pInst->State != CLI_STATE_COMMAND_WAIT)
                    CONSOLE_PutMessage(pInst, "Ok\r\n");
                }
                else
                {
                    CONSOLE_PutMessage(pInst, "Error\r\n");
                }
                return;
            }
            else
                break;
       }
    }

    CONSOLE_PutMessage(pInst, "Unknown command %s\r\n", argv[0]);
    return;
}  /* End of console_CmdExecute() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_CollectCommand()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static S16_T console_CollectCommand(CONSOLE_Inst *pInst, S8_T buf[], S16_T size, S16_T echo)
{
    S16_T c;
    S8_T tmp;

    c = pInst->GetChar();
        
    switch (c)
    {
        case CLI_CLOSED :  /* telnet quit */
            return -1;

        case CTRL_C:
            return -5;  /* user abort, return -1, means nothing */

        case BACKSP:
        case DELCHAR:
            if (pInst->BufIndex == 0)
            {
                tmp = BELL;
                pInst->PutChar(tmp);
                return 0;
            }
            /* if echo flag do back space */
            if(echo)
                console_PutString(pInst, CLI_ECHO_ON, strlen(CLI_ECHO_ON));

            if( pInst->BufIndex != 0 )  /* get rid of one char from buf */
                pInst->BufIndex -= 1;

            break;

        case 21:
            /* ^U */
            console_ClearLine(pInst);
            return 0;

        case '\r':
            buf[pInst->BufIndex] = '\0';  /* NULL char would not be counted */
            tmp = '\r';
            pInst->PutChar(tmp);
            tmp = '\n';
            pInst->PutChar(tmp);
            return CLI_COMMAND_DONE;

        case '\n':
#if (INCLUDE_TELNET_SERVER)
            if (pInst->LowLayerId == LL_TELNET)
            {
                buf[pInst->BufIndex] = '\0';  /* NULL char would not be counted */
                tmp = '\r';
                pInst->PutChar(tmp);
                tmp = '\n';
                pInst->PutChar(tmp);
                return CLI_COMMAND_DONE;
            }
            else
#endif
                return 0;
        default:
            if ( c < SPACE) /* if want printable ascii only */
                return 0;
 
            if (pInst->BufIndex <= (size - 1))
            {
                buf[pInst->BufIndex] = c;
                pInst->BufIndex++;
                tmp = (char)c;
                if (echo)
                    pInst->PutChar(tmp); /* for passwd no echo */
                if (pInst->BufIndex == size)
                     return CLI_COMMAND_DONE;
            }
            else
            {   
                tmp = BELL;                   /* buffer full, but no cr/lf */
                pInst->PutChar(tmp); /* send bell, notify user */
            }
            break;
    }

    return 1;
} /* End of console_CollectCommand() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: console_ClearLine()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
static void console_ClearLine(CONSOLE_Inst *pInst)
{
    pBuf[0] = '\r';
    memset(&pBuf[1], ' ', 79);
    pBuf[80] = '\r';
    console_PutString(pInst, pBuf, 81);

} /* End of console_ClearLine() */

/* EXPORTED SUBPROGRAM BODIES */

#if (INCLUDE_TELNET_SERVER)
/*
 * ----------------------------------------------------------------------------
 * Function Name: CONSOLE_GetInstance()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
CONSOLE_Inst *CONSOLE_GetInstance(U8_T id)
{
    U8_T i;

    for (i= 0; i < MAX_INST_NUM; i++)
    {
        if (console_Instance[i].LowLayerId == id)
            return (CONSOLE_Inst *)&console_Instance[i];
    }

    return (CONSOLE_Inst *)0;

} /* End of CONSOLE_GetInstance() */
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: CONSOLE_Init()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void CONSOLE_Init(void)
{
    CONSOLE_Inst *pInst;

    console_CmdTableSize = CLICMD_GetCmdTableSize();
    pInst = &console_Instance[LL_UART];
    pInst->State = CLI_STATE_LOGIN;
    pInst->PromptEnable = 1;
    pInst->Privilege = 5;
    pInst->BufIndex = 0;
    pInst->GetChar = UART0_NoBlockGetkey;
    pInst->PutChar = UART0_PutChar;
    pInst->LowLayerId = LL_UART;
    memset(pInst->UserName, 0, CLI_MAX_USERNAME_LEN);
    memset(pInst->Passwd, 0, CLI_MAX_PASSWD_LEN);
    memset(pInst->CmdBuf, 0, CLI_MAX_COMMAND_LEN);
    strcpy(pInst->PromptStr, CONSOLE_PROMPT);
    console_LoadUserAccount();

#if (INCLUDE_TELNET_SERVER)
    pInst = &console_Instance[LL_TELNET];
    pInst->State = CLI_STATE_BLOCK;
    pInst->PromptEnable = 1;
    pInst->Privilege = 5;
    pInst->BufIndex = 0;
    pInst->GetChar = TELNET_GetChar;
    pInst->PutChar = TELNET_PutChar;
    pInst->LowLayerId = LL_TELNET;
    memset(pInst->UserName, 0, CLI_MAX_USERNAME_LEN);
    memset(pInst->Passwd, 0, CLI_MAX_PASSWD_LEN);
    memset(pInst->CmdBuf, 0, CLI_MAX_COMMAND_LEN);
    strcpy(pInst->PromptStr, "telnet> ");
#endif

} /* End of CONSOLE_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: CONSOLE_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void CONSOLE_Task(void)
{
    CONSOLE_Inst *pInst;
    U8_T retCode, i;

    for (i = 0; i < MAX_INST_NUM; i++)
    {
        retCode=0;
        pInst = &console_Instance[i];
        
        switch(pInst->State)
        {
            case CLI_STATE_LOGIN:
                if (pInst->PromptEnable == 1)
                {
                    CONSOLE_PutMessage(pInst, "%s", USERNAME_STR);
                    pInst->PromptEnable = 0;
                    pInst->BufIndex = 0;
                    memset(pInst->UserName, 0, CLI_MAX_USERNAME_LEN);
                }
                retCode = console_CollectCommand(pInst, pInst->UserName, CLI_MAX_USERNAME_LEN, 1);
                if (retCode == CLI_COMMAND_DONE)
                {
                    pInst->PromptEnable = 1;
                    pInst->State = CLI_STATE_PASSWD;
                }
                else if (retCode < 0)
                {
                    pInst->PromptEnable = 1;
#if (INCLUDE_TELNET_SERVER)
                    if (pInst->LowLayerId == LL_TELNET)
                    {
                        pInst->State = CLI_STATE_BLOCK;
                        /* Notify telnet to close connection */
                        TELNET_NotifyClose();
                    }
#endif
                }
                break;
            case CLI_STATE_PASSWD:
                if (pInst->PromptEnable == 1)
                {
                    CONSOLE_PutMessage(pInst, "%s", PASSWD_STR);
                    memset(pInst->Passwd, 0, CLI_MAX_PASSWD_LEN);
                    pInst->PromptEnable = 0;
                    pInst->BufIndex = 0;
                }
                retCode = console_CollectCommand(pInst, pInst->Passwd, CLI_MAX_PASSWD_LEN, 0);
                if (retCode == CLI_COMMAND_DONE)
                {
                    pInst->PromptEnable = 1;
                    if (console_Authenticate(pInst) > 0)
                    {
                        pInst->State = CLI_STATE_COMMAND;
                    }
                    else
                    {
#if (INCLUDE_EVENT_DETECT)
						GEVENT_SetAuthFailEvent(1);//*** Add for authentication fail detect ***
#endif
                        CONSOLE_PutMessage(pInst, "Authentication failed!\r\n\r\n");
                        pInst->State = CLI_STATE_LOGIN;
                    }
                }
                else if (retCode < 0)
                {
                    pInst->PromptEnable = 1;
#if (INCLUDE_TELNET_SERVER)
                    if (pInst->LowLayerId == LL_TELNET)
                    {
                        pInst->State = CLI_STATE_BLOCK;
                        /* Notify telnet to close connection */
                        TELNET_NotifyClose();
                    }
                    else
#endif
                        pInst->State = CLI_STATE_LOGIN;
                }
                break;
            case CLI_STATE_COMMAND:
                if (pInst->PromptEnable == 1)
                {
                    CONSOLE_PutMessage(pInst, "%s", pInst->PromptStr);
                    memset(pInst->CmdBuf, 0, CLI_MAX_COMMAND_LEN);
                    pInst->PromptEnable = 0;
                    pInst->BufIndex = 0;
                }
                
                retCode = console_CollectCommand(pInst, pInst->CmdBuf, CLI_MAX_COMMAND_LEN, 1);
                if (retCode == CLI_COMMAND_DONE)
                {
                    pInst->PromptEnable = 1;
                    if (console_CommandProcess(pInst, pInst->CmdBuf, pInst->Privilege) < 0)
                    {
#if (INCLUDE_TELNET_SERVER)
                        if (pInst->LowLayerId == LL_TELNET)
                        {
                            pInst->State = CLI_STATE_BLOCK;
                            /* Notify telnet to close connection */
                            TELNET_NotifyClose();
                        }
                        else
#endif
                        {
                            pInst->State = CLI_STATE_LOGIN;
                        }
                    }
                }
                else if (retCode < 0)
                {
                    pInst->PromptEnable = 1;
#if (INCLUDE_TELNET_SERVER)
                    if (pInst->LowLayerId == LL_TELNET)
                    {
                        pInst->State = CLI_STATE_BLOCK;
                        /* Notify telnet to close connection */
                        TELNET_NotifyClose();
                    }
                    else
#endif
                        pInst->State = CLI_STATE_LOGIN;
                }
                break;
				/* for task type command */
			case CLI_STATE_COMMAND_WAIT:
				console_CommandProcess(pInst, pInst->CmdBuf, pInst->Privilege);
					
				break;
            case CLI_STATE_BLOCK:
                /* instance has been block */
                break;
            case CLI_STATE_PASSWD_CHANGE1:
                if (pInst->PromptEnable == 1)
                {
                    CONSOLE_PutMessage(pInst, "Old Password: ");
                    memset(pInst->Passwd, 0, CLI_MAX_PASSWD_LEN);
                    pInst->PromptEnable = 0;
                    pInst->BufIndex = 0;
                }
                retCode = console_CollectCommand(pInst, pInst->Passwd, CLI_MAX_PASSWD_LEN, 0);
                if (retCode == CLI_COMMAND_DONE)
                {
                    pInst->PromptEnable = 1;
                    pInst->State = CLI_STATE_PASSWD_CHANGE2;
                }
                else if (retCode < 0)
                {
                    pInst->PromptEnable = 1;
                    pInst->State = CLI_STATE_COMMAND;
                }
                break;
        
            case CLI_STATE_PASSWD_CHANGE2:
                if (pInst->PromptEnable == 1)
                {
                    CONSOLE_PutMessage(pInst, "New Password: ");
                    memset(pInst->PasswdNew, 0, CLI_MAX_PASSWD_LEN);
                    pInst->PromptEnable = 0;
                    pInst->BufIndex = 0;
                }
                retCode = console_CollectCommand(pInst, pInst->PasswdNew, CLI_MAX_PASSWD_LEN, 0);
                if (retCode == CLI_COMMAND_DONE)
                {
                    pInst->PromptEnable = 1;
                    pInst->State = CLI_STATE_PASSWD_CHANGE3;
                }
                else if (retCode < 0)
                {
                    pInst->PromptEnable = 1;
                    pInst->State = CLI_STATE_COMMAND;
                }
                break;
        
            case CLI_STATE_PASSWD_CHANGE3:
                if (pInst->PromptEnable == 1)
                {
                    CONSOLE_PutMessage(pInst, "Re-enter New Password: ");
                    memset(pInst->CmdBuf, 0, CLI_MAX_PASSWD_LEN);
                    pInst->PromptEnable = 0;
                    pInst->BufIndex = 0;
                }
                retCode = console_CollectCommand(pInst, pInst->CmdBuf, CLI_MAX_PASSWD_LEN, 0);
                if (retCode == CLI_COMMAND_DONE)
                {
                    pInst->PromptEnable = 1;
                    if (console_ChangePasswd(pInst) < 0)
                    {
                        CONSOLE_PutMessage(pInst, "Failed to change password\r\n");
                    }
                    else
                    {
                        CONSOLE_PutMessage(pInst, "Success to change password\r\n");
                    }
                    pInst->State = CLI_STATE_COMMAND;
                }
                else if (retCode < 0)
                {
                    pInst->PromptEnable = 1;
                    pInst->State = CLI_STATE_COMMAND;
                }
                break;
            default:
                CONSOLE_PutMessage(pInst, "Unknow state was detected, reset to LOGIN state\r\n");
                pInst->PromptEnable = 1;
                pInst->State = CLI_STATE_LOGIN;
                break;
        } /* switch */
    } /* for */

} /* End of CONSOLE_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: CONSOLE_PutMessage()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
S16_T CONSOLE_PutMessage(CONSOLE_Inst *pInst, S8_T *fmt, ...)
{
    S16_T ret;
    va_list args;

    va_start(args, fmt);
    ret = vsprintf(pBuf,fmt,args);  /* process fmt & args into buf */
    console_PutString(pInst, pBuf, ret);
    va_end(args);

    return ret;
} /* End of CONSOLE_PutMessage() */

/* End of console.c */