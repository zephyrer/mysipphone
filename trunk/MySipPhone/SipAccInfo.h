#pragma once


// CSipAccInfo dialog

class CSipAccInfo : public CDialog
{
// 	DECLARE_DYNAMIC(CSipAccInfo)

public:
	PConfig config;

	CSipAccInfo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSipAccInfo();

// Dialog Data
	enum { IDD = IDD_SIP_ACCOUNT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
