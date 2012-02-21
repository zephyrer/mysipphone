// MySipPhoneDlg.h : header file
//

#pragma once

#include "Manager.h"
#include "DebugWindow.h"
#include "SipAccInfo.h"
#include "MySettings.h"
#include "afxwin.h"

#define MYWM_NOTIFYICON (WM_USER+1)

// CMySipPhoneDlg dialog
class CMySipPhoneDlg : public CDialog
{
// Construction
public:

	/**** OPAL Start ****/
	class PWLibProcess : public PProcess {
		PCLASSINFO(PWLibProcess, PProcess);

		PWLibProcess():PProcess("Luimas", "MySipPhone", 0, 0, BetaCode, 1)
		{ 
		}

		void Main() { }
	} pwlibProcess;
	/**** OPAL End ****/


	CMySipPhoneDlg(CWnd* pParent = NULL);	// standard constructor
	~CMySipPhoneDlg();

// Dialog Data
	enum { IDD = IDD_MYSIPPHONE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);


// Implementation
protected:
	HICON m_hIcon;
	HACCEL m_hAccel;
	NOTIFYICONDATA tnd;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy( );

	afx_msg LRESULT onTrayNotify(WPARAM, LPARAM);
	afx_msg LRESULT OnClipChange(WPARAM, LPARAM);


// 	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
// 	afx_msg void OnPaint();
// 	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	CManager * main;
	CDebugWindow *dlgDebug;
	CSipAccInfo *sipAccDlg;
	CMySettings *settingsDlg;
	CComboBox m_cdestination;
	CButton m_call;
	CButton m_hangup;
	CButton m_hold;
	CButton m_transfer;

	void AddDigit(char numKey);
	void GetClipboardText();

	afx_msg void OnMenuExit();
	afx_msg void OnMenuAccount();
	afx_msg void OnMenuSettings();
	afx_msg void OnBnClickedDebugButton();
	afx_msg void OnBnClickedCall();
	afx_msg void OnBnClickedHangup();
	afx_msg void OnBnClickedKey(UINT);
	afx_msg void OnBnClickedKeyStar();
	afx_msg void OnBnClickedKeyPound();
	afx_msg void OnBnClickedHold();
	afx_msg void OnBnClickedTransfer();
	afx_msg void OnBnClickedButtonMenu();
};


class TextCtrlChannel : public PChannel
{
	PCLASSINFO(TextCtrlChannel, PChannel)
public:
	TextCtrlChannel()
		: dlgDebug(NULL)
	{ }

	virtual PBoolean Write(
		const void * buf, /// Pointer to a block of memory to write.
		PINDEX len        /// Number of bytes to write.
		) {
			if (dlgDebug == NULL)
				return false;

			PString psOutPut( (const char *)buf, (size_t)len );
			dlgDebug->DebugOutput(psOutPut.AsUCS2().GetPointer());
			return true;
	}

	void SetFrame(CDebugWindow * frame) 
	{ 
		dlgDebug = frame; 
	}

	static TextCtrlChannel & Instance()
	{
		static TextCtrlChannel instance;
		return instance;
	}

	CDebugWindow *dlgDebug;
};


#define LogWindow TextCtrlChannel::Instance()