
// ======================================================================
//
// ResourceHarvesterActivatePageMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ResourceHarvesterActivatePageMessage_H
#define INCLUDED_ResourceHarvesterActivatePageMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

//-----------------------------------------------------------------------

class ResourceHarvesterActivatePageMessage : public GameNetworkMessage
{
public:

	static const char * const MESSAGE_TYPE;

	typedef std::vector<NetworkId> NetworkIdVector;

	           ResourceHarvesterActivatePageMessage (const NetworkId & harvesterId);
	explicit   ResourceHarvesterActivatePageMessage (Archive::ReadIterator & source);
	virtual   ~ResourceHarvesterActivatePageMessage ();

	const NetworkId &                         getHarvesterId () const;

private:

	Archive::AutoVariable<NetworkId>           m_harvesterId;

private:
	ResourceHarvesterActivatePageMessage ();
	ResourceHarvesterActivatePageMessage(const ResourceHarvesterActivatePageMessage&);
	ResourceHarvesterActivatePageMessage& operator= (const ResourceHarvesterActivatePageMessage&);
};

//----------------------------------------------------------------------

inline const NetworkId &                         ResourceHarvesterActivatePageMessage::getHarvesterId () const
{
	return m_harvesterId.get ();
}

//-----------------------------------------------------------------

#endif
