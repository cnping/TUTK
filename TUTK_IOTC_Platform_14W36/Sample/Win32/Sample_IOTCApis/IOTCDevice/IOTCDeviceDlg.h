// p2pDeviceDlg.h : header file
//

#pragma once
#include "afxwin.h"


// Cp2pDeviceDlg dialog
class Cp2pDeviceDlg : public CDialog
{
// Construction
public:
	Cp2pDeviceDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_P2PDEVICE_DIALOG };

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
	CString mPort;
	CString mMaster1;
	CString mMaster2;
	CString mUID;
	afx_msg void OnBnClickedButton1();
	CString mDebugPrint;
	afx_msg void OnBnClickedButton2();
	HANDLE mhListenThread;
	char mFlagExit;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit mCtrlDebugPrint;
	CString m_LoginInfo;
};
