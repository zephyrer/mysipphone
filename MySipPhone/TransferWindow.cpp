// TransferWindow.cpp : implementation file
//

#include "stdafx.h"
#include "MySipPhone.h"
#include "TransferWindow.h"


// CTransferWindow dialog

IMPLEMENT_DYNAMIC(CTransferWindow, CDialog)

CTransferWindow::CTransferWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CTransferWindow::IDD, pParent)
	, m_transferCombo(_T(""))
{

}

CTransferWindow::~CTransferWindow()
{
}

void CTransferWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_TRANSFER_COMBO, m_transferCombo);
}


BEGIN_MESSAGE_MAP(CTransferWindow, CDialog)
END_MESSAGE_MAP()


// CTransferWindow message handlers
