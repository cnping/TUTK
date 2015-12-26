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
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define CLI_MAX_MESSAGE_LEN 128
#define CLI_MAX_COMMAND_LEN 80
#define CLI_MAX_CMD_LEN 24
#define CLI_MAX_USERNAME_LEN 16
#define CLI_MAX_PASSWD_LEN 16
#define CLI_MAX_PROMPT_LEN 16
#define CLI_MAX_CMD_BUF_LEN 100

enum LLID_Ind 
{
    LL_UART,
    LL_TELNET,
    LL_MAX
};

enum _CONSOLE_STATE_IND {
    CLI_STATE_BLOCK=0,
    CLI_STATE_LOGIN,
    CLI_STATE_PASSWD,
    CLI_STATE_COMMAND,
    CLI_STATE_COMMAND_WAIT,	
    CLI_STATE_PASSWD_CHANGE1,
    CLI_STATE_PASSWD_CHANGE2,
    CLI_STATE_PASSWD_CHANGE3,
    CLI_STATE_MAX
};
 // Add for linked list algorithm //
struct _CLILINK
{
	struct _CLILINK *pPre;
	struct _CLILINK *pNext;
	U16_T WaitTime;
	U16_T ReplyLen;
	U8_T Buf[CLI_MAX_CMD_BUF_LEN];
};
typedef struct _CLILINK tsCLILINK;

/* TYPE DECLARATIONS */
typedef struct
{
    S8_T (*PutChar)(S8_T c); /* write one character */
    S8_T (*GetChar)(void);   /* read one character */  
    U8_T Privilege;
    U8_T State;
    U8_T PromptEnable;
    U8_T LowLayerId;
    U16_T BufIndex;
    U16_T Argc;
    S8_T **Argv;
    S8_T CmdBuf[CLI_MAX_COMMAND_LEN];
    S8_T UserName[CLI_MAX_USERNAME_LEN];
    S8_T Passwd[CLI_MAX_PASSWD_LEN];
    S8_T PasswdNew[CLI_MAX_PASSWD_LEN];
    S8_T PromptStr[CLI_MAX_PROMPT_LEN];
	U8_T CmdId;
	tsCLILINK Cmd; // Add for linked list algorithm //
} CONSOLE_Inst;

typedef S16_T (*CmdPtr)(CONSOLE_Inst *pInst);
typedef S16_T (*HelpPtr)(CONSOLE_Inst *pInst);

typedef struct
{
    S8_T Cmd[CLI_MAX_CMD_LEN];
    CmdPtr CmdFunc;
    HelpPtr Help;
    U8_T Level;
} CONSOLE_CmdEntry;

typedef struct
{
    S8_T Name[CLI_MAX_USERNAME_LEN];
    S8_T Passwd[CLI_MAX_PASSWD_LEN];
    U8_T Level;
} CONSOLE_Account;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */ 
void CONSOLE_Init(void);
void CONSOLE_Task(void);
S16_T CONSOLE_PutMessage(CONSOLE_Inst *pInst, S8_T *fmt, ...);
S16_T CONSOLE_ChangeUsername(CONSOLE_Inst *pInst, U8_T *username);
#if (INCLUDE_TELNET_SERVER)
CONSOLE_Inst *CONSOLE_GetInstance(U8_T id);
#endif

#endif /* __CONSOLE_H__ */

/* End of console.h */