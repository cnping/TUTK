// p2pClientDlg.h : header file
//

#pragma once
#include "afxwin.h"


// Cp2pClientDlg dialog
class Cp2pClientDlg : public CDialog
{
// Construction
public:
	Cp2pClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_P2PCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	CString mUID;
	CString mDeviceName;
	CString mDevicePWD;
	CString mDebugPrint;
	CString mUDPPort;
	CString mMaster1;
	CString mMaster2;
	int mSID;
	char mFlagExit;
	HANDLE mhClientThread;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit mCtrlDebugPrint;
};
