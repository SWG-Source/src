// ======================================================================
//
// CharacterListMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CharacterListMessage_H
#define INCLUDED_CharacterListMessage_H

// ======================================================================

#include <string>

#include "Unicode.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * This is declared as a separate class so that it can be forwarded.
 * Otherwise, it would be nested in CharacterListMessage
 */
class CharacterListMessageData
{
  public:
	Unicode::String m_name;
	std::string m_objectTemplate;
	NetworkId m_characterId;
	NetworkId m_containerId;
	std::string m_location;
	Vector m_coordinates;

	CharacterListMessageData();
	CharacterListMessageData(const Unicode::String &name, const std::string &objectTemplate, const NetworkId &characterId, const NetworkId &containerId, const std::string &location, const Vector &coordinates);
};

// ----------------------------------------------------------------------

/**
 * List a character for an account
 * Sent from:  DBProcess
 * Sent to:  Central
 * Action:  Forward the character list to the client.
 */
class CharacterListMessage : public GameNetworkMessage
{
  public:
	CharacterListMessage  (uint32 accountNumber, const std::vector<CharacterListMessageData> &data);
	CharacterListMessage  (Archive::ReadIterator & source);
	~CharacterListMessage ();
	
	uint32                       getAccountNumber() const;
	const std::vector<CharacterListMessageData> &getData() const;

  private:
	Archive::AutoArray<CharacterListMessageData>  m_data;
	Archive::AutoVariable<uint32> m_accountNumber;
	
	CharacterListMessage();
	CharacterListMessage(const CharacterListMessage&);
	CharacterListMessage& operator= (const CharacterListMessage&);
};

// ----------------------------------------------------------------------

inline uint32 CharacterListMessage::getAccountNumber(void) const
{
	return m_accountNumber.get();
}

// ----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, CharacterListMessageData &c);
	void put(ByteStream & target, const CharacterListMessageData &c);
}

// ======================================================================

#endif
