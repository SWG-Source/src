// ======================================================================
//
// AddResourceTypeMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AddResourceTypeMessage_H
#define INCLUDED_AddResourceTypeMessage_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

namespace AddResourceTypeMessageNamespace
{
	class ResourceTypeData
	{
	  public:
		NetworkId m_networkId;
		std::string m_name;
		uint32 m_depletedTimestamp;
		std::string m_parentClass;
		std::vector<std::pair<std::string, int> > m_attributes;
		std::vector<std::pair<NetworkId, int> > m_fractalSeeds;
	};
}

// ======================================================================

/**
 * Sent from:  Game Server or DB Process
 * Sent to:  Game Server or DB Process
 * Action:  Add the specified resource types
 */
class AddResourceTypeMessage : public GameNetworkMessage
{
  public:
	AddResourceTypeMessage  (std::vector<AddResourceTypeMessageNamespace::ResourceTypeData> const &data);
	AddResourceTypeMessage  (Archive::ReadIterator & source);
	~AddResourceTypeMessage ();

	std::vector<AddResourceTypeMessageNamespace::ResourceTypeData> const & getData() const;
	
  private:
	Archive::AutoArray<AddResourceTypeMessageNamespace::ResourceTypeData> m_data;
	
  private:        
	AddResourceTypeMessage(const AddResourceTypeMessage&);
	AddResourceTypeMessage& operator= (const AddResourceTypeMessage&);
};

namespace Archive
{
	void get(ReadIterator & source, AddResourceTypeMessageNamespace::ResourceTypeData & target);
	void put(ByteStream & target, const AddResourceTypeMessageNamespace::ResourceTypeData & source);
}

// ======================================================================

#endif
