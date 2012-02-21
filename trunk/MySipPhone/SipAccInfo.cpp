// SipAccInfo.cpp : implementation file
//

#include "stdafx.h"
#include "MySipPhone.h"
#include "SipAccInfo.h"
#include "MySipPhoneDlg.h"

// CSipAccInfo dialog

// IMPLEMENT_DYNAMIC(CSipAccInfo, CDialog)

CSipAccInfo::CSipAccInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CSipAccInfo::IDD, pParent)
{
	Create (IDD, pParent);
	CMySipPhoneDlg* mySipPhoneDlg = (CMySipPhoneDlg* ) this->m_pParentWnd;
	config = mySipPhoneDlg->main->config;
}


CSipAccInfo::~CSipAccInfo()
{
	CMySipPhoneDlg* mySipPhoneDlg = (CMySipPhoneDlg* ) this->m_pParentWnd;
	mySipPhoneDlg->sipAccDlg = NULL;
}


BOOL CSipAccInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	CEdit* edit;
// 	CComboBox *combobox;
// 	int i;

	PString hostname, username, password;
 	int m_SIPProxyUsed = config.GetBoolean(SIPProxyUsedKey, false);
	hostname = config.GetString(SIPProxyKey, "");
	username = config.GetString(SIPProxyUsernameKey, "");
	password = config.GetString(SIPProxyPasswordKey, "");


	edit = (CEdit*)GetDlgItem(IDC_PROXY_HOST);
	edit->SetWindowText(hostname.AsUCS2());

	edit = (CEdit*)GetDlgItem(IDC_PROXY_LOGIN);
	edit->SetWindowText(username.AsUCS2());

	edit = (CEdit*)GetDlgItem(IDC_PROXY_PASS);
	edit->SetWindowText(password.AsUCS2());

	((CButton*)GetDlgItem(IDC_USE_PROXY))->SetCheck( m_SIPProxyUsed );


	PString sipDomain, sipUser, sipPassword, sipAuthId, sipProxy;
	PString sipRegTTL( config.GetInteger(RegistrarTimeToLiveKey, 300) );
	sipDomain = config.GetString(RegistrarDomainKey, "");
	sipUser = config.GetString(RegistrarUsernameKey, "");
	sipPassword = config.GetString(RegistrarPasswordKey, "");
	sipAuthId = config.GetString(RegistrarAuthIDKey, "");
	sipProxy = config.GetString(RegistrarProxyKey, "");
	
	edit = (CEdit*)GetDlgItem(IDC_SIP_HOST);
	edit->SetWindowText(sipDomain.AsUCS2());

	edit = (CEdit*)GetDlgItem(IDC_SIP_PROXY);
	edit->SetWindowText(sipProxy.AsUCS2());

	edit = (CEdit*)GetDlgItem(IDC_SIP_USER);
	edit->SetWindowText(sipUser.AsUCS2());

	edit = (CEdit*)GetDlgItem(IDC_SIP_PASS);
	edit->SetWindowText(sipPassword.AsUCS2());

	edit = (CEdit*)GetDlgItem(IDC_SIP_AUTH_ID);
	edit->SetWindowText(sipAuthId.AsUCS2());

	edit = (CEdit*)GetDlgItem(IDC_SIP_REGISTER_TTL);
	edit->SetWindowText(sipRegTTL.AsUCS2());

	return TRUE;
}


void CSipAccInfo::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}


void CSipAccInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSipAccInfo, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &CSipAccInfo::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSipAccInfo::OnBnClickedCancel)
END_MESSAGE_MAP()


// CSipAccInfo message handlers

void CSipAccInfo::OnBnClickedOk()
{
	CEdit* edit;
	CString str;
	int iUseProxy;


	edit = (CEdit*)GetDlgItem(IDC_PROXY_HOST);
	edit->GetWindowText(str);
	config.SetString(SIPProxyKey, PString(str.Trim()));

	edit = (CEdit*)GetDlgItem(IDC_PROXY_LOGIN);
	edit->GetWindowText(str);
	config.SetString(SIPProxyUsernameKey, PString(str.Trim()));

	edit = (CEdit*)GetDlgItem(IDC_PROXY_PASS);
	edit->GetWindowText(str);
	config.SetString(SIPProxyPasswordKey, PString(str.Trim()));

	iUseProxy = ((CButton*)GetDlgItem(IDC_USE_PROXY))->GetCheck( );
	config.SetBoolean(SIPProxyUsedKey, iUseProxy);


	edit = (CEdit*)GetDlgItem(IDC_SIP_HOST);
	edit->GetWindowText(str);
	config.SetString(RegistrarDomainKey, PString(str.Trim()));

	edit = (CEdit*)GetDlgItem(IDC_SIP_PROXY);
	edit->GetWindowText(str);
	config.SetString(RegistrarProxyKey, PString(str.Trim()));

	edit = (CEdit*)GetDlgItem(IDC_SIP_USER);
	edit->GetWindowText(str);
	config.SetString(RegistrarUsernameKey, PString(str.Trim()));

	edit = (CEdit*)GetDlgItem(IDC_SIP_PASS);
	edit->GetWindowText(str);
	config.SetString(RegistrarPasswordKey, PString(str.Trim()));

	edit = (CEdit*)GetDlgItem(IDC_SIP_AUTH_ID);
	edit->GetWindowText(str);
	config.SetString(RegistrarAuthIDKey, PString(str.Trim()));

	edit = (CEdit*)GetDlgItem(IDC_SIP_REGISTER_TTL);
	edit->GetWindowText(str);
	config.SetString(RegistrarTimeToLiveKey, PString(str.Trim()).AsInteger());

	OnClose();
	//OnOK();
}

void CSipAccInfo::OnBnClickedCancel()
{
//	OnCancel();
	OnClose();
}


void CSipAccInfo::OnClose() 
{
	DestroyWindow();
}


