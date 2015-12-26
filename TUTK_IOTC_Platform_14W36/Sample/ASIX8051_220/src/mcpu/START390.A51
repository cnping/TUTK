$NOMOD51
#include	"mcpu_cfg.h"

;------------------------------------------------------------------------------
;  This file is part of the C51 Compiler package
;  Copyright (c) 1988-2001 Keil Elektronik GmbH and Keil Software, Inc.
;  Startup code for Flipper CPU, Contigious Mode
;------------------------------------------------------------------------------
;  START390.A51:  This code is executed after processor reset.
;  You may add this file to a uVision2 project.
;
;  To translate this file use A51 with the following invocation:
;
;     AX51 START390.A51 MOD_CONT
;
;  To link the modified STARTUP.OBJ file to your application use the following
;  LX51 invocation:
;
;     LX51 <your object file list>, START390.OBJ <controls>
;
;------------------------------------------------------------------------------
;
;------------------------------------------------------------------------------
;

;  Code Block Size
;
;  This Block Size must be set before system initialization.
;  Hardware will check this value for memory shadow function first.
;  It could be obtained after compilied whole source code.
;
MCPU_CODEBLOCK	EQU	MCPU_BLOCK_END		; the MCPU block size of code space.
WCPU_CODEBLOCK	EQU	WCPU_BLOCK_START	; the WCPU block size of code space.
;
;  Notes:  Each Block has 32KBytes memory space, and must not be set to '00H'.
;          The MCPU must be placed at address 0x3FFF of code space.
;          The WCPU must be placed at address 0x3FFE of code space.
;------------------------------------------------------------------------------
;
;  User-defined Power-On Initialization of Memory
;
;  With the following EQU statements the initialization of memory
;  at processor reset can be defined:
;
;		; the absolute start-address of IDATA memory is always 0
IDATALEN	EQU	80H	; the length of IDATA memory in bytes.
;
XDATASTART	EQU	0H	; the absolute start-address of XDATA memory
XDATALEN	EQU	10000H	; the length of XDATA memory in bytes.
;
PDATASTART	EQU	0H	; the absolute start-address of PDATA memory
PDATALEN	EQU	0H	; the length of PDATA memory in bytes.
;
;  Notes:  The IDATA space overlaps physically the DATA and BIT areas of the
;          8051 CPU. At minimum the memory space occupied from the C51 
;          run-time routines must be set to zero.
;------------------------------------------------------------------------------
;
;  Reentrant Stack Initilization
;
;  The following EQU statements define the stack pointer for reentrant
;  functions and initialized it:
;
;  Stack Space for reentrant functions in the SMALL model.
IBPSTACK		EQU	0		; set to 1 if small reentrant is used.
IBPSTACKTOP		EQU	0FFH	; set top of stack to highest location+1.
;
;  Stack Space for reentrant functions in the LARGE model.	
XBPSTACK		EQU	1		; set to 1 if large reentrant is used.
XBPSTACKTOP		EQU	(07FFFH - ((PAGES_OF_XMIT + PAGES_OF_RCV + 1) * 256))
;				;set top of stack to highest location below STOE buffer.
;
;  Stack Space for reentrant functions in the COMPACT model.	
PBPSTACK		EQU	0		; set to 1 if compact reentrant is used.
PBPSTACKTOP		EQU	0FFFFH	; set top of stack to highest location+1.
;
;------------------------------------------------------------------------------
;
;  Page Definition for Using the Compact Model with 64 KByte xdata RAM
;
;  The following EQU statements define the xdata page used for pdata
;  variables. The EQU PPAGE must conform with the PPAGE control used
;  in the linker invocation.
;
PPAGEENABLE		EQU	0		; set to 1 if pdata object are used.
PPAGE			EQU	0F4H	; define PPAGE number. 
;
;------------------------------------------------------------------------------
#include	"ax22000.h"

		NAME	?C_STARTUP

?C_C51STARTUP	SEGMENT		CODE

?STACK	SEGMENT		IDATA
		RSEG		?STACK
		DS			1

		EXTRN CODE	(?C_START)
		PUBLIC		?C_STARTUP

$IF RuntimeCodeAt0KH
?C_CPURESET?0	SEGMENT CODE OFFS 0
$ELSEIF RuntimeCodeAt32KH
?C_CPURESET?0	SEGMENT CODE OFFS 0x8020
$ENDIF
		RSEG	?C_CPURESET?0	

?C_STARTUP LABEL	NEAR
		DB		02H	; LJMP in Classic 8051 Mode
		DW		WORD0 STARTUP1
		RSEG	?C_C51STARTUP

$IF RuntimeCodeAt0KH
		CSEG AT 2000H		; Force the code to start from 0x2000H
$ENDIF

STARTUP1:     
		MOV     TA,#0xAA        ; Enable access to ACON
		MOV     TA,#0x55
		MOV     ACON,#0x02

IF IDATALEN <> 0
		MOV		R0,#IDATALEN - 1
		CLR		A
IDATALOOP:
		MOV		@R0,A
		DJNZ	R0,IDATALOOP
ENDIF

IF XDATALEN <> 0
		MOV		DPTR,#XDATASTART
		MOV		R7,#BYTE0 (XDATALEN)
  IF (BYTE0 (XDATALEN)) <> 0
		MOV		R6,#(BYTE1 XDATALEN) +1
  ELSE
		MOV		R6,#BYTE1 (XDATALEN)
  ENDIF
  IF (BYTE1 (XDATALEN)) <> 0
		MOV		R5,#(BYTE2 XDATALEN) +1
  ELSE
		MOV		R5,#BYTE2 (XDATALEN)
  ENDIF
		CLR		A
XDATALOOP:
		MOVX	@DPTR,A
		INC		DPTR
		DJNZ	R7,XDATALOOP
		DJNZ	R6,XDATALOOP
		DJNZ	R5,XDATALOOP
ENDIF

IF PPAGEENABLE <> 0
		MOV	P2,#PPAGE
                MOV     MXAX,#0
ENDIF

IF PDATALEN <> 0
		MOV		R0,#PDATASTART
		MOV		R7,#LOW (PDATALEN)
		CLR		A
PDATALOOP:
		MOVX	@R0,A
		INC		R0
		DJNZ	R7,PDATALOOP
ENDIF

IF IBPSTACK <> 0
EXTRN DATA (?C_IBP)

		MOV		?C_IBP,#LOW IBPSTACKTOP
ENDIF

IF XBPSTACK <> 0
EXTRN DATA (?C_XBP)

		MOV		?C_XBP,#HIGH XBPSTACKTOP
		MOV		?C_XBP+1,#LOW XBPSTACKTOP
ENDIF

IF PBPSTACK <> 0
EXTRN DATA (?C_PBP)
		MOV		?C_PBP,#LOW PBPSTACKTOP
ENDIF

		MOV		SP,#?STACK-1
		LJMP	?C_START

$IF RuntimeCodeAt0KH
MCB		SEGMENT CODE	AT 3FFFH ; MCPU Code End Block Number
		RSEG	MCB
BlockNumM:
		DB		MCPU_CODEBLOCK

WCB		SEGMENT CODE	AT 3FFEH ; WCPU Code Start Block Number
		RSEG	WCB
BlockNumW:
		DB		WCPU_CODEBLOCK

$ENDIF

		END
