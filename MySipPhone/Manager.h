#pragma once
#include <opal/manager.h>
#include <opal/ivr.h>

#include "SIPEndPoint.h"
#include "PCSSEndPoint.h"
#include "MySipPhoneDlg.h"

const char LocalIPKey[]				= "LocalIP";
const char ListenPortKey[]			= "ListenPort";
const char StunServerKey[]			= "STUNServer";
const char StunServerUsedKey[]		= "STUNServerUsed";
const char SoundPlayConfigKey[]		= "SoundPlayDevice";
const char SoundRecordConfigKey[]	= "SoundRecordDevice";
const char RegistrarUsedKey[]		= "RegistrarUsed";
const char RegistrarUsernameKey[]	= "RegistrarUsername";
const char RegistrarDomainKey[]		= "RegistrarDomain";
const char RegistrarContactKey[]	= "RegistrarContact";
const char RegistrarAuthIDKey[]		= "RegistrarAuthID";
const char RegistrarPasswordKey[]	= "RegistrarPassword";
const char RegistrarTimeToLiveKey[] = "RegistrarTimeToLive";
const char RegistrarProxyKey[]		= "RegistrarProxy";

const char SIPProxyUsedKey[]		= "SIPProxyUsed";
const char SIPProxyKey[]			= "SIPProxy";
const char SIPProxyUsernameKey[]	= "SIPProxyUsername";
const char SIPProxyPasswordKey[]	= "SIPProxyPassword";


class CSIPEndPoint;
class CPCSSEndPoint;
class CMySipPhoneDlg;
class OpalIVREndPoint;

class RegistrationInfo
{
public:
	// these must match the drop-down box
	// on the Registration/Subcription dialog box
	enum Types {
		Register,
		SubscribeMWI,
		SubcribePresence,
		SubscribeMLA
	};

	RegistrationInfo();

	bool operator==(const RegistrationInfo & other) const;

	bool Read();
	void Write();
	bool Start(SIPEndPoint & sipEP);
	bool Stop(SIPEndPoint & sipEP);

	Types     m_Type;
	bool      m_Active;
	PString	m_User;
	PString m_Domain;
	PString m_Contact;
	PString m_AuthID;
	PString m_Password;
	int       m_TimeToLive;
	PString m_Proxy;
	SIPRegister::CompatibilityModes m_Compatibility;

	PString   m_aor;
};

typedef list<RegistrationInfo> RegistrationList;


class MyMedia
{
public:
	MyMedia();
	MyMedia(const OpalMediaFormat & format);

	bool operator<(const MyMedia & other) { return preferenceOrder < other.preferenceOrder; }

	OpalMediaFormat mediaFormat;
	const char  * validProtocols;
	int             preferenceOrder;
};

typedef std::list<MyMedia> MyMediaList;


class CManager :	public OpalManager
{
public:
	friend class CSIPEndPoint;
	
	CMySipPhoneDlg * m_dialog;
	CSIPEndPoint	* sipEP;
	CPCSSEndPoint   * pcssEP;
	OpalIVREndPoint  * ivrEP;

	PSafePtr<OpalCall> m_activeCall;
	PString     m_incomingToken;
	PString     m_incomingTokenForTransfer;

	int			m_NATHandling;
	PString		m_NATRouter;
	PString		m_STUNServer;
	RegistrationList m_registrations;
	bool             m_SIPProxyUsed;

	vector<PString> m_LocalInterfaces;
	
	MyMediaList m_mediaInfo;
	void ApplyMediaInfo();

	bool      m_autoAnswer;
	PString m_LastDialed;
	PString m_LastReceived;

	PString m_ForwardingAddress;
	int       m_ForwardingTimeout;
	PTimer    m_ForwardingTimer;
	PDECLARE_NOTIFIER(PTimer, CManager, OnForwardingTimeout);

	PString     m_RingSoundDeviceName;
	PString     m_RingSoundFileName;
	PSoundChannel m_RingSoundChannel;
	PTimer        m_RingSoundTimer;
	PDECLARE_NOTIFIER(PTimer, CManager, OnRingSoundAgain);
	void StopRingSound();
	
//	void UpdateAudioDevices();

	struct CallsOnHold {
		CallsOnHold() { }
		CallsOnHold(OpalCall & call);

		PSafePtr<OpalCall> m_call;
	};
	list<CallsOnHold>    m_callsOnHold;
	PString            m_switchHoldToken;

/////////////////////////////////////////
//	PConfig *config;
	PConfig config;
	PString psTokenForHangup;

	CManager(void);
	~CManager(void);

	bool Initialise(CMySipPhoneDlg * dlg);
	void MakeCall(const PString & address, const PString & local = "", OpalConnection::StringOptions * options = NULL);
	void AnswerCall();
	void RejectCall();
	void HangUpCall();
	void SendUserInput(char tone);

	void OnRequestHold();
	void OnRetrieve();
	void OnTransfer();

	void AddCallOnHold(
		OpalCall & call
		);
	bool RemoveCallOnHold(
		const PString & token
		);
	void OnHoldChanged(
		const PString & token,
		bool onHold
		);

	PSafePtr<OpalCall>       GetCall(PSafetyMode mode);
	PSafePtr<OpalConnection> GetConnection(bool user, PSafetyMode mode);

	CPCSSEndPoint & GetPCSSEP() { return *pcssEP; }

private:
	// OpalManager overrides
	virtual PBoolean OnIncomingConnection(
		OpalConnection & connection,   ///<  Connection that is calling
		unsigned options,              ///<  options for new connection (can't use default as overrides will fail)
		OpalConnection::StringOptions * stringOptions
		);
	virtual void OnEstablishedCall(
		OpalCall & call   /// Call that was completed
		);
	virtual void OnClearedCall(
		OpalCall & call   /// Connection that was established
		);
	virtual void OnHold(
		OpalConnection & connection,   ///<  Connection that was held/retrieved
		bool fromRemote,               ///<  Indicates remote has held local connection
		bool onHold                    ///<  Indicates have just been held/retrieved.
		);
	virtual bool OnTransferNotify(
		OpalConnection & connection,  ///< Connection being transferred.
		const PStringToString & info  ///< Information on the transfer
		);
	virtual void AdjustMediaFormats(
		bool local,                         ///<  Media formats a local ones to be presented to remote
		const OpalConnection & connection,  ///<  Connection that is about to use formats
		OpalMediaFormatList & mediaFormats  ///<  Media formats to use
		) const;
	virtual PBoolean OnOpenMediaStream(
		OpalConnection & connection,  /// Connection that owns the media stream
		OpalMediaStream & stream    /// New media stream being opened
		);
	virtual void OnClosedMediaStream(
		const OpalMediaStream & stream     ///<  Stream being closed
		);
	virtual void OnUserInputString(
		OpalConnection & connection,  /// Connection input has come from
		const PString & value         /// String value of indication
		);
	virtual void OnUserInputTone(
		OpalConnection & connection,  ///<  Connection input has come from
		char tone,                    ///<  Tone received
		int duration                  ///<  Duration of tone
		);
// 	virtual PString ReadUserInput(
// 		OpalConnection & connection,        ///<  Connection to read input from
// 		const char * terminators = "#\r\n", ///<  Characters that can terminte input
// 		unsigned lastDigitTimeout = 4,      ///<  Timeout on last digit in string
// 		unsigned firstDigitTimeout = 30     ///<  Timeout on receiving any digits
// 		);
// 	virtual PBoolean CreateVideoInputDevice(
// 		const OpalConnection & connection,    ///<  Connection needing created video device
// 		const OpalMediaFormat & mediaFormat,  ///<  Media format for stream
// 		PVideoInputDevice * & device,         ///<  Created device
// 		PBoolean & autoDelete                     ///<  Flag for auto delete device
// 		);
// 	virtual PBoolean CreateVideoOutputDevice(
// 		const OpalConnection & connection,    ///<  Connection needing created video device
// 		const OpalMediaFormat & mediaFormat,  ///<  Media format for stream
// 		PBoolean preview,                         ///<  Flag indicating is a preview output
// 		PVideoOutputDevice * & device,        ///<  Created device
// 		PBoolean & autoDelete                     ///<  Flag for auto delete device
// 		);


public:
	void SetNATHandling();
	void StartAllListeners();
	void StartRegistrations();
	void OnEvtRinging( PString psIncomingCallToken );
	void OnEvtOnHold();
	void OnEvtEstablished(PString psToken);
	void OnEvtCleared(PString psToken);
	void OnSendAudioFile();

};


