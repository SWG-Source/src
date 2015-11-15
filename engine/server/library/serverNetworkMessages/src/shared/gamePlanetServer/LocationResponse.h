// ======================================================================
//
// LocationResponse.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_LocationResponse_H
#define INCLUDED_LocationResponse_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class LocationResponse : public GameNetworkMessage
{
public:

	LocationResponse (NetworkId const & networkId, bool valid, std::string const & locationId, float x, float z, float radius);
	explicit LocationResponse (Archive::ReadIterator & source);
	~LocationResponse ();

	NetworkId const & getNetworkId () const;
	bool getValid () const;
	std::string const & getLocationId () const;
	float getX () const;
	float getZ () const;
	float getRadius () const;

private:

	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<bool> m_valid;
	Archive::AutoVariable<std::string> m_locationId;
	Archive::AutoVariable<float> m_x;
	Archive::AutoVariable<float> m_z;
	Archive::AutoVariable<float> m_radius;
	
private:

	LocationResponse ();
	LocationResponse (LocationResponse const &);
	LocationResponse & operator= (LocationResponse const &);
};

// ======================================================================

#endif
