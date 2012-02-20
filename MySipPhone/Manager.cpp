#include "StdAfx.h"
#include "Manager.h"
#include "TransferWindow.h"


CManager::CManager() : m_dialog(NULL)
					, pcssEP(NULL)
					, sipEP(NULL)
					, ivrEP(NULL)
{
}


CManager::~CManager()
{

//   	m_activeCall.SetNULL();
//   	m_callsOnHold.clear();
//   	m_registrations.clear();
//   	ShutDownEndpoints();

// 	if(pcssEP)
// 		delete pcssEP;
// 	if(sipEP)
// 		delete sipEP;

}



bool CManager::Initialise(CMySipPhoneDlg * dlg)
{

	m_dialog = dlg;
	m_autoAnswer = false;


	LogWindow << PProcess::Current().GetName()
		<< " Version " << PProcess::Current().GetVersion(TRUE)
		<< " by " << PProcess::Current().GetManufacturer()
		<< " on " << PProcess::Current().GetOSClass() << ' ' << PProcess::Current().GetOSName()
		<< " (" << PProcess::Current().GetOSVersion() << '-' << PProcess::Current().GetOSHardware() << ')' << endl;

#if PTRACING
	////////////////////////////////////////
	// Tracing fields
	int traceLevelThreshold = 4;
//	int traceOptions = PTrace::DateAndTime|PTrace::Thread|PTrace::FileAndLine;
	int traceOptions = PTrace::DateAndTime|PTrace::FileAndLine;
	PTrace::Initialise(traceLevelThreshold, "TRACE-MYSIPPHONE.TXT", traceOptions);
#endif

	////////////////////////////////////////
	// Creating the endpoints
	sipEP = new CSIPEndPoint(*this);
	pcssEP = new CPCSSEndPoint(*this);
	ivrEP = new OpalIVREndPoint(*this);

	////////////////////////////////////////
	// General fields
//	SetDefaultUserName("luimas");
//	SetDefaultDisplayName("luimas");


	////////////////////////////////////////
	// Networking fields
	PIPSocket::InterfaceTable interfaceTable;
	if (PIPSocket::GetInterfaceTable(interfaceTable)) {
 		LogWindow << "Detected " << interfaceTable.GetSize() << " network interfaces:\n"
 		<< setfill('\n') << interfaceTable << setfill(' ') << flush;
	}

	PString psLocalIP = config.GetString(LocalIPKey, "*");
	PString psLocalPort = config.GetString(ListenPortKey, "5060");
	PString localInterface = "sip:udp$" + psLocalIP + ":" + psLocalPort;
	m_LocalInterfaces.push_back(localInterface);	
	StartAllListeners();

	////////////////////////////////////////
	// Sound fields
// 	PConfig config;

 	if( pcssEP->SetSoundChannelPlayDevice( config.GetString(SoundPlayConfigKey) ) ) {
		LogWindow << "PLAY DEVICE: " << pcssEP->GetSoundChannelPlayDevice() << endl;
		m_RingSoundDeviceName = PSoundChannel::GetDefaultDevice(PSoundChannel::Player);
		LogWindow << "RING SOUND DEVICE: " << m_RingSoundDeviceName << endl;
	}
	if( pcssEP->SetSoundChannelRecordDevice( config.GetString(SoundRecordConfigKey) ) ) {
		LogWindow << "RECORD DEVICE: " << pcssEP->GetSoundChannelRecordDevice() << endl;
	}
// 	pcssEP->SetSoundChannelBufferTime(value1);
	pcssEP->SetSoundChannelBufferDepth(10);	
	SetAudioJitterDelay ( 20, 500);



	////////////////////////////////////////
	// Codec fields
	{
		OpalMediaFormatList mediaFormats;
		mediaFormats += pcssEP->GetMediaFormats();
		mediaFormats += ivrEP->GetMediaFormats();

		OpalMediaFormatList possibleFormats = OpalTranscoder::GetPossibleFormats(mediaFormats);
		for (OpalMediaFormatList::iterator format = possibleFormats.begin(); format != possibleFormats.end(); ++format) {
			if (format->IsTransportable()) {
				m_mediaInfo.push_back(MyMedia(*format));
//				LogWindow << "POSSIBLE CODECS " << m_mediaInfo.back().mediaFormat << endl;
			}
		}

#if 1 
		int codecIndex = 0;
		for (MyMediaList::iterator mm = m_mediaInfo.begin(); mm != m_mediaInfo.end(); ++mm) {
			PString psMyFormats =  mm->mediaFormat;
			if( P_MAX_INDEX != psMyFormats.Find("G.711-ALaw-64k") ) {
				LogWindow << "POSSIBLE CODECS " << psMyFormats << endl;
				mm->preferenceOrder = 0;	//codecIndex;
				codecIndex++;
			} else if( P_MAX_INDEX != psMyFormats.Find("G.711-uLaw-64k") ) {
				LogWindow << "POSSIBLE CODECS " << psMyFormats << endl;
				mm->preferenceOrder = 1;	//codecIndex;
				codecIndex++;
			} else if( P_MAX_INDEX != psMyFormats.Find("UserInput/RFC2833") ) {
				LogWindow << "POSSIBLE CODECS " << psMyFormats << endl;
				mm->preferenceOrder = 2;	//codecIndex;
				codecIndex++;
			} else {
				mm->preferenceOrder = -1;
			}
		}
		ApplyMediaInfo();
#endif
	}


#if PTRACING
	if (PTrace::CanTrace(4)) {
		OpalMediaFormatList mediaFormats = OpalMediaFormat::GetAllRegisteredMediaFormats();
		ostream & traceStream = PTrace::Begin(4, __FILE__, __LINE__);
		traceStream << "MySipPhone\tRegistered media formats:\n";
		for (PINDEX i = 0; i < mediaFormats.GetSize(); i++)
			mediaFormats[i].PrintOptions(traceStream);
		traceStream << PTrace::End;
	}
#endif


	////////////////////////////////////////
	// SIP fields

	const SIPURL & proxy = sipEP->GetProxy();
 	PString hostname, username, password;
	m_SIPProxyUsed = config.GetBoolean(SIPProxyUsedKey, false);
	hostname = config.GetString(SIPProxyKey);
	username = config.GetString(SIPProxyUsernameKey);
	password = config.GetString(SIPProxyPasswordKey);

	if (m_SIPProxyUsed)
		sipEP->SetProxy(hostname, username, password);

	RegistrationInfo registration;

	registration.m_Active = true;
	registration.m_Domain	= config.GetString(RegistrarDomainKey);
	registration.m_User		= config.GetString(RegistrarUsernameKey);
	registration.m_Password = config.GetString(RegistrarPasswordKey);
	registration.m_AuthID	= config.GetString(RegistrarAuthIDKey);
	registration.m_Proxy	= config.GetString(RegistrarProxyKey);
	registration.m_TimeToLive = config.GetInteger(RegistrarTimeToLiveKey, 300);
	m_registrations.push_back(registration);

	StartRegistrations();


	AddRouteEntry("pc:.*             = sip:<da>");
	AddRouteEntry("sip:.*			 = pc:");

 	LogWindow << "Ready ..." << endl;
	return true;
}

void CManager::SetNATHandling()
{
	switch (m_NATHandling) {
	case 1 :
		if (!m_NATRouter.IsEmpty())
			SetTranslationHost(m_NATRouter);
		SetSTUNServer(PString::Empty());
		break;

	case 2 :
		if (!m_STUNServer.IsEmpty()) {
			LogWindow << "STUN server \"" << m_STUNServer << "\" being contacted ..." << endl;
//			GetEventHandler()->ProcessPendingEvents();
//			Update();

			PSTUNClient::NatTypes nat = SetSTUNServer(m_STUNServer);

 			LogWindow << "STUN server \"" << stun->GetServer() << "\" replies " << nat;
			PIPSocket::Address externalAddress;
			if (nat != PSTUNClient::BlockedNat && GetSTUNClient()->GetExternalAddress(externalAddress)){
				LogWindow << " with address " << externalAddress;
			}
 			LogWindow << endl;
		}
		SetTranslationHost(PString::Empty());
		break;

	default :
		SetTranslationHost(PString::Empty());
		SetSTUNServer(PString::Empty());
	}
}


static void StartListenerForEP(OpalEndPoint * ep, const vector<PString> & allInterfaces)
{
	if (ep == NULL)
		return;

	PStringArray interfacesForEP;
	PString prefixAndColon = ep->GetPrefixName() + ':';

	for (size_t i = 0; i < allInterfaces.size(); i++) {
		PCaselessString iface = allInterfaces[i];
		if (iface.NumCompare("all:", 4) == PObject::EqualTo) {
			interfacesForEP += iface.Mid(4);
		} else if (iface.NumCompare(prefixAndColon) == PObject::EqualTo) {
			interfacesForEP.AppendString(iface.Mid(prefixAndColon.GetLength()));
		}
	}

	ep->RemoveListener(NULL);
	if (ep->StartListeners(interfacesForEP))
		LogWindow << ep->GetPrefixName().ToUpper() << " listening on " << setfill(',') << ep->GetListeners() << setfill(' ') << endl;
	else {
		LogWindow << ep->GetPrefixName().ToUpper() << " listen failed";
		if (!interfacesForEP.IsEmpty())
			LogWindow << " with interfaces" << setfill(',') << interfacesForEP << setfill(' ');
		LogWindow << endl;
	}
}


void CManager::StartRegistrations()
{
	if (sipEP == NULL)
		return;

	for (RegistrationList::iterator iter = m_registrations.begin(); iter != m_registrations.end(); ++iter)
		iter->Start(*sipEP);
}


void CManager::StartAllListeners()
{
 	StartListenerForEP(sipEP, m_LocalInterfaces);
}


void CManager::MakeCall(const PString & address, const PString & local, OpalConnection::StringOptions * options)
{
	if (address.IsEmpty())
		return;

	PString from = local;
	if (from.IsEmpty())
		from = "pc:*";

	m_activeCall = SetUpCall(from, address, NULL, 0, options);
	if (m_activeCall == NULL)
		LogWindow << "Could not call \"" << address << '"' << endl;
	else {
		LogWindow << "Calling \"" << address << '"' << endl;
	}
}


void CManager::AnswerCall()
{
	if (PAssert(!m_incomingToken.IsEmpty(), PLogicError)) {
		StopRingSound();

		pcssEP->AcceptIncomingConnection(m_incomingToken);
		m_incomingTokenForTransfer = m_incomingToken;
		m_incomingToken = "";
	}
}


void CManager::RejectCall()
{
	if (PAssert(!m_incomingToken.IsEmpty(), PLogicError)) {
		StopRingSound();
		pcssEP->RejectIncomingConnection(m_incomingToken);
		m_incomingToken = "";
	}
}


void CManager::HangUpCall()
{
	if(!psTokenForHangup.IsEmpty()) {
		PSafePtr<OpalCall> activeCall = FindCallWithLock(psTokenForHangup, PSafeReadOnly);
		if (PAssertNULL(activeCall) != NULL) {
			LogWindow << "Hanging up \"" << *activeCall << '"' << endl;
			activeCall->Clear();
		}
		return;
	}

	if (PAssert(m_activeCall != NULL, PLogicError)) {
 		LogWindow << "Hanging up \"" << *m_activeCall << '"' << endl;
		m_activeCall->Clear();
	}
}


void CManager::OnEvtOnHold( )
{
	if (m_activeCall != NULL) {
		AddCallOnHold(*m_activeCall);
		m_activeCall.SetNULL();
	}
}


void CManager::OnEvtRinging( PString psIncomingCallToken )
{
	m_incomingToken = psIncomingCallToken;
	PSafePtr<OpalCall> call = FindCallWithLock(m_incomingToken, PSafeReadOnly);
	if (!PAssert(call != NULL, PLogicError))
		return;

	PSafePtr<OpalConnection> connection = call->GetConnection(0, PSafeReadOnly);
	if (!PAssert(connection != NULL, PLogicError))
		return;

	PString alertingType;
	PSafePtr<OpalConnection> network = connection->GetOtherPartyConnection();
	if (network != NULL)
		alertingType = network->GetAlertingType();

	PTime now;
	LogWindow << "\nIncoming call at " << now.AsString("w h:mma")
		<< " from " << connection->GetRemotePartyName();
	if (!alertingType.IsEmpty())
		LogWindow << ", type=" << alertingType;
	LogWindow << endl;

	m_LastReceived = connection->GetRemotePartyAddress();

	if (!m_ForwardingAddress.IsEmpty()) {
		if (m_ForwardingTimeout != 0)
			m_ForwardingTimer.SetInterval(0, m_ForwardingTimeout);
		else
			OnForwardingTimeout(m_ForwardingTimer, 0);
	}

	if ((m_autoAnswer && m_activeCall == NULL) || connection->GetStringOptions().Contains("Auto-Answer")) {

		pcssEP->AcceptIncomingConnection(m_incomingToken);
		m_incomingToken = "";
	} else {
		m_dialog->m_call.EnableWindow(TRUE);
		m_dialog->m_call.SetWindowText(_T("Answer"));
		m_dialog->m_hangup.EnableWindow(TRUE);
		m_dialog->m_hangup.SetWindowText(_T("Reject"));

		if (!m_RingSoundFileName.IsEmpty()) {
			PTRACE(4, "MySipPhone\tPlaying ring file \"" << m_RingSoundFileName << '"');
			m_RingSoundChannel.Open(m_RingSoundDeviceName, PSoundChannel::Player);
			m_RingSoundChannel.PlayFile(m_RingSoundFileName, false); //If false then the sound play is begun asynchronously and	the function returns immediately.
			m_RingSoundTimer.RunContinuous(5000);
		}
	}
}


void CManager::OnRingSoundAgain(PTimer &, INT)
{
	PTRACE(4, "MySipPhone\tReplaying ring file \"" << m_RingSoundFileName << '"');
	m_RingSoundChannel.PlayFile(m_RingSoundFileName, false);
}


void CManager::StopRingSound()
{
	PTRACE(4, "MySipPhone\tStopping play of ring file \"" << m_RingSoundFileName << '"');
	m_RingSoundTimer.Stop();
	m_RingSoundChannel.Close();
}


PBoolean CManager::OnIncomingConnection(OpalConnection & connection, unsigned options, OpalConnection::StringOptions * stringOptions)
{
	if (!OpalManager::OnIncomingConnection(connection, options, stringOptions))
		return false;
	return true;
}


void CManager::OnEstablishedCall(OpalCall & call)
{
	m_activeCall = &call;

 	LogWindow << "Established call from " << call.GetPartyA() << " to " << call.GetPartyB() << endl;
	OnEvtEstablished(call.GetToken());

	PSafePtr<SIPConnection> connection = call.GetConnectionAs<SIPConnection>(0);
	if (connection != NULL) {
// 		wxConfigBase * config = wxConfig::Get();
// 		config->SetPath(GeneralGroup);
// 		config->Write(CurrentSIPConnectionKey, PwxString(connection->GetDialog().AsString()));
		m_dialog->m_call.EnableWindow(FALSE);
		m_dialog->m_hangup.EnableWindow(TRUE);
		m_dialog->m_hangup.SetWindowText(_T("Hangup"));
		m_dialog->m_hold.EnableWindow(TRUE);
		m_dialog->m_hold.SetWindowText(_T("Hold"));
		m_dialog->m_transfer.EnableWindow(TRUE);
		m_dialog->m_transfer.SetWindowText(_T("Transfer"));

// 		OnSendAudioFile();
// 		LogWindow << "Transfered to IVR" << endl;


// 		PString digit = connection->GetUserInput(10);
// 
// 		PString input;
// 		while (input.GetLength() < 3) {
// 			input += digit;
// 			digit = connection->GetUserInput(5);
// 			if (digit.IsEmpty()) {
// 				LogWindow << "ReadUserInput last character timeout (" << 5 << " seconds)";
// 			}
// 		}
// 		LogWindow << "USER ENTERED " << input << endl;
	}

}


void CManager::OnEvtEstablished(PString psToken)
{
	if (m_activeCall == NULL) {
		// Retrieve call from hold
		RemoveCallOnHold(psToken);
		m_activeCall = FindCallWithLock(psToken, PSafeReference);
	} else {
		PString title = m_activeCall->IsNetworkOriginated() ? m_activeCall->GetPartyA() : m_activeCall->GetPartyB();
		RemoveCallOnHold(psToken);
	}
}


void CManager::OnClearedCall(OpalCall & call)
{
	StopRingSound();

	PString name = call.GetPartyB().IsEmpty() ? call.GetPartyA() : call.GetPartyB();

	switch (call.GetCallEndReason()) {
	case OpalConnection::EndedByRemoteUser :
		LogWindow << '"' << name << "\" has cleared the call";
		break;
	case OpalConnection::EndedByCallerAbort :
		LogWindow << '"' << name << "\" has stopped calling";
		break;
	case OpalConnection::EndedByRefusal :
		LogWindow << '"' << name << "\" did not accept your call";
		break;
	case OpalConnection::EndedByNoAnswer :
		LogWindow << '"' << name << "\" did not answer your call";
		break;
	case OpalConnection::EndedByNoAccept :
		LogWindow << "Did not accept incoming call from \"" << name << '"';
		break;
	case OpalConnection::EndedByNoUser :
		LogWindow << "Could find user \"" << name << '"';
		break;
	case OpalConnection::EndedByUnreachable :
		LogWindow << '"' << name << "\" could not be reached.";
		break;
	case OpalConnection::EndedByNoEndPoint :
		LogWindow << "No phone running for \"" << name << '"';
		break;
	case OpalConnection::EndedByHostOffline :
		LogWindow << '"' << name << "\" is not online.";
		break;
	case OpalConnection::EndedByConnectFail :
		LogWindow << "Transport error calling \"" << name << '"';
		break;
	default :
		LogWindow << call.GetCallEndReasonText() << " with \"" << name << '"';
	}
	PTime now;
	LogWindow << ", on " << now.AsString("w h:mma") << ". Duration "
		<< setprecision(0) << setw(5) << (now - call.GetStartTime())
		<< "s." << endl;

	OnEvtCleared( call.GetToken() );

	m_dialog->m_call.EnableWindow(TRUE);
	m_dialog->m_call.SetWindowText(_T("Call"));
	m_dialog->m_hangup.EnableWindow(FALSE);
	m_dialog->m_hangup.SetWindowText(_T("Hangup"));
	m_dialog->m_hold.EnableWindow(FALSE);
	m_dialog->m_transfer.EnableWindow(FALSE);
}


void CManager::OnEvtCleared(PString psToken)
{
	if (m_activeCall == NULL || psToken != m_activeCall->GetToken()) {
		// A call on hold got cleared
		if (RemoveCallOnHold(psToken))
			return;
	}

	m_activeCall.SetNULL();
}


void CManager::OnHold(OpalConnection & connection, bool fromRemote, bool onHold)
{
	OpalManager::OnHold(connection, fromRemote, onHold);

	if (fromRemote) {
		LogWindow << "Remote " << connection.GetRemotePartyName() << " has "
			<< (onHold ? "put you on" : "released you from") << " hold." << endl;
		return;
	}

	LogWindow << "Remote " << connection.GetRemotePartyName() << " has been "
		<< (onHold ? "put on" : "released from") << " hold." << endl;
	if (onHold) {
		OnEvtOnHold();
	} else {
		OnEvtEstablished( connection.GetCall().GetToken() ) ;
	}
}


bool CManager::OnTransferNotify(OpalConnection & connection, const PStringToString & info)
{
	LogWindow << "Transfer ";
	if (info["party"] == "A")
		LogWindow << "by ";
	else if (info["party"] == "B")
		LogWindow << "to ";
	else
		LogWindow << "from ";
	LogWindow << connection.GetRemotePartyName() << ' ' << info["result"] << endl;
	return OpalManager::OnTransferNotify(connection, info);
}



static void LogMediaStream(const char * stopStart, const OpalMediaStream & stream, const OpalConnection & connection)
{
	if (!connection.IsNetworkConnection())
		return;

	OpalMediaFormat mediaFormat = stream.GetMediaFormat();
	LogWindow << stopStart << (stream.IsSource() ? " receiving " : " sending ") << mediaFormat;

	if (!stream.IsSource() && mediaFormat.GetMediaType() == OpalMediaType::Audio())
		LogWindow << " (" << mediaFormat.GetOptionInteger(OpalAudioFormat::TxFramesPerPacketOption())*mediaFormat.GetFrameTime()/mediaFormat.GetTimeUnits() << "ms)";

	LogWindow << (stream.IsSource() ? " from " : " to ")
		<< connection.GetPrefixName() << " endpoint"
		<< endl;
}


void CManager::AdjustMediaFormats(bool   local,
								   const OpalConnection & connection,
								   OpalMediaFormatList & mediaFormats) const
{
	OpalManager::AdjustMediaFormats(local, connection, mediaFormats);

	if (local) {
		for (MyMediaList::const_iterator mm = m_mediaInfo.begin(); mm != m_mediaInfo.end(); ++mm) {
			if (mm->preferenceOrder >= 0) {
				for (OpalMediaFormatList::iterator it = mediaFormats.begin(); it != mediaFormats.end(); ++it) {
					if (*it == mm->mediaFormat) {
						*it = mm->mediaFormat;
						LogWindow << "Adjusting ============ " << mm->mediaFormat << endl;
					}
				}
			}
		}
	}
}


PBoolean CManager::OnOpenMediaStream(OpalConnection & connection, OpalMediaStream & stream)
{
	if (!OpalManager::OnOpenMediaStream(connection, stream))
		return false;

 	LogMediaStream("Started", stream, connection);
// 	PostEvent(wxEvtStreamsChanged, ID_STREAMS_CHANGED, connection.GetCall().GetToken());
	return true;
}


void CManager::OnClosedMediaStream(const OpalMediaStream & stream)
{
	OpalManager::OnClosedMediaStream(stream);

 	LogMediaStream("Stopped", stream, stream.GetConnection());
// 
// 	PostEvent(wxEvtStreamsChanged, ID_STREAMS_CHANGED);
// 
// 	if (PIsDescendant(&stream, OpalVideoMediaStream)) {
// 		PVideoOutputDevice * device = ((const OpalVideoMediaStream &)stream).GetVideoOutputDevice();
// 		if (device != NULL && device->GetDeviceName().FindRegEx(" ([0-9])\"") == P_MAX_INDEX) {
// 			int x, y;
// 			if (device->GetPosition(x, y)) {
// 				wxConfigBase * config = wxConfig::Get();
// 				config->SetPath(VideoGroup);
// 
// 				if (stream.IsSource()) {
// 					if (x != m_localVideoFrameX || y != m_localVideoFrameY) {
// 						config->Write(LocalVideoFrameXKey, m_localVideoFrameX = x);
// 						config->Write(LocalVideoFrameYKey, m_localVideoFrameY = y);
// 					}
// 				}
// 				else {
// 					if (x != m_remoteVideoFrameX || y != m_remoteVideoFrameY) {
// 						config->Write(RemoteVideoFrameXKey, m_remoteVideoFrameX = x);
// 						config->Write(RemoteVideoFrameYKey, m_remoteVideoFrameY = y);
// 					}
// 				}
// 			}
// 		}
// 	}
}


PSafePtr<OpalCall> CManager::GetCall(PSafetyMode mode)
{
	if (m_activeCall == NULL)
		return NULL;

	PSafePtr<OpalCall> call = m_activeCall;
	return call.SetSafetyMode(mode) ? call : NULL;
}


PSafePtr<OpalConnection> CManager::GetConnection(bool user, PSafetyMode mode)
{
	if (m_activeCall == NULL)
		return NULL;

	PSafePtr<OpalConnection> connection = m_activeCall->GetConnection(0, PSafeReference);
	while (connection != NULL && connection->IsNetworkConnection() == user)
		++connection;

	return connection.SetSafetyMode(mode) ? connection : NULL;
}


CManager::CallsOnHold::CallsOnHold(OpalCall & call)
: m_call(&call, PSafeReference)
{
	static int lastMenuId;
// 	m_retrieveMenuId   = lastMenuId + ID_RETRIEVE_MENU_BASE;
// 	m_conferenceMenuId = lastMenuId + ID_CONFERENCE_MENU_BASE;
// 	m_transferMenuId   = lastMenuId + ID_TRANSFER_MENU_BASE;
	lastMenuId++;
}



void CManager::AddCallOnHold(OpalCall & call)
{
	m_callsOnHold.push_back(call);

	PString otherParty = call.IsNetworkOriginated() ? call.GetPartyA() : call.GetPartyB();

	OnHoldChanged(call.GetToken(), true);

	if (!m_switchHoldToken.IsEmpty()) {
		PSafePtr<OpalCall> call = FindCallWithLock(m_switchHoldToken, PSafeReadWrite);
		if (call != NULL)
			call->Retrieve();
		//m_switchHoldToken.clear();
		m_switchHoldToken = "";
	}
}


bool CManager::RemoveCallOnHold(const PString & token)
{
	list<CallsOnHold>::iterator it = m_callsOnHold.begin();
	for (;;) {
		if (it == m_callsOnHold.end())
			return false;
		if (it->m_call->GetToken() == token)
			break;
		++it;
	}

	m_callsOnHold.erase(it);
	OnHoldChanged(token, false);

	return true;
}


void CManager::OnHoldChanged(const PString & token, bool onHold)
{
// 	for (size_t i = 0; i < m_tabs->GetPageCount(); ++i) {
// 		InCallPanel * panel = dynamic_cast<InCallPanel *>(m_tabs->GetPage(i));
// 		if (panel != NULL && panel->GetToken() == token) {
// 			panel->OnHoldChanged(onHold);
// 			break;
// 		}
// 	}
	if(onHold) {
		m_dialog->m_hold.SetWindowText(_T("Retrive"));
		m_dialog->m_call.EnableWindow(TRUE);
		m_dialog->m_call.SetWindowText(_T("Call"));
		psTokenForHangup = token;
	} else {
		m_dialog->m_hold.SetWindowText(_T("Hold"));
		m_dialog->m_call.EnableWindow(FALSE);
		m_dialog->m_call.SetWindowText(_T("Call"));
		psTokenForHangup = "";
	}
}


void CManager::SendUserInput(char tone)
{
	PSafePtr<OpalConnection> connection = GetConnection(true, PSafeReadWrite);
	if (connection != NULL)
		connection->OnUserInputTone(tone, 100);
}


void CManager::OnUserInputString(OpalConnection & connection, const PString & value)
{
	if (connection.IsNetworkConnection())
		LogWindow << "User input \"" << value << "\" received from \"" << connection.GetRemotePartyName() << '"' << endl;
	OpalManager::OnUserInputString(connection, value);
}


void CManager::OnUserInputTone(OpalConnection & connection, char tone, int duration)
{
	LogWindow << "User input \"" << tone << "\" received from \"" << connection.GetRemotePartyName() << '"' << endl;
	OpalManager::OnUserInputTone(connection, tone, duration);
}


void CManager::OnRequestHold()
{
	PSafePtr<OpalCall> call = GetCall(PSafeReadWrite);
	if (call != NULL)
		call->Hold();
}


void CManager::OnRetrieve()
{
	if (PAssert(m_activeCall == NULL, PLogicError)) {
		for (list<CallsOnHold>::iterator it = m_callsOnHold.begin(); it != m_callsOnHold.end(); ++it) {
				it->m_call->Retrieve();
		}
	}
}


// void CManager::OnConference(wxCommandEvent& theEvent)
// {
// 	if (PAssert(m_activeCall != NULL, PLogicError)) {
// 		for (list<CallsOnHold>::iterator it = m_callsOnHold.begin(); it != m_callsOnHold.end(); ++it) {
// 			if (theEvent.GetId() == it->m_conferenceMenuId) {
// 				AddToConference(*it->m_call);
// 				return;
// 			}
// 		}
// 	}
// }
// 
// 
// void CManager::AddToConference(OpalCall & call)
// {
// 	if (m_activeCall != NULL) {
// 		PSafePtr<OpalConnection> connection = GetConnection(true, PSafeReference);
// 		m_activeCall->Transfer("mcu:"CONFERENCE_NAME, connection);
// 		LogWindow << "Added \"" << connection->GetRemotePartyName() << "\" to conference." << endl;
// 		PString pc = "pc:*";
// 		if (connection->GetMediaStream(OpalMediaType::Video(), true) == NULL)
// 			pc += ";OPAL-AutoStart=video:no";
// 		SetUpCall(pc, "mcu:"CONFERENCE_NAME);
// 		m_activeCall.SetNULL();
// 	}
// 
// 	PSafePtr<OpalLocalConnection> connection = call.GetConnectionAs<OpalLocalConnection>();
// 	call.Transfer("mcu:"CONFERENCE_NAME, connection);
// 	call.Retrieve();
// 	LogWindow << "Added \"" << connection->GetRemotePartyName() << "\" to conference." << endl;
// }


void CManager::OnTransfer()
{
	if (PAssert(m_activeCall != NULL, PLogicError)) {
		for (list<CallsOnHold>::iterator it = m_callsOnHold.begin(); it != m_callsOnHold.end(); ++it) {
 			if(m_activeCall->Transfer(it->m_call->GetToken(), GetConnection(false, PSafeReference))){
				LogWindow << "Forwarded \"" << /*m_call->GetPartyB() <<*/ "\" to \"" <<  it->m_call->GetToken() << '"' << endl;
			} else {
				LogWindow << "Could not forward \"" << /*m_call->GetPartyB() <<*/ "\" to \"" << it->m_call->GetToken() << '"' << endl;
			}
			return;
		}
// 	return;
		CTransferWindow transferDlg;
		if(transferDlg.DoModal() != IDOK)
			return;
		if(!transferDlg.m_transferCombo.IsEmpty()) {
			if(m_activeCall->Transfer(PString(transferDlg.m_transferCombo), GetConnection(false, PSafeReference))) {
				LogWindow << "Forwarded \"" << /*call->GetPartyB() <<*/ "\" to \"" << PString(transferDlg.m_transferCombo) << '"' << endl;
			} else {
				LogWindow << "Could not forward \"" << /*call->GetPartyB() <<*/ "\" to \"" << PString(transferDlg.m_transferCombo) << '"' << endl;
			}
		}
	}
}


void CManager::OnSendAudioFile()
{
	PSafePtr<OpalPCSSConnection> connection = PSafePtrCast<OpalConnection, OpalPCSSConnection>(GetConnection(true, PSafeReadOnly));
	if (connection == NULL)
		return;

	if (m_activeCall != NULL) {
		PString m_lastPlayFile = "IvrTest.wav";
		LogWindow << "Playing " << m_lastPlayFile << ", please wait ..." << endl;

		PStringStream ivrXML;
		ivrXML << "ivr:<?xml version=\"1.0\"?>"
			"<vxml version=\"1.0\">"
			"<form id=\"PlayFile\">"
			"<transfer bridge=\"false\" dest=\"pc:*;Auto-Answer=1\">"
			"<audio src=\"" << PURL(PFilePath(m_lastPlayFile)) << "\"/>"
			"</transfer>"
			"</form>"
			"</vxml>";
		if (!m_activeCall->Transfer(ivrXML, connection))
			LogWindow << "Cannot play -> " << m_lastPlayFile << endl;
	}
}



void CManager::OnForwardingTimeout(PTimer &, INT)
{
	if (m_incomingToken.IsEmpty())
		return;

	// Transfer the incoming call to the forwarding address
	PSafePtr<OpalCall> call = FindCallWithLock(m_incomingToken, PSafeReadWrite);
	if (call == NULL)
		return;

	if (call->Transfer(m_ForwardingAddress, call->GetConnection(1)))
		LogWindow << "Forwarded \"" << call->GetPartyB() << "\" to \"" << m_ForwardingAddress << '"' << endl;
	else
		LogWindow << "Could not forward \"" << call->GetPartyB() << "\" to \"" << m_ForwardingAddress << '"' << endl;

	m_incomingToken = "";
}


void CManager::ApplyMediaInfo()
{
	PStringList mediaFormatOrder, mediaFormatMask;

// 	m_mediaInfo.sort();

	for (MyMediaList::iterator mm = m_mediaInfo.begin(); mm != m_mediaInfo.end(); ++mm) {
		if (mm->preferenceOrder < 0) {
			mediaFormatMask.AppendString(mm->mediaFormat);
		} else {
			mediaFormatOrder.AppendString(mm->mediaFormat);
			if (mm->mediaFormat.GetMediaType() == OpalMediaType::Audio()) {
			} else if (mm->mediaFormat.GetMediaType() == OpalMediaType::Video()) {
			}
		}
	}

	if (!mediaFormatOrder.IsEmpty()) {
		PTRACE(3, "MySipPhone\tMedia order:\n"<< setfill('\n') << mediaFormatOrder << setfill(' '));
		LogWindow << "MySipPhone\tMedia order:\n"<< setfill('\n') << mediaFormatOrder << setfill(' ');
		SetMediaFormatOrder(mediaFormatOrder);
		PTRACE(3, "MySipPhone\tMedia mask:\n"<< setfill('\n') << mediaFormatMask << setfill(' '));
//		LogWindow << "MySipPhone\tMedia mask:\n"<< setfill('\n') << mediaFormatMask << setfill(' ');
		SetMediaFormatMask(mediaFormatMask);
	}
}


///////////////////////////////////////////////////////////////////////////////

RegistrationInfo::RegistrationInfo()
: m_Type(Register)
, m_Active(true)
, m_TimeToLive(300)
, m_Compatibility(SIPRegister::e_FullyCompliant)
{
 }


bool RegistrationInfo::operator==(const RegistrationInfo & other) const
{
	return m_Type          == other.m_Type &&
		m_Active        == other.m_Active &&
		m_User          == other.m_User &&
		m_Domain        == other.m_Domain &&
		m_Contact       == other.m_Contact &&
		m_AuthID        == other.m_AuthID &&
		m_Password      == other.m_Password &&
		m_TimeToLive    == other.m_TimeToLive &&
		m_Proxy         == other.m_Proxy &&
		m_Compatibility == other.m_Compatibility;
}


bool RegistrationInfo::Read()
{
	return true;
}


void RegistrationInfo::Write()
{

}

// these must match the drop-down box on the Registration/Subcription dialog box
static SIPSubscribe::PredefinedPackages const EventPackageMapping[] = {
	SIPSubscribe::NumPredefinedPackages,             // Skip Register enum
	SIPSubscribe::MessageSummary,                    // MWI
	SIPSubscribe::Presence,                          // presence
	SIPSubscribe::Dialog,                            // line appearance
	SIPSubscribe::NumPredefinedPackages,             // Skip PublishPresence enum
	SIPSubscribe::Presence | SIPSubscribe::Watcher   // watch presence
};


bool RegistrationInfo::Start(SIPEndPoint & sipEP)
{
	if (!m_Active)
		return false;

	int status;

	switch (m_Type) {
	case Register :
		if (sipEP.IsRegistered(m_aor, true))
			status = 0;
		else {
			SIPRegister::Params param;
			param.m_addressOfRecord = m_User;
			param.m_registrarAddress = m_Domain;
			param.m_contactAddress = m_Contact;
			param.m_authID = m_AuthID;
			param.m_password = m_Password;
			param.m_proxyAddress = m_Proxy;
			param.m_expire = m_TimeToLive;
			param.m_compatibility = m_Compatibility;
			status = sipEP.Register(param, m_aor) ? 1 : 2;
		}
		break;

	default :
		if (sipEP.IsSubscribed(EventPackageMapping[m_Type], m_aor, true))
			status = 0;
		else {
			SIPSubscribe::Params param(EventPackageMapping[m_Type]);
			param.m_addressOfRecord = m_User;
			param.m_agentAddress = m_Domain;
			param.m_contactAddress = m_Contact;
			param.m_authID = m_AuthID;
			param.m_password = m_Password;
			param.m_expire = m_TimeToLive;
			status = sipEP.Subscribe(param, m_aor) ? 1 : 2;
		}
	}

	if (status == 0)
		return true;

	static const char * const TypeNames[] = {
		"Register",
		"MWI subscribe",
		"Presence subscribe",
		"Appearance subscribe"
	};
 	LogWindow << "SIP " << TypeNames[m_Type] << ' ' << (status == 1 ? "start" : "fail") << "ed for " << m_aor << endl;
	return status != 2;
}


bool RegistrationInfo::Stop(SIPEndPoint & sipEP)
{
	if (!m_Active || m_aor.IsEmpty())
		return false;

	if (m_Type == Register)
		sipEP.Unregister(m_aor);
	else
		sipEP.Unsubscribe(EventPackageMapping[m_Type], m_aor);

	m_aor.MakeEmpty();
	return true;
}


///////////////////////////////////////////////////////////////////////////////

MyMedia::MyMedia()
: validProtocols(NULL)
, preferenceOrder(-1) // -1 indicates disabled
{
}


MyMedia::MyMedia(const OpalMediaFormat & format)
: mediaFormat(format)
, preferenceOrder(-1) // -1 indicates disabled
{
	bool hasSIP = mediaFormat.IsValidForProtocol("sip");
	validProtocols = " (SIP only)";
}

