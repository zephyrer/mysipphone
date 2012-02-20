#pragma once
#include <sip/sip.h>
#include "Manager.h"

class CManager;

class CSIPEndPoint : public SIPEndPoint
{
public:
	CManager & m_manager;
	CSIPEndPoint(CManager & manager);
// 	CSIPEndPoint(void);
// 	~CSIPEndPoint(void);


private:
	virtual void OnRegistrationStatus(
		const RegistrationStatus & status
		);
	virtual void OnSubscriptionStatus(
		const SubscriptionStatus & status
		);
	virtual void OnDialogInfoReceived(
		const SIPDialogNotification & info  ///< Information on dialog state change
		);

};
