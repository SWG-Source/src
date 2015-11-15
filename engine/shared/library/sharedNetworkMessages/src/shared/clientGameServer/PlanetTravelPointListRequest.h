// ==================================================================
//
// PlanetTravelPointListRequest.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_PlanetTravelPointListRequest_H
#define	INCLUDED_PlanetTravelPointListRequest_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class PlanetTravelPointListRequest : public GameNetworkMessage
{
public: 

	PlanetTravelPointListRequest (const NetworkId& networkId, const std::string& planetName, int sequenceId);
	explicit PlanetTravelPointListRequest (Archive::ReadIterator &source);
	virtual ~PlanetTravelPointListRequest();

	const NetworkId&   getNetworkId  () const;
	const std::string& getPlanetName () const;
	int                getSequenceId () const;

public:

	static const char* const cms_name;

private:

	Archive::AutoVariable<NetworkId>   m_networkId;
	Archive::AutoVariable<std::string> m_planetName;
	Archive::AutoVariable<int>         m_sequenceId;
};

// ==================================================================

#endif 
