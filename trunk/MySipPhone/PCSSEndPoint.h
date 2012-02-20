#pragma once
#include <opal/pcss.h>
#include "Manager.h"

class CManager;

class CPCSSEndPoint :	public OpalPCSSEndPoint
{
	PCLASSINFO(CPCSSEndPoint, OpalPCSSEndPoint);

public:
	CManager & m_manager;

	CPCSSEndPoint(CManager & manager);
// 	CPCSSEndPoint(void);
// 	~CPCSSEndPoint(void);

	virtual OpalMediaFormatList GetMediaFormats() const;
	virtual PSoundChannel * CreateSoundChannel(const OpalPCSSConnection & connection, const OpalMediaFormat & mediaFormat, PBoolean isSource);

private:
	virtual PBoolean OnShowIncoming(const OpalPCSSConnection & connection);
	virtual PBoolean OnShowOutgoing(const OpalPCSSConnection & connection);
};
