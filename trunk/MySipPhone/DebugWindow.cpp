// DebugWindow.cpp : implementation file
//

#include "stdafx.h"
#include "MySipPhone.h"
#include "DebugWindow.h"
#include "MySipPhoneDlg.h"

// CDebugWindow dialog

//IMPLEMENT_DYNAMIC(CDebugWindow, CDialog)

CDebugWindow::CDebugWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugWindow::IDD, pParent)
	, m_debug_status(_T(""))
{
	AfxInitRichEdit2();
	Create(IDD, pParent);
	LogWindow.SetFrame(this);
}


CDebugWindow::~CDebugWindow()
{
	CMySipPhoneDlg* mySipPhoneDlg = (CMySipPhoneDlg* ) this->m_pParentWnd;
	mySipPhoneDlg->dlgDebug = NULL;
}


void CDebugWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEBUG_STATUS, m_cstatus);
	DDX_Text(pDX, IDC_DEBUG_STATUS, m_debug_status);
}


BOOL CDebugWindow::OnInitDialog()
{
	CDialog::OnInitDialog();
//	AfxMessageBox(_T("Dialog init"));
	return TRUE;
}


void CDebugWindow::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}


BEGIN_MESSAGE_MAP(CDebugWindow, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CDebugWindow::OnClose() 
{
	//DestroyWindow();
	this->ShowWindow(SW_HIDE);
}

// CDebugWindow message handlers

void CDebugWindow::DebugOutput(CString csText)
{
	int nLineCount = m_cstatus.GetLineCount();
// 	if (nLineCount > 0) {
// 		m_cstatus.LineScroll(nLineCount, 0); //For put to TOP
// 	}
	// formatting
	CHARFORMAT cf, oldcf;
	m_cstatus.GetDefaultCharFormat(cf);
	m_cstatus.GetDefaultCharFormat(oldcf);
	cf.dwMask = CFM_COLOR|CFM_BOLD;
	cf.crTextColor = RGB(0,153,0);
	cf.dwEffects = CFE_DISABLED;
	CString strNew, strOld;
	m_cstatus.GetWindowText(strOld);
	m_cstatus.SetSel( strOld.GetLength(), strOld.GetLength());	
	m_cstatus.SetSelectionCharFormat(cf);
	strNew = csText; // + _T("\r\n");
	m_cstatus.ReplaceSel((LPCTSTR) strNew.GetBuffer(0));
	m_cstatus.SetSel( strNew.GetLength()+strOld.GetLength(), strNew.GetLength()+strOld.GetLength());	
	m_cstatus.SetSelectionCharFormat(oldcf);

	//AfxMessageBox(_T("TEST"));
	//  Scroll the rich edit control so that the first visible line is the first line of text.
	// 	int nFirstVisible = m_cstatus.GetFirstVisibleLine();
	// 	if (nFirstVisible > 0) {
	// 		m_cstatus.LineScroll(-nFirstVisible, 0); //For put to TOP
	// 	}
}
