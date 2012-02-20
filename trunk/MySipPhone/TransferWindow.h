#pragma once


// CTransferWindow dialog

class CTransferWindow : public CDialog
{
	DECLARE_DYNAMIC(CTransferWindow)

public:
	CTransferWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTransferWindow();

// Dialog Data
	enum { IDD = IDD_TRANSFER_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_transferCombo;
};
