// p2pClient.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cp2pClientApp:
// See p2pClient.cpp for the implementation of this class
//

class Cp2pClientApp : public CWinApp
{
public:
	Cp2pClientApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cp2pClientApp theApp;