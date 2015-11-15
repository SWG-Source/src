//======================================================================
//
// AuctionManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AuctionManager_H
#define INCLUDED_AuctionManager_H

//======================================================================

class NetworkId;

//----------------------------------------------------------------------

class AuctionManager
{
public:
	static bool separateLocation          (const std::string & location, std::string & planet, std::string & region, std::string & name, NetworkId & id, int & x, int & z);
	static bool separateLocation          (const std::string & location, std::string & planet, std::string & region, std::string & name, NetworkId & id);
	static bool separateLocation          (const std::string & location, std::string & planet, std::string & region, std::string & name);
};

//----------------------------------------------------------------------

inline bool AuctionManager::separateLocation          (const std::string & location, std::string & planet, std::string & region, std::string & name, NetworkId & id)
{
	int x = 0;
	int z = 0;
	return separateLocation (location, planet, region, name, id, x, z);
}

//======================================================================

#endif
