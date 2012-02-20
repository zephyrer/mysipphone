#pragma once
#include "Resource.h"


// CDebugWindow dialog

class CDebugWindow : public CDialog
{
// 	DECLARE_DYNAMIC(CDebugWindow)

public:
	CDebugWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDebugWindow();

// Dialog Data
	enum { IDD = IDD_DEBUG_WINDOW };

protected:
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();

 	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnClose();

public:
	CRichEditCtrl m_cstatus;
	CString m_debug_status;
	void DebugOutput(CString csText);

};
