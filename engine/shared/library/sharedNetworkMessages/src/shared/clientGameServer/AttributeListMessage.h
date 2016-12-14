// ======================================================================
//
// AttributeListMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AttributeListMessage_H
#define	_AttributeListMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class AttributeListMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	typedef std::pair<std::string, Unicode::String> AttributePair;
	typedef std::vector<AttributePair> AttributeVector;

	AttributeListMessage(NetworkId const & id, AttributeVector const & data, int const revision);
	AttributeListMessage(std::string const & staticItemName, AttributeVector const & data, int const revision);
	explicit AttributeListMessage(Archive::ReadIterator &source);

	~AttributeListMessage();

public:

	AttributeVector const & getData() const;
	NetworkId const & getNetworkId () const;
	std::string const & getStaticItemName () const;
	int getRevision() const;

private:
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<std::string> m_staticItemName;
	Archive::AutoArray<AttributePair> m_data;
	Archive::AutoVariable<int> m_revision;
};

// ----------------------------------------------------------------------

inline const AttributeListMessage::AttributeVector & AttributeListMessage::getData() const
{
	return m_data.get();
}

//----------------------------------------------------------------------

inline const NetworkId & AttributeListMessage::getNetworkId () const
{
	return m_networkId.get ();
}

//----------------------------------------------------------------------

inline std::string const & AttributeListMessage::getStaticItemName () const
{
	return m_staticItemName.get ();
}

//----------------------------------------------------------------------

inline int AttributeListMessage::getRevision() const
{
	return m_revision.get();
}

// ----------------------------------------------------------------------

#endif // _AttributeListMessage_H

