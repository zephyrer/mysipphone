// MySipPhoneDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MySipPhone.h"
#include "MySipPhoneDlg.h"


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


// CMySipPhoneDlg dialog




CMySipPhoneDlg::CMySipPhoneDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMySipPhoneDlg::IDD, pParent)
{
// 	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	dlgDebug = NULL;
	sipAccDlg = NULL;
	settingsDlg = NULL;
	Create (IDD, pParent);
}

CMySipPhoneDlg::~CMySipPhoneDlg()
{
//	delete main;
}

void CMySipPhoneDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	Shell_NotifyIcon(NIM_DELETE, &tnd);
	delete this;
}


void CMySipPhoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEST, m_cdestination);
	DDX_Control(pDX, IDC_CALL, m_call);
	DDX_Control(pDX, IDC_HANGUP, m_hangup);
	DDX_Control(pDX, IDC_HOLD, m_hold);
	DDX_Control(pDX, IDC_TRANSFER, m_transfer);
}

BEGIN_MESSAGE_MAP(CMySipPhoneDlg, CDialog)
	ON_MESSAGE(MYWM_NOTIFYICON,onTrayNotify)
	ON_COMMAND(ID_SETTINGS_EXIT,OnMenuExit)
	ON_COMMAND(ID_SETTINGS_ACCOUNT,OnMenuAccount)
	ON_COMMAND(ID_SETTINGS_SETTINGS,OnMenuSettings)

// 	ON_WM_SYSCOMMAND()
// 	ON_WM_PAINT()
// 	ON_WM_QUERYDRAGICON()

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DEBUG_BUTTON, &CMySipPhoneDlg::OnBnClickedDebugButton)
	ON_BN_CLICKED(IDC_CALL, &CMySipPhoneDlg::OnBnClickedCall)
	ON_BN_CLICKED(IDC_HANGUP, &CMySipPhoneDlg::OnBnClickedHangup)
	ON_BN_CLICKED(IDC_KEY_STAR, &CMySipPhoneDlg::OnBnClickedKeyStar)
	ON_BN_CLICKED(IDC_KEY_POUND, &CMySipPhoneDlg::OnBnClickedKeyPound)
	ON_MESSAGE(WM_DRAWCLIPBOARD, OnClipChange)
	ON_COMMAND_RANGE(IDC_KEY_0, IDC_KEY_9, &CMySipPhoneDlg::OnBnClickedKey)
	ON_BN_CLICKED(IDC_HOLD, &CMySipPhoneDlg::OnBnClickedHold)
	ON_BN_CLICKED(IDC_TRANSFER, &CMySipPhoneDlg::OnBnClickedTransfer)
	ON_BN_CLICKED(IDC_BUTTON_MENU, &CMySipPhoneDlg::OnBnClickedButtonMenu)
END_MESSAGE_MAP()


// CMySipPhoneDlg message handlers

BOOL CMySipPhoneDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_hIcon = theApp.LoadIcon(IDR_MAINFRAME);
	HICON iconSmall = (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDR_MAINFRAME),
		IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED );
	PostMessage(WM_SETICON,ICON_SMALL,(LPARAM)iconSmall);

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

#if 1
	// Note that LoadAccelerator does not require DestroyAcceleratorTable
	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR));
#endif

	//For systray icon
	CString csSystrayTitle= _T("MySipPhone");
	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = this->GetSafeHwnd();
	tnd.uID = IDR_MAINFRAME;
	tnd.uCallbackMessage = MYWM_NOTIFYICON;
	tnd.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP; 
 	tnd.hIcon = iconSmall;
	lstrcpyn(tnd.szTip, (LPCTSTR)csSystrayTitle, sizeof(tnd.szTip));
	DWORD dwMessage = NIM_ADD;
	Shell_NotifyIcon(dwMessage, &tnd);

	// make sure various URL types are registered to this application
	{
		PString urlTypes("sip");
		PProcess::HostSystemURLHandlerInfo::RegisterTypes(urlTypes, true);
	}
	
	if(!dlgDebug) {
		dlgDebug = new CDebugWindow(this); //create this first other wise logwindow will not work
	}
	dlgDebug->ShowWindow(SW_HIDE);
	m_hangup.EnableWindow(FALSE);
	m_call.SetWindowText(_T("Call"));
	m_hold.EnableWindow(FALSE);
	m_hold.SetWindowText(_T("Hold"));
	m_transfer.EnableWindow(FALSE);


	main = new CManager();
	bool ok = main->Initialise(this);

	//tell the system we want clipboard notification
	SetClipboardViewer();

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CMySipPhoneDlg::OnMenuExit()
{
	DestroyWindow ();
}


void CMySipPhoneDlg::OnMenuSettings()
{
	if (!settingsDlg)
	{
		settingsDlg = new CMySettings(this);
	}

	settingsDlg->ShowWindow(SW_SHOW);
	::SetForegroundWindow(settingsDlg->m_hWnd);
}


void CMySipPhoneDlg::OnMenuAccount()
{
	if (!sipAccDlg)
	{
		sipAccDlg = new CSipAccInfo(this);
	}
	sipAccDlg->ShowWindow(SW_SHOW);
	::SetForegroundWindow(sipAccDlg->m_hWnd);
}


BOOL CMySipPhoneDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (m_hAccel != NULL && TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}


LRESULT CMySipPhoneDlg::onTrayNotify(WPARAM wParam,LPARAM lParam)
{
	UINT uMsg = (UINT) lParam; 
	switch (uMsg ) 
	{ 
	case WM_LBUTTONUP:
		if (this->IsWindowVisible())
		{
			this->ShowWindow(SW_HIDE);
		} else
		{
			this->ShowWindow(SW_SHOW);
			::SetForegroundWindow(m_hWnd);
		}
		break;
	case WM_RBUTTONUP:
		CPoint point;    
		GetCursorPos(&point);
		CMenu menu;
		menu.LoadMenu(IDR_MENU_SETTINGS);
		CMenu* tracker = menu.GetSubMenu(0);
		SetForegroundWindow();
		tracker->TrackPopupMenu( 0, point.x, point.y, this );
		PostMessage(WM_NULL, 0, 0);
		break;
	} 
	return TRUE;
}


// void CMySipPhoneDlg::OnSysCommand(UINT nID, LPARAM lParam)
// {
// 	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
// 	{
// 		CAboutDlg dlgAbout;
// 		dlgAbout.DoModal();
// 	}
// 	else
// 	{
// 		CDialog::OnSysCommand(nID, lParam);
// 	}
// }
// 
// // If you add a minimize button to your dialog, you will need the code below
// //  to draw the icon.  For MFC applications using the document/view model,
// //  this is automatically done for you by the framework.
// 
// void CMySipPhoneDlg::OnPaint()
// {
// 	if (IsIconic())
// 	{
// 		CPaintDC dc(this); // device context for painting
// 
// 		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
// 
// 		// Center icon in client rectangle
// 		int cxIcon = GetSystemMetrics(SM_CXICON);
// 		int cyIcon = GetSystemMetrics(SM_CYICON);
// 		CRect rect;
// 		GetClientRect(&rect);
// 		int x = (rect.Width() - cxIcon + 1) / 2;
// 		int y = (rect.Height() - cyIcon + 1) / 2;
// 
// 		// Draw the icon
// 		dc.DrawIcon(x, y, m_hIcon);
// 	}
// 	else
// 	{
// 		CDialog::OnPaint();
// 	}
// }
// 
// // The system calls this function to obtain the cursor to display while the user drags
// //  the minimized window.
// HCURSOR CMySipPhoneDlg::OnQueryDragIcon()
// {
// 	return static_cast<HCURSOR>(m_hIcon);
// }


void CMySipPhoneDlg::OnBnClickedDebugButton()
{
	dlgDebug->ShowWindow(SW_SHOW);
//	::SetForegroundWindow(dlgDebug->m_hWnd);
}


void CMySipPhoneDlg::GetClipboardText()
{
	TCHAR * cbuffer;
	OpenClipboard();
	cbuffer = (TCHAR*)GetClipboardData(CF_UNICODETEXT);
	CloseClipboard();
	PString psClipBoardData(cbuffer);
	if(!psClipBoardData.IsEmpty()) {
		psClipBoardData.Replace(_T(" "), _T(""), true);
		psClipBoardData.Replace(_T("+"), _T(""), true);
		psClipBoardData.Replace(_T("-"), _T(""), true);
		psClipBoardData.Replace(_T("("), _T(""), true);
		psClipBoardData.Replace(_T(")"), _T(""), true);
		psClipBoardData	= psClipBoardData.Left(25);
		if(!psClipBoardData.IsEmpty() && (psClipBoardData.AsInt64() > 0)) {
			m_cdestination.SetWindowText(psClipBoardData.AsUCS2());
		}
	}
}


LRESULT CMySipPhoneDlg::OnClipChange(WPARAM wParam,LPARAM lParam) 
{
	GetClipboardText();
	return TRUE;
}


void CMySipPhoneDlg::AddDigit(char numKey)
{
	CString str, tmp;			
	tmp.Format(_T("%c"), numKey);

	main->SendUserInput(numKey);

	m_cdestination.GetWindowText(str);
	str += tmp;
	m_cdestination.SetWindowText(str);
	m_cdestination.HideCaret();
	UpdateData();
}


void CMySipPhoneDlg::OnBnClickedKey(UINT nKey)
{
	int iKey = nKey - IDC_KEY_0;
	if ( iKey == 0) {
		AddDigit('0');
	} else if ( iKey == 1) {
		AddDigit('1');
	} else if ( iKey == 2) {
		AddDigit('2');
	} else if ( iKey == 3) {
		AddDigit('3');
	} else if ( iKey == 4) {
		AddDigit('4');
	} else if ( iKey == 5) {
		AddDigit('5');
	} else if ( iKey == 6) {
		AddDigit('6');
	} else if ( iKey == 7) {
		AddDigit('7');
	} else if ( iKey == 8) {
		AddDigit('8');
	} else if ( iKey == 9) {
		AddDigit('9');
	}
}


void CMySipPhoneDlg::OnBnClickedKeyStar()
{
	AddDigit('*');
}


void CMySipPhoneDlg::OnBnClickedKeyPound()
{
	AddDigit('#');
}


void CMySipPhoneDlg::OnBnClickedCall()
{
	CString csType;
	m_call.GetWindowText(csType);
	if(!csType.CompareNoCase(_T("call")))  {
		CString destNumber;
		m_cdestination.GetWindowText(destNumber);
		PString psDest(destNumber);
		if( P_MAX_INDEX == psDest.Find(_T("@")) ) {
			psDest = psDest + "@" + main->config.GetString(RegistrarDomainKey);
		}
		main->MakeCall(psDest);
	} else {
		main->AnswerCall();
	}
}


void CMySipPhoneDlg::OnBnClickedHangup()
{
	CString csType;
	m_hangup.GetWindowText(csType);
	if(!csType.CompareNoCase(_T("hangup")))  {
		main->HangUpCall();
	} else {
		main->RejectCall();
	}
}


void CMySipPhoneDlg::OnBnClickedHold()
{
	CString csType;
	m_hold.GetWindowText(csType);
	if(!csType.CompareNoCase(_T("hold")))  {
		main->OnRequestHold();
	} else {
		main->OnRetrieve();
	}
}

void CMySipPhoneDlg::OnBnClickedTransfer()
{
	main->OnTransfer();
}

void CMySipPhoneDlg::OnBnClickedButtonMenu()
{
	CPoint point;    
	GetCursorPos(&point);
	CMenu menu;
	menu.LoadMenu(IDR_MENU_SETTINGS);
	CMenu* tracker = menu.GetSubMenu(0);
	SetForegroundWindow();
	tracker->TrackPopupMenu( 0, point.x, point.y, this );
	PostMessage(WM_NULL, 0, 0);
}
