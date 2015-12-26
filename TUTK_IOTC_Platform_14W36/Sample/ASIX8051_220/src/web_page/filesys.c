/*
 ******************************************************************************
 *     Copyright (c) 2010 ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name: filesys.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: filesys.c,v $
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "filesys.h"
#include "httpd.h"


/* GLOBAL VARIABLE DECLARATIONS */
#if (MAX_POST_RECORDS)
POST_RECORD XDATA POST_Record[MAX_POST_RECORDS];
#endif
#if (MAX_POST_BUF_SUBMIT)
static BUF_SUBMIT XDATA BUF_Submit[MAX_POST_BUF_SUBMIT];
#endif
#if (MAX_POST_BUF_RADIO)
static BUF_RADIO XDATA BUF_Radio[MAX_POST_BUF_RADIO];
#endif
#if (MAX_POST_BUF_TEXT)
static BUF_TEXT XDATA BUF_Text[MAX_POST_BUF_TEXT];
#endif
#if (MAX_POST_BUF_PASSWORD)
static BUF_PASSWORD XDATA BUF_Password[MAX_POST_BUF_PASSWORD];
#endif
#if (MAX_POST_BUF_TAG)
static BUF_TAG XDATA BUF_Tag[MAX_POST_BUF_TAG];
#endif
#if (MAX_POST_BUF_SELECT)
static BUF_SELECT XDATA BUF_Select[MAX_POST_BUF_SELECT];
#endif


/* STATIC VARIABLE DECLARATIONS */
extern U8_T const FAR WebImg_asix[];
extern U8_T const FAR WebImg_Logo[];
extern U8_T const FAR WebAdvsetting[];
extern U8_T const FAR WebAxcss0[];
extern U8_T const FAR WebAxjs0[];
extern U8_T const FAR WebBscsetting[];
extern U8_T const FAR WebIndex[];
extern U8_T const FAR WebSecurity[];
extern U8_T const FAR WebSysconfirm[];
extern U8_T const FAR WebSysmonitor[];
extern U8_T const FAR WebSysmsg[];
extern U8_T const FAR WebWifisetting[];

#if (MAX_STORE_FILE_NUM)
FILE_MANAGEMEMT const FAR FSYS_Manage[MAX_STORE_FILE_NUM] = 
{
    /* for file 0 */ {1, 1, "img_asix.gif", 1993, WebImg_asix, 0, (void*)0},
    /* for file 1 */ {1, 1, "img_Logo.jpg", 2841, WebImg_Logo, 1, (void*)0},
    /* for file 2 */ {1, 0, "advsetting.htm", 8359, WebAdvsetting, 2, (void*)0},
    /* for file 3 */ {1, 3, "axcss0.css", 2033, WebAxcss0, 3, (void*)0},
    /* for file 4 */ {1, 2, "axjs0.js", 4698, WebAxjs0, 4, (void*)0},
    /* for file 5 */ {1, 0, "bscsetting.htm", 9426, WebBscsetting, 5, (void*)0},
    /* for file 6 */ {1, 0, "index.htm", 2525, WebIndex, 6, (void*)0},
    /* for file 7 */ {1, 0, "security.htm", 6554, WebSecurity, 7, (void*)0},
    /* for file 8 */ {1, 0, "sysconfirm.htm", 2195, WebSysconfirm, 8, (void*)0},
    /* for file 9 */ {1, 0, "sysmonitor.htm", 5293, WebSysmonitor, 9, (void*)0},
    /* for file 10 */ {1, 0, "sysmsg.htm", 2224, WebSysmsg, 10, (void*)0},
    /* for file 11 */ {1, 0, "wifisetting.htm", 11668, WebWifisetting, 11, (void*)0},
};
#endif

/* LOCAL SUBPROGRAM DECLARATIONS */
static void fsys_InitPostRecord(void);

/*
 * ----------------------------------------------------------------------------
 * Function Name: FSYS_Init
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void FSYS_Init(void)
{


	fsys_InitPostRecord();
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: fsys_InitPostRecord
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void fsys_InitPostRecord(void)
{
	BUF_SUBMIT XDATA*	pSubmit;
	BUF_TEXT XDATA*		pText;
	BUF_PASSWORD XDATA*		pPassword;
	BUF_TAG XDATA*		pTag;
#if (MAX_POST_BUF_RADIO)
	BUF_RADIO XDATA*	pRadio;
#endif
#if (MAX_POST_BUF_SELECT)
	BUF_SELECT XDATA*	pSelect;
#endif
	U8_T				index;

	static U8_T XDATA postbuf_text0[2][16];
	static U8_T XDATA postbuf_text1[2][65];
	static U8_T XDATA postbuf_text2[2][36];
	static U8_T XDATA postbuf_text3[2][36];
	static U8_T XDATA postbuf_text4[2][36];
	static U8_T XDATA postbuf_text5[2][36];
	static U8_T XDATA postbuf_text6[2][36];
	static U8_T XDATA postbuf_text7[2][16];
	static U8_T XDATA postbuf_text8[2][16];
	static U8_T XDATA postbuf_text9[2][16];
	static U8_T XDATA postbuf_text10[2][16];
	static U8_T XDATA postbuf_text11[2][17];
	static U8_T XDATA postbuf_text12[2][16];
	static U8_T XDATA postbuf_text13[2][16];
	static U8_T XDATA postbuf_text14[2][16];
	static U8_T XDATA postbuf_text15[2][16];
	static U8_T XDATA postbuf_text16[2][6];
	static U8_T XDATA postbuf_text17[2][6];
	static U8_T XDATA postbuf_text18[2][36];
	static U8_T XDATA postbuf_text19[2][6];
	static U8_T XDATA postbuf_text20[2][17];
	static U8_T XDATA postbuf_text21[2][16];
	static U8_T XDATA postbuf_text22[2][16];
	static U8_T XDATA postbuf_text23[2][16];
	static U8_T XDATA postbuf_text24[2][16];
	static U8_T XDATA postbuf_text25[2][16];
	static U8_T XDATA postbuf_text26[2][33];
	static U8_T XDATA postbuf_text27[2][27];
	static U8_T XDATA postbuf_text28[2][27];
	static U8_T XDATA postbuf_text29[2][27];
	static U8_T XDATA postbuf_text30[2][27];
	static U8_T XDATA postbuf_text31[2][64];
	static U8_T XDATA postbuf_text32[2][7];
	static U8_T XDATA postbuf_text33[2][7];
	static U8_T XDATA postbuf_password0[2][17];
	static U8_T XDATA postbuf_password1[2][16];
	static U8_T XDATA postbuf_password2[2][16];
	static U8_T XDATA postbuf_password3[2][16];

	for (index = 1; index < MAX_POST_RECORDS; index++)
		POST_Record[index].Occupy = 0;

	/* for record 0 */
	POST_Record[0].Occupy = 1;
	POST_Record[0].PostType = POST_TYPE_TEXT;
	POST_Record[0].Name[0] = 't';
	POST_Record[0].Name[1] = 'f';
	POST_Record[0].Name[2] = 't';
	POST_Record[0].Name[3] = 'p';
	POST_Record[0].Name[4] = 's';
	POST_Record[0].Name[5] = '_';
	POST_Record[0].Name[6] = 'i';
	POST_Record[0].Name[7] = 'p';
	POST_Record[0].NameLen = 8;
	POST_Record[0].FileIndex = 2;
	POST_Record[0].UpdateSelf = FALSE;
	POST_Record[0].PValue = &BUF_Text[0];
	pText = POST_Record[0].PValue;
	pText->CurrValue = postbuf_text0[0];
	pText->UserValue = postbuf_text0[1];
	pText->Offset = 2542;
	pText->DefaultLength = 13;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '9';
	pText->CurrValue[2] = '2';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '1';
	pText->CurrValue[5] = '6';
	pText->CurrValue[6] = '8';
	pText->CurrValue[7] = '.';
	pText->CurrValue[8] = '0';
	pText->CurrValue[9] = '.';
	pText->CurrValue[10] = '8';
	pText->CurrValue[11] = '0';
	pText->CurrValue[12] = '"';
	pText->CurrLength = 13;

	/* for record 1 */
	POST_Record[1].Occupy = 1;
	POST_Record[1].PostType = POST_TYPE_TEXT;
	POST_Record[1].Name[0] = 'f';
	POST_Record[1].Name[1] = 'i';
	POST_Record[1].Name[2] = 'l';
	POST_Record[1].Name[3] = 'e';
	POST_Record[1].Name[4] = '_';
	POST_Record[1].Name[5] = 'n';
	POST_Record[1].Name[6] = 'a';
	POST_Record[1].Name[7] = 'm';
	POST_Record[1].Name[8] = 'e';
	POST_Record[1].NameLen = 9;
	POST_Record[1].FileIndex = 2;
	POST_Record[1].UpdateSelf = FALSE;
	POST_Record[1].PValue = &BUF_Text[1];
	pText = POST_Record[1].PValue;
	pText->CurrValue = postbuf_text1[0];
	pText->UserValue = postbuf_text1[1];
	pText->Offset = 2768;
	pText->DefaultLength = 7;
	pText->CurrValue[0] = 'a';
	pText->CurrValue[1] = 'x';
	pText->CurrValue[2] = '.';
	pText->CurrValue[3] = 'b';
	pText->CurrValue[4] = 'i';
	pText->CurrValue[5] = 'n';
	pText->CurrValue[6] = '"';
	pText->CurrLength = 7;

	/* for record 2 */
	POST_Record[2].Occupy = 1;
	POST_Record[2].PostType = POST_TYPE_SUBMIT;
	POST_Record[2].Name[0] = 'A';
	POST_Record[2].Name[1] = 'p';
	POST_Record[2].Name[2] = 'p';
	POST_Record[2].Name[3] = 'l';
	POST_Record[2].Name[4] = 'y';
	POST_Record[2].NameLen = 5;
	POST_Record[2].FileIndex = 0xff;
	POST_Record[2].UpdateSelf = FALSE;
	POST_Record[2].PValue = &BUF_Submit[0];
	pSubmit = POST_Record[2].PValue;
	pSubmit->Value[0] = 'A';
	pSubmit->Value[1] = 'p';
	pSubmit->Value[2] = 'p';
	pSubmit->Value[3] = 'l';
	pSubmit->Value[4] = 'y';
	pSubmit->DefaultVlaueLen = 5;
	pSubmit->IsApply = 1;

	/* for record 3 */
	POST_Record[3].Occupy = 1;
	POST_Record[3].PostType = POST_TYPE_SUBMIT;
	POST_Record[3].Name[0] = 'u';
	POST_Record[3].Name[1] = 'p';
	POST_Record[3].Name[2] = 'g';
	POST_Record[3].Name[3] = 'r';
	POST_Record[3].Name[4] = 'a';
	POST_Record[3].Name[5] = 'd';
	POST_Record[3].Name[6] = 'e';
	POST_Record[3].NameLen = 7;
	POST_Record[3].FileIndex = 0xff;
	POST_Record[3].UpdateSelf = FALSE;
	POST_Record[3].PValue = &BUF_Submit[1];
	pSubmit = POST_Record[3].PValue;
	pSubmit->Value[0] = 'F';
	pSubmit->Value[1] = 'i';
	pSubmit->Value[2] = 'r';
	pSubmit->Value[3] = 'm';
	pSubmit->Value[4] = 'w';
	pSubmit->Value[5] = 'a';
	pSubmit->Value[6] = 'r';
	pSubmit->Value[7] = 'e';
	pSubmit->Value[8] = 'U';
	pSubmit->Value[9] = 'p';
	pSubmit->Value[10] = 'g';
	pSubmit->Value[11] = 'r';
	pSubmit->Value[12] = 'a';
	pSubmit->Value[13] = 'd';
	pSubmit->Value[14] = 'e';
	pSubmit->DefaultVlaueLen = 15;
	pSubmit->IsApply = 1;

	/* for record 4 */
	POST_Record[4].Occupy = 1;
	POST_Record[4].PostType = POST_TYPE_TEXT;
	POST_Record[4].Name[0] = 'e';
	POST_Record[4].Name[1] = 'm';
	POST_Record[4].Name[2] = 'a';
	POST_Record[4].Name[3] = 'i';
	POST_Record[4].Name[4] = 'l';
	POST_Record[4].Name[5] = '_';
	POST_Record[4].Name[6] = 'a';
	POST_Record[4].Name[7] = 'd';
	POST_Record[4].Name[8] = 'd';
	POST_Record[4].Name[9] = 'r';
	POST_Record[4].NameLen = 10;
	POST_Record[4].FileIndex = 2;
	POST_Record[4].UpdateSelf = FALSE;
	POST_Record[4].PValue = &BUF_Text[2];
	pText = POST_Record[4].PValue;
	pText->CurrValue = postbuf_text2[0];
	pText->UserValue = postbuf_text2[1];
	pText->Offset = 3699;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 5 */
	POST_Record[5].Occupy = 1;
	POST_Record[5].PostType = POST_TYPE_TEXT;
	POST_Record[5].Name[0] = 'e';
	POST_Record[5].Name[1] = 'm';
	POST_Record[5].Name[2] = 'a';
	POST_Record[5].Name[3] = 'i';
	POST_Record[5].Name[4] = 'l';
	POST_Record[5].Name[5] = '_';
	POST_Record[5].Name[6] = 'f';
	POST_Record[5].Name[7] = 'r';
	POST_Record[5].Name[8] = 'o';
	POST_Record[5].Name[9] = 'm';
	POST_Record[5].NameLen = 10;
	POST_Record[5].FileIndex = 2;
	POST_Record[5].UpdateSelf = FALSE;
	POST_Record[5].PValue = &BUF_Text[3];
	pText = POST_Record[5].PValue;
	pText->CurrValue = postbuf_text3[0];
	pText->UserValue = postbuf_text3[1];
	pText->Offset = 4089;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 6 */
	POST_Record[6].Occupy = 1;
	POST_Record[6].PostType = POST_TYPE_TEXT;
	POST_Record[6].Name[0] = 'e';
	POST_Record[6].Name[1] = 'm';
	POST_Record[6].Name[2] = 'a';
	POST_Record[6].Name[3] = 'i';
	POST_Record[6].Name[4] = 'l';
	POST_Record[6].Name[5] = '_';
	POST_Record[6].Name[6] = 't';
	POST_Record[6].Name[7] = 'o';
	POST_Record[6].Name[8] = '1';
	POST_Record[6].NameLen = 9;
	POST_Record[6].FileIndex = 2;
	POST_Record[6].UpdateSelf = FALSE;
	POST_Record[6].PValue = &BUF_Text[4];
	pText = POST_Record[6].PValue;
	pText->CurrValue = postbuf_text4[0];
	pText->UserValue = postbuf_text4[1];
	pText->Offset = 4313;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 7 */
	POST_Record[7].Occupy = 1;
	POST_Record[7].PostType = POST_TYPE_TEXT;
	POST_Record[7].Name[0] = 'e';
	POST_Record[7].Name[1] = 'm';
	POST_Record[7].Name[2] = 'a';
	POST_Record[7].Name[3] = 'i';
	POST_Record[7].Name[4] = 'l';
	POST_Record[7].Name[5] = '_';
	POST_Record[7].Name[6] = 't';
	POST_Record[7].Name[7] = 'o';
	POST_Record[7].Name[8] = '2';
	POST_Record[7].NameLen = 9;
	POST_Record[7].FileIndex = 2;
	POST_Record[7].UpdateSelf = FALSE;
	POST_Record[7].PValue = &BUF_Text[5];
	pText = POST_Record[7].PValue;
	pText->CurrValue = postbuf_text5[0];
	pText->UserValue = postbuf_text5[1];
	pText->Offset = 4537;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 8 */
	POST_Record[8].Occupy = 1;
	POST_Record[8].PostType = POST_TYPE_TEXT;
	POST_Record[8].Name[0] = 'e';
	POST_Record[8].Name[1] = 'm';
	POST_Record[8].Name[2] = 'a';
	POST_Record[8].Name[3] = 'i';
	POST_Record[8].Name[4] = 'l';
	POST_Record[8].Name[5] = '_';
	POST_Record[8].Name[6] = 't';
	POST_Record[8].Name[7] = 'o';
	POST_Record[8].Name[8] = '3';
	POST_Record[8].NameLen = 9;
	POST_Record[8].FileIndex = 2;
	POST_Record[8].UpdateSelf = FALSE;
	POST_Record[8].PValue = &BUF_Text[6];
	pText = POST_Record[8].PValue;
	pText->CurrValue = postbuf_text6[0];
	pText->UserValue = postbuf_text6[1];
	pText->Offset = 4761;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 9 */
	POST_Record[9].Occupy = 1;
	POST_Record[9].PostType = POST_TYPE_SELECT;
	POST_Record[9].Name[0] = 'c';
	POST_Record[9].Name[1] = 'o';
	POST_Record[9].Name[2] = 'l';
	POST_Record[9].Name[3] = 'd';
	POST_Record[9].Name[4] = 's';
	POST_Record[9].Name[5] = 't';
	POST_Record[9].Name[6] = 'a';
	POST_Record[9].Name[7] = 'r';
	POST_Record[9].Name[8] = 't';
	POST_Record[9].NameLen = 9;
	POST_Record[9].FileIndex = 2;
	POST_Record[9].UpdateSelf = FALSE;
	POST_Record[9].PValue = &BUF_Select[0];
	pSelect = POST_Record[9].PValue;
	pSelect->Offset[0] = 4983;
	pSelect->Offset[1] = 5044;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 10 */
	POST_Record[10].Occupy = 1;
	POST_Record[10].PostType = POST_TYPE_SELECT;
	POST_Record[10].Name[0] = 'a';
	POST_Record[10].Name[1] = 'u';
	POST_Record[10].Name[2] = 't';
	POST_Record[10].Name[3] = 'h';
	POST_Record[10].Name[4] = 'f';
	POST_Record[10].Name[5] = 'a';
	POST_Record[10].Name[6] = 'i';
	POST_Record[10].Name[7] = 'l';
	POST_Record[10].NameLen = 8;
	POST_Record[10].FileIndex = 2;
	POST_Record[10].UpdateSelf = FALSE;
	POST_Record[10].PValue = &BUF_Select[1];
	pSelect = POST_Record[10].PValue;
	pSelect->Offset[0] = 5289;
	pSelect->Offset[1] = 5350;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 11 */
	POST_Record[11].Occupy = 1;
	POST_Record[11].PostType = POST_TYPE_SELECT;
	POST_Record[11].Name[0] = 'i';
	POST_Record[11].Name[1] = 'p';
	POST_Record[11].Name[2] = 'c';
	POST_Record[11].Name[3] = 'h';
	POST_Record[11].Name[4] = 'g';
	POST_Record[11].NameLen = 5;
	POST_Record[11].FileIndex = 2;
	POST_Record[11].UpdateSelf = FALSE;
	POST_Record[11].PValue = &BUF_Select[2];
	pSelect = POST_Record[11].PValue;
	pSelect->Offset[0] = 5594;
	pSelect->Offset[1] = 5655;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 12 */
	POST_Record[12].Occupy = 1;
	POST_Record[12].PostType = POST_TYPE_SELECT;
	POST_Record[12].Name[0] = 'p';
	POST_Record[12].Name[1] = 's';
	POST_Record[12].Name[2] = 'w';
	POST_Record[12].Name[3] = 'c';
	POST_Record[12].Name[4] = 'h';
	POST_Record[12].Name[5] = 'g';
	POST_Record[12].NameLen = 6;
	POST_Record[12].FileIndex = 2;
	POST_Record[12].UpdateSelf = FALSE;
	POST_Record[12].PValue = &BUF_Select[3];
	pSelect = POST_Record[12].PValue;
	pSelect->Offset[0] = 5892;
	pSelect->Offset[1] = 5953;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 13 */
	POST_Record[13].Occupy = 1;
	POST_Record[13].PostType = POST_TYPE_RADIO;
	POST_Record[13].Name[0] = 'd';
	POST_Record[13].Name[1] = 'h';
	POST_Record[13].Name[2] = 'c';
	POST_Record[13].Name[3] = 'p';
	POST_Record[13].Name[4] = 's';
	POST_Record[13].Name[5] = '_';
	POST_Record[13].Name[6] = 'o';
	POST_Record[13].Name[7] = 'n';
	POST_Record[13].Name[8] = 'o';
	POST_Record[13].Name[9] = 'f';
	POST_Record[13].Name[10] = 'f';
	POST_Record[13].NameLen = 11;
	POST_Record[13].FileIndex = 2;
	POST_Record[13].UpdateSelf = FALSE;
	POST_Record[13].PValue = &BUF_Radio[0];
	pRadio = POST_Record[13].PValue;
	pRadio->Offset[0] = 6715;
	pRadio->Value[0][0] = '0';
	pRadio->Length[0] = 1;
	pRadio->Offset[1] = 6810;
	pRadio->Value[1][0] = '1';
	pRadio->Length[1] = 1;
	pRadio->Count = 2;
	pRadio->DefaultSet = 1;
	pRadio->CurrentSet = 1;
	pRadio->UserSet = 1;

	/* for record 14 */
	POST_Record[14].Occupy = 1;
	POST_Record[14].PostType = POST_TYPE_TEXT;
	POST_Record[14].Name[0] = 'd';
	POST_Record[14].Name[1] = 'h';
	POST_Record[14].Name[2] = 'c';
	POST_Record[14].Name[3] = 'p';
	POST_Record[14].Name[4] = 's';
	POST_Record[14].Name[5] = '_';
	POST_Record[14].Name[6] = 's';
	POST_Record[14].Name[7] = 't';
	POST_Record[14].Name[8] = 'a';
	POST_Record[14].Name[9] = 'r';
	POST_Record[14].Name[10] = 't';
	POST_Record[14].Name[11] = 'i';
	POST_Record[14].Name[12] = 'p';
	POST_Record[14].NameLen = 13;
	POST_Record[14].FileIndex = 2;
	POST_Record[14].UpdateSelf = FALSE;
	POST_Record[14].PValue = &BUF_Text[7];
	pText = POST_Record[14].PValue;
	pText->CurrValue = postbuf_text7[0];
	pText->UserValue = postbuf_text7[1];
	pText->Offset = 7043;
	pText->DefaultLength = 12;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '9';
	pText->CurrValue[2] = '2';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '1';
	pText->CurrValue[5] = '6';
	pText->CurrValue[6] = '8';
	pText->CurrValue[7] = '.';
	pText->CurrValue[8] = '0';
	pText->CurrValue[9] = '.';
	pText->CurrValue[10] = '2';
	pText->CurrValue[11] = '"';
	pText->CurrLength = 12;

	/* for record 15 */
	POST_Record[15].Occupy = 1;
	POST_Record[15].PostType = POST_TYPE_TEXT;
	POST_Record[15].Name[0] = 'd';
	POST_Record[15].Name[1] = 'h';
	POST_Record[15].Name[2] = 'c';
	POST_Record[15].Name[3] = 'p';
	POST_Record[15].Name[4] = 's';
	POST_Record[15].Name[5] = '_';
	POST_Record[15].Name[6] = 'e';
	POST_Record[15].Name[7] = 'n';
	POST_Record[15].Name[8] = 'd';
	POST_Record[15].Name[9] = 'i';
	POST_Record[15].Name[10] = 'p';
	POST_Record[15].NameLen = 11;
	POST_Record[15].FileIndex = 2;
	POST_Record[15].UpdateSelf = FALSE;
	POST_Record[15].PValue = &BUF_Text[8];
	pText = POST_Record[15].PValue;
	pText->CurrValue = postbuf_text8[0];
	pText->UserValue = postbuf_text8[1];
	pText->Offset = 7283;
	pText->DefaultLength = 14;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '9';
	pText->CurrValue[2] = '2';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '1';
	pText->CurrValue[5] = '6';
	pText->CurrValue[6] = '8';
	pText->CurrValue[7] = '.';
	pText->CurrValue[8] = '0';
	pText->CurrValue[9] = '.';
	pText->CurrValue[10] = '2';
	pText->CurrValue[11] = '5';
	pText->CurrValue[12] = '4';
	pText->CurrValue[13] = '"';
	pText->CurrLength = 14;

	/* for record 16 */
	POST_Record[16].Occupy = 1;
	POST_Record[16].PostType = POST_TYPE_TEXT;
	POST_Record[16].Name[0] = 'd';
	POST_Record[16].Name[1] = 'h';
	POST_Record[16].Name[2] = 'c';
	POST_Record[16].Name[3] = 'p';
	POST_Record[16].Name[4] = 's';
	POST_Record[16].Name[5] = '_';
	POST_Record[16].Name[6] = 'g';
	POST_Record[16].Name[7] = 'a';
	POST_Record[16].Name[8] = 't';
	POST_Record[16].Name[9] = 'e';
	POST_Record[16].Name[10] = 'w';
	POST_Record[16].Name[11] = 'a';
	POST_Record[16].Name[12] = 'y';
	POST_Record[16].NameLen = 13;
	POST_Record[16].FileIndex = 2;
	POST_Record[16].UpdateSelf = FALSE;
	POST_Record[16].PValue = &BUF_Text[9];
	pText = POST_Record[16].PValue;
	pText->CurrValue = postbuf_text9[0];
	pText->UserValue = postbuf_text9[1];
	pText->Offset = 7520;
	pText->DefaultLength = 12;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '9';
	pText->CurrValue[2] = '2';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '1';
	pText->CurrValue[5] = '6';
	pText->CurrValue[6] = '8';
	pText->CurrValue[7] = '.';
	pText->CurrValue[8] = '0';
	pText->CurrValue[9] = '.';
	pText->CurrValue[10] = '1';
	pText->CurrValue[11] = '"';
	pText->CurrLength = 12;

	/* for record 17 */
	POST_Record[17].Occupy = 1;
	POST_Record[17].PostType = POST_TYPE_TEXT;
	POST_Record[17].Name[0] = 'd';
	POST_Record[17].Name[1] = 'h';
	POST_Record[17].Name[2] = 'c';
	POST_Record[17].Name[3] = 'p';
	POST_Record[17].Name[4] = 's';
	POST_Record[17].Name[5] = '_';
	POST_Record[17].Name[6] = 'm';
	POST_Record[17].Name[7] = 'a';
	POST_Record[17].Name[8] = 's';
	POST_Record[17].Name[9] = 'k';
	POST_Record[17].NameLen = 10;
	POST_Record[17].FileIndex = 2;
	POST_Record[17].UpdateSelf = FALSE;
	POST_Record[17].PValue = &BUF_Text[10];
	pText = POST_Record[17].PValue;
	pText->CurrValue = postbuf_text10[0];
	pText->UserValue = postbuf_text10[1];
	pText->Offset = 7748;
	pText->DefaultLength = 14;
	pText->CurrValue[0] = '2';
	pText->CurrValue[1] = '5';
	pText->CurrValue[2] = '5';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '2';
	pText->CurrValue[5] = '5';
	pText->CurrValue[6] = '5';
	pText->CurrValue[7] = '.';
	pText->CurrValue[8] = '2';
	pText->CurrValue[9] = '5';
	pText->CurrValue[10] = '5';
	pText->CurrValue[11] = '.';
	pText->CurrValue[12] = '0';
	pText->CurrValue[13] = '"';
	pText->CurrLength = 14;

	/* for record 18 */
	POST_Record[18].Occupy = 1;
	POST_Record[18].PostType = POST_TYPE_TEXT;
	POST_Record[18].Name[0] = 'd';
	POST_Record[18].Name[1] = 's';
	POST_Record[18].Name[2] = 'm';
	POST_Record[18].Name[3] = '_';
	POST_Record[18].Name[4] = 'n';
	POST_Record[18].Name[5] = 'a';
	POST_Record[18].Name[6] = 'm';
	POST_Record[18].Name[7] = 'e';
	POST_Record[18].NameLen = 8;
	POST_Record[18].FileIndex = 5;
	POST_Record[18].UpdateSelf = FALSE;
	POST_Record[18].PValue = &BUF_Text[11];
	pText = POST_Record[18].PValue;
	pText->CurrValue = postbuf_text11[0];
	pText->UserValue = postbuf_text11[1];
	pText->Offset = 2520;
	pText->DefaultLength = 5;
	pText->CurrValue[0] = 'D';
	pText->CurrValue[1] = 'S';
	pText->CurrValue[2] = 'M';
	pText->CurrValue[3] = '1';
	pText->CurrValue[4] = '"';
	pText->CurrLength = 5;

	/* for record 19 */
	POST_Record[19].Occupy = 1;
	POST_Record[19].PostType = POST_TYPE_SELECT;
	POST_Record[19].Name[0] = 'd';
	POST_Record[19].Name[1] = 'b';
	POST_Record[19].Name[2] = 'r';
	POST_Record[19].NameLen = 3;
	POST_Record[19].FileIndex = 5;
	POST_Record[19].UpdateSelf = FALSE;
	POST_Record[19].PValue = &BUF_Select[4];
	pSelect = POST_Record[19].PValue;
	pSelect->Offset[0] = 2770;
	pSelect->Offset[1] = 2824;
	pSelect->Offset[2] = 2869;
	pSelect->Offset[3] = 2913;
	pSelect->Offset[4] = 2957;
	pSelect->Offset[5] = 3001;
	pSelect->Offset[6] = 3044;
	pSelect->Offset[7] = 3087;
	pSelect->Offset[8] = 3130;
	pSelect->Count = 9;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 20 */
	POST_Record[20].Occupy = 1;
	POST_Record[20].PostType = POST_TYPE_SELECT;
	POST_Record[20].Name[0] = 'd';
	POST_Record[20].Name[1] = 'a';
	POST_Record[20].Name[2] = 't';
	POST_Record[20].Name[3] = 'a';
	POST_Record[20].NameLen = 4;
	POST_Record[20].FileIndex = 5;
	POST_Record[20].UpdateSelf = FALSE;
	POST_Record[20].PValue = &BUF_Select[5];
	pSelect = POST_Record[20].PValue;
	pSelect->Offset[0] = 3360;
	pSelect->Offset[1] = 3419;
	pSelect->Offset[2] = 3469;
	pSelect->Offset[3] = 3519;
	pSelect->Count = 4;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 21 */
	POST_Record[21].Occupy = 1;
	POST_Record[21].PostType = POST_TYPE_SELECT;
	POST_Record[21].Name[0] = 'p';
	POST_Record[21].Name[1] = 'a';
	POST_Record[21].Name[2] = 'r';
	POST_Record[21].Name[3] = 'i';
	POST_Record[21].Name[4] = 't';
	POST_Record[21].Name[5] = 'y';
	POST_Record[21].NameLen = 6;
	POST_Record[21].FileIndex = 5;
	POST_Record[21].UpdateSelf = FALSE;
	POST_Record[21].PValue = &BUF_Select[6];
	pSelect = POST_Record[21].PValue;
	pSelect->Offset[0] = 3768;
	pSelect->Offset[1] = 3829;
	pSelect->Offset[2] = 3882;
	pSelect->Count = 3;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 22 */
	POST_Record[22].Occupy = 1;
	POST_Record[22].PostType = POST_TYPE_SELECT;
	POST_Record[22].Name[0] = 's';
	POST_Record[22].Name[1] = 't';
	POST_Record[22].Name[2] = 'o';
	POST_Record[22].Name[3] = 'p';
	POST_Record[22].NameLen = 4;
	POST_Record[22].FileIndex = 5;
	POST_Record[22].UpdateSelf = FALSE;
	POST_Record[22].PValue = &BUF_Select[7];
	pSelect = POST_Record[22].PValue;
	pSelect->Offset[0] = 4114;
	pSelect->Offset[1] = 4173;
	pSelect->Offset[2] = 4225;
	pSelect->Count = 3;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 23 */
	POST_Record[23].Occupy = 1;
	POST_Record[23].PostType = POST_TYPE_SELECT;
	POST_Record[23].Name[0] = 'f';
	POST_Record[23].Name[1] = 'l';
	POST_Record[23].Name[2] = 'o';
	POST_Record[23].Name[3] = 'w';
	POST_Record[23].NameLen = 4;
	POST_Record[23].FileIndex = 5;
	POST_Record[23].UpdateSelf = FALSE;
	POST_Record[23].PValue = &BUF_Select[8];
	pSelect = POST_Record[23].PValue;
	pSelect->Offset[0] = 4457;
	pSelect->Offset[1] = 4523;
	pSelect->Offset[2] = 4580;
	pSelect->Count = 3;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 24 */
	POST_Record[24].Occupy = 1;
	POST_Record[24].PostType = POST_TYPE_SELECT;
	POST_Record[24].Name[0] = 'r';
	POST_Record[24].Name[1] = 's';
	POST_Record[24].Name[2] = '4';
	POST_Record[24].Name[3] = '8';
	POST_Record[24].Name[4] = '5';
	POST_Record[24].NameLen = 5;
	POST_Record[24].FileIndex = 5;
	POST_Record[24].UpdateSelf = FALSE;
	POST_Record[24].PValue = &BUF_Select[9];
	pSelect = POST_Record[24].PValue;
	pSelect->Offset[0] = 4810;
	pSelect->Offset[1] = 4873;
	pSelect->Offset[2] = 4944;
	pSelect->Offset[3] = 5023;
	pSelect->Count = 4;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 25 */
	POST_Record[25].Occupy = 1;
	POST_Record[25].PostType = POST_TYPE_SELECT;
	POST_Record[25].Name[0] = 'd';
	POST_Record[25].Name[1] = 'h';
	POST_Record[25].Name[2] = 'c';
	POST_Record[25].Name[3] = 'p';
	POST_Record[25].NameLen = 4;
	POST_Record[25].FileIndex = 5;
	POST_Record[25].UpdateSelf = FALSE;
	POST_Record[25].PValue = &BUF_Select[10];
	pSelect = POST_Record[25].PValue;
	pSelect->Offset[0] = 5389;
	pSelect->Offset[1] = 5454;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 26 */
	POST_Record[26].Occupy = 1;
	POST_Record[26].PostType = POST_TYPE_TEXT;
	POST_Record[26].Name[0] = 's';
	POST_Record[26].Name[1] = 't';
	POST_Record[26].Name[2] = 'a';
	POST_Record[26].Name[3] = 't';
	POST_Record[26].Name[4] = 'i';
	POST_Record[26].Name[5] = 'c';
	POST_Record[26].Name[6] = '_';
	POST_Record[26].Name[7] = 'i';
	POST_Record[26].Name[8] = 'p';
	POST_Record[26].NameLen = 9;
	POST_Record[26].FileIndex = 5;
	POST_Record[26].UpdateSelf = FALSE;
	POST_Record[26].PValue = &BUF_Text[12];
	pText = POST_Record[26].PValue;
	pText->CurrValue = postbuf_text12[0];
	pText->UserValue = postbuf_text12[1];
	pText->Offset = 5690;
	pText->DefaultLength = 12;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '9';
	pText->CurrValue[2] = '2';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '1';
	pText->CurrValue[5] = '6';
	pText->CurrValue[6] = '8';
	pText->CurrValue[7] = '.';
	pText->CurrValue[8] = '0';
	pText->CurrValue[9] = '.';
	pText->CurrValue[10] = '3';
	pText->CurrValue[11] = '"';
	pText->CurrLength = 12;

	/* for record 27 */
	POST_Record[27].Occupy = 1;
	POST_Record[27].PostType = POST_TYPE_TEXT;
	POST_Record[27].Name[0] = 'm';
	POST_Record[27].Name[1] = 'a';
	POST_Record[27].Name[2] = 's';
	POST_Record[27].Name[3] = 'k';
	POST_Record[27].NameLen = 4;
	POST_Record[27].FileIndex = 5;
	POST_Record[27].UpdateSelf = FALSE;
	POST_Record[27].PValue = &BUF_Text[13];
	pText = POST_Record[27].PValue;
	pText->CurrValue = postbuf_text13[0];
	pText->UserValue = postbuf_text13[1];
	pText->Offset = 5919;
	pText->DefaultLength = 14;
	pText->CurrValue[0] = '2';
	pText->CurrValue[1] = '5';
	pText->CurrValue[2] = '5';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '2';
	pText->CurrValue[5] = '5';
	pText->CurrValue[6] = '5';
	pText->CurrValue[7] = '.';
	pText->CurrValue[8] = '2';
	pText->CurrValue[9] = '5';
	pText->CurrValue[10] = '5';
	pText->CurrValue[11] = '.';
	pText->CurrValue[12] = '0';
	pText->CurrValue[13] = '"';
	pText->CurrLength = 14;

	/* for record 28 */
	POST_Record[28].Occupy = 1;
	POST_Record[28].PostType = POST_TYPE_TEXT;
	POST_Record[28].Name[0] = 'g';
	POST_Record[28].Name[1] = 'a';
	POST_Record[28].Name[2] = 't';
	POST_Record[28].Name[3] = 'e';
	POST_Record[28].Name[4] = 'w';
	POST_Record[28].Name[5] = 'a';
	POST_Record[28].Name[6] = 'y';
	POST_Record[28].Name[7] = '_';
	POST_Record[28].Name[8] = 'i';
	POST_Record[28].Name[9] = 'p';
	POST_Record[28].NameLen = 10;
	POST_Record[28].FileIndex = 5;
	POST_Record[28].UpdateSelf = FALSE;
	POST_Record[28].PValue = &BUF_Text[14];
	pText = POST_Record[28].PValue;
	pText->CurrValue = postbuf_text14[0];
	pText->UserValue = postbuf_text14[1];
	pText->Offset = 6160;
	pText->DefaultLength = 12;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '9';
	pText->CurrValue[2] = '2';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '1';
	pText->CurrValue[5] = '6';
	pText->CurrValue[6] = '8';
	pText->CurrValue[7] = '.';
	pText->CurrValue[8] = '0';
	pText->CurrValue[9] = '.';
	pText->CurrValue[10] = '1';
	pText->CurrValue[11] = '"';
	pText->CurrLength = 12;

	/* for record 29 */
	POST_Record[29].Occupy = 1;
	POST_Record[29].PostType = POST_TYPE_TEXT;
	POST_Record[29].Name[0] = 'd';
	POST_Record[29].Name[1] = 'n';
	POST_Record[29].Name[2] = 's';
	POST_Record[29].Name[3] = '_';
	POST_Record[29].Name[4] = 'i';
	POST_Record[29].Name[5] = 'p';
	POST_Record[29].NameLen = 6;
	POST_Record[29].FileIndex = 5;
	POST_Record[29].UpdateSelf = FALSE;
	POST_Record[29].PValue = &BUF_Text[15];
	pText = POST_Record[29].PValue;
	pText->CurrValue = postbuf_text15[0];
	pText->UserValue = postbuf_text15[1];
	pText->Offset = 6390;
	pText->DefaultLength = 11;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '6';
	pText->CurrValue[2] = '8';
	pText->CurrValue[3] = '.';
	pText->CurrValue[4] = '9';
	pText->CurrValue[5] = '5';
	pText->CurrValue[6] = '.';
	pText->CurrValue[7] = '1';
	pText->CurrValue[8] = '.';
	pText->CurrValue[9] = '1';
	pText->CurrValue[10] = '"';
	pText->CurrLength = 11;

	/* for record 30 */
	POST_Record[30].Occupy = 1;
	POST_Record[30].PostType = POST_TYPE_SELECT;
	POST_Record[30].Name[0] = 'c';
	POST_Record[30].Name[1] = 'o';
	POST_Record[30].Name[2] = 'n';
	POST_Record[30].Name[3] = 'n';
	POST_Record[30].Name[4] = 't';
	POST_Record[30].Name[5] = 'y';
	POST_Record[30].Name[6] = 'p';
	POST_Record[30].Name[7] = 'e';
	POST_Record[30].NameLen = 8;
	POST_Record[30].FileIndex = 5;
	POST_Record[30].UpdateSelf = FALSE;
	POST_Record[30].PValue = &BUF_Select[11];
	pSelect = POST_Record[30].PValue;
	pSelect->Offset[0] = 6630;
	pSelect->Offset[1] = 6691;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 31 */
	POST_Record[31].Occupy = 1;
	POST_Record[31].PostType = POST_TYPE_TEXT;
	POST_Record[31].Name[0] = 't';
	POST_Record[31].Name[1] = 'x';
	POST_Record[31].Name[2] = 't';
	POST_Record[31].Name[3] = 'i';
	POST_Record[31].Name[4] = 'm';
	POST_Record[31].Name[5] = 'e';
	POST_Record[31].Name[6] = 'r';
	POST_Record[31].NameLen = 7;
	POST_Record[31].FileIndex = 5;
	POST_Record[31].UpdateSelf = FALSE;
	POST_Record[31].PValue = &BUF_Text[16];
	pText = POST_Record[31].PValue;
	pText->CurrValue = postbuf_text16[0];
	pText->UserValue = postbuf_text16[1];
	pText->Offset = 6923;
	pText->DefaultLength = 4;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '0';
	pText->CurrValue[2] = '0';
	pText->CurrValue[3] = '"';
	pText->CurrLength = 4;

	/* for record 32 */
	POST_Record[32].Occupy = 1;
	POST_Record[32].PostType = POST_TYPE_SELECT;
	POST_Record[32].Name[0] = 'c';
	POST_Record[32].Name[1] = 's';
	POST_Record[32].Name[2] = '_';
	POST_Record[32].Name[3] = 'm';
	POST_Record[32].Name[4] = 'o';
	POST_Record[32].Name[5] = 'd';
	POST_Record[32].Name[6] = 'e';
	POST_Record[32].NameLen = 7;
	POST_Record[32].FileIndex = 5;
	POST_Record[32].UpdateSelf = FALSE;
	POST_Record[32].PValue = &BUF_Select[12];
	pSelect = POST_Record[32].PValue;
	pSelect->Offset[0] = 7308;
	pSelect->Offset[1] = 7372;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 33 */
	POST_Record[33].Occupy = 1;
	POST_Record[33].PostType = POST_TYPE_TEXT;
	POST_Record[33].Name[0] = 's';
	POST_Record[33].Name[1] = '_';
	POST_Record[33].Name[2] = 'l';
	POST_Record[33].Name[3] = 's';
	POST_Record[33].Name[4] = 't';
	POST_Record[33].Name[5] = 'p';
	POST_Record[33].Name[6] = 'o';
	POST_Record[33].Name[7] = 'r';
	POST_Record[33].Name[8] = 't';
	POST_Record[33].NameLen = 9;
	POST_Record[33].FileIndex = 5;
	POST_Record[33].UpdateSelf = FALSE;
	POST_Record[33].PValue = &BUF_Text[17];
	pText = POST_Record[33].PValue;
	pText->CurrValue = postbuf_text17[0];
	pText->UserValue = postbuf_text17[1];
	pText->Offset = 7611;
	pText->DefaultLength = 5;
	pText->CurrValue[0] = '5';
	pText->CurrValue[1] = '0';
	pText->CurrValue[2] = '0';
	pText->CurrValue[3] = '0';
	pText->CurrValue[4] = '"';
	pText->CurrLength = 5;

	/* for record 34 */
	POST_Record[34].Occupy = 1;
	POST_Record[34].PostType = POST_TYPE_TEXT;
	POST_Record[34].Name[0] = 'c';
	POST_Record[34].Name[1] = '_';
	POST_Record[34].Name[2] = 'd';
	POST_Record[34].Name[3] = 'e';
	POST_Record[34].Name[4] = 's';
	POST_Record[34].Name[5] = 'h';
	POST_Record[34].Name[6] = 'n';
	POST_Record[34].NameLen = 7;
	POST_Record[34].FileIndex = 5;
	POST_Record[34].UpdateSelf = FALSE;
	POST_Record[34].PValue = &BUF_Text[18];
	pText = POST_Record[34].PValue;
	pText->CurrValue = postbuf_text18[0];
	pText->UserValue = postbuf_text18[1];
	pText->Offset = 7986;
	pText->DefaultLength = 12;
	pText->CurrValue[0] = 'a';
	pText->CurrValue[1] = 's';
	pText->CurrValue[2] = 'i';
	pText->CurrValue[3] = 'x';
	pText->CurrValue[4] = '.';
	pText->CurrValue[5] = 'c';
	pText->CurrValue[6] = 'o';
	pText->CurrValue[7] = 'm';
	pText->CurrValue[8] = '.';
	pText->CurrValue[9] = 't';
	pText->CurrValue[10] = 'w';
	pText->CurrValue[11] = '"';
	pText->CurrLength = 12;

	/* for record 35 */
	POST_Record[35].Occupy = 1;
	POST_Record[35].PostType = POST_TYPE_TEXT;
	POST_Record[35].Name[0] = 'c';
	POST_Record[35].Name[1] = '_';
	POST_Record[35].Name[2] = 'd';
	POST_Record[35].Name[3] = 'e';
	POST_Record[35].Name[4] = 's';
	POST_Record[35].Name[5] = 'p';
	POST_Record[35].Name[6] = 'o';
	POST_Record[35].Name[7] = 'r';
	POST_Record[35].Name[8] = 't';
	POST_Record[35].NameLen = 9;
	POST_Record[35].FileIndex = 5;
	POST_Record[35].UpdateSelf = FALSE;
	POST_Record[35].PValue = &BUF_Text[19];
	pText = POST_Record[35].PValue;
	pText->CurrValue = postbuf_text19[0];
	pText->UserValue = postbuf_text19[1];
	pText->Offset = 8388;
	pText->DefaultLength = 5;
	pText->CurrValue[0] = '5';
	pText->CurrValue[1] = '0';
	pText->CurrValue[2] = '0';
	pText->CurrValue[3] = '0';
	pText->CurrValue[4] = '"';
	pText->CurrLength = 5;

	/* for record 36 */
	POST_Record[36].Occupy = 1;
	POST_Record[36].PostType = POST_TYPE_SUBMIT;
	POST_Record[36].Name[0] = 'r';
	POST_Record[36].Name[1] = 'e';
	POST_Record[36].Name[2] = 's';
	POST_Record[36].Name[3] = 't';
	POST_Record[36].Name[4] = 'o';
	POST_Record[36].Name[5] = 'r';
	POST_Record[36].Name[6] = 'e';
	POST_Record[36].NameLen = 7;
	POST_Record[36].FileIndex = 0xff;
	POST_Record[36].UpdateSelf = FALSE;
	POST_Record[36].PValue = &BUF_Submit[2];
	pSubmit = POST_Record[36].PValue;
	pSubmit->Value[0] = 'R';
	pSubmit->Value[1] = 'e';
	pSubmit->Value[2] = 's';
	pSubmit->Value[3] = 't';
	pSubmit->Value[4] = 'o';
	pSubmit->Value[5] = 'r';
	pSubmit->Value[6] = 'e';
	pSubmit->Value[7] = '_';
	pSubmit->Value[8] = 'd';
	pSubmit->Value[9] = 'e';
	pSubmit->Value[10] = 'f';
	pSubmit->Value[11] = 'a';
	pSubmit->Value[12] = 'u';
	pSubmit->Value[13] = 'l';
	pSubmit->Value[14] = 't';
	pSubmit->DefaultVlaueLen = 15;
	pSubmit->IsApply = 1;

	/* for record 37 */
	POST_Record[37].Occupy = 1;
	POST_Record[37].PostType = POST_TYPE_SUBMIT;
	POST_Record[37].Name[0] = 'r';
	POST_Record[37].Name[1] = 'e';
	POST_Record[37].Name[2] = 'b';
	POST_Record[37].Name[3] = 'o';
	POST_Record[37].Name[4] = 'o';
	POST_Record[37].Name[5] = 't';
	POST_Record[37].NameLen = 6;
	POST_Record[37].FileIndex = 0xff;
	POST_Record[37].UpdateSelf = FALSE;
	POST_Record[37].PValue = &BUF_Submit[3];
	pSubmit = POST_Record[37].PValue;
	pSubmit->Value[0] = 'R';
	pSubmit->Value[1] = 'e';
	pSubmit->Value[2] = 'b';
	pSubmit->Value[3] = 'o';
	pSubmit->Value[4] = 'o';
	pSubmit->Value[5] = 't';
	pSubmit->DefaultVlaueLen = 6;
	pSubmit->IsApply = 1;

	/* for record 38 */
	POST_Record[38].Occupy = 1;
	POST_Record[38].PostType = POST_TYPE_TEXT;
	POST_Record[38].Name[0] = 'u';
	POST_Record[38].Name[1] = 's';
	POST_Record[38].Name[2] = 'e';
	POST_Record[38].Name[3] = 'r';
	POST_Record[38].Name[4] = 'n';
	POST_Record[38].Name[5] = 'a';
	POST_Record[38].Name[6] = 'm';
	POST_Record[38].Name[7] = 'e';
	POST_Record[38].NameLen = 8;
	POST_Record[38].FileIndex = 6;
	POST_Record[38].UpdateSelf = FALSE;
	POST_Record[38].PValue = &BUF_Text[20];
	pText = POST_Record[38].PValue;
	pText->CurrValue = postbuf_text20[0];
	pText->UserValue = postbuf_text20[1];
	pText->Offset = 1774;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 39 */
	POST_Record[39].Occupy = 1;
	POST_Record[39].PostType = POST_TYPE_PASSWORD;
	POST_Record[39].Name[0] = 'p';
	POST_Record[39].Name[1] = 'a';
	POST_Record[39].Name[2] = 's';
	POST_Record[39].Name[3] = 's';
	POST_Record[39].Name[4] = 'w';
	POST_Record[39].Name[5] = 'o';
	POST_Record[39].Name[6] = 'r';
	POST_Record[39].Name[7] = 'd';
	POST_Record[39].NameLen = 8;
	POST_Record[39].FileIndex = 6;
	POST_Record[39].UpdateSelf = FALSE;
	POST_Record[39].PValue = &BUF_Password[0];
	pPassword = POST_Record[39].PValue;
	pPassword->CurrValue = postbuf_password0[0];
	pPassword->UserValue = postbuf_password0[1];
	pPassword->Offset = 2066;
	pPassword->DefaultLength = 1;
	pPassword->CurrValue[0] = '"';
	pPassword->CurrLength = 1;

	/* for record 40 */
	POST_Record[40].Occupy = 1;
	POST_Record[40].PostType = POST_TYPE_SUBMIT;
	POST_Record[40].Name[0] = 'L';
	POST_Record[40].Name[1] = 'o';
	POST_Record[40].Name[2] = 'g';
	POST_Record[40].Name[3] = 'i';
	POST_Record[40].Name[4] = 'n';
	POST_Record[40].NameLen = 5;
	POST_Record[40].FileIndex = 0xff;
	POST_Record[40].UpdateSelf = FALSE;
	POST_Record[40].PValue = &BUF_Submit[4];
	pSubmit = POST_Record[40].PValue;
	pSubmit->Value[0] = 'L';
	pSubmit->Value[1] = 'o';
	pSubmit->Value[2] = 'g';
	pSubmit->Value[3] = 'i';
	pSubmit->Value[4] = 'n';
	pSubmit->DefaultVlaueLen = 5;
	pSubmit->IsApply = 1;

	/* for record 41 */
	POST_Record[41].Occupy = 1;
	POST_Record[41].PostType = POST_TYPE_TEXT;
	POST_Record[41].Name[0] = 'n';
	POST_Record[41].Name[1] = 'e';
	POST_Record[41].Name[2] = 'w';
	POST_Record[41].Name[3] = '_';
	POST_Record[41].Name[4] = 'u';
	POST_Record[41].Name[5] = 's';
	POST_Record[41].Name[6] = 'n';
	POST_Record[41].NameLen = 7;
	POST_Record[41].FileIndex = 7;
	POST_Record[41].UpdateSelf = FALSE;
	POST_Record[41].PValue = &BUF_Text[21];
	pText = POST_Record[41].PValue;
	pText->CurrValue = postbuf_text21[0];
	pText->UserValue = postbuf_text21[1];
	pText->Offset = 2542;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 42 */
	POST_Record[42].Occupy = 1;
	POST_Record[42].PostType = POST_TYPE_PASSWORD;
	POST_Record[42].Name[0] = 'o';
	POST_Record[42].Name[1] = 'l';
	POST_Record[42].Name[2] = 'd';
	POST_Record[42].Name[3] = '_';
	POST_Record[42].Name[4] = 'p';
	POST_Record[42].Name[5] = 's';
	POST_Record[42].Name[6] = 'w';
	POST_Record[42].NameLen = 7;
	POST_Record[42].FileIndex = 7;
	POST_Record[42].UpdateSelf = FALSE;
	POST_Record[42].PValue = &BUF_Password[1];
	pPassword = POST_Record[42].PValue;
	pPassword->CurrValue = postbuf_password1[0];
	pPassword->UserValue = postbuf_password1[1];
	pPassword->Offset = 3279;
	pPassword->DefaultLength = 1;
	pPassword->CurrValue[0] = '"';
	pPassword->CurrLength = 1;

	/* for record 43 */
	POST_Record[43].Occupy = 1;
	POST_Record[43].PostType = POST_TYPE_PASSWORD;
	POST_Record[43].Name[0] = 'n';
	POST_Record[43].Name[1] = 'e';
	POST_Record[43].Name[2] = 'w';
	POST_Record[43].Name[3] = '_';
	POST_Record[43].Name[4] = 'p';
	POST_Record[43].Name[5] = 's';
	POST_Record[43].Name[6] = 'w';
	POST_Record[43].NameLen = 7;
	POST_Record[43].FileIndex = 7;
	POST_Record[43].UpdateSelf = FALSE;
	POST_Record[43].PValue = &BUF_Password[2];
	pPassword = POST_Record[43].PValue;
	pPassword->CurrValue = postbuf_password2[0];
	pPassword->UserValue = postbuf_password2[1];
	pPassword->Offset = 3498;
	pPassword->DefaultLength = 1;
	pPassword->CurrValue[0] = '"';
	pPassword->CurrLength = 1;

	/* for record 44 */
	POST_Record[44].Occupy = 1;
	POST_Record[44].PostType = POST_TYPE_PASSWORD;
	POST_Record[44].Name[0] = 'c';
	POST_Record[44].Name[1] = 'f';
	POST_Record[44].Name[2] = 'm';
	POST_Record[44].Name[3] = '_';
	POST_Record[44].Name[4] = 'p';
	POST_Record[44].Name[5] = 's';
	POST_Record[44].Name[6] = 'w';
	POST_Record[44].NameLen = 7;
	POST_Record[44].FileIndex = 7;
	POST_Record[44].UpdateSelf = FALSE;
	POST_Record[44].PValue = &BUF_Password[3];
	pPassword = POST_Record[44].PValue;
	pPassword->CurrValue = postbuf_password3[0];
	pPassword->UserValue = postbuf_password3[1];
	pPassword->Offset = 3721;
	pPassword->DefaultLength = 1;
	pPassword->CurrValue[0] = '"';
	pPassword->CurrLength = 1;

	/* for record 45 */
	POST_Record[45].Occupy = 1;
	POST_Record[45].PostType = POST_TYPE_TEXT;
	POST_Record[45].Name[0] = 'a';
	POST_Record[45].Name[1] = 'c';
	POST_Record[45].Name[2] = 'c';
	POST_Record[45].Name[3] = '_';
	POST_Record[45].Name[4] = 'i';
	POST_Record[45].Name[5] = 'p';
	POST_Record[45].Name[6] = '1';
	POST_Record[45].NameLen = 7;
	POST_Record[45].FileIndex = 7;
	POST_Record[45].UpdateSelf = FALSE;
	POST_Record[45].PValue = &BUF_Text[22];
	pText = POST_Record[45].PValue;
	pText->CurrValue = postbuf_text22[0];
	pText->UserValue = postbuf_text22[1];
	pText->Offset = 4470;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 46 */
	POST_Record[46].Occupy = 1;
	POST_Record[46].PostType = POST_TYPE_TEXT;
	POST_Record[46].Name[0] = 'a';
	POST_Record[46].Name[1] = 'c';
	POST_Record[46].Name[2] = 'c';
	POST_Record[46].Name[3] = '_';
	POST_Record[46].Name[4] = 'i';
	POST_Record[46].Name[5] = 'p';
	POST_Record[46].Name[6] = '2';
	POST_Record[46].NameLen = 7;
	POST_Record[46].FileIndex = 7;
	POST_Record[46].UpdateSelf = FALSE;
	POST_Record[46].PValue = &BUF_Text[23];
	pText = POST_Record[46].PValue;
	pText->CurrValue = postbuf_text23[0];
	pText->UserValue = postbuf_text23[1];
	pText->Offset = 4678;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 47 */
	POST_Record[47].Occupy = 1;
	POST_Record[47].PostType = POST_TYPE_TEXT;
	POST_Record[47].Name[0] = 'a';
	POST_Record[47].Name[1] = 'c';
	POST_Record[47].Name[2] = 'c';
	POST_Record[47].Name[3] = '_';
	POST_Record[47].Name[4] = 'i';
	POST_Record[47].Name[5] = 'p';
	POST_Record[47].Name[6] = '3';
	POST_Record[47].NameLen = 7;
	POST_Record[47].FileIndex = 7;
	POST_Record[47].UpdateSelf = FALSE;
	POST_Record[47].PValue = &BUF_Text[24];
	pText = POST_Record[47].PValue;
	pText->CurrValue = postbuf_text24[0];
	pText->UserValue = postbuf_text24[1];
	pText->Offset = 4886;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 48 */
	POST_Record[48].Occupy = 1;
	POST_Record[48].PostType = POST_TYPE_TEXT;
	POST_Record[48].Name[0] = 'a';
	POST_Record[48].Name[1] = 'c';
	POST_Record[48].Name[2] = 'c';
	POST_Record[48].Name[3] = '_';
	POST_Record[48].Name[4] = 'i';
	POST_Record[48].Name[5] = 'p';
	POST_Record[48].Name[6] = '4';
	POST_Record[48].NameLen = 7;
	POST_Record[48].FileIndex = 7;
	POST_Record[48].UpdateSelf = FALSE;
	POST_Record[48].PValue = &BUF_Text[25];
	pText = POST_Record[48].PValue;
	pText->CurrValue = postbuf_text25[0];
	pText->UserValue = postbuf_text25[1];
	pText->Offset = 5094;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 49 */
	POST_Record[49].Occupy = 1;
	POST_Record[49].PostType = POST_TYPE_SELECT;
	POST_Record[49].Name[0] = 'a';
	POST_Record[49].Name[1] = 'c';
	POST_Record[49].Name[2] = 'c';
	POST_Record[49].Name[3] = '_';
	POST_Record[49].Name[4] = 'i';
	POST_Record[49].Name[5] = 'p';
	POST_Record[49].Name[6] = 'e';
	POST_Record[49].Name[7] = 'n';
	POST_Record[49].NameLen = 8;
	POST_Record[49].FileIndex = 7;
	POST_Record[49].UpdateSelf = FALSE;
	POST_Record[49].PValue = &BUF_Select[13];
	pSelect = POST_Record[49].PValue;
	pSelect->Offset[0] = 5448;
	pSelect->Offset[1] = 5503;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 50 */
	POST_Record[50].Occupy = 1;
	POST_Record[50].PostType = POST_TYPE_SELECT;
	POST_Record[50].Name[0] = 'n';
	POST_Record[50].Name[1] = 'e';
	POST_Record[50].Name[2] = 't';
	POST_Record[50].Name[3] = 'w';
	POST_Record[50].Name[4] = 'o';
	POST_Record[50].Name[5] = 'r';
	POST_Record[50].Name[6] = 'k';
	POST_Record[50].Name[7] = '_';
	POST_Record[50].Name[8] = 'm';
	POST_Record[50].Name[9] = 'o';
	POST_Record[50].Name[10] = 'd';
	POST_Record[50].Name[11] = 'e';
	POST_Record[50].NameLen = 12;
	POST_Record[50].FileIndex = 11;
	POST_Record[50].UpdateSelf = FALSE;
	POST_Record[50].PValue = &BUF_Select[14];
	pSelect = POST_Record[50].PValue;
	pSelect->Offset[0] = 2525;
	pSelect->Offset[1] = 2593;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 51 */
	POST_Record[51].Occupy = 1;
	POST_Record[51].PostType = POST_TYPE_SELECT;
	POST_Record[51].Name[0] = 'c';
	POST_Record[51].Name[1] = 'h';
	POST_Record[51].Name[2] = 'a';
	POST_Record[51].Name[3] = 'n';
	POST_Record[51].Name[4] = 'n';
	POST_Record[51].Name[5] = 'e';
	POST_Record[51].Name[6] = 'l';
	POST_Record[51].NameLen = 7;
	POST_Record[51].FileIndex = 11;
	POST_Record[51].UpdateSelf = FALSE;
	POST_Record[51].PValue = &BUF_Select[15];
	pSelect = POST_Record[51].PValue;
	pSelect->Offset[0] = 2821;
	pSelect->Offset[1] = 2879;
	pSelect->Offset[2] = 2925;
	pSelect->Offset[3] = 2971;
	pSelect->Offset[4] = 3017;
	pSelect->Offset[5] = 3063;
	pSelect->Offset[6] = 3109;
	pSelect->Offset[7] = 3155;
	pSelect->Offset[8] = 3201;
	pSelect->Offset[9] = 3247;
	pSelect->Offset[10] = 3293;
	pSelect->Offset[11] = 3340;
	pSelect->Count = 12;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 52 */
	POST_Record[52].Occupy = 1;
	POST_Record[52].PostType = POST_TYPE_TEXT;
	POST_Record[52].Name[0] = 's';
	POST_Record[52].Name[1] = 's';
	POST_Record[52].Name[2] = 'i';
	POST_Record[52].Name[3] = 'd';
	POST_Record[52].NameLen = 4;
	POST_Record[52].FileIndex = 11;
	POST_Record[52].UpdateSelf = FALSE;
	POST_Record[52].PValue = &BUF_Text[26];
	pText = POST_Record[52].PValue;
	pText->CurrValue = postbuf_text26[0];
	pText->UserValue = postbuf_text26[1];
	pText->Offset = 3568;
	pText->DefaultLength = 9;
	pText->CurrValue[0] = 'a';
	pText->CurrValue[1] = 's';
	pText->CurrValue[2] = 'i';
	pText->CurrValue[3] = 'x';
	pText->CurrValue[4] = 't';
	pText->CurrValue[5] = 'e';
	pText->CurrValue[6] = 's';
	pText->CurrValue[7] = 't';
	pText->CurrValue[8] = '"';
	pText->CurrLength = 9;

	/* for record 53 */
	POST_Record[53].Occupy = 1;
	POST_Record[53].PostType = POST_TYPE_SELECT;
	POST_Record[53].Name[0] = 's';
	POST_Record[53].Name[1] = 'e';
	POST_Record[53].Name[2] = 'c';
	POST_Record[53].Name[3] = 'u';
	POST_Record[53].Name[4] = 'r';
	POST_Record[53].Name[5] = 'i';
	POST_Record[53].Name[6] = 't';
	POST_Record[53].Name[7] = 'y';
	POST_Record[53].Name[8] = '_';
	POST_Record[53].Name[9] = 'm';
	POST_Record[53].Name[10] = 'o';
	POST_Record[53].Name[11] = 'd';
	POST_Record[53].Name[12] = 'e';
	POST_Record[53].NameLen = 13;
	POST_Record[53].FileIndex = 11;
	POST_Record[53].UpdateSelf = FALSE;
	POST_Record[53].PValue = &BUF_Select[16];
	pSelect = POST_Record[53].PValue;
	pSelect->Offset[0] = 3803;
	pSelect->Offset[1] = 3868;
	pSelect->Offset[2] = 3919;
	pSelect->Offset[3] = 3971;
	pSelect->Offset[4] = 4020;
	pSelect->Count = 5;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 54 */
	POST_Record[54].Occupy = 1;
	POST_Record[54].PostType = POST_TYPE_SELECT;
	POST_Record[54].Name[0] = 'k';
	POST_Record[54].Name[1] = 'e';
	POST_Record[54].Name[2] = 'y';
	POST_Record[54].Name[3] = '_';
	POST_Record[54].Name[4] = 'l';
	POST_Record[54].Name[5] = 'e';
	POST_Record[54].Name[6] = 'n';
	POST_Record[54].Name[7] = 'g';
	POST_Record[54].Name[8] = 't';
	POST_Record[54].Name[9] = 'h';
	POST_Record[54].NameLen = 10;
	POST_Record[54].FileIndex = 11;
	POST_Record[54].UpdateSelf = FALSE;
	POST_Record[54].PValue = &BUF_Select[17];
	pSelect = POST_Record[54].PValue;
	pSelect->Offset[0] = 4804;
	pSelect->Offset[1] = 4865;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 55 */
	POST_Record[55].Occupy = 1;
	POST_Record[55].PostType = POST_TYPE_SELECT;
	POST_Record[55].Name[0] = 'k';
	POST_Record[55].Name[1] = 'e';
	POST_Record[55].Name[2] = 'y';
	POST_Record[55].Name[3] = '_';
	POST_Record[55].Name[4] = 'i';
	POST_Record[55].Name[5] = 'n';
	POST_Record[55].Name[6] = 'd';
	POST_Record[55].Name[7] = 'e';
	POST_Record[55].Name[8] = 'x';
	POST_Record[55].NameLen = 9;
	POST_Record[55].FileIndex = 11;
	POST_Record[55].UpdateSelf = FALSE;
	POST_Record[55].PValue = &BUF_Select[18];
	pSelect = POST_Record[55].PValue;
	pSelect->Offset[0] = 5106;
	pSelect->Offset[1] = 5171;
	pSelect->Offset[2] = 5227;
	pSelect->Offset[3] = 5283;
	pSelect->Count = 4;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 56 */
	POST_Record[56].Occupy = 1;
	POST_Record[56].PostType = POST_TYPE_TEXT;
	POST_Record[56].Name[0] = 'k';
	POST_Record[56].Name[1] = 'e';
	POST_Record[56].Name[2] = 'y';
	POST_Record[56].Name[3] = '_';
	POST_Record[56].Name[4] = 'i';
	POST_Record[56].Name[5] = 'n';
	POST_Record[56].Name[6] = 'd';
	POST_Record[56].Name[7] = 'e';
	POST_Record[56].Name[8] = 'x';
	POST_Record[56].Name[9] = '_';
	POST_Record[56].Name[10] = '0';
	POST_Record[56].NameLen = 11;
	POST_Record[56].FileIndex = 11;
	POST_Record[56].UpdateSelf = FALSE;
	POST_Record[56].PValue = &BUF_Text[27];
	pText = POST_Record[56].PValue;
	pText->CurrValue = postbuf_text27[0];
	pText->UserValue = postbuf_text27[1];
	pText->Offset = 5533;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 57 */
	POST_Record[57].Occupy = 1;
	POST_Record[57].PostType = POST_TYPE_TEXT;
	POST_Record[57].Name[0] = 'k';
	POST_Record[57].Name[1] = 'e';
	POST_Record[57].Name[2] = 'y';
	POST_Record[57].Name[3] = '_';
	POST_Record[57].Name[4] = 'i';
	POST_Record[57].Name[5] = 'n';
	POST_Record[57].Name[6] = 'd';
	POST_Record[57].Name[7] = 'e';
	POST_Record[57].Name[8] = 'x';
	POST_Record[57].Name[9] = '_';
	POST_Record[57].Name[10] = '1';
	POST_Record[57].NameLen = 11;
	POST_Record[57].FileIndex = 11;
	POST_Record[57].UpdateSelf = FALSE;
	POST_Record[57].PValue = &BUF_Text[28];
	pText = POST_Record[57].PValue;
	pText->CurrValue = postbuf_text28[0];
	pText->UserValue = postbuf_text28[1];
	pText->Offset = 5766;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 58 */
	POST_Record[58].Occupy = 1;
	POST_Record[58].PostType = POST_TYPE_TEXT;
	POST_Record[58].Name[0] = 'k';
	POST_Record[58].Name[1] = 'e';
	POST_Record[58].Name[2] = 'y';
	POST_Record[58].Name[3] = '_';
	POST_Record[58].Name[4] = 'i';
	POST_Record[58].Name[5] = 'n';
	POST_Record[58].Name[6] = 'd';
	POST_Record[58].Name[7] = 'e';
	POST_Record[58].Name[8] = 'x';
	POST_Record[58].Name[9] = '_';
	POST_Record[58].Name[10] = '2';
	POST_Record[58].NameLen = 11;
	POST_Record[58].FileIndex = 11;
	POST_Record[58].UpdateSelf = FALSE;
	POST_Record[58].PValue = &BUF_Text[29];
	pText = POST_Record[58].PValue;
	pText->CurrValue = postbuf_text29[0];
	pText->UserValue = postbuf_text29[1];
	pText->Offset = 5999;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 59 */
	POST_Record[59].Occupy = 1;
	POST_Record[59].PostType = POST_TYPE_TEXT;
	POST_Record[59].Name[0] = 'k';
	POST_Record[59].Name[1] = 'e';
	POST_Record[59].Name[2] = 'y';
	POST_Record[59].Name[3] = '_';
	POST_Record[59].Name[4] = 'i';
	POST_Record[59].Name[5] = 'n';
	POST_Record[59].Name[6] = 'd';
	POST_Record[59].Name[7] = 'e';
	POST_Record[59].Name[8] = 'x';
	POST_Record[59].Name[9] = '_';
	POST_Record[59].Name[10] = '3';
	POST_Record[59].NameLen = 11;
	POST_Record[59].FileIndex = 11;
	POST_Record[59].UpdateSelf = FALSE;
	POST_Record[59].PValue = &BUF_Text[30];
	pText = POST_Record[59].PValue;
	pText->CurrValue = postbuf_text30[0];
	pText->UserValue = postbuf_text30[1];
	pText->Offset = 6232;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 60 */
	POST_Record[60].Occupy = 1;
	POST_Record[60].PostType = POST_TYPE_TEXT;
	POST_Record[60].Name[0] = 'a';
	POST_Record[60].Name[1] = 'e';
	POST_Record[60].Name[2] = 's';
	POST_Record[60].Name[3] = '_';
	POST_Record[60].Name[4] = 'p';
	POST_Record[60].Name[5] = 'a';
	POST_Record[60].Name[6] = 's';
	POST_Record[60].Name[7] = 's';
	POST_Record[60].Name[8] = 'p';
	POST_Record[60].Name[9] = 'h';
	POST_Record[60].Name[10] = 'r';
	POST_Record[60].Name[11] = 'a';
	POST_Record[60].Name[12] = 's';
	POST_Record[60].Name[13] = 'e';
	POST_Record[60].NameLen = 14;
	POST_Record[60].FileIndex = 11;
	POST_Record[60].UpdateSelf = FALSE;
	POST_Record[60].PValue = &BUF_Text[31];
	pText = POST_Record[60].PValue;
	pText->CurrValue = postbuf_text31[0];
	pText->UserValue = postbuf_text31[1];
	pText->Offset = 7029;
	pText->DefaultLength = 1;
	pText->CurrValue[0] = '"';
	pText->CurrLength = 1;

	/* for record 61 */
	POST_Record[61].Occupy = 1;
	POST_Record[61].PostType = POST_TYPE_SELECT;
	POST_Record[61].Name[0] = 't';
	POST_Record[61].Name[1] = 'x';
	POST_Record[61].Name[2] = '_';
	POST_Record[61].Name[3] = 'd';
	POST_Record[61].Name[4] = 'a';
	POST_Record[61].Name[5] = 't';
	POST_Record[61].Name[6] = 'a';
	POST_Record[61].Name[7] = '_';
	POST_Record[61].Name[8] = 'r';
	POST_Record[61].Name[9] = 'a';
	POST_Record[61].Name[10] = 't';
	POST_Record[61].Name[11] = 'e';
	POST_Record[61].NameLen = 12;
	POST_Record[61].FileIndex = 11;
	POST_Record[61].UpdateSelf = FALSE;
	POST_Record[61].PValue = &BUF_Select[19];
	pSelect = POST_Record[61].PValue;
	pSelect->Offset[0] = 7910;
	pSelect->Offset[1] = 7968;
	pSelect->Offset[2] = 8019;
	pSelect->Offset[3] = 8070;
	pSelect->Offset[4] = 8123;
	pSelect->Offset[5] = 8174;
	pSelect->Offset[6] = 8225;
	pSelect->Offset[7] = 8277;
	pSelect->Offset[8] = 8329;
	pSelect->Offset[9] = 8381;
	pSelect->Offset[10] = 8433;
	pSelect->Offset[11] = 8485;
	pSelect->Offset[12] = 8537;
	pSelect->Count = 13;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 62 */
	POST_Record[62].Occupy = 1;
	POST_Record[62].PostType = POST_TYPE_SELECT;
	POST_Record[62].Name[0] = 't';
	POST_Record[62].Name[1] = 'x';
	POST_Record[62].Name[2] = '_';
	POST_Record[62].Name[3] = 'p';
	POST_Record[62].Name[4] = 'o';
	POST_Record[62].Name[5] = 'w';
	POST_Record[62].Name[6] = 'e';
	POST_Record[62].Name[7] = 'r';
	POST_Record[62].NameLen = 8;
	POST_Record[62].FileIndex = 11;
	POST_Record[62].UpdateSelf = FALSE;
	POST_Record[62].PValue = &BUF_Select[20];
	pSelect = POST_Record[62].PValue;
	pSelect->Offset[0] = 8781;
	pSelect->Offset[1] = 8839;
	pSelect->Offset[2] = 8887;
	pSelect->Offset[3] = 8935;
	pSelect->Offset[4] = 8985;
	pSelect->Count = 5;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 63 */
	POST_Record[63].Occupy = 1;
	POST_Record[63].PostType = POST_TYPE_SELECT;
	POST_Record[63].Name[0] = 'p';
	POST_Record[63].Name[1] = 'r';
	POST_Record[63].Name[2] = 'e';
	POST_Record[63].Name[3] = 'a';
	POST_Record[63].Name[4] = 'm';
	POST_Record[63].Name[5] = 'b';
	POST_Record[63].Name[6] = 'l';
	POST_Record[63].Name[7] = 'e';
	POST_Record[63].NameLen = 8;
	POST_Record[63].FileIndex = 11;
	POST_Record[63].UpdateSelf = FALSE;
	POST_Record[63].PValue = &BUF_Select[21];
	pSelect = POST_Record[63].PValue;
	pSelect->Offset[0] = 9216;
	pSelect->Offset[1] = 9274;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

	/* for record 64 */
	POST_Record[64].Occupy = 1;
	POST_Record[64].PostType = POST_TYPE_TEXT;
	POST_Record[64].Name[0] = 'b';
	POST_Record[64].Name[1] = 'e';
	POST_Record[64].Name[2] = 'a';
	POST_Record[64].Name[3] = 'c';
	POST_Record[64].Name[4] = 'o';
	POST_Record[64].Name[5] = 'n';
	POST_Record[64].Name[6] = '_';
	POST_Record[64].Name[7] = 'i';
	POST_Record[64].Name[8] = 'n';
	POST_Record[64].Name[9] = 't';
	POST_Record[64].Name[10] = 'v';
	POST_Record[64].NameLen = 11;
	POST_Record[64].FileIndex = 11;
	POST_Record[64].UpdateSelf = FALSE;
	POST_Record[64].PValue = &BUF_Text[32];
	pText = POST_Record[64].PValue;
	pText->CurrValue = postbuf_text32[0];
	pText->UserValue = postbuf_text32[1];
	pText->Offset = 9506;
	pText->DefaultLength = 4;
	pText->CurrValue[0] = '1';
	pText->CurrValue[1] = '0';
	pText->CurrValue[2] = '0';
	pText->CurrValue[3] = '"';
	pText->CurrLength = 4;

	/* for record 65 */
	POST_Record[65].Occupy = 1;
	POST_Record[65].PostType = POST_TYPE_TEXT;
	POST_Record[65].Name[0] = 'r';
	POST_Record[65].Name[1] = 't';
	POST_Record[65].Name[2] = 's';
	POST_Record[65].Name[3] = '_';
	POST_Record[65].Name[4] = 't';
	POST_Record[65].Name[5] = 'h';
	POST_Record[65].Name[6] = 'r';
	POST_Record[65].Name[7] = 'e';
	POST_Record[65].Name[8] = 's';
	POST_Record[65].Name[9] = 'h';
	POST_Record[65].Name[10] = 'o';
	POST_Record[65].Name[11] = 'l';
	POST_Record[65].Name[12] = 'd';
	POST_Record[65].NameLen = 13;
	POST_Record[65].FileIndex = 11;
	POST_Record[65].UpdateSelf = FALSE;
	POST_Record[65].PValue = &BUF_Text[33];
	pText = POST_Record[65].PValue;
	pText->CurrValue = postbuf_text33[0];
	pText->UserValue = postbuf_text33[1];
	pText->Offset = 9859;
	pText->DefaultLength = 5;
	pText->CurrValue[0] = '2';
	pText->CurrValue[1] = '4';
	pText->CurrValue[2] = '3';
	pText->CurrValue[3] = '2';
	pText->CurrValue[4] = '"';
	pText->CurrLength = 5;

	/* for record 66 */
	POST_Record[66].Occupy = 1;
	POST_Record[66].PostType = POST_TYPE_SELECT;
	POST_Record[66].Name[0] = 'a';
	POST_Record[66].Name[1] = 'u';
	POST_Record[66].Name[2] = 't';
	POST_Record[66].Name[3] = 'o';
	POST_Record[66].Name[4] = '_';
	POST_Record[66].Name[5] = 'p';
	POST_Record[66].Name[6] = 'w';
	POST_Record[66].Name[7] = 'r';
	POST_Record[66].Name[8] = '_';
	POST_Record[66].Name[9] = 'c';
	POST_Record[66].Name[10] = 't';
	POST_Record[66].Name[11] = 'r';
	POST_Record[66].Name[12] = 'l';
	POST_Record[66].NameLen = 13;
	POST_Record[66].FileIndex = 11;
	POST_Record[66].UpdateSelf = FALSE;
	POST_Record[66].PValue = &BUF_Select[22];
	pSelect = POST_Record[66].PValue;
	pSelect->Offset[0] = 10230;
	pSelect->Offset[1] = 10291;
	pSelect->Count = 2;
	pSelect->DefaultSet = 0;
	pSelect->CurrentSet = 0;
	pSelect->UserSet = 0;

}

/*
 * ----------------------------------------------------------------------------
 * Function Name: FSYS_FindFile
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T FSYS_FindFile(U8_T XDATA* pFName)
{
	U8_T		index, find;
	U8_T *pSour;
	U8_T *pDesc;

	for (index = 0; index < MAX_STORE_FILE_NUM; index++)
	{
		if (!FSYS_Manage[index].Occupy)
			continue;

		pSour = pFName;
		pDesc = FSYS_Manage[index].FName;
		find = 1;
		while (*pDesc != 0)
		{
			if (*pSour != *pDesc)
			{
				find = 0;
				break;
			}
			pSour++;
			pDesc++;
		}
		if (find == 1)
			return index;
	}
	return index;
}


/* End of filesys.c */