// ======================================================================
//
// AddMapLocationMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AddMapLocationMessage_H
#define	_AddMapLocationMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"
#include "sharedMath/Vector2d.h"

//-----------------------------------------------------------------------

class AddMapLocationMessage : public GameNetworkMessage
{
public:

	AddMapLocationMessage(const std::string &planetName, const NetworkId &locationId, const Unicode::String &locationName, const Vector2d &location, uint8 category, uint8 subCategory );
	explicit AddMapLocationMessage(Archive::ReadIterator &source);

	~AddMapLocationMessage();

public: // methods

	const std::string              & getPlanetName   () const;
	const NetworkId                & getLocationId   () const;
	const Unicode::String          & getLocationName () const;
	float                            getLocationX    () const;
	float                            getLocationY    () const;
	uint8                            getCategory     () const;
	uint8                            getSubCategory  () const;

public: // types

private: 
	Archive::AutoVariable<std::string>           m_planetName; 
	Archive::AutoVariable<NetworkId>             m_locationId; 
	Archive::AutoVariable<Unicode::String>       m_locationName; 
	Archive::AutoVariable<float>                 m_locationX; 
	Archive::AutoVariable<float>                 m_locationY; 
	Archive::AutoVariable<uint8>                 m_category; 
	Archive::AutoVariable<uint8>                 m_subCategory; 
};

// ----------------------------------------------------------------------

inline const std::string &AddMapLocationMessage::getPlanetName() const
{
	return m_planetName.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &AddMapLocationMessage::getLocationId() const
{
	return m_locationId.get();
}

// ----------------------------------------------------------------------

inline float AddMapLocationMessage::getLocationX() const
{
	return m_locationX.get();
}

// ----------------------------------------------------------------------

inline float AddMapLocationMessage::getLocationY() const
{
	return m_locationY.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & AddMapLocationMessage::getLocationName() const
{
	return m_locationName.get();
}

// ----------------------------------------------------------------------

inline uint8 AddMapLocationMessage::getCategory() const
{
	return m_category.get();
}

// ----------------------------------------------------------------------

inline uint8 AddMapLocationMessage::getSubCategory() const
{
	return m_subCategory.get();
}

// ----------------------------------------------------------------------

#endif // _AddMapLocationMessage_H

