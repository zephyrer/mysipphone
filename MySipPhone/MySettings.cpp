// MySettings.cpp : implementation file
//

#include "stdafx.h"
#include "MySipPhone.h"
#include "MySettings.h"
#include "MySipPhoneDlg.h"

// CMySettings dialog

// IMPLEMENT_DYNAMIC(CMySettings, CDialog)

CMySettings::CMySettings(CWnd* pParent /*=NULL*/)
	: CDialog(CMySettings::IDD, pParent)
	, m_PlayDeviceCur(_T(""))
	, m_RecordDeviceCur(_T(""))
	, m_IPCur(_T(""))
	, m_listenPort(_T(""))
{
	CMySipPhoneDlg* mySipPhoneDlg = (CMySipPhoneDlg* ) this->m_pParentWnd;
	config = mySipPhoneDlg->main->config;

	PStringArray playDevices = PSoundChannel::GetDeviceNames(PSoundChannel::Player);
	for (int i = 0; i < playDevices.GetSize(); i++) {
		m_PlayDeviceList.Add( (playDevices[i]).AsUCS2());
		LogWindow << "Playback devices: " << playDevices[i] << endl;
	}

	m_PlayDeviceCur = mySipPhoneDlg->main->pcssEP->GetSoundChannelPlayDevice().AsUCS2();
	LogWindow << "CURRENT Playback devices: " << PString(m_PlayDeviceCur) << endl;

	PStringArray recordDevices = PSoundChannel::GetDeviceNames(PSoundChannel::Recorder);
	for (int i = 0; i < recordDevices.GetSize(); i++) {
		m_RecordDeviceList.Add( (recordDevices[i]).AsUCS2() );
		LogWindow << "Record devices: " << recordDevices[i] << endl;
	}
	m_RecordDeviceCur = mySipPhoneDlg->main->pcssEP->GetSoundChannelRecordDevice().AsUCS2();
	LogWindow << "CURRENT Record devices: " << PString(m_RecordDeviceCur) << endl;


	m_IPList.Add( _T("*") );

	PIPSocket::InterfaceTable ifaces;
	if (PIPSocket::GetInterfaceTable(ifaces)) {
		for (int i = 0; i < ifaces.GetSize(); i++) {
			PString addr = ifaces[i].GetAddress().AsString();
			m_IPList.Add( addr.AsUCS2() );
			LogWindow << "IPs: " << addr << endl;
		}
	}
	m_IPCur = config.GetString(LocalIPKey, "*").AsUCS2();
	m_listenPort = config.GetString(ListenPortKey, "5060").AsUCS2();

	Create (IDD, pParent);
}


CMySettings::~CMySettings()
{
	CMySipPhoneDlg* mySipPhoneDlg = (CMySipPhoneDlg* ) this->m_pParentWnd;
	mySipPhoneDlg->settingsDlg = NULL;
}


BOOL CMySettings::OnInitDialog()
{
	CDialog::OnInitDialog();
	
// 	m_PlayDeviceBox.EnableWindow(TRUE);
// 	m_RecordDeviceBox.EnableWindow(TRUE);

	return TRUE;
}


void CMySettings::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}


void CMySettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//combo box duita initialy blank hoey thake
	DDX_Control(pDX, IDC_PLAYBACK_COMBO, m_PlayDeviceBox);
	DDX_Control(pDX, IDC_RECORDING_COMBO, m_RecordDeviceBox);
	DDX_Control(pDX, IDC_IP_COMBO, m_IPBox);

	if (!pDX->m_bSaveAndValidate)
	{
		int i = 0;
		m_PlayDeviceBox.ResetContent();
		for (i=0;i<m_PlayDeviceList.GetSize();i++)
			m_PlayDeviceBox.AddString(m_PlayDeviceList[i]);

		m_RecordDeviceBox.ResetContent();
		for (i=0;i<m_RecordDeviceList.GetSize();i++)
			m_RecordDeviceBox.AddString(m_RecordDeviceList[i]);

		m_IPBox.ResetContent();
		for (i=0;i<m_IPList.GetSize();i++)
			m_IPBox.AddString(m_IPList[i]);

	}	

	DDX_CBString(pDX, IDC_PLAYBACK_COMBO, m_PlayDeviceCur);
	DDX_CBString(pDX, IDC_RECORDING_COMBO, m_RecordDeviceCur);
	DDX_CBString(pDX, IDC_IP_COMBO, m_IPCur);
	DDX_Text(pDX, IDC_LISTEN_PORT, m_listenPort);
}


BEGIN_MESSAGE_MAP(CMySettings, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &CMySettings::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMySettings::OnBnClickedCancel)
END_MESSAGE_MAP()


// CMySettings message handlers

void CMySettings::OnClose() 
{
	DestroyWindow();
}


void CMySettings::OnBnClickedOk()
{
//	OnOK();
	CMySipPhoneDlg* mySipPhoneDlg = (CMySipPhoneDlg* ) this->m_pParentWnd;
	UpdateData();

	PString str(m_PlayDeviceCur);
	config.SetString(SoundPlayConfigKey, str);
	mySipPhoneDlg->main->pcssEP->SetSoundChannelPlayDevice(str);
	LogWindow << "Setting PCSS PLAY: " << str << endl;

	str = PString(m_RecordDeviceCur);
	config.SetString(SoundRecordConfigKey, str);
	mySipPhoneDlg->main->pcssEP->SetSoundChannelRecordDevice(str);
	LogWindow << "Setting PCSS RECORD: " << str << endl;

	str = PString(m_IPCur);
	config.SetString(LocalIPKey, str);
	LogWindow << "Setting Listen IP: " << str << endl;

	str = PString(m_listenPort);
	config.SetString(ListenPortKey, str);
	LogWindow << "Setting Listen Port: " << str << endl;

	OnClose();
}


void CMySettings::OnBnClickedCancel()
{
// 	OnCancel();
	OnClose();
}
