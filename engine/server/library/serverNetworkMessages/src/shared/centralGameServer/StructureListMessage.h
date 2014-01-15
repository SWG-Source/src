// ======================================================================
//
// CharacterListMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StructureListMessage_H
#define INCLUDED_StructureListMessage_H

// ======================================================================

#include <string>

#include "Unicode.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * This is declared as a separate class so that it can be forwarded.
 * Otherwise, it would be nested in StructureListMessage
 */
class StructureListMessageData
{
  public:
	std::string m_objectTemplate;
	NetworkId m_structureId;
	std::string m_location;
	Vector m_coordinates;
	int32 m_deleted;

	StructureListMessageData();
	StructureListMessageData( const std::string &objectTemplate, const NetworkId &structureId, const std::string &location, const Vector &coordinates, const int32 deleted);
};

// ----------------------------------------------------------------------

/**
 * List a character for an account
 * Sent from:  DBProcess
 * Sent to:  Central
 * Action:  Forward the character list to the client.
 */
class StructureListMessage : public GameNetworkMessage
{
  public:
	StructureListMessage  (uint32 toolId, uint32 loginServerId, const NetworkId& characterId, const std::vector<StructureListMessageData> &data);
	StructureListMessage  (Archive::ReadIterator & source);
	~StructureListMessage ();
	
	uint32 			     getToolId() const;
	uint32 			     getLoginServerId() const;
	const NetworkId&             getCharacterId() const;
	const std::vector<StructureListMessageData> &getData() const;

  private:
	Archive::AutoArray<StructureListMessageData>  m_data;
	Archive::AutoVariable<uint32> m_toolId;
	Archive::AutoVariable<uint32> m_loginServerId;
	Archive::AutoVariable<NetworkId> m_characterId;
	
	StructureListMessage();
	StructureListMessage(const StructureListMessage&);
	StructureListMessage& operator= (const StructureListMessage&);
};

// ----------------------------------------------------------------------

inline uint32 StructureListMessage::getLoginServerId(void) const
{
	return m_loginServerId.get();
}

inline uint32 StructureListMessage::getToolId(void) const
{
	return m_toolId.get();
}

inline const NetworkId& StructureListMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, StructureListMessageData &c);
	void put(ByteStream & target, const StructureListMessageData &c);
}

// ======================================================================

#endif
