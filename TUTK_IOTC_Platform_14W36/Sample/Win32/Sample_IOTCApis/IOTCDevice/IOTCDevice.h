// p2pDevice.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cp2pDeviceApp:
// See p2pDevice.cpp for the implementation of this class
//

class Cp2pDeviceApp : public CWinApp
{
public:
	Cp2pDeviceApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cp2pDeviceApp theApp;