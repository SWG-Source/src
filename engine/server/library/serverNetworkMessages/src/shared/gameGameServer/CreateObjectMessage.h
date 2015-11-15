// ======================================================================
//
// CreateObjectMessage.h - tells Gameserver to create a new object.
//
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_CreateObjectMessage_H
#define	_INCLUDED_CreateObjectMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/Tag.h"

// ======================================================================

class CreateObjectByCrcMessage: public GameNetworkMessage
{
public:
	CreateObjectByCrcMessage(NetworkId const &id, uint32 crc, Tag objectType, bool createAuthoritative, NetworkId const &container);
	CreateObjectByCrcMessage(Archive::ReadIterator &source);
	~CreateObjectByCrcMessage();

public:
	NetworkId const &   getId() const;
	uint32              getCrc() const;
	Tag                 getObjectType() const;
	bool                getCreateAuthoritative() const;
	NetworkId const &   getContainer() const;
	
private:
	Archive::AutoVariable<NetworkId>    m_id;
	Archive::AutoVariable<uint32>       m_crc;
	Archive::AutoVariable<Tag>          m_objType;          // object type
	Archive::AutoVariable<bool>         m_createAuthoritative;
	Archive::AutoVariable<NetworkId>    m_container;
	
	CreateObjectByCrcMessage();
	CreateObjectByCrcMessage(CreateObjectByCrcMessage const &);
	CreateObjectByCrcMessage &operator=(CreateObjectByCrcMessage const &);
};

// ======================================================================

inline NetworkId const &CreateObjectByCrcMessage::getId() const
{
	return m_id.get();
}

// ----------------------------------------------------------------------

inline uint32 CreateObjectByCrcMessage::getCrc() const
{
	return m_crc.get();
}

// ----------------------------------------------------------------------

inline Tag CreateObjectByCrcMessage::getObjectType() const
{
	return m_objType.get();
}

// ----------------------------------------------------------------------

inline bool CreateObjectByCrcMessage::getCreateAuthoritative() const
{
	return m_createAuthoritative.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &CreateObjectByCrcMessage::getContainer() const
{
	return m_container.get();
}

// ======================================================================

#endif	// _INCLUDED_CreateObjectMessage_H

