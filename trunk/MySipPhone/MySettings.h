#pragma once
#include "afxwin.h"


// CMySettings dialog

class CMySettings : public CDialog
{
//	DECLARE_DYNAMIC(CMySettings)

public:
	PConfig config;

	CMySettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMySettings();

// Dialog Data
	enum { IDD = IDD_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	CString m_PlayDeviceCur;
	CString m_RecordDeviceCur;
	CString m_IPCur;
	CComboBox m_PlayDeviceBox;
	CComboBox m_RecordDeviceBox;
	CComboBox m_IPBox;
	CStringArray m_PlayDeviceList;
	CStringArray m_RecordDeviceList;
	CStringArray m_IPList;
	CString m_listenPort;
	CString m_stunServer;
};

