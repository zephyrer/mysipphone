#include "StdAfx.h"
#include "SIPEndPoint.h"


// CSIPEndPoint::CSIPEndPoint(void)
// {
// }
// 
// 
// CSIPEndPoint::~CSIPEndPoint(void)
// {
// }


CSIPEndPoint::CSIPEndPoint(CManager & manager)
: SIPEndPoint(manager),
m_manager(manager)
{
	LogWindow << "SIP ENDPOINT CREATED" << endl;
}


void CSIPEndPoint::OnRegistrationStatus(const RegistrationStatus & status)
{
	SIPEndPoint::OnRegistrationStatus(status);

	switch (status.m_reason) {
	default:
		break;

	case SIP_PDU::Failure_UnAuthorised :
	case SIP_PDU::Information_Trying :
		return;

	case SIP_PDU::Successful_OK :
		if (status.m_reRegistering)
			return;
	}

	SIPURL aor = status.m_addressofRecord;
	aor.Sanitise(SIPURL::ExternalURI);

 	LogWindow << "SIP ";
	if (!status.m_wasRegistering) {
 		LogWindow << "un";
	}
 	LogWindow << "registration of " << aor << ' ';
	switch (status.m_reason) {
	case SIP_PDU::Successful_OK :
 		LogWindow << "successful";
		break;

	case SIP_PDU::Failure_RequestTimeout :
 		LogWindow << "timed out";
		break;

	default : 
 		LogWindow << "failed (" << status.m_reason << ')';
		break;
	}
 	LogWindow << '.' << endl;

	if (!status.m_wasRegistering)
		m_manager.StartRegistrations();
}


void CSIPEndPoint::OnSubscriptionStatus(const SubscriptionStatus & status)
{
	SIPEndPoint::OnSubscriptionStatus(status);

	switch (status.m_reason) {
	default:
		break;
	case SIP_PDU::Failure_UnAuthorised :
	case SIP_PDU::Information_Trying :
		return;

	case SIP_PDU::Successful_OK :
		if (status.m_reSubscribing)
			return;
	}

	SIPURL uri = status.m_addressofRecord;
	uri.Sanitise(SIPURL::ExternalURI);

 	LogWindow << "SIP ";
	if (!status.m_wasSubscribing){
 		LogWindow << "un";
	}
	LogWindow << "subscription of " << uri << " to " << status.m_handler->GetEventPackage() << " events ";
	switch (status.m_reason) {
		case SIP_PDU::Successful_OK :
			LogWindow << "successful";
			break;

		case SIP_PDU::Failure_RequestTimeout :
	 		LogWindow << "timed out";
			break;

		default : 
 			LogWindow << "failed (" << status.m_reason << ')';
			break;
	}
 	LogWindow << '.' << endl;

	if (!status.m_wasSubscribing)
		m_manager.StartRegistrations();
}


void CSIPEndPoint::OnDialogInfoReceived(const SIPDialogNotification & info)
{
	SIPEndPoint::OnDialogInfoReceived(info);

	switch (info.m_state) {
	case SIPDialogNotification::Terminated :
 		LogWindow << "Line " << info.m_entity << " available." << endl;
		break;

	case SIPDialogNotification::Trying :
 		LogWindow << "Line " << info.m_entity << " in use." << endl;
		break;

	case SIPDialogNotification::Confirmed :
 		LogWindow << "Line " << info.m_entity << " connected";
		if (info.m_remote.m_URI.IsEmpty()) {
			LogWindow << " to " << info.m_remote.m_URI;
		}
 		LogWindow << '.' << endl;
		break;

	default :
		break;
	}
}