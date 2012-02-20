#include "StdAfx.h"
#include "PCSSEndPoint.h"


// CPCSSEndPoint::CPCSSEndPoint(void)
// {
// }
// 
// 
// CPCSSEndPoint::~CPCSSEndPoint(void)
// {
// }

CPCSSEndPoint::CPCSSEndPoint(CManager & manager)
: OpalPCSSEndPoint(manager),
m_manager(manager)
{
	LogWindow << "PCSS ENDPOINT CREATED" << endl ;
}


PBoolean CPCSSEndPoint::OnShowIncoming(const OpalPCSSConnection & connection)
{
	LogWindow << "INCOMING CALL " << endl ;
	m_manager.OnEvtRinging( connection.GetCall().GetToken() );
	return true;
}


PBoolean CPCSSEndPoint::OnShowOutgoing(const OpalPCSSConnection & connection)
{
	PTime now;
 	LogWindow << connection.GetRemotePartyName() << " is ringing on "
 		<< now.AsString("w h:mma") << " ..." << endl;
	return true;
}


OpalMediaFormatList CPCSSEndPoint::GetMediaFormats() const
{
	OpalMediaFormatList list = OpalPCSSEndPoint::GetMediaFormats();
	return list;
}


PSoundChannel * CPCSSEndPoint::CreateSoundChannel(const OpalPCSSConnection & connection,
												   const OpalMediaFormat & mediaFormat,
												   PBoolean isSource)
{
	PSoundChannel * channel = OpalPCSSEndPoint::CreateSoundChannel(connection, mediaFormat, isSource);
	if (channel != NULL)
		return channel;

 	LogWindow << "Could not open ";
	if (isSource) {
 		LogWindow << "record device \"" << connection.GetSoundChannelRecordDevice();
	} else {
 		LogWindow << "player device \"" << connection.GetSoundChannelPlayDevice();
	}
	LogWindow << '"' << endl;

	return NULL;
}

