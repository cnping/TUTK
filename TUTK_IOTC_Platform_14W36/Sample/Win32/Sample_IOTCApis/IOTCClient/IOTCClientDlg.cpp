// p2pClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IOTCClient.h"
#include "IOTCClientDlg.h"
#include "IOTCAPIs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// Cp2pClientDlg dialog




Cp2pClientDlg::Cp2pClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cp2pClientDlg::IDD, pParent)
	, mUID(_T(""))
	, mDeviceName(_T(""))
	, mDevicePWD(_T(""))
	, mDebugPrint(_T(""))
	, mUDPPort(_T(""))
	, mMaster1(_T(""))
	, mMaster2(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cp2pClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT3, mUID);
	DDX_Text(pDX, IDC_EDIT1, mDebugPrint);
	DDX_Text(pDX, IDC_EDIT2, mUDPPort);
	DDX_Text(pDX, IDC_EDIT6, mMaster1);
	DDX_Text(pDX, IDC_EDIT7, mMaster2);
	DDX_Control(pDX, IDC_EDIT1, mCtrlDebugPrint);
}

BEGIN_MESSAGE_MAP(Cp2pClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON2, &Cp2pClientDlg::OnBnClickedButton2)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// Cp2pClientDlg message handlers

BOOL Cp2pClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	mDeviceName = "AAAA0002";
	mDevicePWD = "12345678";
	mUID = "00000000000000000001";	 
	mMaster1 = "m1.iotcplatform.com";
	mMaster2 = "m2.iotcplatform.com";
	char buf[256];
	sprintf(buf,"%d",10000+GetTickCount()%20000);
	mUDPPort = buf;

	CString ss;
	this->GetWindowTextA(ss);
	unsigned long version;
	IOTC_Get_Version(&version);
	sprintf(buf,"%d.%d.%d.%d",(version & 0xFF000000) >> 24,(version & 0xFF0000) >> 16,(version & 0xFF00) >> 8,(version & 0xFF) >> 0 );
	ss = ss + _T("   -   IOTC Version: ") + (buf);
	this->SetWindowTextA(ss); 

	UpdateData(false);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cp2pClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Cp2pClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Cp2pClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



DWORD WINAPI ClientThread(void* arg)
{
	char buf[256], ReadBuf[IOTC_MAX_PACKET_SIZE];
	int nByte;	
	Cp2pClientDlg *p = (Cp2pClientDlg*) arg;
	int SID = p->mSID;

	sprintf(buf, "ClientThread Start!!\r\n");
	p->mDebugPrint =  p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);

	st_SInfo Sinfo;
	IOTC_Session_Check(SID, &Sinfo);

	if(Sinfo.Mode ==0)
		sprintf(buf, "Connected!! from %s:%d[%s] Mode=P2P\r\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	else
		sprintf(buf, "Connected!! from %s:%d[%s] Mode=RLY\r\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);

	p->mDebugPrint = p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);

	do{
		nByte = IOTC_Session_Read(SID, ReadBuf, IOTC_MAX_PACKET_SIZE, 1000,0);
		if(nByte>0){
			ReadBuf[nByte] = 0;
			sprintf(buf, "P2P_Session_Read %d:%s\r\n",nByte, ReadBuf);
			p->mDebugPrint =  p->mDebugPrint + buf;
			SetTimer(p->m_hWnd , 1, 100, NULL);

			// echo back
			nByte=IOTC_Session_Write(SID, ReadBuf, (int)strlen(ReadBuf),0);
		}else if(nByte==IOTC_ER_REMOTE_TIMEOUT_DISCONNECT ||
			nByte==IOTC_ER_SESSION_CLOSE_BY_REMOTE || nByte==IOTC_ER_INVALID_SID){
				break;
		}
	}while(1);

	sprintf(buf, "P2P_Session_Read ret=%d\r\n",nByte);
	p->mDebugPrint =  p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);

	IOTC_Session_Close(SID);

	sprintf(buf, "ClientThread Exit!!\r\n");
	p->mDebugPrint =  p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);
	return 0;
}

void Cp2pClientDlg::OnBnClickedButton2()
{
// TODO: 在此加入控制項告知處理常式程式碼
	this->UpdateData(true); 
	char buf[256];
	int ret=0;

	ret = IOTC_Initialize(atoi(mUDPPort), mMaster1.GetBuffer(), mMaster2.GetBuffer(),"m4.iotcplatform.com", "m5.iotcplatform.com");
	sprintf(buf, "P2P_Initialize() ret = %d\r\n",ret);
	mDebugPrint = mDebugPrint + buf;
	this->UpdateData(false); 
	if(!((ret == IOTC_ER_NoERROR) || (ret == IOTC_ER_ALREADY_INITIALIZED)))
		return;

	mSID = IOTC_Connect_ByUID(mUID.GetBuffer());
	sprintf(buf, "P2P_Connect_ByUID() ret = %d\r\n",mSID);
	mDebugPrint = mDebugPrint + buf;
	this->UpdateData(false); 
	if(mSID < 0)
		return;

	// create a thread to reveive data and each back
	mhClientThread = CreateThread(NULL, 0, ClientThread, this, 0, NULL); 
}

void Cp2pClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此加入您的訊息處理常式程式碼和 (或) 呼叫預設值
	this->UpdateData(false); 
	mCtrlDebugPrint.LineScroll(mCtrlDebugPrint.GetLineCount(), 0);
	KillTimer(1);
	CDialog::OnTimer(nIDEvent);
}
