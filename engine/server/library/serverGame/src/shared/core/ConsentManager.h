// ============================================================================
// 
// ConsentManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_ConsentManager_H
#define INCLUDED_ConsentManager_H

#include "Singleton/Singleton.h"
#include "sharedMessageDispatch/Receiver.h"

#include "sharedFoundation/NetworkId.h"

class ConsentResponseMessage;
class ProsePackage;
class StringId;

//-----------------------------------------------------------------------------

class ConsentManager : public Singleton<ConsentManager>
{
public:
	typedef void (*CallbackFunction) (const NetworkId& player, int id, bool response);

	ConsentManager();
	int askConsent(const NetworkId& player, const ProsePackage& question, CallbackFunction callback);
	void removePlayerConsentRequests(const NetworkId& player);
	void handleResponse(const ConsentResponseMessage& msg);

public:

private:
	// Disable
	ConsentManager(ConsentManager const &);
	ConsentManager &operator =(ConsentManager const &);

private:
	stdmap<NetworkId, stdlist<std::pair<int, CallbackFunction> >::fwd >::fwd m_consentMap;
	int m_nextId;
};

// ============================================================================

#endif // INCLUDED_ConsentManager_H
