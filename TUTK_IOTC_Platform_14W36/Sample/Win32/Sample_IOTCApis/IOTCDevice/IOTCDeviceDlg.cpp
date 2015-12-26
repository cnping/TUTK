// p2pDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IOTCDevice.h"
#include "IOTCDeviceDlg.h"
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


// Cp2pDeviceDlg dialog




Cp2pDeviceDlg::Cp2pDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cp2pDeviceDlg::IDD, pParent)
	, mPort(_T(""))
	, mMaster1(_T(""))
	, mMaster2(_T(""))
	, mUID(_T(""))
	, mDebugPrint(_T(""))
	, m_LoginInfo(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cp2pDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, mPort);
	DDX_Text(pDX, IDC_EDIT2, mMaster1);
	DDX_Text(pDX, IDC_EDIT4, mMaster2);
	DDX_Text(pDX, IDC_EDIT3, mUID);
	DDX_Text(pDX, IDC_EDIT5, mDebugPrint);
	DDX_Control(pDX, IDC_EDIT5, mCtrlDebugPrint);
	DDX_Text(pDX, IDC_EDIT6, m_LoginInfo);
}

BEGIN_MESSAGE_MAP(Cp2pDeviceDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &Cp2pDeviceDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &Cp2pDeviceDlg::OnBnClickedButton2)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// Cp2pDeviceDlg message handlers

BOOL Cp2pDeviceDlg::OnInitDialog()
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
	mUID = "00000000000000000001";
	mMaster1 = "m1.iotcplatform.com";
	mMaster2 = "m2.iotcplatform.com";
	char buf[256];
	sprintf(buf,"%d",10000+GetTickCount()%20000);
	mPort = buf;

	this->UpdateData(false); 
	this->GetDlgItem(IDC_BUTTON1)->EnableWindow(true); 
	this->GetDlgItem(IDC_BUTTON2)->EnableWindow(false);

	CString ss;
	this->GetWindowTextA(ss);
	unsigned long version;
	IOTC_Get_Version(&version);
	sprintf(buf,"%d.%d.%d.%d",(version & 0xFF000000) >> 24,(version & 0xFF0000) >> 16,(version & 0xFF00) >> 8,(version & 0xFF) >> 0 );
	ss = ss + _T("   -   IOTC Version: ") + (buf);
	this->SetWindowTextA(ss); 

	SetTimer(2,120000,NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cp2pDeviceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Cp2pDeviceDlg::OnPaint()
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
HCURSOR Cp2pDeviceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

struct Parameter 
{
	Cp2pDeviceDlg *p;
	int SID;
};

DWORD WINAPI ProcessThread(void* arg)
{
	char buf[256], ReadBuf[IOTC_MAX_PACKET_SIZE];
	struct Parameter *pp = (struct Parameter *)arg;
	Cp2pDeviceDlg *p = pp->p;
	int SID , i, nByte;

	sprintf(buf, "ProcessThread Start!!\r\n");
	p->mDebugPrint =  p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);

	SID = pp->SID ;
	st_SInfo Sinfo;
	IOTC_Session_Check(SID, &Sinfo);

	if(Sinfo.Mode ==0)
		sprintf(buf, "Connected!! SID=%d from %s:%d[%s] Mode=P2P\r\n",SID, Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	else
		sprintf(buf, "Connected!! SID=%d from %s:%d[%s] Mode=RLY\r\n",SID, Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
	p->mDebugPrint = p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);
	for(i = 0 ; i < 3 ; i++)
	{
		sprintf(buf, "P2P Hello World: %d",i);
		IOTC_Session_Write(SID, buf, (int)strlen(buf),0);
		Sleep(10);
	}
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

	sprintf(buf, "ProcessThread Exit!!\r\n");
	p->mDebugPrint =  p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);

	return 0;
}



DWORD WINAPI ListenThread(void* arg)
{
	char buf[256];
	int SID;
	Cp2pDeviceDlg *p = (Cp2pDeviceDlg*) arg;
	sprintf(buf, "ListenThread Start!!\r\n");
	p->mDebugPrint =  p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);
	while(!p->mFlagExit)
	{
		SID = IOTC_Listen(1000);  // Listen 1 sec
		if(SID == IOTC_ER_TIMEOUT)
			continue;
		else if(SID >= 0)
		{
			struct Parameter pp;
			pp.p = p;
			pp.SID = SID;
			CreateThread(NULL, 0, ProcessThread, &pp, 0, NULL); 
		}
		else
		{
			sprintf(buf, "P2P_Listen ret = %d\r\n",SID);
			p->mDebugPrint =  p->mDebugPrint + buf;
			SetTimer(p->m_hWnd , 1, 100, NULL);
		}
	}
	sprintf(buf, "ListenThread Exit!!\r\n");
	p->mDebugPrint =  p->mDebugPrint + buf;
	SetTimer(p->m_hWnd , 1, 100, NULL);
	return 0;
}

void Cp2pDeviceDlg::OnBnClickedButton1()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	this->GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
	this->GetDlgItem(IDC_BUTTON2)->EnableWindow(false);
	this->UpdateData(true); 
	mDebugPrint = "";
	char buf[256];
	int ret=0;

	ret = IOTC_Initialize(atoi(mPort), mMaster1.GetBuffer(), mMaster2.GetBuffer(),"m4.iotcplatform.com", "m5.iotcplatform.com");
	sprintf(buf, "P2P_Initialize() ret = %d\r\n",ret);
	mDebugPrint = buf;
	this->UpdateData(false); 
	if(ret != IOTC_ER_NoERROR)
		return;

	ret = IOTC_Device_Login(mUID.GetBuffer(),NULL,NULL); 
	sprintf(buf, "P2P_Device_Login() ret = %d\r\n",ret);
	mDebugPrint = mDebugPrint + buf;
	this->UpdateData(false); 

	if(ret != IOTC_ER_NoERROR)
	{
		//return;  // If you want to use in LAN mode, comment this line ...
	}

	// start up listen thread
	mFlagExit = 0;
	mhListenThread = CreateThread(NULL, 0, ListenThread, this, 0, NULL); 

	this->GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
	this->GetDlgItem(IDC_BUTTON2)->EnableWindow(true);

	unsigned long info;
	IOTC_Get_Login_Info(&info);
	sprintf(buf, "0x%08X", info);
	m_LoginInfo = _T(buf);
	this->GetDlgItem(IDC_EDIT6)->SetWindowTextA(m_LoginInfo); 
}

void Cp2pDeviceDlg::OnBnClickedButton2()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	this->GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
	this->GetDlgItem(IDC_BUTTON2)->EnableWindow(false);
	this->UpdateData(true); 
	char buf[256];
	int ret=0;
	mFlagExit = 1;
	Sleep(1100);

	ret = IOTC_DeInitialize();
	sprintf(buf, "P2P_DeInitialize() ret = %d\r\n",ret);
	mDebugPrint = mDebugPrint + buf;
	this->UpdateData(false); 
	this->GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
	this->GetDlgItem(IDC_BUTTON2)->EnableWindow(false);
}

void Cp2pDeviceDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此加入您的訊息處理常式程式碼和 (或) 呼叫預設值
	
	switch(nIDEvent)
	{
	case 1:
		this->UpdateData(false);
		mCtrlDebugPrint.LineScroll(mCtrlDebugPrint.GetLineCount(), 0);
		KillTimer(1);
		break;
	case 2:
		unsigned long info;
		IOTC_Get_Login_Info(&info);
		char buf[256];
		sprintf(buf, "0x%08X", info);
		m_LoginInfo = _T(buf);
		this->GetDlgItem(IDC_EDIT6)->SetWindowTextA(m_LoginInfo); 
		break;
	}


	CDialog::OnTimer(nIDEvent);
}
