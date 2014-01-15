// ======================================================================
//
// LocationRequest.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_LocationRequest_H
#define INCLUDED_LocationRequest_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class LocationRequest : public GameNetworkMessage
{
public:

	LocationRequest (uint32 processId, NetworkId const & networkId, std::string const & locationId, float searchX, float searchZ, float searchRadius, float locationReservationRadius, bool checkWater, bool checkSlope);
	explicit LocationRequest (Archive::ReadIterator & source);
	~LocationRequest ();

	uint32 getProcessId () const;
	NetworkId const & getNetworkId () const;
	std::string const & getLocationId () const;
	float getSearchX () const;
	float getSearchZ () const;
	float getSearchRadius () const;
	float getLocationReservationRadius () const;
	bool getCheckWater () const;
	bool getCheckSlope () const;

private:

	Archive::AutoVariable<uint32> m_processId;
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<std::string> m_locationId;
	Archive::AutoVariable<float> m_searchX;
	Archive::AutoVariable<float> m_searchZ;
	Archive::AutoVariable<float> m_searchRadius;
	Archive::AutoVariable<float> m_locationReservationRadius;
	Archive::AutoVariable<bool> m_checkWater;
	Archive::AutoVariable<bool> m_checkSlope;
	
private:

	LocationRequest ();
	LocationRequest (LocationRequest const &);
	LocationRequest & operator= (LocationRequest const &);
};

// ======================================================================

#endif
