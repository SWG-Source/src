// ======================================================================
//
// CreateTicketMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CreateTicketMessage_H
#define	_CreateTicketMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

#include <string>

//-----------------------------------------------------------------------

class CreateTicketMessage : public GameNetworkMessage
{
public:

	CreateTicketMessage(const std::string &characterName, unsigned category, unsigned subCategory, const Unicode::String &details, const Unicode::String &hiddenDetails, const Unicode::String &harassingPlayerName, const std::string &language, bool isBug = false);
	explicit CreateTicketMessage(Archive::ReadIterator &source);

	~CreateTicketMessage();

public: 

	const std::string &     getCharacterName() const;
	unsigned int            getCategory() const;
	unsigned int            getSubCategory() const;
	const Unicode::String & getDetails() const;
	const Unicode::String & getHiddenDetails() const;
	const Unicode::String & getHarassingPlayerName() const;
	const std::string &     getLanguage() const;
	unsigned int            getStationId() const;
	void                    setStationId(unsigned int id);
	bool                    isBug() const;

private: 
	Archive::AutoVariable<std::string>     m_characterName;  
	Archive::AutoVariable<unsigned int>    m_category;  
	Archive::AutoVariable<unsigned int>    m_subCategory;  
	Archive::AutoVariable<Unicode::String> m_details;  
	Archive::AutoVariable<Unicode::String> m_hiddenDetails;  
	Archive::AutoVariable<Unicode::String> m_harassingPlayerName;  
	Archive::AutoVariable<std::string>     m_language;  
	Archive::AutoVariable<unsigned int>    m_stationId;  
	Archive::AutoVariable<bool>            m_isBug;  
};

// ----------------------------------------------------------------------

inline const std::string & CreateTicketMessage::getCharacterName() const
{
	return m_characterName.get();
}

// ----------------------------------------------------------------------

inline unsigned int CreateTicketMessage::getCategory() const
{
	return m_category.get();
}

// ----------------------------------------------------------------------

inline unsigned int CreateTicketMessage::getSubCategory() const
{
	return m_subCategory.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & CreateTicketMessage::getDetails() const
{
	return m_details.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & CreateTicketMessage::getHiddenDetails() const
{
	return m_hiddenDetails.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & CreateTicketMessage::getHarassingPlayerName() const
{
	return m_harassingPlayerName.get();
}

// ----------------------------------------------------------------------

inline const std::string & CreateTicketMessage::getLanguage() const
{
	return m_language.get();
}

// ----------------------------------------------------------------------

inline unsigned int CreateTicketMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline void CreateTicketMessage::setStationId(unsigned int id)
{
	m_stationId.set(id);
}

// ----------------------------------------------------------------------

inline bool CreateTicketMessage::isBug() const
{
	return m_isBug.get();
}

// ----------------------------------------------------------------------

#endif // _CreateTicketMessage_H

